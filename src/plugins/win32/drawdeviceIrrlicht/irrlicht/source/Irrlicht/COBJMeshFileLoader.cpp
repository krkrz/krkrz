// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h" 
#ifdef _IRR_COMPILE_WITH_OBJ_LOADER_

#include "COBJMeshFileLoader.h"
#include "SMesh.h"
#include "SMeshBuffer.h"
#include "SAnimatedMesh.h"
#include "IReadFile.h"
#include "fast_atof.h"
#include "coreutil.h"

namespace irr
{
namespace scene
{

//! Constructor
COBJMeshFileLoader::COBJMeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver)
: FileSystem(fs), Driver(driver)
{
	if (FileSystem)
		FileSystem->grab();

	if (Driver)
		Driver->grab();
}



//! destructor
COBJMeshFileLoader::~COBJMeshFileLoader()
{
	if (FileSystem)
		FileSystem->drop();

	if (Driver)
		Driver->drop();
}



//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool COBJMeshFileLoader::isALoadableFileExtension(const c8* filename) const
{
	return strstr(filename, ".obj")!=0;
}



//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* COBJMeshFileLoader::createMesh(io::IReadFile* file)
{
	const long filesize = file->getSize();
	if (!filesize)
		return 0;

	const u32 WORD_BUFFER_LENGTH = 512;

	SMesh* mesh = new SMesh();

	core::array<core::vector3df> vertexBuffer;
	core::array<core::vector2df> textureCoordBuffer;
	core::array<core::vector3df> normalsBuffer;
	SObjMtl * currMtl = new SObjMtl();
	currMtl->Name="";
	Materials.push_back(currMtl);
	u32 smoothingGroup=0;

	// ********************************************************************
	// Patch to locate the file in the same folder as the .obj.
	// If you load the file as "data/some.obj" and mtllib contains
	// "mtlname test.mtl" (as usual), the loading will fail. Instead it
	// must look for data/test.tml. This patch does exactly that.
	//
	// patch by mandrav@codeblocks.org
	// ********************************************************************
	core::stringc obj_fullname = file->getFileName();
	core::stringc obj_relpath = "";
	s32 pathend = obj_fullname.findLast('/');
	if (pathend == -1)
		pathend = obj_fullname.findLast('\\');
	if (pathend != -1)
		obj_relpath = obj_fullname.subString(0, pathend + 1);
	// ********************************************************************
	// end of mtl folder patch
	// ********************************************************************

	c8* buf = new c8[filesize];
	memset(buf, 0, filesize);
	file->read((void*)buf, filesize);
	const c8* const bufEnd = buf+filesize;

	// Process obj information
	const c8* bufPtr = buf;
	while(bufPtr != bufEnd)
	{
		switch(bufPtr[0])
		{
		case 'm':	// mtllib (material)
		{
			c8 name[WORD_BUFFER_LENGTH];
			bufPtr = goAndCopyNextWord(name, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			readMTL(name, obj_relpath);
		}
			break;

		case 'v':               // v, vn, vt
			switch(bufPtr[1])
			{
			case ' ':          // vertex
				{
					core::vector3df vec;
					bufPtr = readVec3(bufPtr, vec, bufEnd);
					vertexBuffer.push_back(vec);
				}
				break;

			case 'n':       // normal
				{
					core::vector3df vec;
					bufPtr = readVec3(bufPtr, vec, bufEnd);
					normalsBuffer.push_back(vec);
				}
				break;

			case 't':       // texcoord
				{
					core::vector2df vec;
					bufPtr = readVec2(bufPtr, vec, bufEnd);
					textureCoordBuffer.push_back(vec);
				}
				break;
			}
			break;

		case 'g': // group names skipped
			{
			}
			break;

		case 's': // smoothing can be a group or off (equiv. to 0)
			{
				c8 smooth[WORD_BUFFER_LENGTH];
				bufPtr = goAndCopyNextWord(smooth, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
				if (core::stringc("off")==smooth)
					smoothingGroup=0;
				else
					smoothingGroup=core::strtol10(smooth, 0);
			}
			break;

		case 'u': // usemtl
			// get name of material
			{
				c8 matName[WORD_BUFFER_LENGTH];
				bufPtr = goAndCopyNextWord(matName, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
				// retrieve the material
				SObjMtl *useMtl = findMtl(matName);
				// only change material if we found it
				if (useMtl)
					currMtl = useMtl;
			}
			break;

		case 'f':               // face
		{
			c8 vertexWord[WORD_BUFFER_LENGTH]; // for retrieving vertex data
			video::S3DVertex v;
			// Assign vertex color from currently active material's diffuse colour
			if (currMtl)
				v.Color = currMtl->Meshbuffer->Material.DiffuseColor;

			// get all vertices data in this face (current line of obj file)
			const core::stringc wordBuffer = copyLine(bufPtr, bufEnd);
			const c8* linePtr = wordBuffer.c_str();
			const c8* const endPtr = linePtr+wordBuffer.size();

			core::array<int> faceCorners;
			faceCorners.reallocate(32); // should be large enough

			// read in all vertices
			linePtr = goNextWord(linePtr, endPtr);
			while (0 != linePtr[0])
			{
				// Array to communicate with retrieveVertexIndices()
				// sends the buffer sizes and gets the actual indices
				// if index not set returns -1
				s32 Idx[3];
				Idx[1] = Idx[2] = -1;

				// read in next vertex's data
				u32 wlength = copyWord(vertexWord, linePtr, WORD_BUFFER_LENGTH, bufEnd);
				// this function will also convert obj's 1-based index to c++'s 0-based index
				retrieveVertexIndices(vertexWord, Idx, vertexWord+wlength+1, vertexBuffer.size(), textureCoordBuffer.size(), normalsBuffer.size());
				v.Pos = vertexBuffer[Idx[0]];
				if ( -1 != Idx[1] )
					v.TCoords = textureCoordBuffer[Idx[1]];
				else
					v.TCoords.set(0.0f,0.0f);
				if ( -1 != Idx[2] )
					v.Normal = normalsBuffer[Idx[2]];
				else
					v.Normal.set(0.0f,0.0f,0.0f);

				int vertLocation;
				core::map<video::S3DVertex, int>::Node* n = currMtl->VertMap.find(v);
				if (n)
				{
					vertLocation = n->getValue();
				}
				else
				{
					currMtl->Meshbuffer->Vertices.push_back(v);
					vertLocation = currMtl->Meshbuffer->Vertices.size() -1;
					currMtl->VertMap.insert(v, vertLocation);
				}
				
				faceCorners.push_back(vertLocation);

				// go to next vertex
				linePtr = goNextWord(linePtr, endPtr);
			}

			// triangulate the face
			for ( u32 i = 1; i < faceCorners.size() - 1; ++i )
			{
				// Add a triangle
				currMtl->Meshbuffer->Indices.push_back( faceCorners[i+1] );
				currMtl->Meshbuffer->Indices.push_back( faceCorners[i] );
				currMtl->Meshbuffer->Indices.push_back( faceCorners[0] );
			}
			faceCorners.set_used(0); // fast clear
			faceCorners.reallocate(32);
		}
		break;

		case '#': // comment 
		default:
			break;
		}	// end switch(bufPtr[0])
		// eat up rest of line
		bufPtr = goNextLine(bufPtr, bufEnd);
	}	// end while(bufPtr && (bufPtr-buf<filesize))

	// Combine all the groups (meshbuffers) into the mesh
	for ( u32 m = 0; m < Materials.size(); ++m )
	{
		if ( Materials[m]->Meshbuffer->getIndexCount() > 0 )
		{
			Materials[m]->Meshbuffer->recalculateBoundingBox();
			mesh->addMeshBuffer( Materials[m]->Meshbuffer );
		}
	}

	// Create the Animated mesh if there's anything in the mesh
	SAnimatedMesh* animMesh = 0;
	if ( 0 != mesh->getMeshBufferCount() )
	{
		mesh->recalculateBoundingBox();
		animMesh = new SAnimatedMesh();
		animMesh->Type = EAMT_OBJ;
		animMesh->addMesh(mesh);
		animMesh->recalculateBoundingBox();
	}

	// Clean up the allocate obj file contents
	delete [] buf;
	// more cleaning up
	cleanUp();
	mesh->drop();

	return animMesh;
}


void COBJMeshFileLoader::readMTL(const c8* fileName, core::stringc relPath)
{
	const u32 WORD_BUFFER_LENGTH = 512;

	io::IReadFile * mtlReader;
	if (FileSystem->existFile(fileName))
		mtlReader = FileSystem->createAndOpenFile(fileName);
	else
		// try to read in the relative path, the .obj is loaded from
		mtlReader = FileSystem->createAndOpenFile((relPath + fileName).c_str());
	if (!mtlReader)	// fail to open and read file
		return;

	const long filesize = mtlReader->getSize();
	if (!filesize)
		return;

	c8* buf = new c8[filesize];
	mtlReader->read((void*)buf, filesize);
	const c8* bufEnd = buf+filesize;

	SObjMtl* currMaterial = 0;

	const c8* bufPtr = buf;
	while(bufPtr != bufEnd)
	{
		switch(*bufPtr)
		{
			case 'n': // newmtl
			{
				// if there's an existing material, store it first
				if ( currMaterial )
					Materials.push_back( currMaterial );

				// extract new material's name
				c8 mtlNameBuf[WORD_BUFFER_LENGTH];
				bufPtr = goAndCopyNextWord(mtlNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);

				currMaterial = new SObjMtl;
				currMaterial->Name = mtlNameBuf;
			}
			break;
			case 'i': // illum - illumination
			if ( currMaterial )
			{
				const u32 COLOR_BUFFER_LENGTH = 16;
				c8 illumStr[COLOR_BUFFER_LENGTH];

				bufPtr = goAndCopyNextWord(illumStr, bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
				currMaterial->Illumination = (c8)atol(illumStr);
			}
			break;
			case 'N': // Ns - shininess
			if ( currMaterial )
			{
				const u32 COLOR_BUFFER_LENGTH = 16;
				c8 nsStr[COLOR_BUFFER_LENGTH];

				bufPtr = goAndCopyNextWord(nsStr, bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
				f32 shininessValue = core::fast_atof(nsStr);

				// wavefront shininess is from [0, 1000], so scale for OpenGL
				shininessValue *= 0.128f;
				currMaterial->Meshbuffer->Material.Shininess = shininessValue;
			}
			break;
			case 'K':
			if ( currMaterial )
			{
				switch(bufPtr[1])
				{
				case 'd':		// Kd = diffuse
					{
						bufPtr = readColor(bufPtr, currMaterial->Meshbuffer->Material.DiffuseColor, bufEnd);

					}
					break;

				case 's':		// Ks = specular
					{
						bufPtr = readColor(bufPtr, currMaterial->Meshbuffer->Material.SpecularColor, bufEnd);
					}
					break;

				case 'a':		// Ka = ambience
					{
						bufPtr=readColor(bufPtr, currMaterial->Meshbuffer->Material.AmbientColor, bufEnd);
					}
					break;
				case 'e':		// Ke = emissive
					{
						bufPtr=readColor(bufPtr, currMaterial->Meshbuffer->Material.EmissiveColor, bufEnd);
					}
					break;
				}	// end switch(bufPtr[1])
			}	// end case 'K': if ( 0 != currMaterial )...
			break;
			case 'm': // texture maps
			if (currMaterial)
			{
				u8 type=0; // map_Kd - diffuse texture map
				if (!strncmp(bufPtr,"map_bump",8))
					type=1;
				else if (!strncmp(bufPtr,"map_d",5))
					type=2;
				else if (!strncmp(bufPtr,"map_refl",8))
					type=3;
				// extract new material's name
				c8 textureNameBuf[WORD_BUFFER_LENGTH];
				bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
				// handle options
				while (textureNameBuf[0]=='-')
				{
					if (!strncmp(bufPtr,"-blendu",7))
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					if (!strncmp(bufPtr,"-blendv",7))
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					if (!strncmp(bufPtr,"-cc",3))
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					if (!strncmp(bufPtr,"-clamp",6))
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					if (!strncmp(bufPtr,"-texres",7))
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					if (!strncmp(bufPtr,"-mm",3))
					{
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
					}
					if (!strncmp(bufPtr,"-o",2))
					{
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						// next parameters are optional, so skip rest of loop if no number is found
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
					}
					if (!strncmp(bufPtr,"-s",2))
					{
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						// next parameters are optional, so skip rest of loop if no number is found
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
					}
					if (!strncmp(bufPtr,"-t",2))
					{
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						// next parameters are optional, so skip rest of loop if no number is found
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
						bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
						if (!core::isdigit(textureNameBuf[0]))
							continue;
					}
					// get next word
					bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
				}
				if (type==1)
				{
					currMaterial->Meshbuffer->Material.MaterialTypeParam=core::fast_atof(textureNameBuf);
					bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
				}

				video::ITexture * texture = 0;
				if (FileSystem->existFile(textureNameBuf))
					texture = Driver->getTexture( textureNameBuf );
				else
					// try to read in the relative path, the .obj is loaded from
					texture = Driver->getTexture( (relPath + textureNameBuf).c_str() );
				if ( texture )
				{
					if (type==0)
						currMaterial->Meshbuffer->Material.setTexture(0, texture);
					else if (type==1)
					{
						Driver->makeNormalMapTexture(texture);
						currMaterial->Meshbuffer->Material.setTexture(1, texture);
						currMaterial->Meshbuffer->Material.MaterialType=video::EMT_PARALLAX_MAP_SOLID;
					}
					else if (type==2)
					{
						currMaterial->Meshbuffer->Material.setTexture(0, texture);
						currMaterial->Meshbuffer->Material.MaterialType=video::EMT_TRANSPARENT_ADD_COLOR;
					}
					else if (type==3)
					{
//						currMaterial->Meshbuffer->Material.Textures[1] = texture;
//						currMaterial->Meshbuffer->Material.MaterialType=video::EMT_REFLECTION_2_LAYER;
					}
					// Set diffuse material colour to white so as not to affect texture colour
					// Because Maya set diffuse colour Kd to black when you use a diffuse colour map
					// But is this the right thing to do?
					currMaterial->Meshbuffer->Material.DiffuseColor.set(
						currMaterial->Meshbuffer->Material.DiffuseColor.getAlpha(), 255, 255, 255 );
				}
			}
			break;
			case 'd': // d - transparency
			if ( currMaterial )
			{
				const u32 COLOR_BUFFER_LENGTH = 16;
				c8 dStr[COLOR_BUFFER_LENGTH];

				bufPtr = goAndCopyNextWord(dStr, bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
				f32 dValue = core::fast_atof(dStr);

				currMaterial->Meshbuffer->Material.DiffuseColor.setAlpha( (s32)(dValue * 255) );
				if (dValue<1.0f)
					currMaterial->Meshbuffer->Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
			}
			break;
			case 'T':
			if ( currMaterial )
			{
				switch ( bufPtr[1] )
				{
				case 'f':		// Tf - Transmitivity
					const u32 COLOR_BUFFER_LENGTH = 16;
					c8 redStr[COLOR_BUFFER_LENGTH];
					c8 greenStr[COLOR_BUFFER_LENGTH];
					c8 blueStr[COLOR_BUFFER_LENGTH];

					bufPtr = goAndCopyNextWord(redStr,   bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
					bufPtr = goAndCopyNextWord(greenStr, bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
					bufPtr = goAndCopyNextWord(blueStr,  bufPtr, COLOR_BUFFER_LENGTH, bufEnd);

					f32 transparency = ( core::fast_atof(redStr) + core::fast_atof(greenStr) + core::fast_atof(blueStr) ) / 3;

					currMaterial->Meshbuffer->Material.DiffuseColor.setAlpha( (s32)(transparency * 255) );
					if (transparency < 1.0f)
						currMaterial->Meshbuffer->Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
				}
			}
			break;
			default: // comments or not recognised
			break;
		} // end switch(bufPtr[0])
		// go to next line
		bufPtr = goNextLine(bufPtr, bufEnd);
	}	// end while (bufPtr)

	// end of file. if there's an existing material, store it
	if ( currMaterial )
	{
		Materials.push_back( currMaterial );
		currMaterial = 0;
	}

	delete [] buf;
	mtlReader->drop();
}

//! Read RGB color
const c8* COBJMeshFileLoader::readColor(const c8* bufPtr, video::SColor& color, const c8* const bufEnd)
{
	const u32 COLOR_BUFFER_LENGTH = 16;
	c8 colStr[COLOR_BUFFER_LENGTH];

	color.setAlpha(255);
	bufPtr = goAndCopyNextWord(colStr, bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
	color.setRed((s32)(core::fast_atof(colStr) * 255.0f));
	bufPtr = goAndCopyNextWord(colStr,   bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
	color.setGreen((s32)(core::fast_atof(colStr) * 255.0f));
	bufPtr = goAndCopyNextWord(colStr,   bufPtr, COLOR_BUFFER_LENGTH, bufEnd);
	color.setBlue((s32)(core::fast_atof(colStr) * 255.0f));
	return bufPtr;
}


//! Read 3d vector of floats
const c8* COBJMeshFileLoader::readVec3(const c8* bufPtr, core::vector3df& vec, const c8* const bufEnd)
{
	const u32 WORD_BUFFER_LENGTH = 256;
	c8 wordBuffer[WORD_BUFFER_LENGTH];

	bufPtr = goAndCopyNextWord(wordBuffer, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	vec.X=-core::fast_atof(wordBuffer); // change handedness
	bufPtr = goAndCopyNextWord(wordBuffer, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	vec.Y=core::fast_atof(wordBuffer);
	bufPtr = goAndCopyNextWord(wordBuffer, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	vec.Z=core::fast_atof(wordBuffer);
	return bufPtr;
}


//! Read 2d vector of floats
const c8* COBJMeshFileLoader::readVec2(const c8* bufPtr, core::vector2df& vec, const c8* const bufEnd)
{
	const u32 WORD_BUFFER_LENGTH = 256;
	c8 wordBuffer[WORD_BUFFER_LENGTH];

	bufPtr = goAndCopyNextWord(wordBuffer, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	vec.X=core::fast_atof(wordBuffer);
	bufPtr = goAndCopyNextWord(wordBuffer, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	vec.Y=-core::fast_atof(wordBuffer); // change handedness
	return bufPtr;
}


//! Read boolean value represented as 'on' or 'off'
const c8* COBJMeshFileLoader::readBool(const c8* bufPtr, bool& tf, const c8* const bufEnd)
{
	const u32 BUFFER_LENGTH = 8;
	c8 tfStr[BUFFER_LENGTH];

	bufPtr = goAndCopyNextWord(tfStr, bufPtr, BUFFER_LENGTH, bufEnd);
	tf = strcmp(tfStr, "off") != 0;
	return bufPtr;
}


COBJMeshFileLoader::SObjMtl* COBJMeshFileLoader::findMtl(const c8* mtlName)
{
	for (u32 i = 0; i < Materials.size(); ++i)
	{
		if ( Materials[i]->Name == mtlName )
			return Materials[i];
	}
	return 0;
}



//! skip space characters and stop on first non-space
const c8* COBJMeshFileLoader::goFirstWord(const c8* buf, const c8* const bufEnd)
{
	// skip space characters
	while((buf != bufEnd) && core::isspace(*buf))
		++buf;

	return buf;
}


//! skip current word and stop at beginning of next one
const c8* COBJMeshFileLoader::goNextWord(const c8* buf, const c8* const bufEnd)
{
	// skip current word
	while(( buf != bufEnd ) && !core::isspace(*buf))
		++buf;

	return goFirstWord(buf, bufEnd);
}


//! Read until line break is reached and stop at the next non-space character
const c8* COBJMeshFileLoader::goNextLine(const c8* buf, const c8* const bufEnd)
{
	// look for newline characters
	while(buf != bufEnd)
	{
		// found it, so leave
		if (*buf=='\n' || *buf=='\r')
			break;
		++buf;
	}
	return goFirstWord(buf, bufEnd);
}


u32 COBJMeshFileLoader::copyWord(c8* outBuf, const c8* const inBuf, u32 outBufLength, const c8* const bufEnd)
{
	if (!outBufLength)
		return 0;
	if (!inBuf)
	{
		*outBuf = 0;
		return 0;
	}

	u32 i = 0;
	while(inBuf[i])
	{
		if (core::isspace(inBuf[i]) || &(inBuf[i]) == bufEnd)
			break;
		++i;
	}

	u32 length = core::min_(i, outBufLength-1);
	for (u32 j=0; j<length; ++j)
		outBuf[j] = inBuf[j];

	outBuf[i] = 0;
	return length;
}


core::stringc COBJMeshFileLoader::copyLine(const c8* inBuf, const c8* bufEnd)
{
	if (!inBuf)
		return core::stringc();

	const c8* ptr = inBuf;
	while (ptr<bufEnd)
	{
		if (*ptr=='\n' || *ptr=='\r')
			break;
		++ptr;
	}
	return core::stringc(inBuf, ptr-inBuf+1);
}


const c8* COBJMeshFileLoader::goAndCopyNextWord(c8* outBuf, const c8* inBuf, u32 outBufLength, const c8* bufEnd)
{
	inBuf = goNextWord(inBuf, bufEnd);
	copyWord(outBuf, inBuf, outBufLength, bufEnd);
	return inBuf;
}


bool COBJMeshFileLoader::retrieveVertexIndices(c8* vertexData, s32* idx, const c8* bufEnd, u32 vbsize, u32 vtsize, u32 vnsize)
{
	c8 word[16] = "";
	const c8* p = goFirstWord(vertexData, bufEnd);
	u32 idxType = 0;	// 0 = posIdx, 1 = texcoordIdx, 2 = normalIdx

	u32 i = 0;
	while ( p != bufEnd )
	{
		if ( ( core::isdigit(*p)) || (*p == '-') )
		{
			// build up the number
			word[i++] = *p;
		}
		else if ( *p == '/' || *p == ' ' || *p == '\0' )
		{
			// number is completed. Convert and store it
			word[i] = '\0';
			// if no number was found index will become 0 and later on -1 by decrement
			if (word[0]=='-')
			{
				idx[idxType] = core::strtol10(word+1,0);
				idx[idxType] *= -1;
				switch (idxType)
				{
					case 0:
						idx[idxType] += vbsize;
						break;
					case 1:
						idx[idxType] += vtsize;
						break;
					case 2:
						idx[idxType] += vnsize;
						break;
				}
			}
			else
				idx[idxType] = core::strtol10(word,0)-1;

			// reset the word
			word[0] = '\0';
			i = 0;

			// go to the next kind of index type
			if (*p == '/')
			{
				if ( ++idxType > 2 )
				{
					// error checking, shouldn't reach here unless file is wrong
					idxType = 0;
				}
			}
			else
			{
				// set all missing values to disable (=-1)
				while (++idxType < 3)
					idx[idxType]=-1;
				++p;
				break; // while
			}
		}

		// go to the next char
		++p;
	}

	return true;
}


void COBJMeshFileLoader::cleanUp()
{
	u32 i;

	for (i = 0; i < Materials.size(); ++i )
	{
		Materials[i]->Meshbuffer->drop();
		delete Materials[i];
	}

	Materials.clear();
}


} // end namespace scene
} // end namespace irr


#endif // _IRR_COMPILE_WITH_OBJ_LOADER_
