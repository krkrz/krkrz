// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_X_LOADER_

#include "CXMeshFileLoader.h"
#include "os.h"

#include "fast_atof.h"
#include "coreutil.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "IReadFile.h"

#ifdef _DEBUG
#define _XREADER_DEBUG
#endif
//#define BETTER_MESHBUFFER_SPLITTING_FOR_X

namespace irr
{
namespace scene
{

//! Constructor
CXMeshFileLoader::CXMeshFileLoader(scene::ISceneManager* smgr)
: SceneManager(smgr), AnimatedMesh(0), MajorVersion(0), MinorVersion(0),
	BinaryFormat(false), BinaryNumCount(0), Buffer(0), P(0), End(0),
	FloatSize(0), CurFrame(0)
{
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CXMeshFileLoader::isALoadableFileExtension(const c8* filename) const
{
	return strstr(filename, ".x") != 0;
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CXMeshFileLoader::createMesh(io::IReadFile* f)
{
	if (!f)
		return 0;

	u32 time = os::Timer::getRealTime();

	AnimatedMesh = new CSkinnedMesh();

	if ( load(f) )
	{
		AnimatedMesh->finalize();
	}
	else
	{
		AnimatedMesh->drop();
		AnimatedMesh = 0;
	}
#ifdef _XREADER_DEBUG
	time = os::Timer::getRealTime() - time;
	core::stringc tmpString = "Time to load ";
	tmpString += BinaryFormat ? "binary" : "ascii";
	tmpString += " X file: ";
	tmpString += time;
	tmpString += "ms";
	os::Printer::log(tmpString.c_str());
#endif
	//Clear up

	MajorVersion=0;
	MinorVersion=0;
	BinaryFormat=0;
	BinaryNumCount=0;
	FloatSize=0;
	P=0;
	End=0;
	CurFrame=0;
	TemplateMaterials.clear();

	delete [] Buffer;
	Buffer = 0;

	for (u32 i=0; i<Meshes.size(); ++i)
		delete Meshes[i];
	Meshes.clear();

	return AnimatedMesh;
}


bool CXMeshFileLoader::load(io::IReadFile* file)
{
	if (!readFileIntoMemory(file))
		return false;

	if (!parseFile())
		return false;

	for (u32 n=0; n<Meshes.size(); ++n)
	{
		SXMesh *mesh=Meshes[n];

		// default material if nothing loaded
		if (!mesh->Materials.size())
			mesh->Materials.push_back(video::SMaterial());

		u32 i;

		mesh->Buffers.reallocate(mesh->Materials.size());
		const u32 bufferOffset = AnimatedMesh->getMeshBufferCount();
		for (i=0; i<mesh->Materials.size(); ++i)
		{
			mesh->Buffers.push_back( AnimatedMesh->createBuffer() );
			mesh->Buffers.getLast()->Material = mesh->Materials[i];

			if (!mesh->HasSkinning)
			{
				//Set up rigid animation
				if (mesh->AttachedJointID!=-1)
				{
					AnimatedMesh->getAllJoints()[mesh->AttachedJointID]->AttachedMeshes.push_back( AnimatedMesh->getMeshBuffers().size()-1 );
				}
			}

		}

		if (!mesh->HasVertexColors)
		{
			for (u32 j=0;j<mesh->FaceMaterialIndices.size();++j)
			{
				for (u32 id=j*3+0;id<=j*3+2;++id)
				{
					mesh->Vertices[ mesh->Indices[id] ].Color = mesh->Buffers[mesh->FaceMaterialIndices[j]]->Material.DiffuseColor;
				}
			}
		}

		#ifdef BETTER_MESHBUFFER_SPLITTING_FOR_X
		{
			//the same vertex can be used in many different meshbuffers, but it's slow to work out

			core::array< core::array< u32 > > verticesLinkIndex;
			verticesLinkIndex.reallocate(mesh->Vertices.size());
			core::array< core::array< u16 > > verticesLinkBuffer;
			verticesLinkBuffer.reallocate(mesh->Vertices.size());

			for (i=0;i<mesh->Vertices.size();++i)
			{
				verticesLinkIndex.push_back( core::array< u32 >() );
				verticesLinkBuffer.push_back( core::array< u16 >() );
			}

			for (i=0;i<mesh->FaceMaterialIndices.size();++i)
			{
				for (u32 id=i*3+0;id<=i*3+2;++id)
				{
					core::array< u16 > &Array=verticesLinkBuffer[ mesh->Indices[id] ];
					bool found=false;

					for (u32 j=0; j < Array.size(); ++j)
					{
						if (Array[j]==mesh->FaceMaterialIndices[i])
						{
							found=true;
							break;
						}
					}

					if (!found)
						Array.push_back( mesh->FaceMaterialIndices[i] );
				}
			}

			for (i=0;i<verticesLinkBuffer.size();++i)
			{
				if (!verticesLinkBuffer[i].size())
					verticesLinkBuffer[i].push_back(0);
			}

			for (i=0;i<mesh->Vertices.size();++i)
			{
				core::array< u16 > &Array = verticesLinkBuffer[i];
				verticesLinkIndex[i].reallocate(Array.size());
				for (u32 j=0; j < Array.size(); ++j)
				{
					scene::SSkinMeshBuffer *buffer = mesh->Buffers[ Array[j] ];
					verticesLinkIndex[i].push_back( buffer->Vertices_Standard.size() );
					buffer->Vertices_Standard.push_back( mesh->Vertices[i] );
				}
			}

			for (i=0;i<mesh->FaceMaterialIndices.size();++i)
			{
				scene::SSkinMeshBuffer *buffer=mesh->Buffers[ mesh->FaceMaterialIndices[i] ];

				for (u32 id=i*3+0;id<=i*3+2;++id)
				{
					core::array< u16 > &Array=verticesLinkBuffer[ mesh->Indices[id] ];

					for (u32 j=0;j<  Array.size() ;++j)
					{
						if ( Array[j]== mesh->FaceMaterialIndices[i] )
							buffer->Indices.push_back( verticesLinkIndex[ mesh->Indices[id] ][j] );
					}
				}
			}

			for (u32 j=0;j<mesh->WeightJoint.size();++j)
			{
				ISkinnedMesh::SWeight& weight = (AnimatedMesh->getAllJoints()[mesh->WeightJoint[j]]->Weights[mesh->WeightNum[j]]);

				u32 id = weight.vertex_id;

				if (id>=verticesLinkIndex.size())
				{
					os::Printer::log("X loader: Weight id out of range", ELL_WARNING);
					id=0;
					weight.strength=0.f;
				}

				if (verticesLinkBuffer[id].size()==1)
				{
					weight.vertex_id=verticesLinkIndex[id][0];
					weight.buffer_id=verticesLinkBuffer[id][0];
				}
				else if (verticesLinkBuffer[id].size() != 0)
				{
					for (u32 k=1; k < verticesLinkBuffer[id].size(); ++k)
					{
						ISkinnedMesh::SWeight* WeightClone = AnimatedMesh->createWeight(joint);
						WeightClone->strength = weight.strength;
						WeightClone->vertex_id = verticesLinkIndex[id][k];
						WeightClone->buffer_id = verticesLinkBuffer[id][k];
					}
				}
			}
		}
		#else
		{
			core::array< u32 > verticesLinkIndex;
			core::array< u16 > verticesLinkBuffer;
			verticesLinkBuffer.set_used(mesh->Vertices.size());
			verticesLinkIndex.set_used(mesh->Vertices.size());

			// init with 0
			for (i=0;i<mesh->Vertices.size();++i)
			{
				verticesLinkBuffer[i]=0;
				verticesLinkIndex[i]=0;
			}

			// store meshbuffer number per vertex
			for (i=0;i<mesh->FaceMaterialIndices.size();++i)
			{
				for (u32 id=i*3+0;id<=i*3+2;++id)
				{
					verticesLinkBuffer[ mesh->Indices[id] ] = mesh->FaceMaterialIndices[i];
				}
			}

			if (mesh->FaceMaterialIndices.size() != 0)
			{
				// store vertices in buffers and remember relation in verticesLinkIndex
				u32* vCountArray = new u32[mesh->Buffers.size()];
				memset(vCountArray, 0, mesh->Buffers.size()*sizeof(u32));
				// count vertices in each buffer and reallocate
				for (i=0;i<mesh->Vertices.size();++i)
					++vCountArray[verticesLinkBuffer[i]];
				for (i=0; i!=mesh->Buffers.size(); ++i)
					mesh->Buffers[i]->Vertices_Standard.reallocate(vCountArray[i]);

				// actually store vertices
				for (i=0;i<mesh->Vertices.size();++i)
				{
					scene::SSkinMeshBuffer *buffer = mesh->Buffers[ verticesLinkBuffer[i] ];

					verticesLinkIndex[i] = buffer->Vertices_Standard.size();
					buffer->Vertices_Standard.push_back( mesh->Vertices[i] );
				}

				// count indices per buffer and reallocate
				memset(vCountArray, 0, mesh->Buffers.size()*sizeof(u32));
				for (i=0;i<mesh->FaceMaterialIndices.size();++i)
					++vCountArray[ mesh->FaceMaterialIndices[i] ];
				for (i=0; i!=mesh->Buffers.size(); ++i)
					mesh->Buffers[i]->Indices.reallocate(vCountArray[i]);
				delete [] vCountArray;
				// create indices per buffer
				for (i=0;i<mesh->FaceMaterialIndices.size();++i)
				{
					scene::SSkinMeshBuffer *buffer = mesh->Buffers[ mesh->FaceMaterialIndices[i] ];
					for (u32 id=i*3+0;id!=i*3+3;++id)
						buffer->Indices.push_back( verticesLinkIndex[ mesh->Indices[id] ] );
				}
			}


			for (u32 j=0;j<mesh->WeightJoint.size();++j)
			{
				ISkinnedMesh::SWeight& weight = (AnimatedMesh->getAllJoints()[mesh->WeightJoint[j]]->Weights[mesh->WeightNum[j]]);

				u32 id = weight.vertex_id;

				if (id>=verticesLinkIndex.size())
				{
					os::Printer::log("X loader: Weight id out of range", ELL_WARNING);
					id=0;
					weight.strength=0.f;
				}

				weight.vertex_id=verticesLinkIndex[id];
				weight.buffer_id=verticesLinkBuffer[id] + bufferOffset;;
			}
		}
		#endif

	}

	return true;
}


//! Reads file into memory
bool CXMeshFileLoader::readFileIntoMemory(io::IReadFile* file)
{
	const long size = file->getSize();
	if (size < 12)
	{
		os::Printer::log("X File is too small.", ELL_WARNING);
		return false;
	}

	Buffer = new c8[size];

	//! read all into memory
	if (file->read(Buffer, size) != size)
	{
		os::Printer::log("Could not read from x file.", ELL_WARNING);
		return false;
	}

	End = Buffer + size;

	//! check header "xof "
	if (strncmp(Buffer, "xof ", 4)!=0)
	{
		os::Printer::log("Not an x file, wrong header.", ELL_WARNING);
		return false;
	}

	//! read minor and major version, e.g. 0302 or 0303
	c8 tmp[3];
	tmp[2] = 0x0;
	tmp[0] = Buffer[4];
	tmp[1] = Buffer[5];
	MajorVersion = core::strtol10(tmp);

	tmp[0] = Buffer[6];
	tmp[1] = Buffer[7];
	MinorVersion = core::strtol10(tmp);

	//! read format
	if (strncmp(&Buffer[8], "txt ", 4) ==0)
		BinaryFormat = false;
	else if (strncmp(&Buffer[8], "bin ", 4) ==0)
		BinaryFormat = true;
	else
	{
		os::Printer::log("Only uncompressed x files currently supported.", ELL_WARNING);
		return false;
	}
	BinaryNumCount=0;

	//! read float size
	if (strncmp(&Buffer[12], "0032", 4) ==0)
		FloatSize = 4;
	else if (strncmp(&Buffer[12], "0064", 4) ==0)
		FloatSize = 8;
	else
	{
		os::Printer::log("Float size not supported.", ELL_WARNING);
		return false;
	}

	P = &Buffer[16];

	readUntilEndOfLine();
	FilePath = stripPathFromString(file->getFileName(),true);

	return true;
}


//! Parses the file
bool CXMeshFileLoader::parseFile()
{
	while(parseDataObject())
	{
		// loop
	}

	return true;
}


//! Parses the next Data object in the file
bool CXMeshFileLoader::parseDataObject()
{
	core::stringc objectName = getNextToken();

	if (objectName.size() == 0)
		return false;

	// parse specific object
#ifdef _XREADER_DEBUG
	os::Printer::log("debug DataObject:", objectName.c_str() );
#endif

	if (objectName == "template")
		return parseDataObjectTemplate();
	else
	if (objectName == "Frame")
	{
		return parseDataObjectFrame( 0 );
	}
	else
	if (objectName == "Mesh")
	{
		// some meshes have no frames at all
		//CurFrame = AnimatedMesh->createJoint(0);

		//CurFrame->Meshes.push_back(SXMesh());
		//return parseDataObjectMesh(CurFrame->Meshes.getLast());

		SXMesh *mesh=new SXMesh;

		//mesh->Buffer=AnimatedMesh->createBuffer();
		Meshes.push_back(mesh);

		return parseDataObjectMesh ( *mesh );
	}
	else
	if (objectName == "AnimationSet")
	{
		return parseDataObjectAnimationSet();
	}
	else
	if (objectName == "Material")
	{
		// template materials now available thanks to joeWright
		TemplateMaterials.push_back(SXTemplateMaterial());
		TemplateMaterials.getLast().Name = getNextToken();
		return parseDataObjectMaterial(TemplateMaterials.getLast().Material);
	}
	else
	if (objectName == "}")
	{
		os::Printer::log("} found in dataObject", ELL_WARNING);
		return true;
	}

	os::Printer::log("Unknown data object in animation of .x file", objectName.c_str(), ELL_WARNING);

	return parseUnknownDataObject();
}


bool CXMeshFileLoader::parseDataObjectTemplate()
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading template");
#endif

	// parse a template data object. Currently not stored.
	core::stringc name;

	if (!readHeadOfDataObject(&name))
	{
		os::Printer::log("Left delimiter in template data object missing.",
			name.c_str(), ELL_ERROR);
		return false;
	}

	// read GUID
	core::stringc guid = getNextToken();

	// read and ignore data members
	while(true)
	{
		core::stringc s = getNextToken();

		if (s == "}")
			break;

		if (s.size() == 0)
			return false;
	}

	return true;
}



bool CXMeshFileLoader::parseDataObjectFrame( CSkinnedMesh::SJoint *Parent )
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading frame");
#endif

	// A coordinate frame, or "frame of reference." The Frame template
	// is open and can contain any object. The Direct3D extensions (D3DX)
	// mesh-loading functions recognize Mesh, FrameTransformMatrix, and
	// Frame template instances as child objects when loading a Frame
	// instance.

	u32 JointID=0;

	core::stringc name;

	if (!readHeadOfDataObject(&name))
	{
		os::Printer::log("No opening brace in Frame found in x file", ELL_WARNING);
		return false;
	}

	CSkinnedMesh::SJoint *joint=0;

	if (name.size())
	{
		for (u32 n=0;n < AnimatedMesh->getAllJoints().size();++n)
		{
			if (AnimatedMesh->getAllJoints()[n]->Name==name)
			{
				joint=AnimatedMesh->getAllJoints()[n];
				JointID=n;
				break;
			}
		}
	}

	if (!joint)
	{
#ifdef _XREADER_DEBUG
		os::Printer::log("creating joint ", name.c_str());
#endif
		joint=AnimatedMesh->createJoint(Parent);
		joint->Name=name;
		JointID=AnimatedMesh->getAllJoints().size()-1;
	}
	else
	{
#ifdef _XREADER_DEBUG
		os::Printer::log("using joint ", name.c_str());
#endif
		if (Parent)
			Parent->Children.push_back(joint);
	}

	// Now inside a frame.
	// read tokens until closing brace is reached.

	while(true)
	{
		core::stringc objectName = getNextToken();

#ifdef _XREADER_DEBUG
		os::Printer::log("debug DataObject in frame:", objectName.c_str() );
#endif

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Frame in x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // frame finished
		}
		else
		if (objectName == "Frame")
		{

			if (!parseDataObjectFrame(joint))
				return false;
		}
		else
		if (objectName == "FrameTransformMatrix")
		{
			if (!parseDataObjectTransformationMatrix(joint->LocalMatrix))
				return false;

			//joint->LocalAnimatedMatrix
			//joint->LocalAnimatedMatrix.makeInverse();
			//joint->LocalMatrix=tmp*joint->LocalAnimatedMatrix;
		}
		else
		if (objectName == "Mesh")
		{
			/*
			frame.Meshes.push_back(SXMesh());
			if (!parseDataObjectMesh(frame.Meshes.getLast()))
				return false;
			*/
			SXMesh *mesh=new SXMesh;

			mesh->AttachedJointID=JointID;

			Meshes.push_back(mesh);

			if (!parseDataObjectMesh(*mesh))
				return false;
		}
		else
		{
			os::Printer::log("Unknown data object in frame in x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectTransformationMatrix(core::matrix4 &mat)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading Transformation Matrix");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Transformation Matrix found in x file", ELL_WARNING);
		return false;
	}

	for (u32 i=0; i<4; ++i)
		for (u32 j=0; j<4; ++j)
			mat(i,j)=readFloat();

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Transformation Matrix found in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Transformation Matrix found in x file", ELL_WARNING);
		return false;
	}

	return true;
}



bool CXMeshFileLoader::parseDataObjectMesh(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading mesh");
#endif

	core::stringc name;

	if (!readHeadOfDataObject(&name))
	{
		os::Printer::log("No opening brace in Mesh found in x file", ELL_WARNING);
		return false;
	}

	// read vertex count
	const u32 nVertices = readInt();

	// read vertices
	mesh.Vertices.set_used(nVertices); //luke: change

	for (u32 n=0; n<nVertices; ++n)
	{
		readVector3(mesh.Vertices[n].Pos);
		mesh.Vertices[n].Color=0xFFFFFFFF;
	}

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Vertex Array found in x file", ELL_WARNING);
		return false;
	}

	// read faces
	const u32 nFaces = readInt();

	mesh.Indices.set_used(nFaces * 3);
	mesh.IndexCountPerFace.set_used(nFaces);

	core::array<u32> polygonfaces;
	u32 currentIndex = 0;

	for (u32 k=0; k<nFaces; ++k)
	{
		const u32 fcnt = readInt();

		if (fcnt != 3)
		{
			if (fcnt < 3)
			{
				os::Printer::log("Invalid face count (<3) found in Mesh x file reader.", ELL_WARNING);
				return false;
			}

			// read face indices
			polygonfaces.set_used(fcnt);
			u32 triangles = (fcnt-2);
			mesh.Indices.set_used(mesh.Indices.size() + ((triangles-1)*3));
			mesh.IndexCountPerFace[k] = triangles * 3;

			for (u32 f=0; f<fcnt; ++f)
				polygonfaces[f] = readInt();

			for (u32 jk=0; jk<triangles; ++jk)
			{
				mesh.Indices[currentIndex++] = polygonfaces[0];
				mesh.Indices[currentIndex++] = polygonfaces[jk+1];
				mesh.Indices[currentIndex++] = polygonfaces[jk+2];
			}

			// TODO: change face indices in material list
		}
		else
		{
			mesh.Indices[currentIndex++] = readInt();
			mesh.Indices[currentIndex++] = readInt();
			mesh.Indices[currentIndex++] = readInt();
			mesh.IndexCountPerFace[k] = 3;
		}
	}

	if (BinaryFormat && BinaryNumCount)
	{
		os::Printer::log("Binary X: Mesh: Integer count mismatch", ELL_WARNING);
		return false;
	}
	else if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Face Array found in x file", ELL_WARNING);
		return false;
	}

	// here, other data objects may follow

	while(true)
	{
		core::stringc objectName = getNextToken();

#ifdef _XREADER_DEBUG
		os::Printer::log("debug DataObject in mesh:", objectName.c_str() );
#endif

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Mesh in x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // mesh finished
		}
		else
		if (objectName == "MeshNormals")
		{
			if (!parseDataObjectMeshNormals(mesh))
				return false;
		}
		else
		if (objectName == "MeshTextureCoords")
		{
			if (!parseDataObjectMeshTextureCoords(mesh))
				return false;
		}
		else
		if (objectName == "MeshVertexColors")
		{
			if (!parseDataObjectMeshVertexColors(mesh))
				return false;
		}
		else
		if (objectName == "MeshMaterialList")
		{
			if (!parseDataObjectMeshMaterialList(mesh))
					return false;
		}
		else
		if (objectName == "VertexDuplicationIndices")
		{
			// we'll ignore vertex duplication indices
			// TODO: read them
			if (!parseUnknownDataObject())
				return false;
		}
		else
		if (objectName == "XSkinMeshHeader")
		{
			if (!parseDataObjectSkinMeshHeader(mesh))
				return false;
		}
		else
		if (objectName == "SkinWeights")
		{
			//mesh.SkinWeights.push_back(SXSkinWeight());
			//if (!parseDataObjectSkinWeights(mesh.SkinWeights.getLast()))
			if (!parseDataObjectSkinWeights(mesh))
				return false;
		}
		else
		{
			os::Printer::log("Unknown data object in mesh in x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectSkinWeights(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading mesh skin weights");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Skin Weights found in .x file", ELL_WARNING);
		return false;
	}

	core::stringc TransformNodeName;

	if (!getNextTokenAsString(TransformNodeName))
	{
		os::Printer::log("Unknown syntax while reading transfrom node name string in .x file", ELL_WARNING);
		return false;
	}


	mesh.HasSkinning=true;

	CSkinnedMesh::SJoint *joint=0;

	u32 n;
	for (n=0; n < AnimatedMesh->getAllJoints().size(); ++n)
	{
		if (AnimatedMesh->getAllJoints()[n]->Name==TransformNodeName)
		{
			joint=AnimatedMesh->getAllJoints()[n];
			break;
		}
	}

	if (!joint)
	{
#ifdef _XREADER_DEBUG
		os::Printer::log("creating joint for skinning ", TransformNodeName.c_str());
#endif
		n = AnimatedMesh->getAllJoints().size();
		joint=AnimatedMesh->createJoint(0);
		joint->Name=TransformNodeName;
	}

	// read vertex weights
	const u32 nWeights = readInt();

	// read vertex indices
	u32 i;

	const u32 jointStart = joint->Weights.size();
	joint->Weights.reallocate(jointStart+nWeights);

	mesh.WeightJoint.reallocate( mesh.WeightJoint.size() + nWeights );
	mesh.WeightNum.reallocate( mesh.WeightNum.size() + nWeights );

	for (i=0; i<nWeights; ++i)
	{
		mesh.WeightJoint.push_back(n);
		mesh.WeightNum.push_back(joint->Weights.size());

		CSkinnedMesh::SWeight *weight=AnimatedMesh->createWeight(joint);

		weight->buffer_id=0;
		weight->vertex_id=readInt();
	}

	// read vertex weights

	for (i=0; i<nWeights; ++i)
		joint->Weights[i].strength = readFloat();

	// read matrix offset

	// transforms the mesh vertices to the space of the bone
	// When concatenated to the bone's transform, this provides the
	// world space coordinates of the mesh as affected by the bone
	core::matrix4& MatrixOffset = joint->GlobalInversedMatrix;

	for (i=0; i<4; ++i)
	{
		for (u32 j=0; j<4; ++j)
			MatrixOffset(i,j) = readFloat();
	}

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Skin Weights found in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Skin Weights found in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectSkinMeshHeader(SXMesh& mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading skin mesh header");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Skin Mesh header found in .x file", ELL_WARNING);
		return false;
	}

	mesh.MaxSkinWeightsPerVertex = readInt();
	mesh.MaxSkinWeightsPerFace = readInt();
	mesh.BoneCount = readInt();

	if (!BinaryFormat)
		getNextToken(); // skip semicolon

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in skin mesh header in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectMeshNormals(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading mesh normals");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Mesh Normals found in x file", ELL_WARNING);
		return false;
	}

	// read count
	const u32 nNormals = readInt();
	core::array<core::vector3df> normals;
	normals.set_used(nNormals);

	// read normals
	for (u32 i=0; i<nNormals; ++i)
		readVector3(normals[i]);

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Normals Array found in x file", ELL_WARNING);
		return false;
	}

	core::array<u32> normalIndices;
	normalIndices.set_used(mesh.Indices.size());

	// read face normal indices
	const u32 nFNormals = readInt();

	u32 normalidx = 0;
	core::array<u32> polygonfaces;
	for (u32 k=0; k<nFNormals; ++k)
	{
		const u32 fcnt = readInt();
		u32 triangles = fcnt - 2;
		u32 indexcount = triangles * 3;

		if (indexcount != mesh.IndexCountPerFace[k])
		{
			os::Printer::log("Not matching normal and face index count found in x file", ELL_WARNING);
			return false;
		}

		if (indexcount == 3)
		{
			// default, only one triangle in this face
			for (u32 h=0; h<3; ++h)
			{
				const u32 normalnum = readInt();
				mesh.Vertices[mesh.Indices[normalidx++]].Normal.set(normals[normalnum]);
			}
		}
		else
		{
			polygonfaces.set_used(fcnt);
			// multiple triangles in this face
			for (u32 h=0; h<fcnt; ++h)
				polygonfaces[h] = readInt();

			for (u32 jk=0; jk<triangles; ++jk)
			{
				mesh.Vertices[mesh.Indices[normalidx++]].Normal.set(normals[polygonfaces[0]]);
				mesh.Vertices[mesh.Indices[normalidx++]].Normal.set(normals[polygonfaces[jk+1]]);
				mesh.Vertices[mesh.Indices[normalidx++]].Normal.set(normals[polygonfaces[jk+2]]);
			}
		}
	}

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Face Normals Array found in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Mesh Normals found in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectMeshTextureCoords(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading mesh texture coordinates");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Mesh Texture Coordinates found in x file", ELL_WARNING);
		return false;
	}

	const u32 nCoords = readInt();
	for (u32 i=0; i<nCoords; ++i)
		readVector2(mesh.Vertices[i].TCoords);

	if (!checkForTwoFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Texture Coordinates Array found in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Mesh Texture Coordinates Array found in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectMeshVertexColors(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading mesh vertex colors");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace for Mesh Vertex Colors found in x file", ELL_WARNING);
		return false;
	}

	mesh.HasVertexColors=true;
	const u32 nColors = readInt();
	for (u32 i=0; i<nColors; ++i)
	{
		const u32 Index=readInt();
		if (Index>=mesh.Vertices.size())
		{
			os::Printer::log("index value in parseDataObjectMeshVertexColors out of bounds", ELL_WARNING);
			return false;
		}
		readRGBA(mesh.Vertices[Index].Color);
		checkForOneFollowingSemicolons();
	}

	if (!checkForOneFollowingSemicolons())
	{
		os::Printer::log("No finishing semicolon in Mesh Vertex Colors Array found in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Mesh Texture Coordinates Array found in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectMeshMaterialList(SXMesh &mesh)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading mesh material list");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Mesh Material List found in x file", ELL_WARNING);
		return false;
	}

	// read material count
	mesh.Materials.reallocate(readInt());

	// read non triangulated face material index count
	const u32 nFaceIndices = readInt();

	if (nFaceIndices != mesh.IndexCountPerFace.size())
	{
		os::Printer::log("Index count per face not equal to face material index count in x file.", ELL_WARNING);
		return false;
	}

	// read non triangulated face indices and create triangulated ones
	mesh.FaceMaterialIndices.set_used( mesh.Indices.size() / 3);
	u32 triangulatedindex = 0;
	for (u32 tfi=0; tfi<nFaceIndices; ++tfi)
	{
		const u32 ind = readInt();
		const u32 fc = mesh.IndexCountPerFace[tfi]/3;
		for (u32 k=0; k<fc; ++k)
			mesh.FaceMaterialIndices[triangulatedindex++] = ind;
	}

	// in version 03.02, the face indices end with two semicolons.
	// commented out version check, as version 03.03 exported from blender also has 2 semicolons
	if (!BinaryFormat) // && MajorVersion == 3 && MinorVersion <= 2)
	{
		if (P[0] == ';')
			++P;
	}

	// read following data objects

	while(true)
	{
		core::stringc objectName = getNextToken();

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Mesh Material list in .x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // material list finished
		}
		else
		if (objectName == "{")
		{
			// template materials now available thanks to joeWright
			objectName = getNextToken();
			for (u32 i=0; i<TemplateMaterials.size(); ++i)
				if (TemplateMaterials[i].Name == objectName)
					mesh.Materials.push_back(TemplateMaterials[i].Material);
			getNextToken(); // skip }
		}
		else
		if (objectName == "Material")
		{
			mesh.Materials.push_back(video::SMaterial());
			if (!parseDataObjectMaterial(mesh.Materials.getLast()))
				return false;
		}
		else
		if (objectName == ";")
		{
			// ignore
		}
		else
		{
			os::Printer::log("Unknown data object in material list in x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}
	return true;
}


bool CXMeshFileLoader::parseDataObjectMaterial(video::SMaterial& material)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading mesh material");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Mesh Material found in .x file", ELL_WARNING);
		return false;
	}

	// read RGBA
	readRGBA(material.DiffuseColor); checkForOneFollowingSemicolons();

	// read power
	material.Shininess = readFloat();

	// read specular
	readRGB(material.SpecularColor); checkForOneFollowingSemicolons();

	// read emissive
	readRGB(material.EmissiveColor); checkForOneFollowingSemicolons();

	// read other data objects
	while(true)
	{
		core::stringc objectName = getNextToken();

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Mesh Material in .x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // material finished
		}
		else
		if (objectName.equals_ignore_case("TextureFilename"))
		{
			// some exporters write "TextureFileName" instead.
			core::stringc TextureFileName;
			if (!parseDataObjectTextureFilename(TextureFileName))
				return false;

			// original name
			material.setTexture(0, SceneManager->getVideoDriver()->getTexture ( TextureFileName.c_str() ));
			// mesh path
			if (!material.getTexture(0))
			{
				TextureFileName=FilePath + stripPathFromString(TextureFileName,false);
				material.setTexture(0, SceneManager->getVideoDriver()->getTexture ( TextureFileName.c_str() ));
			}
			// working directory
			if (!material.getTexture(0))
				material.setTexture(0, SceneManager->getVideoDriver()->getTexture ( stripPathFromString(TextureFileName,false).c_str() ));
		}
		else
		{
			os::Printer::log("Unknown data object in material in .x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectAnimationSet()
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: Reading animation set");
#endif

	core::stringc AnimationName;

	if (!readHeadOfDataObject(&AnimationName))
	{
		os::Printer::log("No opening brace in Animation Set found in x file", ELL_WARNING);
		return false;
	}

	while(true)
	{
		core::stringc objectName = getNextToken();

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Animation set in x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // animation set finished
		}
		else
		if (objectName == "Animation")
		{
			if (!parseDataObjectAnimation())
				return false;
		}
		else
		{
			os::Printer::log("Unknown data object in animation set in x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}
	return true;
}


bool CXMeshFileLoader::parseDataObjectAnimation()
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading animation");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Animation found in x file", ELL_WARNING);
		return false;
	}

	//anim.closed = true;
	//anim.linearPositionQuality = true;
	CSkinnedMesh::SJoint animationDump;

	core::stringc FrameName;

	while(true)
	{
		core::stringc objectName = getNextToken();

		if (objectName.size() == 0)
		{
			os::Printer::log("Unexpected ending found in Animation in x file.", ELL_WARNING);
			return false;
		}
		else
		if (objectName == "}")
		{
			break; // animation finished
		}
		else
		if (objectName == "AnimationKey")
		{
			if (!parseDataObjectAnimationKey(&animationDump))
				return false;
		}
		else
		if (objectName == "AnimationOptions")
		{
			//TODO: parse options.
			if (!parseUnknownDataObject())
				return false;
		}
		else
		if (objectName == "{")
		{
			// read frame name
			FrameName = getNextToken();

			if (!checkForClosingBrace())
			{
				os::Printer::log("Unexpected ending found in Animation in x file.", ELL_WARNING);
				return false;
			}
		}
		else
		{
			os::Printer::log("Unknown data object in animation in x file", objectName.c_str(), ELL_WARNING);
			if (!parseUnknownDataObject())
				return false;
		}
	}

	if (FrameName.size() != 0)
	{
#ifdef _XREADER_DEBUG
		os::Printer::log("getting name: ", FrameName.c_str());
#endif
		CSkinnedMesh::SJoint *joint=0;

		u32 n;
		for (n=0;n < AnimatedMesh->getAllJoints().size();++n)
		{
			if (AnimatedMesh->getAllJoints()[n]->Name==FrameName)
			{
				joint=AnimatedMesh->getAllJoints()[n];
				break;
			}
		}

		if (!joint)
		{
#ifdef _XREADER_DEBUG
			os::Printer::log("creating joint for animation ", FrameName.c_str());
#endif
			joint=AnimatedMesh->createJoint(0);
			joint->Name=FrameName;
		}

		joint->PositionKeys.reallocate(joint->PositionKeys.size()+animationDump.PositionKeys.size());
		for (n=0;n<animationDump.PositionKeys.size();++n)
		{
			ISkinnedMesh::SPositionKey *key=&animationDump.PositionKeys[n];

			//key->position+=joint->LocalMatrix.getTranslation();

			joint->PositionKeys.push_back(*key);
		}

		joint->ScaleKeys.reallocate(joint->ScaleKeys.size()+animationDump.ScaleKeys.size());
		for (n=0;n<animationDump.ScaleKeys.size();++n)
		{
			ISkinnedMesh::SScaleKey *key=&animationDump.ScaleKeys[n];

			//key->scale*=joint->LocalMatrix.getScale();

			joint->ScaleKeys.push_back(*key);
		}

		joint->RotationKeys.reallocate(joint->RotationKeys.size()+animationDump.RotationKeys.size());
		for (n=0;n<animationDump.RotationKeys.size();++n)
		{
			ISkinnedMesh::SRotationKey *key=&animationDump.RotationKeys[n];

			core::matrix4 tmpMatrix;

			tmpMatrix.setRotationRadians(
				core::vector3df(key->rotation.X, key->rotation.Y, key->rotation.Z) );

			tmpMatrix=joint->LocalMatrix*tmpMatrix;

			//key->rotation  = core::quaternion(tmpMatrix);

			joint->RotationKeys.push_back(*key);
		}
	}
	else
		os::Printer::log("joint name was never given", ELL_WARNING);

	return true;
}


bool CXMeshFileLoader::parseDataObjectAnimationKey(ISkinnedMesh::SJoint *joint)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading animation key");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Animation Key found in x file", ELL_WARNING);
		return false;
	}

	// read key type

	const u32 keyType = readInt();

	if (keyType > 4)
	{
		os::Printer::log("Unknown key type found in Animation Key in x file", ELL_WARNING);
		return false;
	}

	// read number of keys
	const u32 numberOfKeys = readInt();

	// eat the semicolon after the "0".  if there are keys present, readInt()
	// does this for us.  If there aren't, we need to do it explicitly
	if (numberOfKeys == 0)
		checkForOneFollowingSemicolons();

	for (u32 i=0; i<numberOfKeys; ++i)
	{
		// read time
		const u32 time = readInt();

		// read keys
		switch(keyType)
		{
		case 0: //rotation
			{
				//read quaternions

				// read count
				if (readInt() != 4)
				{
					os::Printer::log("Expected 4 numbers in animation key in x file", ELL_WARNING);
					return false;
				}

				f32 W = -readFloat();
				f32 X = -readFloat();
				f32 Y = -readFloat();
				f32 Z = -readFloat();

				if (!checkForTwoFollowingSemicolons())
				{
					os::Printer::log("No finishing semicolon after quaternion animation key in x file", ELL_WARNING);
					return false;
				}

				ISkinnedMesh::SRotationKey *key=AnimatedMesh->createRotationKey(joint);
				key->frame=(f32)time;
				key->rotation.set(X,Y,Z,W);
			}
			break;
		case 1: //scale
		case 2: //position
			{
				// read vectors

				// read count
				if (readInt() != 3)
				{
					os::Printer::log("Expected 3 numbers in animation key in x file", ELL_WARNING);
					return false;
				}

				core::vector3df vector;
				readVector3(vector);

				if (!checkForTwoFollowingSemicolons())
				{
					os::Printer::log("No finishing semicolon after vector animation key in x file", ELL_WARNING);
					return false;
				}

				if (keyType==2)
				{
					ISkinnedMesh::SPositionKey *key=AnimatedMesh->createPositionKey(joint);
					key->frame=(f32)time;
					key->position=vector;
				}
				else
				{
					ISkinnedMesh::SScaleKey *key=AnimatedMesh->createScaleKey(joint);
					key->frame=(f32)time;
					key->scale=vector;
				}
			}
			break;
		case 3:
		case 4:
			{
				// read matrix

				// read count
				if (readInt() != 16)
				{
					os::Printer::log("Expected 16 numbers in animation key in x file", ELL_WARNING);
					return false;
				}

				// read matrix
				core::matrix4 Matrix;

				for (u32 m=0; m<4; ++m)
					for (u32 n=0; n<4; ++n)
						Matrix(m,n) = readFloat();


				//Matrix=joint->LocalMatrix*Matrix;

				if (!checkForTwoFollowingSemicolons())
				{
					os::Printer::log("No finishing semicolon after matrix animation key in x file", ELL_WARNING);
					return false;
				}

				//core::vector3df rotation = Matrix.getRotationDegrees();

				ISkinnedMesh::SRotationKey *keyR=AnimatedMesh->createRotationKey(joint);
				keyR->frame=(f32)time;
				//keyR->rotation.set(rotation.X*core::DEGTORAD,rotation.Y*core::DEGTORAD,rotation.Z*core::DEGTORAD);
				keyR->rotation= core::quaternion(Matrix);


				ISkinnedMesh::SPositionKey *keyP=AnimatedMesh->createPositionKey(joint);
				keyP->frame=(f32)time;
				keyP->position=Matrix.getTranslation();

				core::vector3df scale=Matrix.getScale();

				//if (scale.X==0) scale.X=1;
				//if (scale.Y==0) scale.Y=1;
				//if (scale.Z==0) scale.Z=1;
/*
				ISkinnedMesh::SScaleKey *keyS=AnimatedMesh->createScaleKey(joint);
				keyS->frame=(f32)time;
				keyS->scale=scale;
*/
			}
			break;
		} // end switch
	}

	checkForOneFollowingSemicolons();

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in animation key in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseDataObjectTextureFilename(core::stringc& texturename)
{
#ifdef _XREADER_DEBUG
	os::Printer::log("CXFileReader: reading texture filename");
#endif

	if (!readHeadOfDataObject())
	{
		os::Printer::log("No opening brace in Texture filename found in x file", ELL_WARNING);
		return false;
	}

	if (!getNextTokenAsString(texturename))
	{
		os::Printer::log("Unknown syntax while reading texture filename string in x file", ELL_WARNING);
		return false;
	}

	if (!checkForClosingBrace())
	{
		os::Printer::log("No closing brace in Texture filename found in x file", ELL_WARNING);
		return false;
	}

	return true;
}


bool CXMeshFileLoader::parseUnknownDataObject()
{
	// find opening delimiter
	while(true)
	{
		core::stringc t = getNextToken();

		if (t.size() == 0)
			return false;

		if (t == "{")
			break;
	}

	u32 counter = 1;

	// parse until closing delimiter

	while(counter)
	{
		core::stringc t = getNextToken();

		if (t.size() == 0)
			return false;

		if (t == "{")
			++counter;
		else
		if (t == "}")
			--counter;
	}

	return true;
}


//! checks for closing curly brace, returns false if not there
bool CXMeshFileLoader::checkForClosingBrace()
{
	return (getNextToken() == "}");
}


//! checks for one following semicolon, returns false if not there
bool CXMeshFileLoader::checkForOneFollowingSemicolons()
{
	if (BinaryFormat)
		return true;

	return (getNextToken() == ";");
}


//! checks for two following semicolons, returns false if they are not there
bool CXMeshFileLoader::checkForTwoFollowingSemicolons()
{
	if (BinaryFormat)
		return true;

	for (u32 k=0; k<2; ++k)
	{
		if (getNextToken() != ";")
			return false;
	}

	return true;
}


//! reads header of dataobject including the opening brace.
//! returns false if error happened, and writes name of object
//! if there is one
bool CXMeshFileLoader::readHeadOfDataObject(core::stringc* outname)
{
	core::stringc nameOrBrace = getNextToken();
	if (nameOrBrace != "{")
	{
		if (outname)
			(*outname) = nameOrBrace;

		if (getNextToken() != "{")
			return false;
	}

	return true;
}


//! returns next parseable token. Returns empty string if no token there
core::stringc CXMeshFileLoader::getNextToken()
{
	core::stringc s;

	// process binary-formatted file
	if (BinaryFormat)
	{
		// in binary mode it will only return NAME and STRING token
		// and (correctly) skip over other tokens.

		s16 tok = readBinWord();
		u32 len;

		// standalone tokens
		switch (tok) {
			case 1:
				// name token
				len = readBinDWord();
				s = core::stringc(P, len);
				P += len;
				return s;
			case 2:
				// string token
				len = readBinDWord();
				s = core::stringc(P, len);
				P += (len + 2);
				return s;
			case 3:
				// integer token
				P += 4;
				return "<integer>";
			case 5:
				// GUID token
				P += 16;
				return "<guid>";
			case 6:
				len = readBinDWord();
				P += (len * 4);
				return "<int_list>";
			case 7:
				len = readBinDWord();
				P += (len * FloatSize);
				return "<flt_list>";
			case 0x0a:
				return "{";
			case 0x0b:
				return "}";
			case 0x0c:
				return "(";
			case 0x0d:
				return ")";
			case 0x0e:
				return "[";
			case 0x0f:
				return "]";
			case 0x10:
				return "<";
			case 0x11:
				return ">";
			case 0x12:
				return ".";
			case 0x13:
				return ",";
			case 0x14:
				return ";";
			case 0x1f:
				return "template";
			case 0x28:
				return "WORD";
			case 0x29:
				return "DWORD";
			case 0x2a:
				return "FLOAT";
			case 0x2b:
				return "DOUBLE";
			case 0x2c:
				return "CHAR";
			case 0x2d:
				return "UCHAR";
			case 0x2e:
				return "SWORD";
			case 0x2f:
				return "SDWORD";
			case 0x30:
				return "void";
			case 0x31:
				return "string";
			case 0x32:
				return "unicode";
			case 0x33:
				return "cstring";
			case 0x34:
				return "array";
		}
	}
	// process text-formatted file
	else
	{
		findNextNoneWhiteSpace();

		if (P >= End)
			return s;

		while((P < End) && !core::isspace(P[0]))
		{
			// either keep token delimiters when already holding a token, or return if first valid char
			if (P[0]==';' || P[0]=='}' || P[0]=='{' || P[0]==',')
			{
				if (!s.size())
				{
					s.append(P[0]);
					++P;
				}
				break; // stop for delimiter
			}
			s.append(P[0]);
			++P;
		}
	}
	return s;
}


//! places pointer to next begin of a token, which must be a number,
// and ignores comments
void CXMeshFileLoader::findNextNoneWhiteSpaceNumber()
{
	if (BinaryFormat)
		return;

	while(true)
	{
		while((P < End) && (P[0] != '-') && (P[0] != '.') &&
			!( core::isdigit(P[0])))
			++P;

		if (P >= End)
			return;

		// check if this is a comment
		if ((P[0] == '/' && P[1] == '/') || P[0] == '#')
			readUntilEndOfLine();
		else
			break;
	}
}


// places pointer to next begin of a token, and ignores comments
void CXMeshFileLoader::findNextNoneWhiteSpace()
{
	if (BinaryFormat)
		return;

	while(true)
	{
		while((P < End) && core::isspace(P[0]))
			++P;

		if (P >= End)
			return;

		// check if this is a comment
		if ((P[0] == '/' && P[1] == '/') ||
			P[0] == '#')
			readUntilEndOfLine();
		else
			break;
	}
}


//! reads a x file style string
bool CXMeshFileLoader::getNextTokenAsString(core::stringc& out)
{
	if (BinaryFormat)
	{
		out=getNextToken();
		return true;
	}
	findNextNoneWhiteSpace();

	if (P >= End)
		return false;

	if (P[0] != '"')
		return false;
	++P;

	while(P < End && P[0]!='"')
	{
		out.append(P[0]);
		++P;
	}

	if ( P[1] != ';' || P[0] != '"')
		return false;
	P+=2;

	return true;
}


void CXMeshFileLoader::readUntilEndOfLine()
{
	if (BinaryFormat)
		return;

	while(P < End)
	{
		if (P[0] == '\n' || P[0] == '\r')
		{
			++P;
			return;
		}

		++P;
	}
}


u16 CXMeshFileLoader::readBinWord()
{
	u8 *Q = (u8 *)P;
	u16 tmp = 0;
	tmp = Q[0] + (Q[1] << 8);
	P += 2;
	return tmp;
}


u32 CXMeshFileLoader::readBinDWord()
{
	u8 *Q = (u8 *)P;
	u32 tmp = 0;
	tmp = Q[0] + (Q[1] << 8) + (Q[2] << 16) + (Q[3] << 24);
	P += 4;
	return tmp;
}


u32 CXMeshFileLoader::readInt()
{
	if (BinaryFormat)
	{
		if (!BinaryNumCount)
		{
			u16 tmp = readBinWord(); // 0x06 or 0x03
			if (tmp == 0x06)
				BinaryNumCount = readBinDWord();
			else
				BinaryNumCount = 1; // single int
		}
		--BinaryNumCount;
		return readBinDWord();
	}
	else
	{
		findNextNoneWhiteSpaceNumber();
		return core::strtol10(P, &P);
	}
}


f32 CXMeshFileLoader::readFloat()
{
	if (BinaryFormat)
	{
		if (!BinaryNumCount)
		{
			u16 tmp = readBinWord(); // 0x07 or 0x42
			if (tmp == 0x07)
				BinaryNumCount = readBinDWord();
			else
				BinaryNumCount = 1; // single int
		}
		--BinaryNumCount;
		if (FloatSize == 8)
		{
			char tmp[8];
			memcpy(tmp, P, 8);
			P += 8;
			return (f32)(*(f64 *)tmp);
		}
		else
		{
			char tmp[4];
			memcpy(tmp, P, 4);
			P += 4;
			return *(f32 *)tmp;
		}
	}
	findNextNoneWhiteSpaceNumber();
	f32 ftmp;
	P = core::fast_atof_move(P, ftmp);
	return ftmp;
}


// read 2-dimensional vector. Stops at semicolon after second value for text file format
bool CXMeshFileLoader::readVector2(core::vector2df& vec)
{
	vec.X = readFloat();
	vec.Y = readFloat();
	return true;
}


// read 3-dimensional vector. Stops at semicolon after third value for text file format
bool CXMeshFileLoader::readVector3(core::vector3df& vec)
{
	vec.X = readFloat();
	vec.Y = readFloat();
	vec.Z = readFloat();
	return true;
}


// read color without alpha value. Stops after second semicolon after blue value
bool CXMeshFileLoader::readRGB(video::SColorf& color)
{
	color.r = readFloat();
	color.g = readFloat();
	color.b = readFloat();
	color.a = 1.0f;
	return checkForOneFollowingSemicolons();
}


// read color with alpha value. Stops after second semicolon after blue value
bool CXMeshFileLoader::readRGBA(video::SColorf& color)
{
	color.r = readFloat();
	color.g = readFloat();
	color.b = readFloat();
	color.a = readFloat();
	return checkForOneFollowingSemicolons();
}


// read color without alpha value. Stops after second semicolon after blue value
bool CXMeshFileLoader::readRGB(video::SColor& color)
{
	color.setRed( (u32)(readFloat()*255)) ;
	color.setGreen( (u32)(readFloat()*255)) ;
	color.setBlue( (u32)(readFloat()*255)) ;
	color.setAlpha( 255 );
	return checkForOneFollowingSemicolons();
}


// read color with alpha value. Stops after second semicolon after blue value
bool CXMeshFileLoader::readRGBA(video::SColor& color)
{
	color.setRed( (u32)(readFloat()*255)) ;
	color.setGreen( (u32)(readFloat()*255)) ;
	color.setBlue( (u32)(readFloat()*255)) ;
	color.setAlpha( (u32)(readFloat()*255)) ;
	return checkForOneFollowingSemicolons();
}


core::stringc CXMeshFileLoader::stripPathFromString(core::stringc string, bool returnPath)
{
	s32 slashIndex=string.findLast('/'); // forward slash
	s32 backSlash=string.findLast('\\'); // back slash

	if (backSlash>slashIndex) slashIndex=backSlash;

	if (slashIndex==-1)//no slashes found
	{
		if (returnPath)
			return core::stringc(); //no path to return
		else
			return string;
	}

	if (returnPath)
		return string.subString(0, slashIndex + 1);
	else
		return string.subString(slashIndex+1, string.size() - (slashIndex+1));
}


} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_X_LOADER_

