// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SCENE_PARAMETERS_H_INCLUDED__
#define __I_SCENE_PARAMETERS_H_INCLUDED__

/*! \file SceneParameters.h
	\brief Header file containing all scene parameters for modifying mesh loading etc.

	This file includes all parameter names which can be set using ISceneManager::getParameters()
	to modify the behaviour of plugins and mesh loaders.
*/

namespace irr
{
namespace scene
{
	//! Name of the parameter for changing how Irrlicht handles the ZWrite flag for transparent (blending) materials
	/** The default behavior in Irrlicht is to disable writing to the
	z-buffer for all really transparent, i.e. blending materials. This
	avoids problems with intersecting faces, but can also break renderings.
	If transparent materials should use the SMaterial flag for ZWriteEnable
	just as other material types use this attribute.
	Use it like this:
	\code
	SceneManager->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);
	\endcode
	**/
	const c8* const ALLOW_ZWRITE_ON_TRANSPARENT = "Allow_ZWrite_On_Transparent";

	//! Name of the parameter for changing the texture path of the built-in csm loader.
	/** Use it like this:
	\code
	SceneManager->getParameters()->setAttribute(scene::CSM_TEXTURE_PATH, "path/to/your/textures");
	\endcode
	**/
	const c8* const CSM_TEXTURE_PATH = "CSM_TexturePath";

	//! Name of the parameter for changing the texture path of the built-in lmts loader.
	/** Use it like this:
	\code
	SceneManager->getParameters()->setAttribute(scene::LMTS_TEXTURE_PATH, "path/to/your/textures");
	\endcode
	**/
	const c8* const LMTS_TEXTURE_PATH = "LMTS_TexturePath";

	//! Name of the parameter for changing the texture path of the built-in my3d loader.
	/** Use it like this:
	\code
	SceneManager->getParameters()->setAttribute(scene::MY3D_TEXTURE_PATH, "path/to/your/textures");
	\endcode
	**/
	const c8* const MY3D_TEXTURE_PATH = "MY3D_TexturePath";

	//! Name of the parameter specifying the COLLADA mesh loading mode
	/**
	Specifies if the COLLADA loader should create instances of the models, lights and
	cameras when loading COLLADA meshes. By default, this is set to false. If this is
	set to true, the ISceneManager::getMesh() method will only return a pointer to a
	dummy mesh and create instances of all meshes and lights and cameras in the collada
	file by itself. Example:
	\code
	SceneManager->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);
	\endcode
	*/
	const c8* const COLLADA_CREATE_SCENE_INSTANCES = "COLLADA_CreateSceneInstances";

	//! Name of the parameter for changing the texture path of the built-in DMF loader.
	/** Use it like this:
	\code
	SceneManager->getStringParameters()->setAttribute(scene::DMF_TEXTURE_PATH, "path/to/your/textures");
	\endcode
	**/
	const c8* const DMF_TEXTURE_PATH = "DMF_TexturePath";

	//! Name of the parameter for preserving DMF textures dir structure with built-in DMF loader.
	/** Use it like this:
	\code
	//this way you won't use this setting
	SceneManager->getParameters()->setAttribute(scene::DMF_USE_MATERIALS_DIRS, false);
	\endcode
	\code
	//this way you'll use this setting
	SceneManager->getParameters()->setAttribute(scene::DMF_USE_MATERIALS_DIRS, true);
	\endcode
	**/
	const c8* const DMF_USE_MATERIALS_DIRS = "DMF_MaterialsDir";

	//! Name of the parameter for setting reference value of alpha in transparent materials.
	/** Use it like this:
	\code
	//this way you'll set alpha ref to 0.1
	SceneManager->getParameters()->setAttribute(scene::DMF_ALPHA_CHANNEL_REF, 0.1);
	\endcode
	**/
	const c8* const DMF_ALPHA_CHANNEL_REF = "DMF_AlphaRef";

	//! Name of the parameter for choose to flip or not tga files.
	/** Use it like this:
	\code
	//this way you'll choose to flip alpha textures
	SceneManager->()->setAttribute(scene::DMF_FLIP_ALPHA_TEXTURES, true);
	\endcode
	**/
	const c8* const DMF_FLIP_ALPHA_TEXTURES = "DMF_FlipAlpha";


	//! Flag set as parameter when the scene manager is used as editor
	/** In this way special animators like deletion animators can be stopped from
	deleting scene nodes for example */
	const c8* const IRR_SCENE_MANAGER_IS_EDITOR = "IRR_Editor";

} // end namespace scene
} // end namespace irr

#endif

