// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h" 

#ifdef _IRR_COMPILE_WITH_LWO_LOADER_

#include "CLWOMeshFileLoader.h"
#include "os.h"
#include "SAnimatedMesh.h"
#include "SMesh.h"
#include "IReadFile.h"
#include "ISceneManager.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "IMeshManipulator.h"

using namespace std;

#ifdef _DEBUG
#if defined(_IRR_WINDOWS_API_)
#include <stdarg.h>
#include <windows.h>
static void
LwoDebugPrint(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char str[1024];
  vsnprintf_s(str, 1024, _TRUNCATE, format, args);
  irr::os::Printer::log(str);
  va_end(args);
}
#define LWO_DPRINT(...) LwoDebugPrint(__VA_ARGS__)
#else
#define LWO_DPRINT(...) {printf(__VA_ARGS__); printf("\n"); }
#endif
#else
#define LWO_DPRINT(...) ((void)0)
#endif

namespace irr
{
namespace scene
{

#ifdef _DEBUG
// #define LWO_READER_DEBUG
//  #define LWO_READER_DEBUG_2 // 追加コード、およびスキップ系デバッグメッセージ
#endif

#define charsToUIntD(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)
inline unsigned int charsToUInt(const char *str)
{
	return (str[0] << 24) | (str[1] << 16) | (str[2] << 8) | str[3];
}


struct tLWOTextureInfo
{
	tLWOTextureInfo() : Flags(0), WidthWrap(2), HeightWrap(2), OpacType(0),
			Color(0xffffffff), Value(0.0f), AntiAliasing(1.0f),
			Opacity(1.0f), Active(false) {};
	core::stringc Type;
	core::stringc Map;
	core::stringc AlphaMap;
	u16 Flags;
	u16 WidthWrap;
	u16 HeightWrap;
	u16 OpacType;
	u16 IParam[3];
	core::vector3df Size;
	core::vector3df Center;
	core::vector3df Falloff;
	core::vector3df Velocity;
	video::SColor Color;
	f32 Value;
	f32 AntiAliasing;
	f32 Opacity;
	f32 FParam[3];
  core::stringc VMapName;
	bool Active;
};

struct CLWOMeshFileLoader::tLWOMaterial
{
	tLWOMaterial() : Meshbuffer(0), TagType(0), Flags(0), ReflMode(3), TranspMode(3),
		Glow(0), AlphaMode(2), Luminance(0.0f), Diffuse(1.0f), Specular(0.0f),
		Reflection(0.0f), Transparency(0.0f), Translucency(0.0f),
		Sharpness(0.0f), ReflSeamAngle(0.0f), ReflBlur(0.0f),
		RefrIndex(1.0f), TranspBlur(0.0f), SmoothingAngle(0.0f),
		EdgeTransparency(0.0f), HighlightColor(0.0f), ColorFilter(0.0f),
		AdditiveTransparency(0.0f), GlowIntensity(0.0f), GlowSize(0.0f),
		AlphaValue(0.0f), VertexColorIntensity(0.0f), VertexColor() {};

	core::stringc Name;
	scene::SMeshBuffer *Meshbuffer;
	core::stringc ReflMap;
	u16 TagType;
	u16 Flags;
	u16 ReflMode;
	u16 TranspMode;
	u16 Glow;
	u16 AlphaMode;
	f32 Luminance;
	f32 Diffuse;
	f32 Specular;
	f32 Reflection;
	f32 Transparency;
	f32 Translucency;
	f32 Sharpness;
	f32 ReflSeamAngle;
	f32 ReflBlur;
	f32 RefrIndex;
	f32 TranspBlur;
	f32 SmoothingAngle;
	f32 EdgeTransparency;
	f32 HighlightColor;
	f32 ColorFilter;
	f32 AdditiveTransparency;
	f32 GlowIntensity;
	f32 GlowSize;
	f32 AlphaValue;
	f32 VertexColorIntensity;
	video::SColorf VertexColor;
	u32 Envelope[23];
	tLWOTextureInfo Texture[7];
};

struct tLWOLayerInfo
{
	u16 Number;
	u16 Parent;
	u16 Flags;
	bool Active;
	core::stringc Name;
	core::vector3df Pivot;
};

//! Constructor
CLWOMeshFileLoader::CLWOMeshFileLoader(scene::ISceneManager* smgr,
		io::IFileSystem* fs)
: SceneManager(smgr), FileSystem(fs), File(0), Mesh(0)
{
	#ifdef _DEBUG
	setDebugName("CLWOMeshFileLoader");
	#endif
}


//! destructor
CLWOMeshFileLoader::~CLWOMeshFileLoader()
{
	if (Mesh)
		Mesh->drop();
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CLWOMeshFileLoader::isALoadableFileExtension(const c8* filename) const
{
	return strstr(filename, ".lwo")!=0;
}



//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IUnknown::drop() for more information.
IAnimatedMesh* CLWOMeshFileLoader::createMesh(io::IReadFile* file)
{
	File = file;

  // レイヤ処理の状態変数をリセット
  CurrentLayerId = -1;
  CurrentPolyIndexOffset = 0;
  CurrentPolyNum = 0;

	if (Mesh)
		Mesh->drop();

	Mesh = new SMesh();

	if (!readFileHeader())
		return false;

	if (!readChunks())
		return false;

	SAnimatedMesh* am = new SAnimatedMesh();
	am->Type = EAMT_3DS;

	for (u32 polyIndex=0; polyIndex<MaterialMapping.size(); ++polyIndex)
	{
		const u16 tag=MaterialMapping[polyIndex];
		scene::SMeshBuffer *mb=Materials[tag]->Meshbuffer;
		const s32 vertCount=mb->Vertices.size();
		const core::array<u32>& poly = Indices[polyIndex];
		const u32 polySize=poly.size();
    const core::array<core::vector3df>& TargetPoints = Points[PolyMapping[polyIndex]];
    video::S3DVertex vertex;

    // VMapName に現在のレイヤIDを混ぜ込んでユニークにして、mapから検索する
    u32 layerId = PolyMapping[polyIndex];
		core::stringc vmapName = getVMapNameByLayerId(layerId, Materials[tag]->Texture[0].VMapName);
    core::map<core::stringc, core::array<core::vector2df>*>::Node *vmapNode = VMAPMap.find(vmapName);
		core::map<core::stringc, core::map<VMADId, core::vector2df>* >::Node *vmadNode = VMADMap.find(vmapName);
		core::array<core::vector2df>* vmapTCoords = NULL;
		core::map<VMADId, core::vector2df>* vmadTCoordsMap = NULL;
		if (vmapNode)
			vmapTCoords = vmapNode->getValue();
		if (vmadNode)
			vmadTCoordsMap = vmadNode->getValue();
		for (u32 i=0; i<polySize; ++i)
		{
			const s32 j=poly[i];
			vertex.Pos=TargetPoints[j];

			// テクスチャ座標: VMAD に情報があればそれを、無ければ VMAP から取得
			core::map<VMADId, core::vector2df>::Node *vmadTCoordsNode =  NULL;
			if (vmadTCoordsMap)
				vmadTCoordsNode = vmadTCoordsMap->find(VMADId(polyIndex, j));
			if (vmadTCoordsNode) {
				vertex.TCoords=vmadTCoordsNode->getValue();
#ifdef LWO_READER_DEBUG_2
				LWO_DPRINT("  FIND VMAD: %d-%d > %f,%f", polyIndex, j, vertex.TCoords.X, vertex.TCoords.Y);
#endif
			} else 
				if (vmapTCoords && vmapTCoords->size() > 0) {
				vertex.TCoords=(*vmapTCoords)[j];
			} else {
				vertex.TCoords=core::vector2df(0, 0);
			}
			mb->Vertices.push_back(vertex);
		}
		if (polySize>2)
		{
			for (u32 i=1; i<polySize-1; ++i)
			{
				core::vector3df normal = core::plane3df(mb->Vertices[vertCount].Pos,mb->Vertices[vertCount+i].Pos,mb->Vertices[vertCount+i+1].Pos).Normal.normalize();
				mb->Vertices[vertCount].Normal=normal;
				mb->Vertices[vertCount+i].Normal=normal;
				mb->Vertices[vertCount+i+1].Normal=normal;
				mb->Indices.push_back(vertCount);
				mb->Indices.push_back(vertCount+i);
				mb->Indices.push_back(vertCount+i+1);
			}
		}
	}
	for (u32 i=0; i<Materials.size(); ++i)
	{
		for (u32 j=0; j<Materials[i]->Meshbuffer->Vertices.size(); ++j)
			Materials[i]->Meshbuffer->Vertices[j].Color=Materials[i]->Meshbuffer->Material.DiffuseColor;
		Materials[i]->Meshbuffer->recalculateBoundingBox();
		if (Materials[i]->Meshbuffer->Material.MaterialType==video::EMT_NORMAL_MAP_SOLID)
		{
			SMesh tmpmesh;
			tmpmesh.addMeshBuffer(Materials[i]->Meshbuffer);
			SceneManager->getMeshManipulator()->createMeshWithTangents(&tmpmesh);
			Mesh->addMeshBuffer(tmpmesh.getMeshBuffer(0));
		}
		else
			Mesh->addMeshBuffer(Materials[i]->Meshbuffer);
		Mesh->getMeshBuffer(Mesh->getMeshBufferCount()-1)->drop();
	}

	Mesh->recalculateBoundingBox();
	am->addMesh(Mesh);
	am->recalculateBoundingBox();
	Mesh->drop();
	Mesh = 0;

	Points.clear();
  PolyMapping.clear();
	Indices.clear();
	MaterialMapping.clear();
	core::map<core::stringc, core::array<core::vector2df>*>::Iterator vmapIt = VMAPMap.getIterator();
	for (;!vmapIt.atEnd();vmapIt++)
		delete (*vmapIt).getValue(); // Value 側は new してあるので自分で開放が必要
  VMAPMap.clear();
	core::map<core::stringc, core::map<VMADId, core::vector2df>* >::Iterator vmadIt = VMADMap.getIterator();
	for (;!vmapIt.atEnd();vmadIt++)
		delete (*vmadIt).getValue(); // Value 側は new してあるので自分で開放が必要
	VMADMap.clear();
	Materials.clear();
	Images.clear();

	return am;
}


bool CLWOMeshFileLoader::readChunks()
{
	s32 lastPos;
	u32 size;
	unsigned int uiType;
	char type[5];
	type[4]=0;
	tLWOLayerInfo layer;

	while(File->getPos()<File->getSize())
	{
		File->read(&type, 4);
		//Convert 4-char string to 4-byte integer
		//Makes it possible to do a switch statement
		uiType = charsToUInt(type);
		File->read(&size, 4);
#ifndef __BIG_ENDIAN__
		size=os::Byteswap::byteswap(size);
#endif
		lastPos=File->getPos();

		switch(uiType)
		{
			case charsToUIntD('L','A','Y','R'):
				{
					++CurrentLayerId;
#ifdef LWO_READER_DEBUG_2
					// os::Printer::log("LWO loader: loading layer----------------------");
					LWO_DPRINT("LWO loader: loading layer[%d] -----------", CurrentLayerId);
#endif
					u16 tmp16;
					File->read(&tmp16, 2); // number
					File->read(&tmp16, 2); // flags
					size -= 4;
#ifndef __BIG_ENDIAN__
					tmp16=os::Byteswap::byteswap(tmp16);
#endif
          if (((FormatVersion==1)&&(tmp16!=1)) ||
						((FormatVersion==2)&&(tmp16&1)))
						layer.Active=false;
					else
						layer.Active=true;
					if (FormatVersion==2)
						size -= readVec(layer.Pivot);
					size -= readString(layer.Name);
					if (size)
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						layer.Parent = tmp16;
					}
          // 新規レイヤになったので、前までのポリオブジェクト数をオフセット値に加算し、現在のポリ数を初期化
          CurrentPolyIndexOffset += CurrentPolyNum;
          CurrentPolyNum = 0;
				}
				break;
			case charsToUIntD('P','N','T','S'):
				{
#ifdef LWO_READER_DEBUG
					os::Printer::log("LWO loader: loading points.");
#endif
					core::vector3df vec;
					// Points.clear();
          Points.push_back(core::array<core::vector3df>());
          core::array<core::vector3df>& CurrentPoints = Points.getLast();
					const u32 tmpsize = size/12;
					CurrentPoints.reallocate(tmpsize);
					for (u32 i=0; i<tmpsize; ++i)
					{
						readVec(vec);
						CurrentPoints.push_back(vec);
					}
				}
				break;
			case charsToUIntD('V','M','A','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading Vertex mapping.");
#endif
				readVertexMapping(size);
				break;
			case charsToUIntD('P','O','L','S'):
			case charsToUIntD('P','T','C','H'): // TODO: should be a subdivison mesh
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading polygons.");
#endif
				if (FormatVersion!=2)
					readObj1(size);
				else
					readObj2(size);
				break;
			case charsToUIntD('T','A','G','S'):
			case charsToUIntD('S','R','F','S'):
				{
#ifdef LWO_READER_DEBUG
					os::Printer::log("LWO loader: loading surface names.");
#endif
					while (size!=0)
					{
						tLWOMaterial *mat=new tLWOMaterial();
						mat->Name="";
						mat->Meshbuffer=new scene::SMeshBuffer();
						size -= readString(mat->Name);
						if (FormatVersion!=2)
							mat->TagType = 1; // format 2 has more types
						Materials.push_back(mat);
					}
				}
				break;
			case charsToUIntD('P','T','A','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading tag mapping.");
#endif
				readTagMapping(size);
				break;
			case charsToUIntD('V','M','P','A'):
				{
					s32 vtype, scolor;
					File->read(&vtype, 4);
					File->read(&scolor, 4);
#ifndef __BIG_ENDIAN__
					vtype=os::Byteswap::byteswap(vtype);
					scolor=os::Byteswap::byteswap(scolor);
#endif
#ifdef LWO_READER_DEBUG_2
					LWO_DPRINT("XXX VMPA is unsupported --> type: %d, color:0x%x", vtype, scolor);
#endif
				}
				break;
			case charsToUIntD('V','M','A','D'): // dicontinuous vertex mapping, i.e. additional texcoords
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading Discontinuous Vertex mapping.");
#endif
				readDiscontinuousVertexMapping(size);
				break;
//			case charsToUIntD('V','M','P','A'):
//			case charsToUIntD('E','N','V','L'):
//				break;
			case charsToUIntD('C','L','I','P'):
				{
#ifdef LWO_READER_DEBUG
					os::Printer::log("LWO loader: loading clips.");
#endif
					u32 index;
					u16 subsize;
					File->read(&index, 4);
#ifndef __BIG_ENDIAN__
					index=os::Byteswap::byteswap(index);
#endif
					size -= 4;
					while (size != 0)
					{
						File->read(&type, 4);
						File->read(&subsize, 2);
#ifndef __BIG_ENDIAN__
						subsize=os::Byteswap::byteswap(subsize);
#endif
						size -= 6;
						if (strncmp(type, "STIL", 4))
						{
							File->seek(subsize, true);
							size -= subsize;
							continue;
						}
						core::stringc path;
						size -= readString(path, subsize);
	#ifdef LWO_READER_DEBUG
						os::Printer::log("LWO loader: loaded clip", path.c_str());
	#endif
						Images.push_back(path);
					}
				}
				break;
			case charsToUIntD('S','U','R','F'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading material.");
#endif
				readMat(size);
				break;
			case charsToUIntD('B','B','O','X'):
				{
#ifdef LWO_READER_DEBUG
					os::Printer::log("LWO loader: loading bbox.");
#endif
					// not stored
					core::vector3df vec;
					for (u32 i=0; i<2; ++i)
						readVec(vec);
					size -= 24;
				}
				break;
			case charsToUIntD('D','E','S','C'):
			case charsToUIntD('T','E','X','T'):
				{
#ifdef LWO_READER_DEBUG
					os::Printer::log("LWO loader: loading text.");
#endif
					core::stringc text;
					size -= readString(text, size);
				}
				break;
			// not needed
			case charsToUIntD('I','C','O','N'):
			// not yet supported
			case charsToUIntD('P','C','H','S'):
			case charsToUIntD('C','R','V','S'):
			default:
#ifdef LWO_READER_DEBUG_2
				os::Printer::log("LWO loader: skipping ", type);
#endif
				//Go to next chunk
				File->seek(lastPos + size, false);
				break;
		}
	}
	return true;
}


void CLWOMeshFileLoader::readObj1(u32 size)
{
	u32 pos;
	u16 numVerts, vertIndex;
	s16 material;
	video::S3DVertex vertex;

	while (size!=0)
	{
		File->read(&numVerts, 2);
#ifndef __BIG_ENDIAN__
		numVerts=os::Byteswap::byteswap(numVerts);
#endif
		pos=File->getPos();
		File->seek(2*numVerts, true);
		File->read(&material, 2);
#ifndef __BIG_ENDIAN__
		material=os::Byteswap::byteswap(material);
#endif
		size -=2*numVerts+4;
		// detail meshes ?
		scene::SMeshBuffer *mb;
		if (material<0)
			mb=Materials[-material-1]->Meshbuffer;
		else
			mb=Materials[material-1]->Meshbuffer;
		File->seek(pos, false);

		u16 vertCount=mb->Vertices.size();
		for (u16 i=0; i<numVerts; ++i)
		{
			File->read(&vertIndex, 2);
#ifndef __BIG_ENDIAN__
			vertIndex=os::Byteswap::byteswap(vertIndex);
#endif
      core::array<core::vector3df>& CurrentPoints = Points.getLast();
			vertex.Pos=CurrentPoints[vertIndex];
			mb->Vertices.push_back(vertex);
		}
		for (u16 i=1; i<numVerts-1; ++i)
		{
			core::vector3df normal = core::plane3df(mb->Vertices[vertCount].Pos,mb->Vertices[vertCount+i].Pos,mb->Vertices[vertCount+i+1].Pos).Normal.normalize();
			mb->Vertices[vertCount].Normal=normal;
			mb->Vertices[vertCount+i].Normal=normal;
			mb->Vertices[vertCount+i+1].Normal=normal;
			mb->Indices.push_back(vertCount);
			mb->Indices.push_back(vertCount+i);
			mb->Indices.push_back(vertCount+i+1);
		}
		// skip material number and detail surface count
		if (material<0)
			File->read(&material, 2);
		File->read(&material, 2);
	}
}

// カレントレイヤIDに合わせた名前に変える
core::stringc CLWOMeshFileLoader::getVMapNameByLayerId(u32 layerId, core::stringc& name)
{
  return core::stringc(layerId) + "_" + name;
}

// VMAD 読み込み
void CLWOMeshFileLoader::readDiscontinuousVertexMapping(u32 size)
{
	char type[5];
	type[4]=0;
	u16 dimension;
	core::stringc name;
	File->read(&type, 4);
#ifdef LWO_READER_DEBUG
	os::Printer::log("LWO loader: Discontinuous Vertex map type", type);
#endif
	File->read(&dimension, 2);
#ifndef __BIG_ENDIAN__
	dimension=os::Byteswap::byteswap(dimension);
#endif
	size -= 6;
	size -= readString(name);
#ifdef LWO_READER_DEBUG
	os::Printer::log("LWO loader: Discontinuous Vertex map", name.c_str());
#endif
	if (strncmp(type, "TXUV", 4)) // also support RGB, RGBA, WGHT, ...
	{
		File->seek(size, true);
		return;
	}

  // 現在のレイヤのVMADマップに、読み込んだTXUV情報を追加
  // stringc("レイヤID_" + VMapName) → map(vtx index, TXUV)   というマップになっている
  // VMapName は、(おそらく…)レイヤ内でのみ一律なので、レイヤIDをつけてuniqueにしている
	// core::map<core::stringc, core::map<u16, core::vector2df>* > VMADMap;
  core::map<VMADId, core::vector2df> *tCoordsMap = new core::map<VMADId, core::vector2df>();
  VMADMap.insert(getVMapNameByLayerId(CurrentLayerId, name), tCoordsMap);

	VMADId vmadId;
	core::vector2df tcoord;
	while (size!=0)
	{
		size -= readVX(vmadId.vtxId);	  // vtxId
		size -= readVX(vmadId.polyId);  // polyId
		vmadId.polyId += CurrentPolyIndexOffset;  // ポリIDにレイヤオフセットを加算
		File->read(&tcoord.X, 4);
#ifndef __BIG_ENDIAN__
		tcoord.X=os::Byteswap::byteswap(tcoord.X);
#endif
		File->read(&tcoord.Y, 4);
#ifndef __BIG_ENDIAN__
		tcoord.Y=os::Byteswap::byteswap(tcoord.Y);
#endif
		tcoord.Y=-tcoord.Y;
		tCoordsMap->insert(vmadId, tcoord);
		size -= 8;
#ifdef LWO_READER_DEBUG_2
		LWO_DPRINT("VMAD TXUV(%d-%d: %f, %f)", vmadId.polyId, vmadId.vtxId, tcoord.X, tcoord.Y);
#endif
	}
}

// VMAP 読み込み
void CLWOMeshFileLoader::readVertexMapping(u32 size)
{
	char type[5];
	type[4]=0;
	u16 dimension;
	core::stringc name;
	File->read(&type, 4);
#ifdef LWO_READER_DEBUG
	os::Printer::log("LWO loader: Vertex map type", type);
#endif
	File->read(&dimension, 2);
#ifndef __BIG_ENDIAN__
	dimension=os::Byteswap::byteswap(dimension);
#endif
	size -= 6;
	size -= readString(name);
#ifdef LWO_READER_DEBUG
	os::Printer::log("LWO loader: Vertex map", name.c_str());
#endif
	if (strncmp(type, "TXUV", 4)) // also support RGB, RGBA, WGHT, ...
	{
		File->seek(size, true);
		return;
	}

  // 現在のレイヤのテクスチャ座標マップに、読み込んだTXUV情報を追加
  // stringc("レイヤID_" + VMapName) → array(TXUV)   というマップになっている
  // VMapName は、(おそらく…)レイヤ内でのみ一律なので、レイヤIDをつけてuniqueにしている
  core::array<core::vector2df> *tCoords = new core::array<core::vector2df>();
  VMAPMap.insert(getVMapNameByLayerId(CurrentLayerId, name), tCoords);
	
  u32 index;
	core::vector2df tcoord;
  core::array<core::vector3df>& CurrentPoints = Points.getLast();
  tCoords->set_used(CurrentPoints.size());
	while (size!=0)
	{
		size -= readVX(index);
		File->read(&tcoord.X, 4);
#ifndef __BIG_ENDIAN__
		tcoord.X=os::Byteswap::byteswap(tcoord.X);
#endif
		File->read(&tcoord.Y, 4);
#ifndef __BIG_ENDIAN__
		tcoord.Y=os::Byteswap::byteswap(tcoord.Y);
#endif
		tcoord.Y=-tcoord.Y;
    (*tCoords)[index]=tcoord;
		size -= 8;
		// LWO_DPRINT("(%d: %f, %f)", index, tcoord.X, tcoord.Y);
	}
	// static int count=0;
	// LWO_DPRINT("TXUV:%d: name:%s", ++count, name);
}

void CLWOMeshFileLoader::readTagMapping(u32 size)
{
	char type[5];
	type[4]=0;
	File->read(&type, 4);
	size -= 4;
	if ((strncmp(type, "SURF", 4))||(Indices.size()==0))
	{
		File->seek(size, true);
		return;
	}

	while (size!=0)
	{
		u16 tag;
		u32 polyIndex;
		size-=readVX(polyIndex);
		File->read(&tag, 2);
#ifndef __BIG_ENDIAN__
		tag=os::Byteswap::byteswap(tag);
#endif
		size -= 2;
#ifdef LWO_READER_DEBUG_2
     // LWO_DPRINT("XXX readTagMapping: MaterialMapping: polyIndex %d + %d -> tag %d", polyIndex, CurrentPolyIndexOffset,tag);
#endif
		MaterialMapping[CurrentPolyIndexOffset+polyIndex]=tag;
		Materials[tag]->TagType=1;
	} 
}

void CLWOMeshFileLoader::readObj2(u32 size)
{
	char type[5];
	type[4]=0;
	File->read(&type, 4);
	size -= 4;
	// Indices.clear();
	if (strncmp(type, "FACE", 4)) // also possible are splines, subdivision patches, metaballs, and bones
	{
		File->seek(size, true);
		return;
	}
  u16 numPolys=0;
	u16 numVerts=0;
	while (size!=0)
	{
		File->read(&numVerts, 2);
#ifndef __BIG_ENDIAN__
		numVerts=os::Byteswap::byteswap(numVerts);
#endif
		// mask out flags
		numVerts &= 0x03FF;

		size -= 2;
		Indices.push_back(core::array<u32>());
		u32 vertIndex;
		core::array<u32>& polyArray = Indices.getLast();
		polyArray.reallocate(numVerts);
		for (u16 i=0; i<numVerts; ++i)
		{
			size -= readVX(vertIndex);
			polyArray.push_back(vertIndex);
		}
    numPolys++;
#ifdef LWO_READER_DEBUG_2
    // LWO_DPRINT("XXX Idx:%d -> Pnt:%d",Indices.size()-1, Points.size()-1);
    // LWO_DPRINT("XXX Idx:%d -> LayerId:%d", Indices.size()-1, CurrentLayerId);
#endif
    PolyMapping.push_back(CurrentLayerId);
	}
  // 読み込んだポリの分だけマテリアルマッピングのハコを用意する
  for (u32 j=0; j<numPolys; ++j) {
    MaterialMapping.push_back(0);
  }

  CurrentPolyNum += numPolys;
}


void CLWOMeshFileLoader::readMat(u32 size)
{
	core::stringc name;

	tLWOMaterial* mat=0;
	size -= readString(name);
  u32 matId;
	for (u32 i=0; i<Materials.size(); ++i)
	{
		if ((Materials[i]->TagType==1) && (Materials[i]->Name==name))
		{
#ifdef LWO_READER_DEBUG_2
      // LWO_DPRINT("XXX MATERIAL: id:%d -> %s ---------------", i, name.c_str());
#endif
			mat=Materials[i];
      matId = i;
			break;
		}
	}
	if (!mat)
	{
		File->seek(size, true);
		return;
	}
	if (FormatVersion==2)
		size -= readString(name);

	video::SMaterial *irrMat=&mat->Meshbuffer->Material;

	u8 currTexture=0;
	while (size!=0)
	{
		char type[5];
		type[4]=0;
		u32 uiType;
		u32 tmp32;
		u16 subsize, tmp16;
		f32 tmpf32;
		File->read(&type, 4);
		//Convert 4-char string to 4-byte integer
		//Makes it possible to do a switch statement
		uiType = charsToUInt(type);
		File->read(&subsize, 2);
#ifndef __BIG_ENDIAN__
		subsize=os::Byteswap::byteswap(subsize);
#endif
		size -= 6;
		switch (uiType)
		{
			case charsToUIntD('C','O','L','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading Ambient color.");
#endif
				{
					s32 colSize = readColor(irrMat->DiffuseColor);
					irrMat->AmbientColor=irrMat->DiffuseColor;
					size -= colSize;
					subsize -= colSize;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[0]);
				}
				break;
			case charsToUIntD('D','I','F','F'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading Diffuse color.");
#endif
				{
					if (FormatVersion==2)
					{
						File->read(&mat->Diffuse, 4);
#ifndef __BIG_ENDIAN__
						mat->Diffuse=os::Byteswap::byteswap(mat->Diffuse);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[1]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						mat->Diffuse=tmp16/256.0f;
						size -= 2;
						subsize -= 2;
					}
				}
				break;
			case charsToUIntD('V','D','I','F'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading Diffuse color.");
#endif
				{
					File->read(&mat->Diffuse, 4);
#ifndef __BIG_ENDIAN__
					mat->Diffuse=os::Byteswap::byteswap(mat->Diffuse);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('L','U','M','I'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading luminance.");
#endif
				{
					if (FormatVersion==2)
					{
						File->read(&mat->Luminance, 4);
#ifndef __BIG_ENDIAN__
						mat->Luminance=os::Byteswap::byteswap(mat->Luminance);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[2]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						mat->Luminance=tmp16/256.0f;
						size -= 2;
						subsize -= 2;
					}				}
				break;
			case charsToUIntD('V','L','U','M'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading luminance.");
#endif
				{
					File->read(&mat->Luminance, 4);
#ifndef __BIG_ENDIAN__
					mat->Luminance=os::Byteswap::byteswap(mat->Luminance);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('S','P','E','C'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading specular.");
#endif
				{
					if (FormatVersion==2)
					{
						File->read(&mat->Specular, 4);
#ifndef __BIG_ENDIAN__
						mat->Specular=os::Byteswap::byteswap(mat->Specular);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[3]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						mat->Specular=tmp16/256.0f;;
						size -= 2;
						subsize -= 2;
					}
				}
				break;
			case charsToUIntD('V','S','P','C'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading specular.");
#endif
				{
					File->read(&mat->Specular, 4);
#ifndef __BIG_ENDIAN__
					mat->Specular=os::Byteswap::byteswap(mat->Specular);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('R','E','F','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection.");
#endif
				{
					if (FormatVersion==2)
					{
						File->read(&mat->Reflection, 4);
#ifndef __BIG_ENDIAN__
						mat->Reflection=os::Byteswap::byteswap(mat->Reflection);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[4]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						mat->Reflection=tmp16/256.0f;
						size -= 2;
						subsize -= 2;
					}
				}
				break;
			case charsToUIntD('V','R','F','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection.");
#endif
				{
					File->read(&mat->Reflection, 4);
#ifndef __BIG_ENDIAN__
					mat->Reflection=os::Byteswap::byteswap(mat->Reflection);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','R','A','N'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading transparency.");
#endif
				{
					if (FormatVersion==2)
					{
						File->read(&mat->Transparency, 4);
#ifndef __BIG_ENDIAN__
						mat->Transparency=os::Byteswap::byteswap(mat->Transparency);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[5]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						mat->Transparency=tmp16/256.0f;
						size -= 2;
						subsize -= 2;
					}
				}
				break;
			case charsToUIntD('V','T','R','N'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading transparency.");
#endif
				{
					File->read(&mat->Transparency, 4);
#ifndef __BIG_ENDIAN__
					mat->Transparency=os::Byteswap::byteswap(mat->Transparency);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','R','N','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading translucency.");
#endif
				{
					File->read(&mat->Translucency, 4);
#ifndef __BIG_ENDIAN__
					mat->Translucency=os::Byteswap::byteswap(mat->Translucency);
#endif
					size -= 4;
					subsize -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[6]);
				}
				break;
			case charsToUIntD('G','L','O','S'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading glossy.");
#endif
				{
					if (FormatVersion == 2)
					{
						File->read(&irrMat->Shininess, 4);
#ifndef __BIG_ENDIAN__
						irrMat->Shininess=os::Byteswap::byteswap(irrMat->Shininess);
#endif
						size -= 4;
						subsize -= 4;
						size -= readVX(mat->Envelope[7]);
					}
					else
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						irrMat->Shininess=tmp16/16.f;
						size -= 2;
						subsize -= 2;
					}
				}
				break;
			case charsToUIntD('S','H','R','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading sharpness.");
#endif
				{
					File->read(&mat->Sharpness, 4);
#ifndef __BIG_ENDIAN__
					mat->Sharpness=os::Byteswap::byteswap(mat->Sharpness);
#endif
					size -= 4;
					subsize -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[8]);
				}
				break;
			case charsToUIntD('B','U','M','P'):
			case charsToUIntD('T','A','M','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading bumpiness.");
#endif
				{
					File->read(&tmpf32, 4);
#ifndef __BIG_ENDIAN__
						tmpf32=os::Byteswap::byteswap(tmpf32);
#endif
					if (currTexture==6)
						irrMat->MaterialTypeParam=tmpf32;
					size -= 4;
					subsize -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[9]);
				}
				break;
			case charsToUIntD('S','I','D','E'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading backface culled.");
#endif
				{
					File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
					tmp16=os::Byteswap::byteswap(tmp16);
#endif
					if (tmp16==1)
						irrMat->BackfaceCulling=true;
					if (tmp16==3)
						irrMat->BackfaceCulling=false;
					size -= 2;
				}
				break;
			case charsToUIntD('S','M','A','N'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading smoothing angle.");
#endif
				{
					File->read(&mat->SmoothingAngle, 4);
#ifndef __BIG_ENDIAN__
					mat->SmoothingAngle=os::Byteswap::byteswap(mat->SmoothingAngle);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('R','F','O','P'):
			case charsToUIntD('R','F','L','T'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection mode.");
#endif
				{
					File->read(&mat->ReflMode, 2);
#ifndef __BIG_ENDIAN__
					mat->ReflMode=os::Byteswap::byteswap(mat->ReflMode);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('R','I','M','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection map.");
#endif
				{
					if (FormatVersion==2)
					{
						size -= readVX(tmp32);
						if (tmp32)
							mat->ReflMap=Images[tmp32-1];
					}
					else
						size -= readString(mat->ReflMap, size);
				}
				break;
			case charsToUIntD('R','S','A','N'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection seam angle.");
#endif
				{
					File->read(&mat->ReflSeamAngle, 4);
#ifndef __BIG_ENDIAN__
					mat->ReflSeamAngle=os::Byteswap::byteswap(mat->ReflSeamAngle);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[10]);
				}
				break;
			case charsToUIntD('R','B','L','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading reflection blur.");
#endif
				{
					File->read(&mat->ReflBlur, 4);
#ifndef __BIG_ENDIAN__
					mat->ReflBlur=os::Byteswap::byteswap(mat->ReflBlur);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[11]);
				}
				break;
			case charsToUIntD('R','I','N','D'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading refraction index.");
#endif
				{
					File->read(&mat->RefrIndex, 4);
#ifndef __BIG_ENDIAN__
					mat->RefrIndex=os::Byteswap::byteswap(mat->RefrIndex);
#endif
					size -= 4;
					subsize -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[12]);
				}
				break;
			case charsToUIntD('T','R','O','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading transparency mode.");
#endif
				{
					File->read(&mat->TranspMode, 2);
#ifndef __BIG_ENDIAN__
					mat->TranspMode=os::Byteswap::byteswap(mat->TranspMode);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('T','I','M','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading refraction map.");
#endif
				{
					if (FormatVersion==2)
					{
						size -= readVX(tmp32);
#ifndef __BIG_ENDIAN__
						tmp32=os::Byteswap::byteswap(tmp32);
#endif
						if (tmp32)
							mat->Texture[currTexture].Map=Images[tmp32-1];
					}
					else
						size -= readString(mat->Texture[currTexture].Map, size);
				}
				break;
			case charsToUIntD('T','B','L','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading transparency blur.");
#endif
				{
					File->read(&mat->TranspBlur, 4);
#ifndef __BIG_ENDIAN__
					mat->TranspBlur=os::Byteswap::byteswap(mat->TranspBlur);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[13]);
				}
				break;
			case charsToUIntD('C','L','R','H'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading highlight color.");
#endif
				{
					File->read(&mat->HighlightColor, 4);
#ifndef __BIG_ENDIAN__
					mat->HighlightColor=os::Byteswap::byteswap(mat->HighlightColor);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[14]);
				}
				break;
			case charsToUIntD('C','L','R','F'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading color filter.");
#endif
				{
					File->read(&mat->ColorFilter, 4);
#ifndef __BIG_ENDIAN__
					mat->ColorFilter=os::Byteswap::byteswap(mat->ColorFilter);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[15]);
				}
				break;
			case charsToUIntD('A','D','T','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading additive transparency.");
#endif
				{
					File->read(&mat->AdditiveTransparency, 4);
#ifndef __BIG_ENDIAN__
					mat->AdditiveTransparency=os::Byteswap::byteswap(mat->AdditiveTransparency);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[16]);
				}
				break;
			case charsToUIntD('G','L','O','W'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading glow.");
#endif
				{
					File->read(&mat->Glow, 2);
#ifndef __BIG_ENDIAN__
					mat->Glow=os::Byteswap::byteswap(mat->Glow);
#endif
					size -= 2;
					File->read(&mat->GlowIntensity, 4);
#ifndef __BIG_ENDIAN__
					mat->GlowIntensity=os::Byteswap::byteswap(mat->GlowIntensity);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[17]);
					File->read(&mat->GlowSize, 4);
#ifndef __BIG_ENDIAN__
					mat->GlowSize=os::Byteswap::byteswap(mat->GlowSize);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[18]);
				}
				break;
			case charsToUIntD('G','V','A','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading glow intensity.");
#endif
				{
					File->read(&mat->GlowIntensity, 4);
#ifndef __BIG_ENDIAN__
					mat->GlowIntensity=os::Byteswap::byteswap(mat->GlowIntensity);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[17]);
				}
				break;
			case charsToUIntD('L','I','N','E'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading isWireframe.");
#endif
				{
					File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
					tmp16=os::Byteswap::byteswap(tmp16);
#endif
					if (tmp16&1)
						irrMat->Wireframe=true;
					size -= 2;
					if (size!=0)
					{
						File->read(&irrMat->Thickness, 4);
#ifndef __BIG_ENDIAN__
						irrMat->Thickness=os::Byteswap::byteswap(irrMat->Thickness);
#endif
						size -= 4;
						if (FormatVersion==2)
							size -= readVX(mat->Envelope[19]);
					}
					if (size!=0)
					{
						video::SColor lineColor;
						size -= readColor(lineColor);
						if (FormatVersion==2)
							size -= readVX(mat->Envelope[20]);
					}
				}
				break;
			case charsToUIntD('A','L','P','H'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading alpha mode.");
#endif
				{
					File->read(&mat->AlphaMode, 2);
#ifndef __BIG_ENDIAN__
					mat->AlphaMode=os::Byteswap::byteswap(mat->AlphaMode);
#endif
					size -= 2;
					File->read(&mat->AlphaValue, 4);
#ifndef __BIG_ENDIAN__
					mat->AlphaValue=os::Byteswap::byteswap(mat->AlphaValue);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('V','C','O','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading vertex color.");
#endif
				{
					File->read(&mat->VertexColorIntensity, 4);
#ifndef __BIG_ENDIAN__
					mat->VertexColorIntensity=os::Byteswap::byteswap(mat->VertexColorIntensity);
#endif
					size -= 4;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[21]);
					File->read(&tmp32, 4); // skip type
					size -= 4;
					core::stringc tmpname;
					size -= readString(tmpname, size);
//					mat->VertexColor = getColorVMAP(tmpname);
				}
				break;
			case charsToUIntD('F','L','A','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading flag.");
#endif
				{
					File->read(&mat->Flags, 2);
#ifndef __BIG_ENDIAN__
					mat->Flags=os::Byteswap::byteswap(mat->Flags);
#endif
					if (mat->Flags&1)
						mat->Luminance=1.0f;
					if (mat->Flags&256)
						irrMat->BackfaceCulling=false;
					size -= 2;
				}
				break;
			case charsToUIntD('E','D','G','E'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading edge.");
#endif
				{
					File->read(&mat->EdgeTransparency, 4);
#ifndef __BIG_ENDIAN__
					mat->EdgeTransparency=os::Byteswap::byteswap(mat->EdgeTransparency);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('C','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading ctex.");
#endif
				currTexture=0;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('D','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading dtex.");
#endif
				currTexture=1;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('S','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading stex.");
#endif
				currTexture=2;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('R','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading rtex.");
#endif
				currTexture=3;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('T','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading ttex.");
#endif
				currTexture=4;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('L','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading ltex.");
#endif
				currTexture=5;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('B','T','E','X'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading btex.");
#endif
				currTexture=6;
				size -= readString(mat->Texture[currTexture].Type, size);
				break;
			case charsToUIntD('T','A','L','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading alpha map.");
#endif
				size -= readString(mat->Texture[currTexture].AlphaMap, size);
				break;
			case charsToUIntD('T','F','L','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture flag.");
#endif
				{
					File->read(&mat->Texture[currTexture].Flags, 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].Flags=os::Byteswap::byteswap(mat->Texture[currTexture].Flags);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('E','N','A','B'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading isEnabled.");
#endif
				{
					File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
					tmp16=os::Byteswap::byteswap(tmp16);
#endif
					mat->Texture[currTexture].Active=(tmp16!=0);
					size -= 2;
				}
				break;
			case charsToUIntD('W','R','A','P'):
			case charsToUIntD('T','W','R','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture wrap.");
#endif
				{
					File->read(&mat->Texture[currTexture].WidthWrap, 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].WidthWrap=os::Byteswap::byteswap(mat->Texture[currTexture].WidthWrap);
#endif
					File->read(&mat->Texture[currTexture].HeightWrap, 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].HeightWrap=os::Byteswap::byteswap(mat->Texture[currTexture].HeightWrap);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','S','I','Z'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture size.");
#endif
				size -= readVec(mat->Texture[currTexture].Size);
				break;
			case charsToUIntD('T','C','T','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture center.");
#endif
				size -= readVec(mat->Texture[currTexture].Center);
				break;
			case charsToUIntD('T','F','A','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture falloff.");
#endif
				size -= readVec(mat->Texture[currTexture].Falloff);
				break;
			case charsToUIntD('T','V','E','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture velocity.");
#endif
				size -= readVec(mat->Texture[currTexture].Velocity);
				break;
			case charsToUIntD('T','C','L','R'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture color.");
#endif
				size -= readColor(mat->Texture[currTexture].Color);
				break;
			case charsToUIntD('A','A','S','T'):
			case charsToUIntD('T','A','A','S'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture antialias.");
#endif
				{
					tmp16=0;
					if (FormatVersion==2)
					{
						File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
						tmp16=os::Byteswap::byteswap(tmp16);
#endif
						size -= 2;
					}
					File->read(&mat->Texture[currTexture].AntiAliasing, 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].AntiAliasing=os::Byteswap::byteswap(mat->Texture[currTexture].AntiAliasing);
#endif
					if (tmp16 & ~0x01)
						mat->Texture[currTexture].AntiAliasing=0.0f; // disabled
					size -= 4;
				}
				break;
			case charsToUIntD('T','O','P','C'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture opacity.");
#endif
				{
					File->read(&mat->Texture[currTexture].Opacity, 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].Opacity=os::Byteswap::byteswap(mat->Texture[currTexture].Opacity);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('O','P','A','C'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture opacity and type.");
#endif
				{
					File->read(&mat->Texture[currTexture].OpacType, 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].OpacType=os::Byteswap::byteswap(mat->Texture[currTexture].OpacType);
#endif
					File->read(&mat->Texture[currTexture].Opacity, 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].Opacity=os::Byteswap::byteswap(mat->Texture[currTexture].Opacity);
#endif
					size -= 6;
					subsize -= 6;
					if (FormatVersion==2)
						size -= readVX(mat->Envelope[22]);
				}
				break;
			case charsToUIntD('T','V','A','L'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture value.");
#endif
				{
					File->read(&tmp16, 2);
#ifndef __BIG_ENDIAN__
					tmp16=os::Byteswap::byteswap(tmp16);
#endif
					mat->Texture[currTexture].Value=tmp16/256.0f;
					size -= 2;
				}
				break;
			case charsToUIntD('T','F','P','0'):
			case charsToUIntD('T','S','P','0'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture param 0.");
#endif
				{
					File->read(&mat->Texture[currTexture].FParam[0], 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].FParam[0]=os::Byteswap::byteswap(mat->Texture[currTexture].FParam[0]);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','F','P','1'):
			case charsToUIntD('T','S','P','1'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture param 1.");
#endif
				{
					File->read(&mat->Texture[currTexture].FParam[1], 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].FParam[1]=os::Byteswap::byteswap(mat->Texture[currTexture].FParam[1]);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','F','P','2'):
			case charsToUIntD('T','S','P','2'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture param 2.");
#endif
				{
					File->read(&mat->Texture[currTexture].FParam[2], 4);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].FParam[2]=os::Byteswap::byteswap(mat->Texture[currTexture].FParam[2]);
#endif
					size -= 4;
				}
				break;
			case charsToUIntD('T','F','R','Q'):
			case charsToUIntD('T','I','P','0'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture iparam 0.");
#endif
				{
					File->read(&mat->Texture[currTexture].IParam[0], 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].IParam[0]=os::Byteswap::byteswap(mat->Texture[currTexture].IParam[0]);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('T','I','P','1'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture param 1.");
#endif
				{
					File->read(&mat->Texture[currTexture].IParam[1], 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].IParam[1]=os::Byteswap::byteswap(mat->Texture[currTexture].IParam[1]);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('T','I','P','2'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading texture param 2.");
#endif
				{
					File->read(&mat->Texture[currTexture].IParam[2], 2);
#ifndef __BIG_ENDIAN__
					mat->Texture[currTexture].IParam[2]=os::Byteswap::byteswap(mat->Texture[currTexture].IParam[2]);
#endif
					size -= 2;
				}
				break;
			case charsToUIntD('V','M','A','P'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading material vmap binding.");
#endif
				{
					// core::stringc tmpname;
					// size -= readString(tmpname);
          size -= readString(mat->Texture[currTexture].VMapName);
          // LWO_DPRINT("VMAP: %s", mat->Texture[currTexture].VMapName.c_str()); 
				}
				break;
			case charsToUIntD('B','L','O','K'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading blok.");
#endif
				{
					core::stringc ordinal;
					File->read(&type, 4);
					File->read(&subsize, 2);
#ifndef __BIG_ENDIAN__
					subsize=os::Byteswap::byteswap(subsize);
#endif
					size -= 6;
					size -= readString(ordinal, size);
				}
				break;
			case charsToUIntD('C','H','A','N'):
				{
					File->read(&type, 4);
					size -= 4;
					if (!strncmp(type, "COLR", 4))
						currTexture=0;
					else if (!strncmp(type, "DIFF", 4))
						currTexture=1;
					else if (!strncmp(type, "LUMI", 4))
						currTexture=5;
					else if (!strncmp(type, "SPEC", 4))
						currTexture=2;
					else if (!strncmp(type, "REFL", 4))
						currTexture=3;
					else if (!strncmp(type, "TRAN", 4))
						currTexture=4;
					else if (!strncmp(type, "BUMP", 4))
						currTexture=6;
				}
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading channel ", type);
#endif
				break;
			case charsToUIntD('I','M','A','G'):
#ifdef LWO_READER_DEBUG
				os::Printer::log("LWO loader: loading channel map.");
#endif
				{
					u16 index;
					File->read(&index, 2);
#ifndef __BIG_ENDIAN__
					index=os::Byteswap::byteswap(index);
#endif
					size -= 2;
					if (index)
						mat->Texture[currTexture].Map=Images[index-1];
				}
				break;
			default:
				{
					File->seek(subsize, true);
					size -= subsize;
				}
		}
	}

	if (mat->Texture[0].Map != "") // diffuse // MEMO 通常のテクスチャマップ
		irrMat->setTexture(0,loadTexture(mat->Texture[0].Map));
	if (mat->Texture[3].Map != "") // reflection
	{
		video::ITexture* reflTexture = loadTexture(mat->Texture[3].Map);
		if (reflTexture)
		{
			irrMat->setTexture(1, irrMat->getTexture(0));
			irrMat->setTexture(0, reflTexture);
			irrMat->MaterialType=video::EMT_REFLECTION_2_LAYER;
		}
	}
	if (mat->Texture[4].Map != "") // transparency
	{
		video::ITexture* transTexture = loadTexture(mat->Texture[4].Map);
		if (transTexture)
		{
			irrMat->setTexture(1, irrMat->getTexture(0));
			irrMat->setTexture(0, transTexture);
			irrMat->MaterialType=video::EMT_TRANSPARENT_ADD_COLOR;
		}
	}
	if (mat->Texture[6].Map != "") // bump
	{
		irrMat->setTexture(1, loadTexture(mat->Texture[6].Map));
		if (irrMat->getTexture(1))
		{
//			SceneManager->getVideoDriver()->makeNormalMapTexture(irrMat->getTexture(1));
			irrMat->MaterialType=video::EMT_NORMAL_MAP_SOLID;
		}
	}
}


u32 CLWOMeshFileLoader::readColor(video::SColor& color)
{
	if (FormatVersion!=2)
	{
		u8 colorComponent;
		File->read(&colorComponent, 1);
		color.setRed(colorComponent);
		File->read(&colorComponent, 1);
		color.setGreen(colorComponent);
		File->read(&colorComponent, 1);
		color.setBlue(colorComponent);
		// unknown value
		File->read(&colorComponent, 1);
		return 4;
	}
	else
	{
		video::SColorf col;
		File->read(&col.r, 4);
#ifndef __BIG_ENDIAN__
		col.r=os::Byteswap::byteswap(col.r);
#endif
		File->read(&col.g, 4);
#ifndef __BIG_ENDIAN__
		col.g=os::Byteswap::byteswap(col.g);
#endif
		File->read(&col.b, 4);
#ifndef __BIG_ENDIAN__
		col.b=os::Byteswap::byteswap(col.b);
#endif
		color=col.toSColor();
		return 12;
	}
}

u32 CLWOMeshFileLoader::readString(core::stringc& name, u32 size)
{
	c8 c;

	name="";
	if (size)
		name.reserve(size);
	File->read(&c, 1);
	while (c)
	{
		name.append(c);
		File->read(&c, 1);
	}
	// read extra 0 upon odd file position
	if (File->getPos() & 0x1)
	{
		File->read(&c, 1);
		return (name.size()+2);
	}
	return (name.size()+1);
}


u32 CLWOMeshFileLoader::readVec(core::vector3df& vec)
{
	File->read(&vec.X, 4);
#ifndef __BIG_ENDIAN__
	vec.X=os::Byteswap::byteswap(vec.X);
#endif
	File->read(&vec.Y, 4);
#ifndef __BIG_ENDIAN__
	vec.Y=os::Byteswap::byteswap(vec.Y);
#endif
	File->read(&vec.Z, 4);
#ifndef __BIG_ENDIAN__
	vec.Z=os::Byteswap::byteswap(vec.Z);
#endif
	return 12;
}


u32 CLWOMeshFileLoader::readVX(u32& num)
{
	u16 tmpIndex;

	File->read(&tmpIndex, 2);
#ifndef __BIG_ENDIAN__
	tmpIndex=os::Byteswap::byteswap(tmpIndex);
#endif
	num=tmpIndex;
	if (num >= 0xFF00)
	{
		File->read(&tmpIndex, 2);
#ifndef __BIG_ENDIAN__
		tmpIndex=os::Byteswap::byteswap(tmpIndex);
#endif
		num=((num << 16)|tmpIndex) & ~0xFF000000;
		return 4;
	}
	return 2;
}


bool CLWOMeshFileLoader::readFileHeader()
{
	u32 Id;

	File->read(&Id, 4);
#ifndef __BIG_ENDIAN__
	Id=os::Byteswap::byteswap(Id);
#endif
	if (Id != 0x464f524d) // FORM
		return false;

	//skip the file length
	File->read(&Id, 4);

	File->read(&Id, 4);
#ifndef __BIG_ENDIAN__
	Id=os::Byteswap::byteswap(Id);
#endif
	// Currently supported: LWOB, LWLO, LWO2
	switch (Id)
	{
		case 0x4c574f42:
			FormatVersion = 0; // LWOB
		break;
		case 0x4c574c4f:
			FormatVersion = 1; // LWLO
		break;
		case 0x4c574f32:
			FormatVersion = 2; // LWO2
		break;
		default:
			return false; // unsupported
	}

	return true;
}

// XXX exist チェックは無駄コストなので決め打ちできるようにする
#define LWO_LOADER_USE_TEX_DIR = 1
#ifdef LWO_LOADER_USE_TEX_DIR
static const core::stringc TEX_DIR = "texture/";
#endif
video::ITexture* CLWOMeshFileLoader::loadTexture(const core::stringc& file)
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

#ifdef LWO_LOADER_USE_TEX_DIR
  // MEMO 区切りはスラッシュのみ、発見できなかった場合のケアはしない
  core::stringc targetName;
  s32 stringPos = file.findLast('/');
  if (stringPos != -1) {
    targetName = TEX_DIR + file.subString(stringPos+1, file.size()-stringPos);
    // TODO exit チェックいらないかな
#ifdef LWO_READER_DEBUG
    LWO_DPRINT("XXX TARGET: %s", targetName.c_str());
#endif
    if (FileSystem->existFile(targetName.c_str()))
      return driver->getTexture(targetName.c_str());
  }
#else // original
	if (FileSystem->existFile(file.c_str()))
		return driver->getTexture(file.c_str());

	core::stringc strippedName;
	s32 stringPos = file.findLast('/');
	if (stringPos==-1)
		stringPos = file.findLast('\\');
	if (stringPos != -1)
	{
		strippedName = file.subString(stringPos+1, file.size()-stringPos);
		if (FileSystem->existFile(strippedName.c_str()))
			return driver->getTexture(strippedName.c_str());
	}
	else
		strippedName = file;
	core::stringc newpath = File->getFileName();
	stringPos = newpath.findLast('/');
	if (stringPos==-1)
		stringPos = newpath.findLast('\\');
	if (stringPos != -1)
	{
		newpath = newpath.subString(0,stringPos+1);
		newpath.append(strippedName);
		if (FileSystem->existFile(newpath.c_str()))
			return driver->getTexture(newpath.c_str());
	}
#endif
	os::Printer::log("Could not load texture", file.c_str(), ELL_WARNING);
	return 0;
}


} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_LWO_LOADER_
