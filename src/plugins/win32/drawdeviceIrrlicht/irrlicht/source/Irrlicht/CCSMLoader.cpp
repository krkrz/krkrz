// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//
// This file was written by Saurav Mohapatra and modified by Nikolaus Gebhardt.
// See CCSMLoader.h for details.

#include "IrrCompileConfig.h" 
#ifdef _IRR_COMPILE_WITH_CSM_LOADER_

#include "CCSMLoader.h"
#include "os.h"
#include "IFileSystem.h"
#include "IReadFile.h"
#include "ISceneManager.h"
#include "IAttributes.h"
#include "SMesh.h"
#include "IVideoDriver.h"
#include "SAnimatedMesh.h"
#include "SMeshBufferLightMap.h"

namespace irr
{
namespace scene
{
	//
	//	the CSM data types
	//
	struct color_rgb_t
	{
		s32 red;
		s32 green;
		s32 blue;

		void clear()
		{
			red = 0;
			green = 0;
			blue = 0;
		}
	};

	class BinaryFileReader;

	//
	//	The file header
	//
	class Header
	{
	public:

		static const s32 VERSION_4;
		static const s32 VERSION_4_1;

		Header(){ clear(); }
		virtual ~Header(){ clear(); }

		s32 getVersion() const{ return version; }
		void clear(){ version = 0; }
		void load(BinaryFileReader* pReader);

	private:

		s32 version;
	};


	//
	//	The groups
	//
	class Group
	{
	public:

		Group(){ clear(); }
		virtual ~Group(){ clear(); }

		void clear();
		void load(BinaryFileReader* pReader);

		s32 getFlags() const { return flags; }
		s32 getParentGroupID() const { return parentGroup; }
		const core::stringc& getProperties() const { return props; }
		const color_rgb_t* getColor() const { return &color; }

	private:

		s32 flags;
		s32 parentGroup;
		core::stringc props;
		color_rgb_t color;
	};


	//
	//	The visgroups
	//
	class VisGroup
	{
	public:

		VisGroup(){ clear(); }
		virtual ~VisGroup(){ clear(); }
		void clear();
		void load(BinaryFileReader* pReader);

		s32 getFlags() const{ return flags; }
		const core::stringc& getName() const{ return name; }
		const color_rgb_t* getColor() const{ return &color; }

	private:

		core::stringc name;
		s32 flags;
		color_rgb_t color;
	};


	//
	//	Lightmaps
	//
	class LightMap
	{
	public:

		LightMap() : pixelData(0){ clear(); }
		virtual ~LightMap(){ clear(); }
		void clear();
		void load(BinaryFileReader* pReader);
		s32 getWidth() const{ return width; }
		s32 getHeight() const{ return height; }
		const s32* getPixelData() const{ return pixelData; }

	private:

		s32 width;
		s32 height;
		s32* pixelData;
	};

	struct Triangle
	{
		s32 a,b,c;
	};


	struct Line
	{
		s32 a,b;
	};


	class Vertex
	{
	public:

		Vertex(){ clear(); }
		virtual ~Vertex(){ clear(); }
		void clear();
		void load(BinaryFileReader* pReader);

		const core::vector3df* getPosition() const { return &position; }
		const core::vector3df* getNormal() const { return &normal; }
		const color_rgb_t* getColor() const { return &color; }
		const core::vector3df* getTextureCoordinates() const { return &texCoords; }
		const core::vector3df* getLightMapCoordinates() const { return &lmapCoords; }

	private:

		core::vector3df position;
		core::vector3df normal;
		color_rgb_t color;
		core::vector3df texCoords;
		core::vector3df lmapCoords;
	};


	class Surface
	{
	public:

		Surface() { clear(); }
		virtual ~Surface(){ clear(); }

		void clear();
		void load(BinaryFileReader *pReader);

		s32 getFlags() const{ return flags; }
		const core::stringc& getTextureName() const{ return textureName; }
		s32 getLightMapId() const{ return lightMapId; }
		const core::vector2df* getUVOffset() const{ return &uvOffset; }
		const core::vector2df* getUVScale() const{ return &uvScale; }
		f32 getUVRotation() const{ return uvRotation; }

		u32 getVertexCount() const{ return vertices.size(); }
		const Vertex* getVertexAt(const s32 index) const{ return vertices[index]; }

		s32 getTriangleCount() const{ return triangles.size(); }
		const Triangle& getTriangleAt(const s32 index) const{ return triangles[index]; }

	private:

		s32 flags;
		core::stringc textureName;
		s32 lightMapId;
		core::vector2df uvOffset;
		core::vector2df uvScale;
		f32 uvRotation;
		core::array<Vertex*> vertices;
		core::array<Triangle> triangles;
		core::array<Line> lines;
	};

	class Mesh
	{
	public:

		Mesh(){ clear(); }
		virtual ~Mesh(){ clear(); }

		void clear();
		void load(BinaryFileReader* pReader, bool bReadVisGroups);

		s32 getFlags() const { return flags; }
		s32 getGroupID() const { return groupId; }
		const core::stringc& getProperties() const { return props; }
		const color_rgb_t* getColor() const { return &color; }
		const core::vector3df* getPosition() const { return &position; }
		s32 getVisgroupID() const { return visgroupId; }
		s32 getSurfaceCount() const { return surfaces.size(); }
		const Surface* getSurfaceAt(const s32 index) const { return surfaces[index]; }

	private:

		s32 flags;
		s32 groupId;
		core::stringc props;
		color_rgb_t color;
		core::vector3df position;
		s32 visgroupId;

		core::array<Surface*> surfaces;
	};

	class Entity
	{
	public:

		Entity() { clear(); }
		virtual ~Entity() { clear(); }

		void clear();
		void load(BinaryFileReader* pReader);
		s32 getVisgroupID() const { return visgroupId; }
		s32 getGroupID() const { return groupId; }
		const core::stringc& getProperties() const { return props; }
		const core::vector3df* getPosition() const { return &position; }

	private:

		s32 visgroupId;
		s32 groupId;
		core::stringc props;
		core::vector3df position;
	};


	class CameraData
	{
	public:

		CameraData(){ clear(); }
		virtual ~CameraData(){ clear(); }

		void clear();
		void load(BinaryFileReader* pReader);

		const core::vector3df* getPosition(){ return &position; }
		f32 getPitch(){ return pitch; }
		f32 getYaw(){ return yaw; }

	private:

		core::vector3df position;
		f32 pitch;
		f32 yaw;
	};

	//
	//	A CSM File
	//
	class CSMFile
	{
	public:

		CSMFile(){ clear(); }
		virtual ~CSMFile(){ clear(); }
		void clear();
		void load(BinaryFileReader* pReader);

		const Header* getHeader() const{ return &header; }

		s32 getGroupCount() const{ return groups.size(); }
		const Group* getGroupAt(const s32 index) const{ return groups[index]; }

		s32 getVisGroupCount() const{ return visgroups.size(); }
		const VisGroup* getVisGroupAt(const s32 index) const{ return visgroups[index]; }

		s32 getLightMapCount() const{ return lightmaps.size(); }
		const LightMap* getLightMapAt(const s32 index) const { return lightmaps[index]; }

		s32 getMeshCount() const{ return meshes.size(); }
		const Mesh* getMeshAt(const s32 index) const{ return meshes[index]; }

		s32 getEntityCount() const{ return entities.size(); }
		const Entity* getEntityAt(const s32 index) const{ return entities[index]; }

		const CameraData* getCameraData() const{ return &cameraData; }

	private:

		Header header;
		core::array<Group*> groups;
		core::array<VisGroup*> visgroups;
		core::array<LightMap*> lightmaps;
		core::array<Mesh*> meshes;
		core::array<Entity*> entities;
		CameraData cameraData;
	};


	//
	//	A Binary File Reader
	//
	class BinaryFileReader
	{
	public:

		BinaryFileReader(io::IReadFile* pFile, bool closeWhenDone=true);
		virtual ~BinaryFileReader();

		virtual s32 readBuffer(void* buffer, s32 len);

		s32 readLong();
		f32 readFloat();
		u8  readByte();

		core::stringc readString();
		void readVec3f(core::vector3df* v);
		void readVec2f(core::vector2df* v);
		void readColorRGB(color_rgb_t* color);

	private:

		io::IReadFile *file;
		bool autoClose;

	};

	CCSMLoader::CCSMLoader(scene::ISceneManager* manager, io::IFileSystem* fs)
		: FileSystem(fs), SceneManager(manager)
	{
	}

	CCSMLoader::~CCSMLoader()
	{
	}

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".bsp")
	bool CCSMLoader::isALoadableFileExtension(const c8* fileName) const
	{
		return strstr(fileName, ".csm")!=0;
	}

	//! creates/loads an animated mesh from the file.
	IAnimatedMesh* CCSMLoader::createMesh(io::IReadFile* file)
	{
		file->grab(); // originally, this loader created the file on its own.

		scene::IMesh* m = createCSMMesh(file);

		if (!m)
			return 0;

		SAnimatedMesh* am = new SAnimatedMesh();
		am->Type = EAMT_CSM;
		am->addMesh(m);
		m->drop();

		am->recalculateBoundingBox();
		return am;
	}

	scene::IMesh* CCSMLoader::createCSMMesh(io::IReadFile* file)
	{
		if (!file)
			return 0;

		if(file)
		{
			BinaryFileReader reader(file, true);
			CSMFile csmFile;
			csmFile.load(&reader);

			scene::IMesh* pMesh = createIrrlichtMesh(&csmFile,
				SceneManager->getParameters()->getAttributeAsString(CSM_TEXTURE_PATH),
				file->getFileName());
			return pMesh;
		}

		return 0;
	}

	scene::IMesh* CCSMLoader::createIrrlichtMesh(const CSMFile* csmFile,
		core::stringc textureRoot, const c8* lmprefix)
	{
		scene::SMesh *pMesh = new scene::SMesh();
		video::IVideoDriver* driver = SceneManager->getVideoDriver();

		for(s32 l = 0; l<csmFile->getLightMapCount(); l++)
		{
			const LightMap* lmap = csmFile->getLightMapAt(l);

			core::stringc lmapName = lmprefix;
			lmapName += "LMAP_";
			lmapName += (int)(l+1);
			os::Printer::log("CCSMLoader loading light map", lmapName.c_str());

			video::IImage* lmapImg = driver->createImageFromData(
				video::ECF_A8R8G8B8,
				core::dimension2d<s32>(lmap->getWidth(),lmap->getHeight()),
				(void *)(lmap->getPixelData()));

			driver->addTexture(lmapName.c_str(), lmapImg);
			lmapImg->drop();

		}

		for(s32 m = 0; m<csmFile->getMeshCount(); m++)
		{
			const Mesh* mshPtr = csmFile->getMeshAt(m);

			for(s32 s = 0; s < mshPtr->getSurfaceCount(); s++)
			{
				const Surface* surface = mshPtr->getSurfaceAt(s);

				core::stringc texName = textureRoot;
				texName+= "/";
				texName+= surface->getTextureName();

				video::ITexture* texture = driver->getTexture(texName.c_str());
				scene::SMeshBufferLightMap *buffer = new scene::SMeshBufferLightMap();

				//material
				core::stringc lmapName = lmprefix;
				lmapName += "LMAP_";
				lmapName += (int)surface->getLightMapId();

				buffer->Material.setTexture(0, texture);
				buffer->Material.setTexture(1, driver->getTexture(lmapName.c_str()));
				buffer->Material.Lighting = false;
				buffer->Material.MaterialType = video::EMT_LIGHTMAP_M4;

				for(u32 v = 0; v < surface->getVertexCount(); v++)
				{
					const Vertex *vtxPtr = surface->getVertexAt(v);
					video::S3DVertex2TCoords vtx;
					vtx.Pos = *(vtxPtr->getPosition());
					vtx.Normal = *(vtxPtr->getPosition());
					vtx.Color.set(255,vtxPtr->getColor()->red,vtxPtr->getColor()->green,vtxPtr->getColor()->blue);
					vtx.TCoords.set(vtxPtr->getTextureCoordinates()->X,vtxPtr->getTextureCoordinates()->Y);
					vtx.TCoords2.set(vtxPtr->getLightMapCoordinates()->X,0.0f - vtxPtr->getLightMapCoordinates()->Y);

					buffer->Vertices.push_back(vtx);
				}

				for(s32 t = 0; t < surface->getTriangleCount(); t++)
				{
					const Triangle& tri = surface->getTriangleAt(t);
					buffer->Indices.push_back(tri.a);
					buffer->Indices.push_back(tri.c);
					buffer->Indices.push_back(tri.b);
				}

				buffer->recalculateBoundingBox();
				pMesh->addMeshBuffer(buffer);
				buffer->drop();
			}
		}

		pMesh->recalculateBoundingBox();
		return pMesh;
	}

	const s32 Header::VERSION_4 = 4;
	const s32 Header::VERSION_4_1 = 5;

	void Header::load(BinaryFileReader* pReader)
	{
		version = pReader->readLong();
	}

	void Group::clear()
	{
		color.clear();
		flags = 0;
		parentGroup = 0;
		props = "";
	}

	void Group::load(BinaryFileReader* pReader)
	{
		flags = pReader->readLong();
		parentGroup = pReader->readLong();
		props = pReader->readString();
		pReader->readColorRGB(&color);
	}

	void VisGroup::clear()
	{
		color.clear();
		flags = 0;
		name = "";

	}

	void VisGroup::load(BinaryFileReader* pReader)
	{
		name = pReader->readString();
		flags = pReader->readLong();
		pReader->readColorRGB(&color);
	}

	void LightMap::clear()
	{
		if(pixelData)
		{
			delete[] pixelData;
			pixelData = 0;
		}
		width = height = 0;
	}

	void LightMap::load(BinaryFileReader* pReader)
	{
		width = pReader->readLong();
		height = pReader->readLong();
		pixelData = new s32[width * height];
		pReader->readBuffer(pixelData, width * height * sizeof(s32));
	}

	void Mesh::clear()
	{
		flags = 0;
		groupId = 0;
		visgroupId = 0;
		props = "";
		color.clear();
		position.set(0,0,0);

		for(u32 s = 0; s < surfaces.size(); s++)
		{
			if(surfaces[s])
			{
				delete surfaces[s];
			}
		}
		surfaces.clear();
	}

	void Mesh::load(BinaryFileReader* pReader, bool bReadVisGroups)
	{
		flags = pReader->readLong();
		groupId = pReader->readLong();
		props = pReader->readString();
		pReader->readColorRGB(&color);
		pReader->readVec3f(&position);
		if(bReadVisGroups)
			visgroupId = pReader->readLong();
		else
			visgroupId = 0;

		s32 count = pReader->readLong();

		for(s32 i = 0; i < count; i++)
		{
			Surface* surf = new Surface();
			surf->load(pReader);
			surfaces.push_back(surf);
		}
	}

	void Surface::clear()
	{
		flags = 0;
		lightMapId = 0;
		textureName = "";
		uvOffset.set(0.0f,0.0f);
		uvScale.set(0.0f,0.0f);
		uvRotation = 0.0f;
		triangles.clear();
		lines.clear();

		for(u32 v = 0; v < vertices.size(); v++)
			delete vertices[v];

		vertices.clear();
	}

	void Surface::load(BinaryFileReader* pReader)
	{
		flags = pReader->readLong();
		textureName = pReader->readString();

		lightMapId = pReader->readLong();
		pReader->readVec2f(&uvOffset);
		pReader->readVec2f(&uvScale);
		uvRotation = pReader->readFloat();
		s32 vtxCount = pReader->readLong();
		s32 triCount = pReader->readLong();
		s32 lineCount = pReader->readLong();

		for(s32 v = 0; v < vtxCount; v++)
		{
			Vertex *vtx = new Vertex();
			vtx->load(pReader);
			vertices.push_back(vtx);
		}

		for(s32 t = 0; t < triCount; t++)
		{
			Triangle tri;
			pReader->readBuffer(&tri, sizeof(tri));
			triangles.push_back(tri);
		}

		for(s32 l = 0; l < lineCount; l++)
		{
			Line line;
			pReader->readBuffer(&line,sizeof(line));
			lines.push_back(line);

		}

	}

	void Vertex::clear()
	{
		position.set(0,0,0);
		normal.set(0,0,0);
		color.clear();
		texCoords.set(0,0,0);
		lmapCoords.set(0,0,0);
	}

	void Vertex::load(BinaryFileReader* pReader)
	{
		pReader->readVec3f(&position);
		pReader->readVec3f(&normal);
		pReader->readColorRGB(&color);
		pReader->readVec3f(&texCoords);
		pReader->readVec3f(&lmapCoords);
	}

	void Entity::clear()
	{
		visgroupId = groupId = 0;
		props = "";
		position.set(0,0,0);
	}

	void Entity::load(BinaryFileReader* pReader)
	{
		visgroupId = pReader->readLong();
		groupId = pReader->readLong();
		props = pReader->readString();
		pReader->readVec3f(&position);
	}

	void CameraData::clear()
	{
		position.set(0,0,0);
		pitch = 0;
		yaw = 0;
	}

	void CameraData::load(BinaryFileReader* pReader)
	{
		pReader->readVec3f(&position);
		pitch = pReader->readFloat();
		yaw = pReader->readFloat();
	}

	void CSMFile::clear()
	{
		header.clear();
		cameraData.clear();

		u32 x =0;
		for( x= 0; x < groups.size(); x++)
			delete groups[x];

		groups.clear();

		for(x= 0; x < visgroups.size(); x++)
			delete visgroups[x];

		visgroups.clear();

		for(x= 0; x < lightmaps.size(); x++)
			delete lightmaps[x];

		lightmaps.clear();

		for(x= 0; x < meshes.size(); x++)
			delete meshes[x];

		meshes.clear();

		for(x= 0; x < entities.size(); x++)
			delete entities[x];

		entities.clear();
	}

	void CSMFile::load(BinaryFileReader* pReader)
	{
		clear();

		header.load(pReader);

		//groups
		{
			s32 count = pReader->readLong();

			for (s32 i = 0; i < count; i++)
			{
				Group* grp = new Group();
				grp->load(pReader);
				groups.push_back(grp);
			}
		}
		bool bHasVGroups = (header.getVersion() == Header::VERSION_4_1);

		if (bHasVGroups)
		{
			//visgroups
			s32 count = pReader->readLong();

			for (s32 i = 0; i < count; i++)
			{
				VisGroup* grp = new VisGroup();
				grp->load(pReader);
				visgroups.push_back(grp);
			}
		}

		//lightmaps
		{
			s32 count = pReader->readLong();

			for(s32 i = 0; i < count; i++)
			{
				LightMap* grp = new LightMap();
				grp->load(pReader);
				lightmaps.push_back(grp);
			}
		}

		//meshes
		{
			s32 count = pReader->readLong();

			for(s32 i = 0; i < count; i++)
			{
				Mesh* grp = new Mesh();
				grp->load(pReader,bHasVGroups);
				meshes.push_back(grp);
			}
		}

		//entities
		{
			s32 count = pReader->readLong();

			for(s32 i = 0; i < count; i++)
			{
				Entity* grp = new Entity();
				grp->load(pReader);
				entities.push_back(grp);
			}
		}

		//camera data
		cameraData.load(pReader);


	}

	BinaryFileReader::BinaryFileReader(io::IReadFile* pFile,
		bool closeWhenDone)
		: file(pFile), autoClose(closeWhenDone)
	{
	}

	BinaryFileReader::~BinaryFileReader()
	{
		if(autoClose && file)
		{
			file->drop();
			file = 0;

		}
	}

	s32 BinaryFileReader::readBuffer(void* buffer, s32 len)
	{
		return file->read(buffer,len);

	}

	s32 BinaryFileReader::readLong()
	{
		int ret = 0;
		readBuffer(&ret,sizeof(int));
		return (s32)ret;
	}

	f32 BinaryFileReader::readFloat()
	{
		float ret = 0;
		readBuffer(&ret,sizeof(float));
		return (f32)ret;
	}

	u8  BinaryFileReader::readByte()
	{
		char ret = 0;
		readBuffer(&ret,sizeof(char));
		return (u8)ret;
	}

	core::stringc BinaryFileReader::readString()
	{
		core::stringc str = "";
		c8 c = (c8)readByte();
		while(c != 0)
		{
			str += c;
			c = (c8)readByte();
		}
		return str;
	}

	void BinaryFileReader::readVec3f(core::vector3df* v)
	{
		v->X = readFloat();
		v->Y = readFloat();
		v->Z = readFloat();
	}

	void BinaryFileReader::readVec2f(core::vector2df* v)
	{
		v->X = readFloat();
		v->Y = readFloat();
	}

	void BinaryFileReader::readColorRGB(color_rgb_t* color)
	{
		readBuffer(color,sizeof(color_rgb_t));
	}

} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_CSM_LOADER_
