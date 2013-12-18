// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BSP_LOADER_

#include "CQ3LevelMesh.h"
#include "ISceneManager.h"
#include "os.h"
#include "SMeshBufferLightMap.h"
#include "irrString.h"
#include "ILightSceneNode.h"
#include "IQ3Shader.h"

namespace irr
{
namespace scene
{


//! constructor
CQ3LevelMesh::CQ3LevelMesh(io::IFileSystem* fs, video::IVideoDriver* driver, scene::ISceneManager* smgr)
: Textures(0), LightMaps(0),
 Vertices(0), Faces(0),	Planes(0), Nodes(0), Leafs(0), LeafFaces(0),
	MeshVerts(0), Brushes(0), Driver(driver), FileSystem(fs), SceneManager ( smgr )
{
	#ifdef _DEBUG
	IReferenceCounted::setDebugName("CQ3LevelMesh");
	#endif

	for ( s32 i = 0; i!= quake3::E_Q3_MESH_SIZE; ++i )
	{
		Mesh[i] = 0;
	}

	if (Driver)
		Driver->grab();

	if (FileSystem)
		FileSystem->grab();

	// load default shaders
	InitShader ();
}


//! destructor
CQ3LevelMesh::~CQ3LevelMesh()
{
	delete [] Textures;
	delete [] LightMaps;
	delete [] Vertices;
	delete [] Faces;
	delete [] Planes;
	delete [] Nodes;
	delete [] Leafs;
	delete [] LeafFaces;
	delete [] MeshVerts;
	delete [] Brushes;

	if (Driver)
		Driver->drop();

	if (FileSystem)
		FileSystem->drop();

	for ( s32 i = 0; i!= quake3::E_Q3_MESH_SIZE; ++i )
	{
		if (Mesh[i])
			Mesh[i]->drop();
	}

	ReleaseShader();
	ReleaseEntity();
}


//! loads a level from a .bsp-File. Also tries to load all needed textures. Returns true if successful.
bool CQ3LevelMesh::loadFile(io::IReadFile* file)
{
	if (!file)
		return false;

	LevelName = file->getFileName();

	tBSPHeader header;
	file->read(&header, sizeof(tBSPHeader));

	#ifdef __BIG_ENDIAN__
		header.strID = os::Byteswap::byteswap(header.strID);
		header.version = os::Byteswap::byteswap(header.version);
	#endif

	if (header.strID != 0x50534249 || header.version != 0x2e)
	{
		os::Printer::log("Could not load .bsp file, unknown header.", file->getFileName(), ELL_ERROR);
		return false;
	}

	// now read lumps

	file->read(&Lumps[0], sizeof(tBSPLump)*kMaxLumps);

	#ifdef __BIG_ENDIAN__
	for (int i=0;i<kMaxLumps;++i)
	{
		Lumps[i].offset = os::Byteswap::byteswap(Lumps[i].offset);
		Lumps[i].length = os::Byteswap::byteswap(Lumps[i].length);
	}
	#endif

	for ( s32 i = 0; i!= quake3::E_Q3_MESH_SIZE; ++i )
	{
		Mesh[i] = new SMesh();
	}
	ReleaseEntity();

	// load everything

	loadTextures(&Lumps[kTextures], file);			// Load the textures
	loadLightmaps(&Lumps[kLightmaps], file);		// Load the lightmaps
	loadVerts(&Lumps[kVertices], file);				// Load the vertices
	loadFaces(&Lumps[kFaces], file);				// Load the faces
	loadPlanes(&Lumps[kPlanes], file);				// Load the Planes of the BSP
	loadNodes(&Lumps[kNodes], file);				// load the Nodes of the BSP
	loadLeafs(&Lumps[kLeafs], file);				// load the Leafs of the BSP
	loadLeafFaces(&Lumps[kLeafFaces], file);		// load the Faces of the Leafs of the BSP
	loadVisData(&Lumps[kVisData], file);			// load the visibility data of the clusters
	loadEntities(&Lumps[kEntities], file);			// load the entities
	loadModels(&Lumps[kModels], file);				// load the models
	loadMeshVerts(&Lumps[kMeshVerts], file);		// load the mesh vertices
	loadBrushes(&Lumps[kBrushes], file);			// load the brushes of the BSP
	loadBrushSides(&Lumps[kBrushSides], file);		// load the brushsides of the BSP
	loadLeafBrushes(&Lumps[kLeafBrushes], file);	// load the brushes of the leaf
	loadShaders(&Lumps[kShaders], file );			// load the shaderes

	PatchTesselation = 8;

	//constructMesh();
	//loadTextures();


	loadTextures2();
	constructMesh2();

	cleanMeshes();
	calcBoundingBoxes();

	return true;
}


//! returns the amount of frames in milliseconds. If the amount is 1, it is a static (=non animated) mesh.
u32 CQ3LevelMesh::getFrameCount() const
{
	return 1;
}


void CQ3LevelMesh::releaseMesh ( s32 index )
{
	if ( Mesh[index] )
	{
		Mesh[index]->drop ();
		Mesh[index] = 0;
	}
}


//! returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail. Note, that some Meshes will ignore the detail level.
IMesh* CQ3LevelMesh::getMesh(s32 frameInMs, s32 detailLevel, s32 startFrameLoop, s32 endFrameLoop)
{
	return Mesh[frameInMs];
}


void CQ3LevelMesh::loadTextures(tBSPLump* l, io::IReadFile* file)
{
	NumTextures = l->length / sizeof(tBSPTexture);
	Textures = new tBSPTexture[NumTextures];

	file->seek(l->offset);
	file->read(Textures, l->length);

	#ifdef __BIG_ENDIAN__
	for (int i=0;i<NumTextures;i++)
	{
		Textures[i].flags = os::Byteswap::byteswap(Textures[i].flags);
		Textures[i].contents = os::Byteswap::byteswap(Textures[i].contents);
	}
	#endif
}


void CQ3LevelMesh::loadLightmaps(tBSPLump* l, io::IReadFile* file)
{
	NumLightMaps = l->length / sizeof(tBSPLightmap);
	LightMaps = new tBSPLightmap[NumLightMaps];

	file->seek(l->offset);
	file->read(LightMaps, l->length);
}


void CQ3LevelMesh::loadVerts(tBSPLump* l, io::IReadFile* file)
{
	NumVertices = l->length / sizeof(tBSPVertex);
	Vertices = new tBSPVertex[NumVertices];

	file->seek(l->offset);
	file->read(Vertices, l->length);

	#ifdef __BIG_ENDIAN__
	for (int i=0;i<NumVertices;i++)
	{
		Vertices[i].vPosition[0] = os::Byteswap::byteswap(Vertices[i].vPosition[0]);
		Vertices[i].vPosition[1] = os::Byteswap::byteswap(Vertices[i].vPosition[1]);
		Vertices[i].vPosition[2] = os::Byteswap::byteswap(Vertices[i].vPosition[2]);
		Vertices[i].vTextureCoord[0] = os::Byteswap::byteswap(Vertices[i].vTextureCoord[0]);
		Vertices[i].vTextureCoord[1] = os::Byteswap::byteswap(Vertices[i].vTextureCoord[1]);
		Vertices[i].vLightmapCoord[0] = os::Byteswap::byteswap(Vertices[i].vLightmapCoord[0]);
		Vertices[i].vLightmapCoord[1] = os::Byteswap::byteswap(Vertices[i].vLightmapCoord[1]);
		Vertices[i].vNormal[0] = os::Byteswap::byteswap(Vertices[i].vNormal[0]);
		Vertices[i].vNormal[1] = os::Byteswap::byteswap(Vertices[i].vNormal[1]);
		Vertices[i].vNormal[2] = os::Byteswap::byteswap(Vertices[i].vNormal[2]);
	}
	#endif
}


void CQ3LevelMesh::loadFaces(tBSPLump* l, io::IReadFile* file)
{
	NumFaces = l->length / sizeof(tBSPFace);
	Faces = new tBSPFace[NumFaces];

	file->seek(l->offset);
	file->read(Faces, l->length);

	#ifdef __BIG_ENDIAN__
	for ( s32 i=0;i<NumFaces;i++)
	{
		Faces[i].textureID = os::Byteswap::byteswap(Faces[i].textureID);
		Faces[i].effect = os::Byteswap::byteswap(Faces[i].effect);
		Faces[i].type = os::Byteswap::byteswap(Faces[i].type);
		Faces[i].vertexIndex = os::Byteswap::byteswap(Faces[i].vertexIndex);
		Faces[i].numOfVerts = os::Byteswap::byteswap(Faces[i].numOfVerts);
		Faces[i].meshVertIndex = os::Byteswap::byteswap(Faces[i].meshVertIndex);
		Faces[i].numMeshVerts = os::Byteswap::byteswap(Faces[i].numMeshVerts);
		Faces[i].lightmapID = os::Byteswap::byteswap(Faces[i].lightmapID);
		Faces[i].lMapCorner[0] = os::Byteswap::byteswap(Faces[i].lMapCorner[0]);
		Faces[i].lMapCorner[1] = os::Byteswap::byteswap(Faces[i].lMapCorner[1]);
		Faces[i].lMapSize[0] = os::Byteswap::byteswap(Faces[i].lMapSize[0]);
		Faces[i].lMapSize[1] = os::Byteswap::byteswap(Faces[i].lMapSize[1]);
		Faces[i].lMapPos[0] = os::Byteswap::byteswap(Faces[i].lMapPos[0]);
		Faces[i].lMapPos[1] = os::Byteswap::byteswap(Faces[i].lMapPos[1]);
		Faces[i].lMapPos[2] = os::Byteswap::byteswap(Faces[i].lMapPos[2]);
		Faces[i].lMapBitsets[0][0] = os::Byteswap::byteswap(Faces[i].lMapBitsets[0][0]);
		Faces[i].lMapBitsets[0][1] = os::Byteswap::byteswap(Faces[i].lMapBitsets[0][1]);
		Faces[i].lMapBitsets[0][2] = os::Byteswap::byteswap(Faces[i].lMapBitsets[0][2]);
		Faces[i].lMapBitsets[1][0] = os::Byteswap::byteswap(Faces[i].lMapBitsets[1][0]);
		Faces[i].lMapBitsets[1][1] = os::Byteswap::byteswap(Faces[i].lMapBitsets[1][1]);
		Faces[i].lMapBitsets[1][2] = os::Byteswap::byteswap(Faces[i].lMapBitsets[1][2]);
		Faces[i].vNormal[0] = os::Byteswap::byteswap(Faces[i].vNormal[0]);
		Faces[i].vNormal[1] = os::Byteswap::byteswap(Faces[i].vNormal[1]);
		Faces[i].vNormal[2] = os::Byteswap::byteswap(Faces[i].vNormal[2]);
		Faces[i].size[0] = os::Byteswap::byteswap(Faces[i].size[0]);
		Faces[i].size[1] = os::Byteswap::byteswap(Faces[i].size[1]);
	}
	#endif
}


void CQ3LevelMesh::loadPlanes(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadNodes(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadLeafs(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadLeafFaces(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadVisData(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadEntities(tBSPLump* l, io::IReadFile* file)
{
	core::array<u8> entity;
	entity.set_used ( l->length + 2 );
	entity[l->length + 1 ] = 0;

	file->seek(l->offset);
	file->read ( entity.pointer(), l->length);

	parser_parse ( entity.pointer(), l->length, &CQ3LevelMesh::scriptcallback_entity );
}


// load shaders named in bsp
void CQ3LevelMesh::loadShaders(tBSPLump* l, io::IReadFile* file)
{
	u32 files = l->length / sizeof(tBSPShader);

	file->seek ( l->offset );

	tBSPShader def;
	for ( u32 i = 0; i!= files; ++i )
	{
		file->read ( &def, sizeof ( def ) );
		getShader ( def.strName, 1 );
	}
}


void CQ3LevelMesh::loadModels(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadMeshVerts(tBSPLump* l, io::IReadFile* file)
{
	NumMeshVerts = l->length / sizeof(s32);
	MeshVerts = new s32[NumMeshVerts];

	file->seek(l->offset);
	file->read(MeshVerts, l->length);

	#ifdef __BIG_ENDIAN__
	for (int i=0;i<NumMeshVerts;i++)
		MeshVerts[i] = os::Byteswap::byteswap(MeshVerts[i]);
	#endif
}


void CQ3LevelMesh::loadBrushes(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadBrushSides(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


void CQ3LevelMesh::loadLeafBrushes(tBSPLump* l, io::IReadFile* file)
{
	// ignore
}


inline bool isQ3WhiteSpace ( const u8 symbol )
{
	return symbol == ' ' || symbol == '\t' || symbol == '\r';
}


void CQ3LevelMesh::parser_nextToken ()
{
	u8 symbol;

	Parser.token = "";
	Parser.tokenresult = Q3_TOKEN_UNRESOLVED;

	// skip white space
	do
	{
		if ( Parser.index >= Parser.sourcesize )
		{
			Parser.tokenresult = Q3_TOKEN_EOF;
			return;
		}

		symbol = Parser.source [ Parser.index ];
		Parser.index += 1;
	} while ( isQ3WhiteSpace ( symbol ) );

	// first symbol, one symbol
	switch ( symbol )
	{
		case 0:
			Parser.tokenresult = Q3_TOKEN_EOF;
			return;

		case '/':
			// comment or divide
			if ( Parser.index >= Parser.sourcesize )
			{
				Parser.tokenresult = Q3_TOKEN_EOF;
				return;
			}
			symbol = Parser.source [ Parser.index ];
			Parser.index += 1;
			if ( isQ3WhiteSpace ( symbol ) )
			{
				Parser.tokenresult = Q3_TOKEN_MATH_DIVIDE;
				return;
			}
			else
			if ( symbol == '*' )
			{
				// C-style comment in quake?
			}
			else
			if ( symbol == '/' )
			{
				// skip to eol
				do
				{
					if ( Parser.index >= Parser.sourcesize )
					{
						Parser.tokenresult = Q3_TOKEN_EOF;
						return;
					}
					symbol = Parser.source [ Parser.index ];
					Parser.index += 1;
				} while ( symbol != '\n' );
				Parser.tokenresult = Q3_TOKEN_COMMENT;
				return;
			}
			// take /[name] as valid token..?!?!?. mhmm, maybe
			break;

		case '\n':
			Parser.tokenresult = Q3_TOKEN_EOL;
			return;
		case '{':
			Parser.tokenresult = Q3_TOKEN_START_LIST;
			return;
		case '}':
			Parser.tokenresult = Q3_TOKEN_END_LIST;
			return;

		case '"':
			// string literal
			do
			{
				if ( Parser.index >= Parser.sourcesize )
				{
					Parser.tokenresult = Q3_TOKEN_EOF;
					return;
				}
				symbol = Parser.source [ Parser.index ];
				Parser.index += 1;
				if ( symbol != '"' )
					Parser.token.append ( symbol );
			} while ( symbol != '"' );
			Parser.tokenresult = Q3_TOKEN_ENTITY;
			return;
	}

	// user identity
	Parser.token.append ( symbol );

	// continue till whitespace
	bool notisWhite = true;
	do
	{
		if ( Parser.index >= Parser.sourcesize )
		{
			Parser.tokenresult = Q3_TOKEN_EOF;
			return;
		}
		symbol = Parser.source [ Parser.index ];

		notisWhite = ! isQ3WhiteSpace ( symbol );
		if ( notisWhite )
		{
			Parser.token.append ( symbol );
		}

		Parser.index += 1;

	} while ( notisWhite );

	Parser.tokenresult = Q3_TOKEN_TOKEN;
	return;
}


/*
	parse entity & shader
	calls callback on content in {}
*/
void CQ3LevelMesh::parser_parse ( const void * data, const u32 size, CQ3LevelMesh::tParserCallback callback )
{
	Parser.source = (const c8*) data;
	Parser.sourcesize = size;
	Parser.index = 0;

	quake3::SVarGroupList *groupList;

	s32 active;
	s32 last;

	quake3::SVariable entity;

	groupList = new quake3::SVarGroupList ();

	groupList->VariableGroup.push_back ( quake3::SVarGroup () );
	active = last = 0;

	do
	{
		parser_nextToken ();

		switch ( Parser.tokenresult )
		{
			case Q3_TOKEN_START_LIST:
			{
				//stack = core::min_ ( stack + 1, 7 );

				groupList->VariableGroup.push_back ( quake3::SVarGroup () );
				last = active;
				active = groupList->VariableGroup.size() - 1;
				entity.clear ();
			}  break;

			// a unregisterd variable is finished
			case Q3_TOKEN_EOL:
			{
				if ( entity.isValid() )
				{
					groupList->VariableGroup[active].Variable.push_back ( entity );
					entity.clear ();
				}
			} break;

			case Q3_TOKEN_TOKEN:
			case Q3_TOKEN_ENTITY:
			{
				Parser.token.make_lower();

				// store content based on line-delemiter
				if ( 0 == entity.isValid() )
				{
					entity.name = Parser.token;
					entity.content = "";

				}
				else
				{
					if ( entity.content.size() )
					{
						entity.content += " ";
					}
					entity.content += Parser.token;
				}
			} break;

			case Q3_TOKEN_END_LIST:
			{
				//stack = core::max_ ( stack - 1, 0 );

				// close tag for first
				if ( active == 1 )
				{
					(this->*callback) ( groupList );

					// new group
					groupList->drop ();
					groupList = new quake3::SVarGroupList ();
					groupList->VariableGroup.push_back ( quake3::SVarGroup () );
					last = 0;
				}

				active = last;
				entity.clear();

			} break;

		}

	} while ( Parser.tokenresult != Q3_TOKEN_EOF );

	groupList->drop ();
}


/*
	this loader applies only textures for stage 1 & 2
*/
s32 CQ3LevelMesh::setShaderMaterial ( video::SMaterial &material, const tBSPFace * face ) const
{
	material.MaterialType = video::EMT_SOLID;
	material.Wireframe = false;
	material.Lighting = false;
	material.BackfaceCulling = true;
	material.setTexture(0, 0);
	material.setTexture(1, 0);
	material.setTexture(2, 0);
	material.setTexture(3, 0);
	material.ZBuffer = true;
	material.ZWriteEnable = true;
	material.MaterialTypeParam = 0.f;

	s32 shaderState = -1;

	if ( face->textureID >= 0 )
	{
		material.setTexture(0, Tex [ face->textureID ].Texture);
		shaderState = Tex [ face->textureID ].ShaderID;
	}

	if ( face->lightmapID >= 0 )
	{
		material.setTexture(1, Lightmap [ face->lightmapID ]);
		material.MaterialType = quake3::defaultLightMap;
	}

	// store shader ID
	material.MaterialTypeParam2 = (f32) shaderState;

	const quake3::SShader *shader = getShader ( shaderState );
	if ( 0 == shader )
		return shaderState;

	const quake3::SVarGroup *group;

	s32 index;

	// generic
	group = shader->getGroup ( 1 );
	if ( group )
	{
		material.BackfaceCulling = quake3::getBackfaceCulling ( group->get ( "cull" ) );

		if ( group->isDefined ( "surfaceparm", "nolightmap" ) )
		{
			material.MaterialType = video::EMT_SOLID;
			material.setTexture(1, 0);
		}

	}

	// try to get the best of the 8 texture stages..

	// texture 1, texture 2
	u32 startPos;
	for ( s32 g = 2; g <= 3; ++g )
	{
		group = shader->getGroup ( g );
		if ( 0 == group )
			continue;

		startPos = 0;

		index = group->getIndex ( "depthwrite" );
		if ( index >= 0 )
		{
			material.ZBuffer = true;
		}

		quake3::SBlendFunc blendfunc;
		quake3::getBlendFunc ( group->get ( "blendfunc" ), blendfunc );
		quake3::getBlendFunc ( group->get ( "alphafunc" ), blendfunc );

		material.MaterialType = blendfunc.type;
		material.MaterialTypeParam = blendfunc.param;

		// try if we can match better
		shaderState |= (material.MaterialType == video::EMT_SOLID ) ? 0x00020000 : 0;
	}

	//material.BackfaceCulling = false;

	if ( shader->VarGroup->VariableGroup.size () <= 4 )
	{
		shaderState |= 0x00010000;
	}

	material.MaterialTypeParam2 = (f32) shaderState;
	return shaderState;
}

//! constructs a mesh from the quake 3 level file.
void CQ3LevelMesh::constructMesh2()
{
	s32 i, j, k;

	s32 *index;

	video::S3DVertex2TCoords temp[3];

	video::SMaterial material;

	SToBuffer item;

	core::array < SToBuffer > toBuffer;

	const s32 mesh0size = (NumTextures+1) * (NumLightMaps+1);
	for ( i=0; i < mesh0size; ++i)
	{
		scene::SMeshBufferLightMap* buffer = new scene::SMeshBufferLightMap();

		Mesh[quake3::E_Q3_MESH_GEOMETRY]->addMeshBuffer(buffer);
		buffer->drop();
	}

	for ( i=0; i<NumFaces; ++i)
	{
		const tBSPFace * face = &Faces [i];

		s32 shaderState = setShaderMaterial ( material, face );
		toBuffer.clear ();

		const quake3::SShader *shader = getShader ( shaderState );

		switch( Faces[i].type )
		{
			case 1: // normal polygons
			case 2: // patches
			case 3: // meshes
				{
					if ( 0 == shader )
					{
						item.takeVertexColor = material.getTexture(0) == 0 || material.getTexture(1) == 0;
						item.index = quake3::E_Q3_MESH_GEOMETRY;
						toBuffer.push_back ( item );
					}
					else
					{
						item.takeVertexColor = 1;
						item.index = quake3::E_Q3_MESH_ITEMS;
						toBuffer.push_back ( item );
					}

				} break;

/*
			case 1: // normal polygons
			case 2: // patches
				if ( material.Textures[0] && 0 == shader )
				{
					item.takeVertexColor = material.Textures[1] == 0;
					item.index = quake3::E_Q3_MESH_GEOMETRY;
					toBuffer.push_back ( item );
				}
				else
				if ( material.Textures[0] )
				{
					item.takeVertexColor = material.Textures[1] == 0;
					item.index = quake3::E_Q3_MESH_GEOMETRY;
					toBuffer.push_back ( item );
					if ( 0 == (shaderState & 0xFFFF0000 ) )
					{
						item.takeVertexColor = 1;
						item.index = quake3::E_Q3_MESH_ITEMS;
						toBuffer.push_back ( item );
					}
				}
				else
				{
					item.takeVertexColor = 1;
					item.index = quake3::E_Q3_MESH_ITEMS;
					toBuffer.push_back ( item );
				}
				break;

			case 3: // mesh vertices
				if ( material.Textures[0] && ( shaderState & 0xFFFF0000 ) == 0x00030000 )
				{
					item.takeVertexColor = material.Textures[1] == 0;
					item.index = quake3::E_Q3_MESH_GEOMETRY;
					toBuffer.push_back ( item );
				}
				else
				{
					item.takeVertexColor = 1;
					item.index = quake3::E_Q3_MESH_ITEMS;
					toBuffer.push_back ( item );
				}
				break;
*/
			case 4: // billboards
				item.takeVertexColor = 1;
				item.index = quake3::E_Q3_MESH_ITEMS;
				toBuffer.push_back ( item );
				break;
		}

		for ( u32 g = 0; g!= toBuffer.size(); ++g )
		{
			scene::SMeshBufferLightMap* buffer;

			if ( toBuffer[g].index == quake3::E_Q3_MESH_GEOMETRY )
			{
				if ( 0 == toBuffer[g].takeVertexColor )
				{
					toBuffer[g].takeVertexColor = material.getTexture(0) == 0 || material.getTexture(1);
				}
				if (Faces[i].lightmapID < -1 || Faces[i].lightmapID > NumLightMaps-1)
				{
					Faces[i].lightmapID = -1;
				}

				// there are lightmapsids and textureid with -1
				const s32 tmp_index = ((Faces[i].lightmapID+1) * (NumTextures+1)) + (Faces[i].textureID+1);
				buffer = (SMeshBufferLightMap*) Mesh[quake3::E_Q3_MESH_GEOMETRY]->getMeshBuffer(tmp_index);
				buffer->getMaterial() = material;
			}
			else
			{
				buffer = (SMeshBufferLightMap*) Mesh[ toBuffer[g].index ]->getMeshBuffer ( material );
				//buffer = 0;
				if ( 0 == buffer )
				{
					buffer = new scene::SMeshBufferLightMap();
					Mesh[ toBuffer[g].index ]->addMeshBuffer ( buffer );
					buffer->drop ();
					buffer->getMaterial() = material;
				}
			}


			switch(Faces[i].type)
			{
				case 4: // billboards
					break;
				case 2: // patches
					createCurvedSurface2(buffer, i, PatchTesselation,toBuffer[g].takeVertexColor);
					break;

				case 1: // normal polygons
				case 3: // mesh vertices

					index = MeshVerts + face->meshVertIndex;
					k = buffer->getVertexCount();

					for ( j = 0; j < face->numMeshVerts; j += 1 )
					{
						buffer->Indices.push_back( k + index [j] );
					}

					for ( j = 0; j != face->numOfVerts; ++j )
					{
						copy ( &temp[0], &Vertices[ j + face->vertexIndex ], toBuffer[g].takeVertexColor );
						buffer->Vertices.push_back( temp[0] );
					}
					break;

			} // end switch
		}
	}
}


//! constructs a mesh from the quake 3 level file.
void CQ3LevelMesh::constructMesh()
{
	// reserve buffer.
	s32 i; // new ISO for scoping problem with some compilers

	for (i=0; i<(NumTextures+1) * (NumLightMaps+1); ++i)
	{
		scene::SMeshBufferLightMap* buffer = new scene::SMeshBufferLightMap();

		buffer->Material.MaterialType = video::EMT_LIGHTMAP_M4;
		buffer->Material.Wireframe = false;
		buffer->Material.Lighting = false;

		Mesh[0]->addMeshBuffer(buffer);

		buffer->drop();
	}

	// go through all faces and add them to the buffer.

	video::S3DVertex2TCoords temp[3];

	for (i=0; i<NumFaces; ++i)
	{
		if (Faces[i].lightmapID < -1)
			Faces[i].lightmapID = -1;

		if (Faces[i].lightmapID > NumLightMaps-1)
			Faces[i].lightmapID = -1;

		// there are lightmapsids and textureid with -1
		s32 meshBufferIndex = ((Faces[i].lightmapID+1) * (NumTextures+1)) + (Faces[i].textureID+1);
		SMeshBufferLightMap* meshBuffer = ((SMeshBufferLightMap*)Mesh[0]->getMeshBuffer(meshBufferIndex));

		switch(Faces[i].type)
		{
			//case 3: // mesh vertices
			case 1: // normal polygons
				{
					for (s32 tf=0; tf<Faces[i].numMeshVerts; tf+=3)
					{
						s32 idx = meshBuffer->getVertexCount();
						s32 vidxes[3];

						vidxes[0] = MeshVerts[Faces[i].meshVertIndex + tf +0]
							+ Faces[i].vertexIndex;
						vidxes[1] = MeshVerts[Faces[i].meshVertIndex + tf +1]
							+ Faces[i].vertexIndex;
						vidxes[2] = MeshVerts[Faces[i].meshVertIndex + tf +2]
							+ Faces[i].vertexIndex;

						// add all three vertices
						copy ( &temp[0], &Vertices[ vidxes[0] ], 0 );
						copy ( &temp[1], &Vertices[ vidxes[1] ], 0 );
						copy ( &temp[2], &Vertices[ vidxes[2] ], 0 );

						meshBuffer->Vertices.push_back( temp[0] );
						meshBuffer->Vertices.push_back( temp[1] );
						meshBuffer->Vertices.push_back( temp[2] );

						// add indexes

						meshBuffer->Indices.push_back(idx);
						meshBuffer->Indices.push_back(idx+1);
						meshBuffer->Indices.push_back(idx+2);
					}
				}
				break;
			case 2: // curved surfaces
				createCurvedSurface(meshBuffer, i);
				break;

			case 4: // billboards
				break;
		} // end switch
	}
}


// helper method for creating curved surfaces, sent in by Dean P. Macri.
inline f32 CQ3LevelMesh::Blend( const f64 s[3], const f64 t[3], const tBSPVertex *v[9], int offset)
{
	f64 res = 0.0;
	f32 *ptr;

	for( int i=0; i<3; i++ )
		for( int j=0; j<3; j++ )
		{
			ptr = (f32 *)( (char*)v[i*3+j] + offset );
			res += s[i] * t[j] *  (*ptr);
		}

	return (f32) res;
}


void CQ3LevelMesh::S3DVertex2TCoords_64::copyto ( video::S3DVertex2TCoords &dest ) const
{
	dest.Pos.X = core::round_( (f32) Pos.X );
	dest.Pos.Y = core::round_( (f32) Pos.Y );
	dest.Pos.Z = core::round_( (f32) Pos.Z );
	//dest.Pos.X = (f32) Pos.X;
	//dest.Pos.Y = (f32) Pos.Y;
	//dest.Pos.Z = (f32) Pos.Z;

	dest.Normal.X = (f32) Normal.X;
	dest.Normal.Y = (f32) Normal.Y;
	dest.Normal.Z = (f32) Normal.Z;
	dest.Normal.normalize();

	dest.Color = Color.toSColor();

	dest.TCoords.X = (f32) TCoords.X;
	dest.TCoords.Y = (f32) TCoords.Y;

	dest.TCoords2.X = (f32) TCoords2.X;
	dest.TCoords2.Y = (f32) TCoords2.Y;
}


void CQ3LevelMesh::copy ( S3DVertex2TCoords_64 * dest, const tBSPVertex * source, s32 vertexcolor ) const
{
	//dest->Pos.X = core::round ( source->vPosition[0] );
	//dest->Pos.Y = core::round ( source->vPosition[2] );
	//dest->Pos.Z = core::round ( source->vPosition[1] );
	dest->Pos.X = source->vPosition[0];
	dest->Pos.Y = source->vPosition[2];
	dest->Pos.Z = source->vPosition[1];

	dest->Normal.X = source->vNormal[0];
	dest->Normal.Y = source->vNormal[2];
	dest->Normal.Z = source->vNormal[1];
	dest->Normal.normalize ();

	dest->TCoords.X = source->vTextureCoord[0];
	dest->TCoords.Y = source->vTextureCoord[1];
	dest->TCoords2.X = source->vLightmapCoord[0];
	dest->TCoords2.Y = source->vLightmapCoord[1];

	if ( vertexcolor )
	{
		u32 a = core::s32_min ( source->color[3] * quake3::defaultModulate, 255 );
		u32 r = core::s32_min ( source->color[0] * quake3::defaultModulate, 255 );
		u32 g = core::s32_min ( source->color[1] * quake3::defaultModulate, 255 );
		u32 b = core::s32_min ( source->color[2] * quake3::defaultModulate, 255 );

		dest->Color.set (a * 1.f/255.f, r * 1.f/255.f,
				g * 1.f/255.f, b * 1.f/255.f);
	}
	else
	{
		dest->Color.set ( 1.f, 1.f, 1.f, 1.f );
	}
}


inline void CQ3LevelMesh::copy ( video::S3DVertex2TCoords * dest, const tBSPVertex * source, s32 vertexcolor ) const
{
	dest->Pos.X = core::round_( source->vPosition[0] );
	dest->Pos.Y = core::round_( source->vPosition[2] );
	dest->Pos.Z = core::round_( source->vPosition[1] );

	//dest->Pos.X = source->vPosition[0];
	//dest->Pos.Y = source->vPosition[2];
	//dest->Pos.Z = source->vPosition[1];

	dest->Normal.X = source->vNormal[0];
	dest->Normal.Y = source->vNormal[2];
	dest->Normal.Z = source->vNormal[1];
	dest->Normal.normalize();

	dest->TCoords.X = source->vTextureCoord[0];
	dest->TCoords.Y = source->vTextureCoord[1];
	dest->TCoords2.X = source->vLightmapCoord[0];
	dest->TCoords2.Y = source->vLightmapCoord[1];

	if ( vertexcolor )
	{
		u32 a = core::s32_min ( source->color[3] * quake3::defaultModulate, 255 );
		u32 r = core::s32_min ( source->color[0] * quake3::defaultModulate, 255 );
		u32 g = core::s32_min ( source->color[1] * quake3::defaultModulate, 255 );
		u32 b = core::s32_min ( source->color[2] * quake3::defaultModulate, 255 );

		dest->Color.color = a << 24 | r << 16 | g << 8 | b;
	}
	else
	{
		dest->Color.color = 0xFFFFFFFF;
	}
}


void CQ3LevelMesh::SBezier::tesselate ( s32 level )
{
	//Calculate how many vertices across/down there are
	s32 j, k;

	u32 idx = Patch->Vertices.size();

	column[0].set_used ( level + 1 );
	column[1].set_used ( level + 1 );
	column[2].set_used ( level + 1 );

	const f64 w = 0.0 + core::reciprocal ( (f32) level );

	//Tesselate along the columns
	for( j = 0; j <= level; ++j)
	{
		const f64 f = w * (f64) j;

		column[0][j] = control[0].getInterpolated_quadratic(control[3], control[6], f );
		column[1][j] = control[1].getInterpolated_quadratic(control[4], control[7], f );
		column[2][j] = control[2].getInterpolated_quadratic(control[5], control[8], f );
	}

	//Tesselate across the rows to get final vertices
	video::S3DVertex2TCoords v;
	S3DVertex2TCoords_64 f;
	for( j = 0; j <= level; ++j)
	{
		for( k = 0; k <= level; ++k)
		{
			f = column[0][j].getInterpolated_quadratic(	column[1][j], column[2][j],	w * (f64) k );
			f.copyto ( v );
			Patch->Vertices.push_back ( v );
		}
	}

	// connect
	for( j = 0; j < level; ++j)
	{
		for( k = 0; k < level; ++k)
		{
			const s32 inx = idx + ( k * ( level + 1 ) ) + j;

			Patch->Indices.push_back ( inx + 0 );
			Patch->Indices.push_back ( inx + (level + 1 ) + 0 );
			Patch->Indices.push_back ( inx + (level + 1 ) + 1 );

			Patch->Indices.push_back ( inx + 0 );
			Patch->Indices.push_back ( inx + (level + 1 ) + 1 );
			Patch->Indices.push_back ( inx + 1 );
		}
	}
}


/*!
	no subdivision
*/
void CQ3LevelMesh::createCurvedSurface3(SMeshBufferLightMap* meshBuffer,
					s32 faceIndex,
					s32 patchTesselation,
					s32 storevertexcolor)
{
	tBSPFace * face = &Faces[faceIndex];
	u32 j,k,m;

	// number of control points across & up
	const u32 controlWidth = face->size[0];
	const u32 controlHeight = face->size[1];

	video::S3DVertex2TCoords v;

	m = meshBuffer->Vertices.size ();
	for ( j = 0; j!= controlHeight * controlWidth; ++j )
	{
		copy ( &v, &Vertices [ face->vertexIndex + j ], storevertexcolor );
		meshBuffer->Vertices.push_back ( v );
	}

	for ( j = 0; j!= controlHeight - 1; ++j )
	{
		for ( k = 0; k!= controlWidth - 1; ++k )
		{
			meshBuffer->Indices.push_back ( m + k + 0 );
			meshBuffer->Indices.push_back ( m + k + controlWidth + 0 );
			meshBuffer->Indices.push_back ( m + k + controlWidth + 1 );

			meshBuffer->Indices.push_back ( m + k + 0 );
			meshBuffer->Indices.push_back ( m + k + controlWidth + 1 );
			meshBuffer->Indices.push_back ( m + k + 1 );
		}
		m += controlWidth;
	}
}


/*!
*/
void CQ3LevelMesh::createCurvedSurface2(SMeshBufferLightMap* meshBuffer,
					s32 faceIndex,
					s32 patchTesselation,
					s32 storevertexcolor)
{
	tBSPFace * face = &Faces[faceIndex];
	u32 j,k;

	// number of control points across & up
	const u32 controlWidth = face->size[0];
	const u32 controlHeight = face->size[1];

	// number of biquadratic patches
	const u32 biquadWidth =  (controlWidth - 1)/2;
	const u32 biquadHeight = (controlHeight -1)/2;

	// Create space for a temporary array of the patch's control points
	core::array<S3DVertex2TCoords_64> controlPoint;
	controlPoint.set_used ( controlWidth * controlHeight );

	for( j = 0; j < controlPoint.size(); ++j)
	{
		copy ( &controlPoint[j], &Vertices [ face->vertexIndex + j ], storevertexcolor );
	}

	// create a temporary patch
	Bezier.Patch = new scene::SMeshBufferLightMap();

	//Loop through the biquadratic patches
	for( j = 0; j < biquadHeight; ++j)
	{
		for( k = 0; k < biquadWidth; ++k)
		{
			// set up this patch
			const s32 inx = j*controlWidth*2 + k*2;

			// setup bezier control points for this patch
			Bezier.control[0] = controlPoint[ inx + 0];
			Bezier.control[1] = controlPoint[ inx + 1];
			Bezier.control[2] = controlPoint[ inx + 2];
			Bezier.control[3] = controlPoint[ inx + controlWidth + 0 ];
			Bezier.control[4] = controlPoint[ inx + controlWidth + 1 ];
			Bezier.control[5] = controlPoint[ inx + controlWidth + 2 ];
			Bezier.control[6] = controlPoint[ inx + controlWidth * 2 + 0];
			Bezier.control[7] = controlPoint[ inx + controlWidth * 2 + 1];
			Bezier.control[8] = controlPoint[ inx + controlWidth * 2 + 2];

			Bezier.tesselate ( patchTesselation );
		}
	}

	// stitch together with existing geometry
	// TODO: only border needs to be checked
	const u32 bsize = Bezier.Patch->getVertexCount();
	const u32 msize = meshBuffer->getVertexCount();
/*
	for ( j = 0; j!= bsize; ++j )
	{
		const core::vector3df &v = Bezier.Patch->Vertices[j].Pos;

		for ( k = 0; k!= msize; ++k )
		{
			const core::vector3df &m = meshBuffer->Vertices[k].Pos;

			if ( !v.equals ( m, tolerance ) )
				continue;

			meshBuffer->Vertices[k].Pos = v;
			//Bezier.Patch->Vertices[j].Pos = m;
		}
	}
*/

	// add Patch to meshbuffer
	for ( j = 0; j!= bsize; ++j )
	{
		meshBuffer->Vertices.push_back ( Bezier.Patch->Vertices[j] );
	}

	// add indices to meshbuffer
	for ( j = 0; j!= Bezier.Patch->getIndexCount(); ++j )
	{
		meshBuffer->Indices.push_back ( msize + Bezier.Patch->Indices[j] );
	}

	delete Bezier.Patch;
}


void CQ3LevelMesh::createCurvedSurface(SMeshBufferLightMap* meshBuffer, s32 i)
{
	// this implementation for loading curved surfaces was
	// sent in by Dean P. Macri. It was a little bit modified
	// by me afterwards.
	s32 idx;
	s32 cpidx[9];

	const tBSPVertex *v[9];
	video::S3DVertex2TCoords currentVertex[4];

	for( s32 row=0; row<Faces[i].size[1]-2; row+=2 )
	{
		for( s32 col=0; col<Faces[i].size[0]-2; col+=2 )
		{
			cpidx[0] = (row * Faces[i].size[0] + col) + Faces[i].vertexIndex;
			cpidx[1] = (row * Faces[i].size[0] + col + 1) + Faces[i].vertexIndex;
			cpidx[2] = (row * Faces[i].size[0] + col + 2) + Faces[i].vertexIndex;
			cpidx[3] = ((row+1) * Faces[i].size[0] + col) + Faces[i].vertexIndex;
			cpidx[4] = ((row+1) * Faces[i].size[0] + col + 1) + Faces[i].vertexIndex;
			cpidx[5] = ((row+1) * Faces[i].size[0] + col + 2) + Faces[i].vertexIndex;
			cpidx[6] = ((row+2) * Faces[i].size[0] + col) + Faces[i].vertexIndex;
			cpidx[7] = ((row+2) * Faces[i].size[0] + col + 1) + Faces[i].vertexIndex;
			cpidx[8] = ((row+2) * Faces[i].size[0] + col + 2) + Faces[i].vertexIndex;

			// For some reason if we tesselate more than this (3x3),
			// some of the patches don't show up!

			f64 s;
			f64 t;
			f64 cs[3], ct[3], nxs[3], nxt[3];

			s32 srun = 3;
			s32 trun = 3;
			const f64 sstep = 1.0 / (f64) srun;
			const f64 tstep = 1.0 / (f64) trun;

			v[0] = &Vertices[cpidx[0]];
			v[1] = 	&Vertices[cpidx[1]];
			v[2] = 	&Vertices[cpidx[2]];
			v[3] = 	&Vertices[cpidx[3]];
			v[4] = 	&Vertices[cpidx[4]];
			v[5] = 	&Vertices[cpidx[5]];
			v[6] = 	&Vertices[cpidx[6]];
			v[7] = 	&Vertices[cpidx[7]];
			v[8] = 	&Vertices[cpidx[8]];


			s32 dos;
			s32 dot;
			for( dos = 0, s = 0; dos != srun; dos +=1,  s+= sstep )
			{
				cs[0] = (1.0-s)*(1.0-s);
				cs[1] = 2.0 * (1.0 - s) * s;
				cs[2] = s * s;
				nxs[0] = (1.0-s-sstep)*(1.0-s-sstep);
				nxs[1] = 2.0 * (1.0 - s -sstep) * (s+sstep);
				nxs[2] = (s+sstep) * (s+sstep);

				for( dot = 0, t = 0; dot != trun; dot += 1, t += tstep )
				{
					idx = meshBuffer->getVertexCount();
					ct[0] = (1.0-t)*(1.0-t);
					ct[1] = 2.0 * (1.0 - t) * t;
					ct[2] = t * t;
					nxt[0] = (1.0-t-tstep)*(1.0-t-tstep);
					nxt[1] = 2.0 * (1.0 - t - tstep) * (t+tstep);
					nxt[2] = (t+tstep) * (t+tstep);

					// Vert 1
					currentVertex[0].Color.set(255,255,255,255);
					currentVertex[0].Pos.X = floorf( Blend( cs, ct, v, (char*)&v[0]->vPosition[0] - (char*)v[0])+ 0.5f);
					currentVertex[0].Pos.Y = floorf( Blend( cs, ct, v, (char*)&v[0]->vPosition[2] - (char*)v[0])+ 0.5f);
					currentVertex[0].Pos.Z = floorf( Blend( cs, ct, v, (char*)&v[0]->vPosition[1] - (char*)v[0])+ 0.5f);
					currentVertex[0].Normal.X = Blend( cs, ct, v, (char*)&v[0]->vNormal[0] - (char*)v[0]);
					currentVertex[0].Normal.Y = Blend( cs, ct, v, (char*)&v[0]->vNormal[2] - (char*)v[0]);
					currentVertex[0].Normal.Z = Blend( cs, ct, v, (char*)&v[0]->vNormal[1] - (char*)v[0]);
					currentVertex[0].TCoords.X = Blend( cs, ct, v, (char*)&v[0]->vTextureCoord[0] - (char*)v[0]);
					currentVertex[0].TCoords.Y = Blend( cs, ct, v, (char*)&v[0]->vTextureCoord[1] - (char*)v[0]);
					currentVertex[0].TCoords2.X = Blend( cs, ct, v, (char*)&v[0]->vLightmapCoord[0] - (char*)v[0]);
					currentVertex[0].TCoords2.Y = Blend( cs, ct, v, (char*)&v[0]->vLightmapCoord[1] - (char*)v[0]);
					// Vert 2
					currentVertex[1].Color.set(255,255,255,255);
					currentVertex[1].Pos.X = floorf( Blend( cs, nxt, v, (char*)&v[0]->vPosition[0] - (char*)v[0])+ 0.5f);
					currentVertex[1].Pos.Y = floorf( Blend( cs, nxt, v, (char*)&v[0]->vPosition[2] - (char*)v[0])+ 0.5f);
					currentVertex[1].Pos.Z = floorf( Blend( cs, nxt, v, (char*)&v[0]->vPosition[1] - (char*)v[0])+ 0.5f);
					currentVertex[1].Normal.X = Blend( cs, nxt, v, (char*)&v[0]->vNormal[0] - (char*)v[0]);
					currentVertex[1].Normal.Y = Blend( cs, nxt, v, (char*)&v[0]->vNormal[2] - (char*)v[0]);
					currentVertex[1].Normal.Z = Blend( cs, nxt, v, (char*)&v[0]->vNormal[1] - (char*)v[0]);
					currentVertex[1].TCoords.X = Blend( cs, nxt, v, (char*)&v[0]->vTextureCoord[0] - (char*)v[0]);
					currentVertex[1].TCoords.Y = Blend( cs, nxt, v, (char*)&v[0]->vTextureCoord[1] - (char*)v[0]);
					currentVertex[1].TCoords2.X = Blend( cs, nxt, v, (char*)&v[0]->vLightmapCoord[0] - (char*)v[0]);
					currentVertex[1].TCoords2.Y = Blend( cs, nxt, v, (char*)&v[0]->vLightmapCoord[1] - (char*)v[0]);
					// Vert 3
					currentVertex[2].Color.set(255,255,255,255);
					currentVertex[2].Pos.X = floorf( Blend( nxs, ct, v, (char*)&v[0]->vPosition[0] - (char*)v[0])+ 0.5f);
					currentVertex[2].Pos.Y = floorf( Blend( nxs, ct, v, (char*)&v[0]->vPosition[2] - (char*)v[0])+ 0.5f);
					currentVertex[2].Pos.Z = floorf( Blend( nxs, ct, v, (char*)&v[0]->vPosition[1] - (char*)v[0])+ 0.5f);
					currentVertex[2].Normal.X = Blend( nxs, ct, v, (char*)&v[0]->vNormal[0] - (char*)v[0]);
					currentVertex[2].Normal.Y = Blend( nxs, ct, v, (char*)&v[0]->vNormal[2] - (char*)v[0]);
					currentVertex[2].Normal.Z = Blend( nxs, ct, v, (char*)&v[0]->vNormal[1] - (char*)v[0]);
					currentVertex[2].TCoords.X = Blend( nxs, ct, v, (char*)&v[0]->vTextureCoord[0] - (char*)v[0]);
					currentVertex[2].TCoords.Y = Blend( nxs, ct, v, (char*)&v[0]->vTextureCoord[1] - (char*)v[0]);
					currentVertex[2].TCoords2.X = Blend( nxs, ct, v, (char*)&v[0]->vLightmapCoord[0] - (char*)v[0]);
					currentVertex[2].TCoords2.Y = Blend( nxs, ct, v, (char*)&v[0]->vLightmapCoord[1] - (char*)v[0]);
					// Vert 4
					currentVertex[3].Color.set(255,255,255,255);
					currentVertex[3].Pos.X = floorf(Blend( nxs, nxt, v, (char*)&v[0]->vPosition[0] - (char*)v[0])+ 0.5f);
					currentVertex[3].Pos.Y = floorf(Blend( nxs, nxt, v, (char*)&v[0]->vPosition[2] - (char*)v[0])+ 0.5f);
					currentVertex[3].Pos.Z = floorf(Blend( nxs, nxt, v, (char*)&v[0]->vPosition[1] - (char*)v[0])+ 0.5f);
					currentVertex[3].Normal.X = Blend( nxs, nxt, v, (char*)&v[0]->vNormal[0] - (char*)v[0]);
					currentVertex[3].Normal.Y = Blend( nxs, nxt, v, (char*)&v[0]->vNormal[2] - (char*)v[0]);
					currentVertex[3].Normal.Z = Blend( nxs, nxt, v, (char*)&v[0]->vNormal[1] - (char*)v[0]);
					currentVertex[3].TCoords.X = Blend( nxs, nxt, v, (char*)&v[0]->vTextureCoord[0] - (char*)v[0]);
					currentVertex[3].TCoords.Y = Blend( nxs, nxt, v, (char*)&v[0]->vTextureCoord[1] - (char*)v[0]);
					currentVertex[3].TCoords2.X = Blend( nxs, nxt, v, (char*)&v[0]->vLightmapCoord[0] - (char*)v[0]);
					currentVertex[3].TCoords2.Y = Blend( nxs, nxt, v, (char*)&v[0]->vLightmapCoord[1] - (char*)v[0]);
					// Put the vertices in the mesh buffer
					meshBuffer->Vertices.push_back(currentVertex[0]);
					meshBuffer->Vertices.push_back(currentVertex[2]);
					meshBuffer->Vertices.push_back(currentVertex[1]);

					meshBuffer->Vertices.push_back(currentVertex[1]);
					meshBuffer->Vertices.push_back(currentVertex[2]);
					meshBuffer->Vertices.push_back(currentVertex[3]);

					// add indexes
					meshBuffer->Indices.push_back(idx);
					meshBuffer->Indices.push_back(idx+1);
					meshBuffer->Indices.push_back(idx+2);
					// add indexes
					meshBuffer->Indices.push_back(idx+3);
					meshBuffer->Indices.push_back(idx+4);
					meshBuffer->Indices.push_back(idx+5);
				}
			}
		}
	}
}


//! get's an interface to the entities
const quake3::tQ3EntityList & CQ3LevelMesh::getEntityList ()
{
	Entity.sort();
	return Entity;
}


/*!
*/
const quake3::SShader * CQ3LevelMesh::getShader ( u32 index  ) const
{
	index &= 0xFFFF;

	if ( index < Shader.size () )
	{
		return &Shader[index];
	}

	return 0;
}


//! loads the shader definition
//  either from file ( we assume /scripts on fileNameIsValid == 0 )
const quake3::SShader * CQ3LevelMesh::getShader ( const c8 * filename, s32 fileNameIsValid )
{
	quake3::SShader search;
	search.name = filename;

	s32 index;

	//! is Shader already in cache?
	index = Shader.linear_search ( search );
	if ( index >= 0 )
	{
		return &Shader[index];
	}

	core::stringc loadFile;

	if ( 0 == fileNameIsValid )
	{
		// extract the shader name from the last path component in filename
		// "scripts/[name].shader"
		core::stringc cut ( filename );

		s32 end = cut.findLast ( '/' );
		s32 start = cut.findLast ( '/', end - 1 );

		loadFile = "scripts";
		loadFile.append ( cut.subString ( start, end - start ) );
		loadFile.append ( ".shader" );
	}
	else
	{
		loadFile = filename;
	}

	// already loaded the file ?
	index = ShaderFile.binary_search ( loadFile );
	if ( index >= 0 )
		return 0;

	if ( !FileSystem->existFile ( loadFile.c_str () ) )
		return 0;

	io::IReadFile *file = FileSystem->createAndOpenFile ( loadFile.c_str () );
	if ( 0 == file )
		return 0;

	core::stringc message;
	message = loadFile + " for " + core::stringc ( filename );
	os::Printer::log("Loaded shader", message.c_str(), ELL_INFORMATION);

	// add file to loaded files
	ShaderFile.push_back ( loadFile );

	// load script
	core::array<u8> script;
	const long len = file->getSize ();

	script.set_used ( len + 2 );
	script[ len + 1 ] = 0;

	file->seek( 0 );
	file->read ( script.pointer(), len );
	file->drop ();

	// start a parser instance
	parser_parse ( script.pointer(), len, &CQ3LevelMesh::scriptcallback_shader );

	// search again
	index = Shader.linear_search ( search );
	if ( index >= 0 )
		return &Shader[index];

	return 0;
}


//! adding default shaders
void CQ3LevelMesh::InitShader ()
{
	ReleaseShader ();

	quake3::SShader element;

	quake3::SVarGroup group;
	quake3::SVariable variable;

	variable.name = "noshader";
	group.Variable.push_back ( variable );

	element.VarGroup = new quake3::SVarGroupList ();
	element.VarGroup->VariableGroup.push_back ( group );
	element.name = element.VarGroup->VariableGroup[0].Variable[0].name.c_str ();
	Shader.push_back ( element );

	// load common named shader
	getShader ( "scripts/common.shader", 1 );
}


//!. script callback for shaders
//! i'm having troubles with the reference counting, during callback.. resorting..
void CQ3LevelMesh::ReleaseShader ()
{
	for ( u32 i = 0; i!= Shader.size(); ++i )
	{
		Shader[i].VarGroup->drop ();
	}
	Shader.clear ();
	ShaderFile.clear();
}

void CQ3LevelMesh::ReleaseEntity ()
{
	for ( u32 i = 0; i!= Entity.size(); ++i )
	{
		Entity[i].VarGroup->drop ();
	}
	Entity.clear ();

}


// entity only has only one valid level.. and no assoziative name..
void CQ3LevelMesh::scriptcallback_entity ( quake3::SVarGroupList *& grouplist )
{
	quake3::SEntity element;

	if ( grouplist->VariableGroup.size () != 2 )
		return;


	element.name = grouplist->VariableGroup[1].get ( "classname" );

	grouplist->grab ();
	element.VarGroup = grouplist;
	element.id = Shader.size();

	Entity.push_back ( element );
}


//!. script callback for shaders
void CQ3LevelMesh::scriptcallback_shader ( quake3::SVarGroupList *& grouplist )
{
	quake3::SShader element;

	// TODO: There might be something wrong with this fix, but it avoids a core dump...
	if (grouplist->VariableGroup[0].Variable.size()==0)
		return;
	// end fix

	grouplist->grab ();

	element.VarGroup = grouplist;
	element.name = element.VarGroup->VariableGroup[0].Variable[0].name.c_str ();
	element.id = Shader.size();

	Shader.push_back ( element );
}


//! loads the textures
void CQ3LevelMesh::loadTextures()
{
	if (!Driver)
		return;

	core::stringc s;
	core::stringc extensions[2];
	extensions[0] = ".jpg";
	extensions[1] = ".tga";

	// load textures

	core::array<video::ITexture*> tex;
	tex.set_used(NumTextures+1);

	tex[0] = 0;

	s32 t;// new ISO for scoping problem with some compilers

	for (t=1; t<(NumTextures+1); ++t)
	{
		tex[t] = 0;

		if ( !tex[t] )
		{
			for (s32 e=0; e<2; ++e)
			{
				s = Textures[t-1].strName;
				s.append(extensions[e]);
				if (FileSystem->existFile(s.c_str()))
				{
					tex[t] = Driver->getTexture(s.c_str());
					break;
				}
			}
		}
		if (!tex[t])
		{
			os::Printer::log("Q3: no texmap for texturename ", Textures[t-1].strName, ELL_WARNING);
		}
	}

	// load lightmaps.
	core::array<video::ITexture*> lig;
	lig.set_used(NumLightMaps+1);

	lig[0] = 0;
	c8 lightmapname[255];
	core::dimension2d<s32> lmapsize(128,128);

	//bool oldMipMapState = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
	//Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

	video::IImage* lmapImg;
	for (t=1; t<(NumLightMaps+1); ++t)
	{
		sprintf(lightmapname, "%s.lightmap.%d", LevelName.c_str(), t);

		// lightmap is a CTexture::R8G8B8 format
		lmapImg = Driver->createImageFromData(
			video::ECF_R8G8B8,
			lmapsize,
			LightMaps[t-1].imageBits, true, false );

		lig[t] = Driver->addTexture ( lightmapname, lmapImg );
		lmapImg->drop ();

	}
	//Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, oldMipMapState);


	// attach textures to materials.
	for (s32 l=0; l<NumLightMaps+1; ++l)
	{
		for (t=0; t<NumTextures+1; ++t)
		{
			SMeshBufferLightMap* b = (SMeshBufferLightMap*)Mesh[0]->getMeshBuffer(l*(NumTextures+1) + t);
			b->Material.setTexture(1, lig[l]);
			b->Material.setTexture(0, tex[t]);

			if (!b->Material.getTexture(1))
				b->Material.MaterialType = video::EMT_SOLID;

			if (!b->Material.getTexture(0))
 				b->Material.MaterialType = video::EMT_SOLID;
		}
	}
}

// delete all buffers without geometry in it.
void CQ3LevelMesh::cleanMeshes ()
{
	// delete all buffers without geometry in it.
	for ( u32 g = 0; g < quake3::E_Q3_MESH_SIZE; ++g )
	{
		u32 i = 0;
		bool texture0important = ( g == 0 );
		while(i < Mesh[g]->MeshBuffers.size())
		{
			if (Mesh[g]->MeshBuffers[i]->getVertexCount() == 0 ||
				Mesh[g]->MeshBuffers[i]->getIndexCount() == 0 ||
				( texture0important && Mesh[g]->MeshBuffers[i]->getMaterial().getTexture(0) == 0 )
				)
			{
				// delete Meshbuffer
				Mesh[g]->MeshBuffers[i]->drop();
				Mesh[g]->MeshBuffers.erase(i);
			}
			else
				++i;
		}
	}

}

// recalculate bounding boxes
void CQ3LevelMesh::calcBoundingBoxes ()
{
	// create bounding box
	for ( u32 g = 0; g != quake3::E_Q3_MESH_SIZE; ++g )
	{
		for ( u32 j=0; j < Mesh[g]->MeshBuffers.size(); ++j)
		{
			((SMeshBufferLightMap*)Mesh[g]->MeshBuffers[j])->recalculateBoundingBox();
		}

		Mesh[g]->recalculateBoundingBox();
	}

}

/*
//! loads a texture
video::ITexture* CQ3LevelMesh::loadTexture ( const tStringList &stringList )
{
	static const char * extension[2] =
	{
		".jpg",
		".tga"
	};

	core::stringc loadFile;
	for ( u32 i = 0; i!= stringList.size (); ++i )
	{
		for ( u32 g = 0; g != 2 ; ++g )
		{
			cutFilenameExtension ( loadFile, stringList[i] ).append ( extension[g] );

			if ( FileSystem->existFile ( loadFile.c_str() ) )
			{
				video::ITexture* t = Driver->getTexture( loadFile.c_str() );
				if ( t )
					return t;
			}
		}
	}
	return 0;
}
*/

//! loads the textures
void CQ3LevelMesh::loadTextures2()
{
	if (!Driver)
		return;

	s32 t;

	// load lightmaps.
	Lightmap.set_used(NumLightMaps+1);

	c8 lightmapname[255];
	core::dimension2d<s32> lmapsize(128,128);

	video::IImage* lmapImg;
	for ( t = 0; t < NumLightMaps ; ++t)
	{
		sprintf(lightmapname, "%s.lightmap.%d", LevelName.c_str(), t);

		// lightmap is a CTexture::R8G8B8 format
		lmapImg = Driver->createImageFromData(
			video::ECF_R8G8B8,
			lmapsize,
			LightMaps[t].imageBits, true, false );

		Lightmap[t] = Driver->addTexture ( lightmapname, lmapImg );
		lmapImg->drop ();

	}

	// load textures
	Tex.set_used( NumTextures+1 );

	const quake3::SShader * shader;

	core::stringc list;
	core::stringc check;
	quake3::tTexArray textureArray;

	for ( t=0; t< NumTextures; ++t)
	{
		Tex[t].ShaderID = -1;
		Tex[t].Texture = 0;

		list = "";

		// get a shader ( if one exists )
		shader = getShader ( Textures[t].strName, 0 );
		if ( shader )
		{
			Tex[t].ShaderID = shader->id;

			// if texture name == stage1 Texture map
			const quake3::SVarGroup * group;

			group = shader->getGroup ( 2 );
			if ( group )
			{
				if ( core::cutFilenameExtension ( check, group->get ( "map" ) ) == Textures[t].strName )
				{
					list += check;
				}
				else
				if ( check == "$lightmap" )
				{
					// we check if lightmap is in stage 1 and texture in stage 2
					group = shader->getGroup ( 3 );
					if ( group )
						list += group->get ( "map" );
				}
			}
		}
		else
		{
			// no shader, take it
			list += Textures[t].strName;
		}

		u32 pos = 0;
		quake3::getTextures ( textureArray, list, pos, FileSystem, Driver );

		Tex[t].Texture = textureArray[0];
	}
}


//! Returns an axis aligned bounding box of the mesh.
//! \return A bounding box of this mesh is returned.
const core::aabbox3d<f32>& CQ3LevelMesh::getBoundingBox() const
{
	return Mesh[0]->getBoundingBox();
}

void CQ3LevelMesh::setBoundingBox( const core::aabbox3df& box)
{
	Mesh[0]->setBoundingBox(box); //?
}


//! Returns the type of the animated mesh.
E_ANIMATED_MESH_TYPE CQ3LevelMesh::getMeshType() const
{
	return scene::EAMT_BSP;
}

} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BSP_LOADER_
