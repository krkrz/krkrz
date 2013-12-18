// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h" 
#ifdef _IRR_COMPILE_WITH_COLLADA_LOADER_

#include "CColladaFileLoader.h"
#include "os.h"
#include "IXMLReader.h"
#include "IDummyTransformationSceneNode.h"
#include "SAnimatedMesh.h"
#include "fast_atof.h"
#include "quaternion.h"
#include "ILightSceneNode.h"
#include "ICameraSceneNode.h"
#include "IMeshManipulator.h"
#include "IReadFile.h"
#include "IAttributes.h"
#include "IMeshCache.h"
#include "IMeshSceneNode.h"
#include "SMeshBufferLightMap.h"

//#define COLLADA_READER_DEBUG
namespace irr
{
namespace scene
{
	// currently supported COLLADA tag names

	const core::stringc colladaSectionName =   "COLLADA";
	const core::stringc librarySectionName =   "library";
	const core::stringc assetSectionName =     "asset";
	const core::stringc sceneSectionName =     "scene";

	const core::stringc lightPrefabName =      "light";
	const core::stringc cameraPrefabName =     "camera";
	const core::stringc materialSectionName =  "material";
	const core::stringc geometrySectionName =  "geometry";
	const core::stringc imageSectionName =     "image";
	const core::stringc textureSectionName =   "texture";

	const core::stringc meshSectionName =      "mesh";
	const core::stringc sourceSectionName =	   "source";
	const core::stringc arraySectionName =     "array";
	const core::stringc accessorSectionName =  "accessor";
	const core::stringc verticesSectionName =  "vertices";
	const core::stringc inputTagName =         "input";
	const core::stringc polygonsSectionName =  "polygons";
	const core::stringc primitivesName =       "p";
	const core::stringc nodeSectionName =      "node";
	const core::stringc lookatNodeName =       "lookat";
	const core::stringc matrixNodeName =       "matrix";
	const core::stringc perspectiveNodeName =  "perspective";
	const core::stringc rotateNodeName =       "rotate";
	const core::stringc scaleNodeName =        "scale";
	const core::stringc translateNodeName =    "translate";
	const core::stringc skewNodeName =         "skew";
	const core::stringc bboxNodeName =         "boundingbox";
	const core::stringc minNodeName =          "min";
	const core::stringc maxNodeName =          "max";
	const core::stringc instanceNodeName =     "instance";
	const core::stringc extraNodeName =        "extra";

	const core::stringc paramTagName =         "param";

	const char* const inputSemanticNames[] = {"POSITION", "VERTEX", "NORMAL", "TEXCOORD",
		"UV", "TANGENT", "IMAGE", "TEXTURE", 0};

	//! following class is for holding and creating instances of library objects,
	//! named prefabs in this loader.
	class CPrefab : public IColladaPrefab
	{
	public:

		CPrefab(const char* id)
		{
			Id = id;
		}

		//! creates an instance of this prefab
		virtual scene::ISceneNode* addInstance(scene::ISceneNode* parent,
			scene::ISceneManager* mgr)
		{
			// empty implementation
			return 0;
		}

		//! returns id of this prefab
		virtual const c8* getId()
		{
			return Id.c_str();
		}

	protected:

		core::stringc Id;
	};


	//! prefab for a light scene node
	class CLightPrefab : public CPrefab
	{
	public:

		CLightPrefab(const char* id) : CPrefab(id)
		{
			#ifdef COLLADA_READER_DEBUG
			os::Printer::log("COLLADA: loaded light prefab:", Id.c_str());
			#endif
		}

		video::SLight LightData; // publically accessible

		//! creates an instance of this prefab
		virtual scene::ISceneNode* addInstance(scene::ISceneNode* parent,
			scene::ISceneManager* mgr)
		{
			#ifdef COLLADA_READER_DEBUG
			os::Printer::log("COLLADA: Constructing light instance:", Id.c_str());
			#endif

			scene::ILightSceneNode* l = mgr->addLightSceneNode(parent);
			l->setLightData ( LightData );
			return l;
		}
	};


	//! prefab for a mesh scene node
	class CGeometryPrefab : public CPrefab
	{
	public:

		CGeometryPrefab(const char* id) : CPrefab(id)
		{
		}

		scene::IMesh* Mesh; // public accessible

		//! creates an instance of this prefab
		virtual scene::ISceneNode* addInstance(scene::ISceneNode* parent,
			scene::ISceneManager* mgr)
		{
			#ifdef COLLADA_READER_DEBUG
			os::Printer::log("COLLADA: Constructing mesh instance:", Id.c_str());
			#endif

			scene::ISceneNode* m = mgr->addMeshSceneNode(Mesh, parent);
			return m;
		}
	};


	//! prefab for a camera scene node
	class CCameraPrefab : public CPrefab
	{
	public:

		CCameraPrefab(const char* id)
			: CPrefab(id), YFov(core::PI / 2.5f), ZNear(1.0f), ZFar(3000.0f)
		{
			#ifdef COLLADA_READER_DEBUG
			os::Printer::log("COLLADA: loaded camera prefab:", Id.c_str());
			#endif
		}

		// public accessible data
		f32 YFov;
		f32 ZNear;
		f32 ZFar;

		//! creates an instance of this prefab
		virtual scene::ISceneNode* addInstance(scene::ISceneNode* parent,
			scene::ISceneManager* mgr)
		{
			#ifdef COLLADA_READER_DEBUG
			os::Printer::log("COLLADA: Constructing camera instance:", Id.c_str());
			#endif

			scene::ICameraSceneNode* c = mgr->addCameraSceneNode(parent);
			c->setFOV(YFov);
			c->setNearValue(ZNear);
			c->setFarValue(ZFar);
			return c;
		}
	};

//! Constructor
CColladaFileLoader::CColladaFileLoader(video::IVideoDriver* driver,
		scene::ISceneManager* smgr, io::IFileSystem* fs)
: Driver(driver), SceneManager(smgr), FileSystem(fs), DummyMesh(0),
	FirstLoadedMesh(0), LoadedMeshCount(0), CreateInstances(false)
{
	
}


//! destructor
CColladaFileLoader::~CColladaFileLoader()
{
	if (DummyMesh)
		DummyMesh->drop();

	if (FirstLoadedMesh)
		FirstLoadedMesh->drop();
}


//! Returns true if the file maybe is able to be loaded by this class.
/** This decision should be based only on the file extension (e.g. ".cob") */
bool CColladaFileLoader::isALoadableFileExtension(const c8* fileName) const
{
	return strstr(fileName, ".xml") ||
		   strstr(fileName, ".dae");
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CColladaFileLoader::createMesh(io::IReadFile* file)
{
	io::IXMLReaderUTF8* reader = FileSystem->createXMLReaderUTF8(file);
	if (!reader)
		return 0;

	CurrentlyLoadingMesh = file->getFileName();
	CreateInstances = SceneManager->getParameters()->getAttributeAsBool(
		scene::COLLADA_CREATE_SCENE_INSTANCES);

	// read until COLLADA section, skip other parts

	while(reader->read())
	{
		if (reader->getNodeType() == io::EXN_ELEMENT)
		{
			if (colladaSectionName == reader->getNodeName())
				readColladaSection(reader);
			else
				skipSection(reader, true); // unknown section
		}
	}

	reader->drop();

	scene::IAnimatedMesh* returnMesh = DummyMesh;

	// because this loader loads and creates a complete scene instead of
	// a single mesh, return an empty dummy mesh to make the scene manager
	// know that everything went well.
	if (!DummyMesh)
	{
		DummyMesh = new SAnimatedMesh();
		returnMesh = DummyMesh;
	}

	// add the first loaded mesh into the mesh cache too, if more than one
	// meshes have been loaded from the file
	if (LoadedMeshCount>1 && FirstLoadedMesh)
	{
		os::Printer::log("Added COLLADA mesh", FirstLoadedMeshName.c_str());
		SceneManager->getMeshCache()->addMesh(FirstLoadedMeshName.c_str(), FirstLoadedMesh);
	}

	// clean up temporary loaded data
	clearData();

	returnMesh->grab(); // store until this loader is destroyed

	DummyMesh->drop();
	DummyMesh = 0;

	if (FirstLoadedMesh)
		FirstLoadedMesh->drop();
	FirstLoadedMesh = 0;
	LoadedMeshCount = 0;

	return returnMesh;
}



//! skips an (unknown) section in the collada document
void CColladaFileLoader::skipSection(io::IXMLReaderUTF8* reader, bool reportSkipping)
{
	#ifndef COLLADA_READER_DEBUG
	if (reportSkipping) // always report in COLLADA_READER_DEBUG mode
	#endif
		os::Printer::log("COLLADA skipping section", core::stringc(reader->getNodeName()).c_str());

	// skip if this element is empty anyway.
	if (reader->isEmptyElement())
		return;

	// read until we've reached the last element in this section
	u32 tagCounter = 1;

	while(tagCounter && reader->read())
	{
		if (reader->getNodeType() == io::EXN_ELEMENT &&
			!reader->isEmptyElement())
		{
			#ifdef COLLADA_READER_DEBUG
			if (reportSkipping)
				os::Printer::log("COLLADA unknown element:", core::stringc(reader->getNodeName()).c_str());
			#endif // COLLADA_READER_DEBUG

			++tagCounter;
		}
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
			--tagCounter;
	}
}


//! reads the <COLLADA> section and its content
void CColladaFileLoader::readColladaSection(io::IXMLReaderUTF8* reader)
{
	if (reader->isEmptyElement())
		return;

	// I ignore version information here. Keep on reading content:

	while(reader->read())
	if (reader->getNodeType() == io::EXN_ELEMENT)
	{
		if (assetSectionName == reader->getNodeName())
			readAssetSection(reader);
		else
		if (librarySectionName == reader->getNodeName())
			readLibrarySection(reader);
		else
		if (sceneSectionName == reader->getNodeName())
			readSceneSection(reader);
		else
		{
			os::Printer::log("COLLADA loader warning: Wrong tag usage found", reader->getNodeName(), ELL_WARNING);
			skipSection(reader, true); // unknown section
		}
	}
}



//! reads a <library> section and its content
void CColladaFileLoader::readLibrarySection(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading library");
	#endif

	while(reader->read())
	if (reader->getNodeType() == io::EXN_ELEMENT)
	{
		// animation section tbd
		if (cameraPrefabName == reader->getNodeName())
			readCameraPrefab(reader);
		else
		// code section tbd
		// controller section tbd
		if (geometrySectionName == reader->getNodeName())
			readGeometry(reader);
		else
		if (imageSectionName == reader->getNodeName())
			readImage(reader);
		else
		if (lightPrefabName == reader->getNodeName())
			readLightPrefab(reader);
		else
		if (materialSectionName == reader->getNodeName())
			readMaterial(reader);
		else
		// program section tbd
		if (textureSectionName == reader->getNodeName())
			readTexture(reader);
		else
			skipSection(reader, true); // unknown section, not all allowed supported yet
	}
	else
	if (reader->getNodeType() == io::EXN_ELEMENT_END)
	{
		if (librarySectionName == reader->getNodeName())
			break; // end reading.
	}
}


//! reads a <scene> section and its content
void CColladaFileLoader::readSceneSection(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading scene");
	#endif

	if (reader->isEmptyElement())
		return;

	// read the scene

	core::matrix4 transform; // transformation of this node
	core::aabbox3df bbox;
	scene::IDummyTransformationSceneNode* node = 0;

	while(reader->read())
	if (reader->getNodeType() == io::EXN_ELEMENT)
	{
		if (lookatNodeName == reader->getNodeName())
			transform *= readLookAtNode(reader);
		else
		if (matrixNodeName == reader->getNodeName())
			transform *= readMatrixNode(reader);
		else
		if (perspectiveNodeName == reader->getNodeName())
			transform *= readPerspectiveNode(reader);
		else
		if (rotateNodeName == reader->getNodeName())
			transform *= readRotateNode(reader);
		else
		if (scaleNodeName == reader->getNodeName())
			transform *= readScaleNode(reader);
		else
		if (skewNodeName == reader->getNodeName())
			transform *= readSkewNode(reader);
		else
		if (translateNodeName == reader->getNodeName())
			transform *= readTranslateNode(reader);
		else
		if (bboxNodeName == reader->getNodeName())
			readBboxNode(reader, bbox);
		else
		if (nodeSectionName == reader->getNodeName())
		{
			// create dummy node if there is none yet.
			if (!node)
				node = SceneManager->addDummyTransformationSceneNode(SceneManager->getRootSceneNode());

			readNodeSection(reader, node);
		}
		else
		if (extraNodeName == reader->getNodeName())
			skipSection(reader, false);
		else
		{
			os::Printer::log("COLLADA loader warning: Wrong tag usage found", reader->getNodeName(), ELL_WARNING);
			skipSection(reader, true); // ignore all other sections
		}
	}
	if (node)
		node->getRelativeTransformationMatrix() = transform;
}



//! reads a <asset> section and its content
void CColladaFileLoader::readAssetSection(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading asset");
	#endif

	// don't need asset data so far, so skip it
	skipSection(reader, false);
}



//! reads a <node> section and its content
void CColladaFileLoader::readNodeSection(io::IXMLReaderUTF8* reader, scene::ISceneNode* parent)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading node");
	#endif

	if (reader->isEmptyElement())
		return;

	core::stringc name = reader->getAttributeValue("name"); // name of the node
	core::matrix4 transform; // transformation of this node
	core::aabbox3df bbox;
	scene::ISceneNode* node = 0; // instance

	// read the node

	while(reader->read())
	{
	if (reader->getNodeType() == io::EXN_ELEMENT)
	{
		if (lookatNodeName == reader->getNodeName())
			transform *= readLookAtNode(reader);
		else
		if (matrixNodeName == reader->getNodeName())
			transform *= readMatrixNode(reader);
		else
		if (perspectiveNodeName == reader->getNodeName())
			transform *= readPerspectiveNode(reader);
		else
		if (rotateNodeName == reader->getNodeName())
			transform *= readRotateNode(reader);
		else
		if (scaleNodeName == reader->getNodeName())
			transform *= readScaleNode(reader);
		else
		if (translateNodeName == reader->getNodeName())
			transform *= readTranslateNode(reader);
		else
		if (skewNodeName == reader->getNodeName())
			transform *= readSkewNode(reader);
		else
		if (bboxNodeName == reader->getNodeName())
			readBboxNode(reader, bbox);
		else
		if (instanceNodeName == reader->getNodeName())
		{
			scene::ISceneNode* newnode = 0;
			readInstanceNode(reader, parent, &newnode);

			if (node && newnode)
			{
				// move children from dummy to new node
				core::list<ISceneNode*>::ConstIterator it = node->getChildren().begin();
				for (; it != node->getChildren().end(); it = node->getChildren().begin())
					(*it)->setParent(newnode);

				// remove previous dummy node
				node->remove();
			}

			node = newnode;
		}
		else
		if (nodeSectionName == reader->getNodeName())
		{
			// create dummy node if there is none yet.
			if (!node)
			{
				scene::IDummyTransformationSceneNode* dummy =
					SceneManager->addDummyTransformationSceneNode(parent);
				dummy->getRelativeTransformationMatrix() = transform;
				node = dummy;
			}

			// read and add child
			readNodeSection(reader, node);
		}
		else
			skipSection(reader, true); // ignore all other sections

	} // end if node
	else
	if (reader->getNodeType() == io::EXN_ELEMENT_END)
	{
		if (nodeSectionName == reader->getNodeName())
			break;
	}
	}

	if (node)
	{
		// set transformation correctly into node.
		node->setPosition(transform.getTranslation());
		node->setRotation(transform.getRotationDegrees());
		node->setScale(transform.getScale());
		node->updateAbsolutePosition();

		node->setName(name.c_str());
	}
}


//! reads a <lookat> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readLookAtNode(io::IXMLReaderUTF8* reader)
{
	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading look at node");
	#endif

	f32 floats[9];
	readFloatsInsideElement(reader, floats, 9);

	mat.buildCameraLookAtMatrixLH(
		core::vector3df(floats[0], floats[1], floats[2]),
		core::vector3df(floats[3], floats[4], floats[5]),
		core::vector3df(floats[6], floats[7], floats[8]));

	return mat;
}


//! reads a <skew> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readSkewNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading skew node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	f32 floats[7]; // angle rotation-axis translation-axis
	readFloatsInsideElement(reader, floats, 7);

	// build skew matrix from these 7 floats
	core::quaternion q;
	q.fromAngleAxis(floats[0]*core::DEGTORAD, core::vector3df(floats[1], floats[2], floats[3]));
	q.getMatrix(mat);

	if (floats[4]==1.f) // along x-axis
	{
		mat[4]=0.f;
		mat[6]=0.f;
		mat[8]=0.f;
		mat[9]=0.f;
	}
	else
	if (floats[5]==1.f) // along y-axis
	{
		mat[1]=0.f;
		mat[2]=0.f;
		mat[8]=0.f;
		mat[9]=0.f;
	}
	else
	if (floats[6]==1.f) // along z-axis
	{
		mat[1]=0.f;
		mat[2]=0.f;
		mat[4]=0.f;
		mat[6]=0.f;
	}

	return mat;
}


//! reads a <boundingbox> element and its content and stores it in bbox
void CColladaFileLoader::readBboxNode(io::IXMLReaderUTF8* reader,
		core::aabbox3df& bbox)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading boundingbox node");
	#endif

	bbox.reset(core::aabbox3df());

	if (reader->isEmptyElement())
		return;

	f32 floats[3];

	while(reader->read())
	{
		if (reader->getNodeType() == io::EXN_ELEMENT)
		{
			if (minNodeName == reader->getNodeName())
			{
				readFloatsInsideElement(reader, floats, 3);
				bbox.MinEdge.set(floats[0], floats[1], floats[2]);
			}
			else
			if (maxNodeName == reader->getNodeName())
			{
				readFloatsInsideElement(reader, floats, 3);
				bbox.MaxEdge.set(floats[0], floats[1], floats[2]);
			}
			else
				skipSection(reader, true); // ignore all other sections
		}
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
		{
			if (bboxNodeName == reader->getNodeName())
				break;
		}
	}
}


//! reads a <matrix> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readMatrixNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading matrix node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	readFloatsInsideElement(reader, mat.pointer(), 16);

	return mat;
}


//! reads a <perspective> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readPerspectiveNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading perspective node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	f32 floats[1];
	readFloatsInsideElement(reader, floats, 1);

	// TODO: build perspecitve matrix from this float

	os::Printer::log("COLLADA loader warning: <perspective> not implemented yet.", ELL_WARNING);

	return mat;
}


//! reads a <rotate> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readRotateNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading rotate node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	f32 floats[4];
	readFloatsInsideElement(reader, floats, 4);

	if (!core::iszero(floats[3]))
	{
		core::quaternion q;
		q.fromAngleAxis(floats[3]*core::DEGTORAD, core::vector3df(floats[0], floats[1], floats[2]));
		return q.getMatrix();
	}
	else
		return core::IdentityMatrix;
}


//! reads a <scale> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readScaleNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading scale node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	f32 floats[3];
	readFloatsInsideElement(reader, floats, 3);

	mat.setScale(core::vector3df(floats[0], floats[1], floats[2]));

	return mat;
}


//! reads a <translate> element and its content and creates a matrix from it
core::matrix4 CColladaFileLoader::readTranslateNode(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading translate node");
	#endif

	core::matrix4 mat;
	if (reader->isEmptyElement())
		return mat;

	f32 floats[3];
	readFloatsInsideElement(reader, floats, 3);

	mat.setTranslation(core::vector3df(floats[0], floats[1], floats[2]));

	return mat;
}


//! reads a <instance> node and creates a scene node from it
void CColladaFileLoader::readInstanceNode(io::IXMLReaderUTF8* reader, scene::ISceneNode* parent,
	scene::ISceneNode** outNode)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading instance");
	#endif

	// find prefab of the specified id
	core::stringc url = reader->getAttributeValue("url");
	uriToId(url);

	if (CreateInstances)
	for (u32 i=0; i<Prefabs.size(); ++i)
		if (url == Prefabs[i]->getId())
		{
			*outNode = Prefabs[i]->addInstance(parent, SceneManager);
			if (*outNode)
				(*outNode)->setName(reader->getAttributeValue("id"));
			return;
		}
}

//! reads a <camera> element and stores it as prefab
void CColladaFileLoader::readCameraPrefab(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading camera prefab");
	#endif

	CCameraPrefab* prefab = new CCameraPrefab(reader->getAttributeValue("id"));

	if (!reader->isEmptyElement())
	{
		// read techniques optics and imager (the latter is completely ignored, though)
		readColladaParameters(reader, cameraPrefabName);

		SColladaParam* p;

		// XFOV not yet supported
		p = getColladaParameter(ECPN_YFOV);
		if (p && p->Type == ECPT_FLOAT)
			prefab->YFov = p->Floats[0];

		p = getColladaParameter(ECPN_ZNEAR);
		if (p && p->Type == ECPT_FLOAT)
			prefab->ZNear = p->Floats[0];

		p = getColladaParameter(ECPN_ZFAR);
		if (p && p->Type == ECPT_FLOAT)
			prefab->ZFar = p->Floats[0];
		// orthographic camera uses LEFT, RIGHT, TOP, and BOTTOM
	}

	Prefabs.push_back(prefab);
}


//! reads a <image> element and stores it in the image section
void CColladaFileLoader::readImage(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading image");
	#endif

	SColladaImage image;
	image.Id = reader->getAttributeValue("id");
	image.Filename = reader->getAttributeValue("source");

	// add image to list of loaded images.
	Images.push_back(image);
}


//! reads a <texture> element and stores it in the texture section
void CColladaFileLoader::readTexture(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading texture");
	#endif

	SColladaTexture texture;
	texture.Id = reader->getAttributeValue("id");

	if (!reader->isEmptyElement())
	{
		readColladaInputs(reader, textureSectionName);
		SColladaInput* input = getColladaInput(ECIS_IMAGE);
		if (input)
		{
			core::stringc imageName = input->Source;
			uriToId(imageName);
			for (u32 i=0; i<Images.size(); ++i)
				if ((imageName == Images[i].Id) && Images[i].Filename.size())
				{
					texture.Texture = Driver->getTexture(Images[i].Filename.c_str());
					break;
				}
		}
	}

	// add texture to list of loaded textures.
	Textures.push_back(texture);
}


//! reads a <material> element and stores it in the material section
void CColladaFileLoader::readMaterial(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading material");
	#endif

	SColladaMaterial material;
	material.Id = reader->getAttributeValue("id");

	if (!reader->isEmptyElement())
	{
		readColladaInputs(reader, materialSectionName);
		SColladaInput* input = getColladaInput(ECIS_TEXTURE);
		if (input)
		{
			core::stringc textureName = input->Source;
			uriToId(textureName);
			for (u32 i=0; i<Textures.size(); ++i)
				if (textureName == Textures[i].Id)
				{
					material.Mat.setTexture(0, Textures[i].Texture);
					break;
				}
		}

		//does not work because the wrong start node is chosen due to reading of inputs before
#if 0
		readColladaParameters(reader, materialSectionName);

		SColladaParam* p;

		p = getColladaParameter(ECPN_AMBIENT);
		if (p && p->Type == ECPT_FLOAT3)
			material.Mat.AmbientColor = video::SColorf(p->Floats[0],p->Floats[1],p->Floats[2]).toSColor();
		p = getColladaParameter(ECPN_DIFFUSE);
		if (p && p->Type == ECPT_FLOAT3)
			material.Mat.DiffuseColor = video::SColorf(p->Floats[0],p->Floats[1],p->Floats[2]).toSColor();
		p = getColladaParameter(ECPN_SPECULAR);
		if (p && p->Type == ECPT_FLOAT3)
			material.Mat.DiffuseColor = video::SColorf(p->Floats[0],p->Floats[1],p->Floats[2]).toSColor();
		p = getColladaParameter(ECPN_SHININESS);
		if (p && p->Type == ECPT_FLOAT)
			material.Mat.Shininess = p->Floats[0];
#endif
	}

	// add material to list of loaded materials.
	Materials.push_back(material);
}

//! reads a <geometry> element and stores it as mesh if possible
void CColladaFileLoader::readGeometry(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading geometry");
	#endif

	core::stringc id = reader->getAttributeValue("id");

	core::stringc VertexPositionSource; // each mesh has exactly one <vertex> member, containing
										// a POSITION input. This string stores the source of this input.
	core::array<SSource> sources;
	bool okToReadArray = false;

	SAnimatedMesh* amesh = new SAnimatedMesh();
	scene::SMesh* mesh = new SMesh();
	amesh->addMesh(mesh);

	// handles geometry node and the mesh childs in this loop
	// read sources with arrays and accessor for each mesh
	if (!reader->isEmptyElement())
	while(reader->read())
	{
		if (reader->getNodeType() == io::EXN_ELEMENT)
		{
			const char* nodeName = reader->getNodeName();
			if (meshSectionName == nodeName)
			{
				// inside a mesh section. Don't have to do anything here.
			}
			else
			if (sourceSectionName == nodeName)
			{
				// create a new source
				sources.push_back(SSource());
				sources.getLast().Id = reader->getAttributeValue("id");

				#ifdef COLLADA_READER_DEBUG
				os::Printer::log("Loaded source", sources.getLast().Id.c_str());
				#endif
			}
			else
			if (arraySectionName == nodeName) // child of source
			{
				// create a new array and read it.
				if (!sources.empty())
				{
					sources.getLast().Array.Name = reader->getAttributeValue("id");

					int count = reader->getAttributeValueAsInt("count");
					sources.getLast().Array.Data.set_used(count); // pre allocate

					// check if type of array is ok
					const char* type = reader->getAttributeValue("type");
					okToReadArray = (!strcmp("float", type) || !strcmp("int", type));

					#ifdef COLLADA_READER_DEBUG
					os::Printer::log("Read array", sources.getLast().Array.Name.c_str());
					#endif
				}
				#ifdef COLLADA_READER_DEBUG
				else
					os::Printer::log("Warning, array found, but no source",
						reader->getAttributeValue("id"));
				#endif

			}
			else
			if (accessorSectionName == nodeName) // child of source (below a technique tag)
			{
				SAccessor accessor;
				accessor.Count = reader->getAttributeValueAsInt("count");
				accessor.Offset = reader->getAttributeValueAsInt("offset");
				accessor.Stride = reader->getAttributeValueAsInt("stride");
				if (accessor.Stride == 0)
					accessor.Stride = 1;

				// the accessor contains some information on how to access (boi!) the array,
				// the info is stored in collada style parameters, so just read them.
				readColladaParameters(reader, accessorSectionName);
				if (!sources.empty())
				{
					sources.getLast().Accessors.push_back(accessor);
					sources.getLast().Accessors.getLast().Parameters = Parameters;
				}
			}
			else
			if (verticesSectionName == nodeName)
			{
				// read vertex input position source
				readColladaInputs(reader, verticesSectionName);
				SColladaInput* input = getColladaInput(ECIS_POSITION);
				if (input)
					VertexPositionSource = input->Source;
			}
			else
			// lines and linestrips missing
			if (polygonsSectionName == nodeName)
			{
				// read polygons section
				readPolygonSection(reader, VertexPositionSource, sources, mesh);
			}
			else
			// triangles, trifans, and tristrips missing
			if (extraNodeName == reader->getNodeName())
				skipSection(reader, false);
			else
			{
//				os::Printer::log("COLLADA loader warning: Wrong tag usage found", reader->getNodeName(), ELL_WARNING);
				skipSection(reader, true); // ignore all other sections
			}

		} // end if node type is element
		else
		if (reader->getNodeType() == io::EXN_TEXT)
		{
			// read array data
			if (okToReadArray && !sources.empty())
			{
				core::array<f32>& a = sources.getLast().Array.Data;
				core::stringc data = reader->getNodeData();
				const c8* p = &data[0];

				for (u32 i=0; i<a.size(); ++i)
				{
					findNextNoneWhiteSpace(&p);
					if (*p)
						a[i] = readFloat(&p);
					else
						a[i] = 0.0f;
				}
			} // end reading array

			okToReadArray = false;

		} // end if node type is text
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
		{
			if (geometrySectionName == reader->getNodeName())
			{
				// end of geometry section reached, cancel out
				break;
			}
		}
	} // end while reader->read();

	// add mesh as geometry

	mesh->recalculateBoundingBox();

	// create virtual file name
	core::stringc filename = CurrentlyLoadingMesh;
	filename += '#';
	filename += id;

	// add to scene manager
	if (LoadedMeshCount)
	{
		SceneManager->getMeshCache()->addMesh(filename.c_str(), amesh);
		os::Printer::log("Added COLLADA mesh", filename.c_str());
	}
	else
	{
		FirstLoadedMeshName = filename;
		FirstLoadedMesh = amesh;
		FirstLoadedMesh->grab();
	}

	++LoadedMeshCount;
	mesh->drop();
	amesh->drop();

	// create geometry prefab
	CGeometryPrefab* prefab = new CGeometryPrefab(id.c_str());
	prefab->Mesh = mesh;
	Prefabs.push_back(prefab);

	// store as dummy mesh if no instances will be created
	if (!CreateInstances && !DummyMesh)
	{
		DummyMesh = amesh;
		DummyMesh->grab();
	}
}


struct SInputSlot
{
	f32* Data; // Pointer to source data
	ECOLLADA_INPUT_SEMANTIC Semantic;
};

struct SPolygon
{
	core::array<s32> Indices;
};

//! reads a polygons section and creates a mesh from it
void CColladaFileLoader::readPolygonSection(io::IXMLReaderUTF8* reader,
	const core::stringc& vertexPositionSource, core::array<SSource>& sources,
	scene::SMesh* mesh)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading polygon section");
	#endif

	core::stringc materialName = reader->getAttributeValue("material");
	uriToId(materialName);
	video::SMaterial mat;

	for (u32 matnum=0; matnum<Materials.size(); ++matnum)
	{
		if (materialName == Materials[matnum].Id)
		{
			mat = Materials[matnum].Mat;
			break;
		}
	}

	// int polygonCount = reader->getAttributeValueAsInt("count"); // Not useful because it only determines the number of p tags, not the number of tris
	core::array<SInputSlot> slots;
	core::array<SPolygon> polygons;
	bool parsePolygonOK = false;
	u32 inputSemanticCount = 0;

	// read all <input> and
	if (!reader->isEmptyElement())
	while(reader->read())
	{
		const char* nodeName = reader->getNodeName();

		if (reader->getNodeType() == io::EXN_ELEMENT)
		{
			// polygon node may contain params
			if (inputTagName == nodeName)
			{
				// read input tag
				readColladaInput(reader);

				// create new input slot
				if (!Inputs.empty())
				{
					SInputSlot slot;
					slot.Data=0;
					slot.Semantic = Inputs.getLast().Semantic;

					core::stringc sourceArrayURI;

					// get input source array id, if it is a vertex input, take
					// the <vertex><input>-source attribute.
					if (slot.Semantic == ECIS_VERTEX)
						sourceArrayURI = vertexPositionSource;
					else
						sourceArrayURI = Inputs.getLast().Source;

					uriToId(sourceArrayURI);

					// find source array (we'll ignore accessors for this implementation)
					u32 s;
					for (s=0; s<sources.size(); ++s)
						if (sources[s].Id == sourceArrayURI)
						{
							// slot found
							slot.Data = sources[s].Array.Data.pointer();
							break;
						}

					if (s == sources.size())
						os::Printer::log("COLLADA Warning, polygon input source not found",
							sourceArrayURI.c_str());
					else
						slots.push_back(slot);

					#ifdef COLLADA_READER_DEBUG
					// print slot
					// core::stringc tmp = "Added slot ";
					// tmp += inputSemanticNames[Inputs.getLast().Semantic];
					// tmp += " sourceArray:";
					// tmp += sourceArrayURI;
					// os::Printer::log(tmp.c_str());
					#endif

					++inputSemanticCount;
				}
			} // end is input node
			else
			if (primitivesName == nodeName)
			{
				parsePolygonOK = true;
				polygons.push_back(SPolygon());
			} // end  is polygon node

		} // end is element node
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
		{
			if (primitivesName == nodeName)
				parsePolygonOK = false; // end parsing a polygon
			else
			if (polygonsSectionName == nodeName)
				break; // cancel out and create mesh

		} // end is element end
		else
		if (reader->getNodeType() == io::EXN_TEXT)
		{
			if (parsePolygonOK && polygons.size())
			{
				// parse this text and add it to the last polygon data
				core::stringc data = reader->getNodeData();
				const c8* p = &data[0];
				SPolygon& poly = polygons.getLast();
				while(*p)
				{
					findNextNoneWhiteSpace(&p);
					if (*p)
						poly.Indices.push_back(readInt(&p));
				}
				parsePolygonOK = false;
			}
		}

	} // end while reader->read()

	if (inputSemanticCount == 0 || inputSemanticCount != slots.size())
		return; // we cannot create the mesh if one of the input semantics wasn't found.

	if (!polygons.size())
		return; // cancel if there are no polygons anyway.

	// analyze content of slots to create a fitting mesh buffer

	u32 u;
	u32 textureCoordSetCount = 0;
	bool normalSlotCount = false;
	u32 secondTexCoordSetIndex = 0xFFFFFFFF;

	for (u=0; u<slots.size(); ++u)
	{
		if (slots[u].Semantic == ECIS_TEXCOORD || slots[u].Semantic == ECIS_UV )
		{
			++textureCoordSetCount;

			if (textureCoordSetCount==2)
				secondTexCoordSetIndex = u;
		}
		else
		if (slots[u].Semantic == ECIS_NORMAL)
			normalSlotCount=true;
	}

	// if there is more than one texture coordinate set, create a lightmap mesh buffer,
	// otherwise use a standard mesh buffer

	scene::IMeshBuffer* buffer = 0;

	if ( textureCoordSetCount == 0 )
	{
		// standard mesh buffer

		scene::SMeshBuffer* mbuffer = new SMeshBuffer();
		mbuffer->Material=mat;
		buffer = mbuffer;

		for (u32 i=0; i<polygons.size(); ++i)
		{
			u32 vertexCount = polygons[i].Indices.size() / inputSemanticCount;

			// for all index/semantic groups
			for (u32 v=0; v<polygons[i].Indices.size(); v+=inputSemanticCount)
			{
				video::S3DVertex vtx;
				vtx.Color.set(255,255,255,255);

				// for all input semantics
				for (u32 k=0; k<slots.size(); ++k)
				{
					if (!slots[k].Data)
						continue;
					// build vertex from input semantics.

					s32 idx = polygons[i].Indices[v+k];

					switch(slots[k].Semantic)
					{
					case ECIS_POSITION:
					case ECIS_VERTEX:
						vtx.Pos.X = slots[k].Data[(idx*3)+0];
						vtx.Pos.Y = slots[k].Data[(idx*3)+1];
						vtx.Pos.Z = slots[k].Data[(idx*3)+2];
						break;
					case ECIS_NORMAL:
						vtx.Normal.X = slots[k].Data[(idx*3)+0];
						vtx.Normal.Y = slots[k].Data[(idx*3)+1];
						vtx.Normal.Z = slots[k].Data[(idx*3)+2];
						break;
					case ECIS_TEXCOORD:
					case ECIS_UV:
						vtx.TCoords.X = slots[k].Data[(idx*2)+0];
						vtx.TCoords.Y = slots[k].Data[(idx*2)+1];
						break;
					case ECIS_TANGENT:
						break;
					default:
						break;
					}
				}

				mbuffer->Vertices.push_back(vtx);

			} // end for all vertices

			// add vertex indices
			const u32 oldVertexCount = mbuffer->Vertices.size() - vertexCount;
			for (u32 face=0; face<vertexCount-2; ++face)
			{
				mbuffer->Indices.push_back(oldVertexCount + 0);
				mbuffer->Indices.push_back(oldVertexCount + 1 + face);
				mbuffer->Indices.push_back(oldVertexCount + 2 + face);
			}

		} // end for all polygons
	}
	else
	{
		// lightmap mesh buffer

		scene::SMeshBufferLightMap* mbuffer = new SMeshBufferLightMap();
		mbuffer->Material=mat;
		buffer = mbuffer;

		for (u32 i=0; i<polygons.size(); ++i)
		{
			u32 vertexCount = polygons[i].Indices.size() / inputSemanticCount;

			// for all vertices in array
			for (u32 v=0; v<polygons[i].Indices.size(); v+=inputSemanticCount)
			{
				video::S3DVertex2TCoords vtx;
				vtx.Color.set(100,255,255,255);

				// for all input semantics
				for (u32 k=0; k<slots.size(); ++k)
				{
					// build vertex from input semantics.

					u32 idx = polygons[i].Indices[v+k];

					switch(slots[k].Semantic)
					{
					case ECIS_POSITION:
					case ECIS_VERTEX:
						vtx.Pos.X = slots[k].Data[(idx*3)+0];
						vtx.Pos.Y = slots[k].Data[(idx*3)+1];
						vtx.Pos.Z = slots[k].Data[(idx*3)+2];
						break;
					case ECIS_NORMAL:
						vtx.Normal.X = slots[k].Data[(idx*3)+0];
						vtx.Normal.Y = slots[k].Data[(idx*3)+1];
						vtx.Normal.Z = slots[k].Data[(idx*3)+2];
						break;
					case ECIS_TEXCOORD:
					case ECIS_UV:
						if (k==secondTexCoordSetIndex)
						{
							vtx.TCoords2.X = slots[k].Data[(idx*2)+0];
							vtx.TCoords2.Y = slots[k].Data[(idx*2)+1];
						}
						else
						{
							vtx.TCoords.X = slots[k].Data[(idx*2)+0];
							vtx.TCoords.Y = slots[k].Data[(idx*2)+1];
						}
						break;
					case ECIS_TANGENT:
						break;
					default:
						break;
					}
				}

				mbuffer->Vertices.push_back(vtx);

			} // end for all vertices

			// add vertex indices
			const u32 oldVertexCount = mbuffer->Vertices.size() - vertexCount;
			for (u32 face=0; face<vertexCount-2; ++face)
			{
				mbuffer->Indices.push_back(oldVertexCount + 0);
				mbuffer->Indices.push_back(oldVertexCount + 1 + face);
				mbuffer->Indices.push_back(oldVertexCount + 2 + face);
			}

		} // end for all polygons
	}

	// calculate normals if there is no slot for it

	if (!normalSlotCount)
		SceneManager->getMeshManipulator()->recalculateNormals(buffer);

	// recalculate bounding box
	buffer->recalculateBoundingBox();

	// add mesh buffer
	mesh->addMeshBuffer(buffer);

	buffer->drop();
}


//! reads a <light> element and stores it as prefab
void CColladaFileLoader::readLightPrefab(io::IXMLReaderUTF8* reader)
{
	#ifdef COLLADA_READER_DEBUG
	os::Printer::log("COLLADA reading light prefab");
	#endif

	CLightPrefab* prefab = new CLightPrefab(reader->getAttributeValue("id"));

	if (!reader->isEmptyElement())
	{
		readColladaParameters(reader, lightPrefabName);

		SColladaParam* p = getColladaParameter(ECPN_COLOR);
		if (p && p->Type == ECPT_FLOAT3)
			prefab->LightData.DiffuseColor.set(p->Floats[0], p->Floats[1], p->Floats[2]);
	}

	Prefabs.push_back(prefab);
}


//! returns a collada parameter or none if not found
SColladaParam* CColladaFileLoader::getColladaParameter(ECOLLADA_PARAM_NAME name)
{
	for (u32 i=0; i<Parameters.size(); ++i)
		if (Parameters[i].Name == name)
			return &Parameters[i];

	return 0;
}

//! returns a collada input or none if not found
SColladaInput* CColladaFileLoader::getColladaInput(ECOLLADA_INPUT_SEMANTIC input)
{
	for (u32 i=0; i<Inputs.size(); ++i)
		if (Inputs[i].Semantic == input)
			return &Inputs[i];

	return 0;
}


//! reads a collada input tag and adds it to the input parameter
void CColladaFileLoader::readColladaInput(io::IXMLReaderUTF8* reader)
{
	// parse param
	SColladaInput p;

	// get type
	core::stringc semanticName = reader->getAttributeValue("semantic");
	for (u32 i=0; inputSemanticNames[i]; ++i)
		if (semanticName == inputSemanticNames[i])
		{
			p.Semantic = (ECOLLADA_INPUT_SEMANTIC)i;
			break;
		}

	// get source
	p.Source = reader->getAttributeValue("source");

	// add input
	Inputs.push_back(p);
}

//! parses all collada inputs inside an element and stores them in Inputs
void CColladaFileLoader::readColladaInputs(io::IXMLReaderUTF8* reader, const core::stringc& parentName)
{
	Inputs.clear();

	while(reader->read())
	{
		if (reader->getNodeType() == io::EXN_ELEMENT &&
			inputTagName == reader->getNodeName())
		{
			readColladaInput(reader);
		}
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
		{
			if (parentName == reader->getNodeName())
				return; // end of parent reached
		}

	} // end while reader->read();
}

//! parses all collada parameters inside an element and stores them in Parameters
void CColladaFileLoader::readColladaParameters(io::IXMLReaderUTF8* reader,
		const core::stringc& parentName)
{
	Parameters.clear();

	const char* const paramNames[] = {"COLOR", "AMBIENT", "DIFFUSE",
		"SPECULAR", "SHININESS", "YFOV", "ZNEAR", "ZFAR", 0};

	const char* const typeNames[] = {"float", "float2", "float3", 0};

	while(reader->read())
	{
		const char* nodeName = reader->getNodeName();
		if (reader->getNodeType() == io::EXN_ELEMENT &&
			paramTagName == nodeName)
		{
			// parse param
			SColladaParam p;

			// get type
			u32 i;
			core::stringc typeName = reader->getAttributeValue("type");
			for (i=0; typeNames[i]; ++i)
				if (typeName == typeNames[i])
				{
					p.Type = (ECOLLADA_PARAM_TYPE)i;
					break;
				}

			// get name
			core::stringc nameName = reader->getAttributeValue("name");
			for (i=0; typeNames[i]; ++i)
				if (nameName == paramNames[i])
				{
					p.Name = (ECOLLADA_PARAM_NAME)i;
					break;
				}

			// read parameter data inside parameter tags
			switch(p.Type)
			{
				case ECPT_FLOAT:
				case ECPT_FLOAT2:
				case ECPT_FLOAT3:
				case ECPT_FLOAT4:
					readFloatsInsideElement(reader, p.Floats, p.Type - ECPT_FLOAT + 1);
					break;

				// TODO: other types of data (ints, bools or whatever)
				default:
					break;
			}

			// add param
			Parameters.push_back(p);
		}
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
		{
			if (parentName == reader->getNodeName())
				return; // end of parent reached
		}

	} // end while reader->read();
}


//! parses a float from a char pointer and moves the pointer
//! to the end of the parsed float
inline f32 CColladaFileLoader::readFloat(const c8** p)
{
	f32 ftmp;
	*p = core::fast_atof_move(*p, ftmp);
	return ftmp;
}

//! parses an int from a char pointer and moves the pointer to
//! the end of the parsed float
inline s32 CColladaFileLoader::readInt(const c8** p)
{
	return (s32)readFloat(p);
}

//! places pointer to next begin of a token
void CColladaFileLoader::findNextNoneWhiteSpace(const c8** start)
{
	const c8* p = *start;

	while(*p && (*p==' ' || *p=='\n' || *p=='\r' || *p=='\t'))
		++p;

	// TODO: skip comments <!-- -->

	*start = p;
}


//! reads floats from inside of xml element until end of xml element
void CColladaFileLoader::readFloatsInsideElement(io::IXMLReaderUTF8* reader, f32* floats, u32 count)
{
	if (reader->isEmptyElement())
		return;

	while(reader->read())
	{
		// TODO: check for comments inside the element
		// and ignore them.

		if (reader->getNodeType() == io::EXN_TEXT)
		{
			// parse float data
			core::stringc data = reader->getNodeData();
			const c8* p = &data[0];

			for (u32 i=0; i<count; ++i)
			{
				findNextNoneWhiteSpace(&p);
				if (*p)
					floats[i] = readFloat(&p);
				else
					floats[i] = 0.0f;
			}
		}
		else
		if (reader->getNodeType() == io::EXN_ELEMENT_END)
			break; // end parsing text
	}
}


//! clears all loaded data
void CColladaFileLoader::clearData()
{
	// delete all prefabs

	for (u32 i=0; i<Prefabs.size(); ++i)
		Prefabs[i]->drop();

	Prefabs.clear();

	// clear all parameters
	Parameters.clear();

	// clear all materials
	Images.clear();

	// clear all materials
	Textures.clear();

	// clear all materials
	Materials.clear();

	// clear all inputs
	Inputs.clear();
}

//! changes the XML URI into an internal id
void CColladaFileLoader::uriToId(core::stringc& str)
{
	// currently, we only remove the # from the begin if there
	// because we simply don't support referencing other files.
	if (!str.size())
		return;

	if (str[0] == '#')
		str.erase(0);
}




} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_COLLADA_LOADER_
