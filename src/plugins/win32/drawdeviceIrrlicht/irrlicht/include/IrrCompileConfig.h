// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_COMPILE_CONFIG_H_INCLUDED__
#define __IRR_COMPILE_CONFIG_H_INCLUDED__

//! Irrlicht SDK Version
#define IRRLICHT_SDK_VERSION "1.4.2"

//! The defines for different operating system are:
//! _IRR_XBOX_PLATFORM_ for XBox
//! _IRR_WINDOWS_ for all irrlicht supported Windows versions
//! _IRR_WINDOWS_API_ for Windows or XBox
//! _IRR_LINUX_PLATFORM_ for Linux (it is defined here if no other os is defined)
//! _IRR_SOLARIS_PLATFORM_ for Solaris
//! _IRR_OSX_PLATFORM_ for Apple systems running OSX
//! _IRR_POSIX_API_ for Posix compatible systems
//! _IRR_USE_SDL_DEVICE_ for platform independent SDL framework
//! _IRR_USE_WINDOWS_DEVICE_ for Windows API based device
//! _IRR_USE_LINUX_DEVICE_ for X11 based device
//! _IRR_USE_OSX_DEVICE_ for Cocoa native windowing on OSX
//! Note: PLATFORM defines the OS specific layer, API can groups several platforms
//! DEVICE is the windowing system used, several PLATFORMs support more than one DEVICE
//! Moreover, the DEVICE defined here is not directly related to the Irrlicht devices created in the app (but may depend on each other).

//#define _IRR_USE_SDL_DEVICE_ 1

//! WIN32 for Windows32
//! WIN64 for Windows64
// The windows platform and API support SDL and WINDOW device
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define _IRR_WINDOWS_
#define _IRR_WINDOWS_API_
#ifndef _IRR_USE_SDL_DEVICE_
#define _IRR_USE_WINDOWS_DEVICE_
#endif
#endif

// XBox only suppots the native Window stuff
#if defined(_XBOX)
#define _IRR_XBOX_PLATFORM_
#define _IRR_WINDOWS_API_
#define _IRR_USE_WINDOWS_DEVICE_
#endif

#if defined(__APPLE__) || defined(MACOSX)
#if !defined(MACOSX)
#define MACOSX // legacy support
#endif
#define _IRR_OSX_PLATFORM_
#if !defined(_IRR_USE_LINUX_DEVICE_) // for X11 windowing declare this
#define _IRR_USE_OSX_DEVICE_
#endif
#endif

#if !defined(_IRR_WINDOWS_API_) && !defined(_IRR_OSX_PLATFORM_)
#if defined(__sparc__) || defined(__sun__)
#define __BIG_ENDIAN__
#define _IRR_SOLARIS_PLATFORM_
#else
#define _IRR_LINUX_PLATFORM_
#endif
#define _IRR_POSIX_API_

#ifndef _IRR_USE_SDL_DEVICE_
#define _IRR_USE_LINUX_DEVICE_
#endif
#endif

#include <stdio.h> // TODO: Although included elsewhere this is required at least for mingw

//! Define _IRR_COMPILE_WITH_DIRECT3D_8_ and _IRR_COMPILE_WITH_DIRECT3D_9_ to
//! compile the Irrlicht engine with Direct3D8 and/or DIRECT3D9.
/** If you only want to use the software device or opengl this can be useful.
This switch is mostly disabled because people do not get the g++ compiler compile
directX header files, and directX is only available on windows platforms. If you
are using Dev-Cpp, and want to compile this using a DX dev pack, you can define
_IRR_COMPILE_WITH_DX9_DEV_PACK_. So you simply need to add something like this
to the compiler settings: -DIRR_COMPILE_WITH_DX9_DEV_PACK
and this to the linker settings: -ld3dx9 -ld3dx8 **/
#if defined(_IRR_WINDOWS_API_) && (!defined(__GNUC__) || defined(IRR_COMPILE_WITH_DX9_DEV_PACK))

//#define _IRR_COMPILE_WITH_DIRECT3D_8_
#define _IRR_COMPILE_WITH_DIRECT3D_9_

#endif

//! Define _IRR_COMPILE_WITH_OPENGL_ to compile the Irrlicht engine with OpenGL.
/** If you do not wish the engine to be compiled with OpengGL, comment this
define out. */
//#define _IRR_COMPILE_WITH_OPENGL_

//! Define _IRR_COMPILE_WITH_SOFTWARE_ to compile the Irrlicht engine with software driver
/** If you do not need the software driver, or want to use Burning's Video instead,
comment this define out */
//#define _IRR_COMPILE_WITH_SOFTWARE_

//! Define _IRR_COMPILE_WITH_BURNINGSVIDEO_ to compile the Irrlicht engine with Burning's video driver
/** If you do not need this software driver, you can comment this define out. */
//#define _IRR_COMPILE_WITH_BURNINGSVIDEO_

//! Define _IRR_COMPILE_WITH_X11_ to compile the Irrlicht engine with X11 support.
/** If you do not wish the engine to be compiled with X11, comment this
define out. */
// Only used in LinuxDevice.
//#define _IRR_COMPILE_WITH_X11_

//! Define _IRR_OPENGL_USE_EXTPOINTER_ if the OpenGL renderer should use OpenGL extensions via function pointers.
/** On some systems there is no support for the dynamic extension of OpenGL
	via function pointers such that this has to be undef'ed. */
#if !defined(_IRR_OSX_PLATFORM_) && !defined(_IRR_SOLARIS_PLATFORM_)
#define _IRR_OPENGL_USE_EXTPOINTER_
#endif

//! On some Linux systems the XF86 vidmode extension or X11 RandR are missing. Use these flags
//! to remove the dependencies such that Irrlicht will compile on those systems, too.
#if defined(_IRR_LINUX_PLATFORM_)
#define _IRR_LINUX_X11_VIDMODE_
//#define _IRR_LINUX_X11_RANDR_
#endif

//! Define _IRR_COMPILE_WITH_GUI_ to compile the engine with the built-in GUI
/** Disable this if you are using an external library to draw the GUI. If you disable this then
you will not be able to use anything provided by the GUI Environment, including loading fonts. */
#define _IRR_COMPILE_WITH_GUI_

//! Define _IRR_COMPILE_WITH_ZLIB_ to enable compiling the engine using zlib.
/** This enables the engine to read from compressed .zip archives. If you
disable this feature, the engine can still read archives, but only uncompressed
ones. */
#define _IRR_COMPILE_WITH_ZLIB_

//! Define _IRR_USE_NON_SYSTEM_ZLIB_ to let irrlicht use the zlib which comes with irrlicht.
/** If this is commented out, Irrlicht will try to compile using the zlib installed in the system.
	This is only used when _IRR_COMPILE_WITH_ZLIB_ is defined. */
#define _IRR_USE_NON_SYSTEM_ZLIB_


//! Define _IRR_COMPILE_WITH_JPEGLIB_ to enable compiling the engine using libjpeg.
/** This enables the engine to read jpeg images. If you comment this out,
the engine will no longer read .jpeg images. */
#define _IRR_COMPILE_WITH_LIBJPEG_

//! Define _IRR_USE_NON_SYSTEM_JPEG_LIB_ to let irrlicht use the jpeglib which comes with irrlicht.
/** If this is commented out, Irrlicht will try to compile using the jpeg lib installed in the system.
	This is only used when _IRR_COMPILE_WITH_LIBJPEG_ is defined. */
#define _IRR_USE_NON_SYSTEM_JPEG_LIB_


//! Define _IRR_COMPILE_WITH_LIBPNG_ to enable compiling the engine using libpng.
/** This enables the engine to read png images. If you comment this out,
the engine will no longer read .png images. */
#define _IRR_COMPILE_WITH_LIBPNG_

//! Define _IRR_USE_NON_SYSTEM_LIBPNG_ to let irrlicht use the libpng which comes with irrlicht.
/** If this is commented out, Irrlicht will try to compile using the libpng installed in the system.
	This is only used when _IRR_COMPILE_WITH_LIBPNG_ is defined. */
#define _IRR_USE_NON_SYSTEM_LIB_PNG_


//! Define _IRR_D3D_NO_SHADER_DEBUGGING to disable shader debugging in D3D9
/** If _IRR_D3D_NO_SHADER_DEBUGGING is undefined in IrrCompileConfig.h,
it is possible to debug all D3D9 shaders in VisualStudio. All shaders
(which have been generated in memory or read from archives for example) will be emitted
into a temporary file at runtime for this purpose. To debug your shaders, choose
Debug->Direct3D->StartWithDirect3DDebugging in Visual Studio, and for every shader a
file named 'irr_dbg_shader_%%.vsh' or 'irr_dbg_shader_%%.psh' will be created. Drag'n'drop
the file you want to debug into visual studio. That's it. You can now set breakpoints and
watch registers, variables etc. This works with ASM, HLSL, and both with pixel and vertex shaders.
Note that the engine will run in D3D REF for this, which is a lot slower than HAL. */
#define _IRR_D3D_NO_SHADER_DEBUGGING


#ifdef _IRR_WINDOWS_API_

#ifndef _IRR_STATIC_LIB_
#ifdef IRRLICHT_EXPORTS
#define IRRLICHT_API __declspec(dllexport)
#else
#define IRRLICHT_API __declspec(dllimport)
#endif // IRRLICHT_EXPORT
#else
#define IRRLICHT_API
#endif // _IRR_STATIC_LIB_

// Declare the calling convention.
#if defined(_STDCALL_SUPPORTED)
#define IRRCALLCONV __stdcall
#else
#define IRRCALLCONV __cdecl
#endif // STDCALL_SUPPORTED

#else
#define IRRLICHT_API
#define IRRCALLCONV
#endif // _IRR_WINDOWS_API_

// We need to disable DIRECT3D9 support for Visual Studio 6.0 because
// those $%&$!! disabled support for it since Dec. 2004 and users are complaining
// about linker errors. Comment this out only if you are knowing what you are
// doing. (Which means you have an old DX9 SDK and VisualStudio6).
#ifdef _MSC_VER
#if (_MSC_VER < 1300 && !defined(__GNUC__))
#undef _IRR_COMPILE_WITH_DIRECT3D_9_
#pragma message("Compiling Irrlicht with Visual Studio 6.0, support for DX9 is disabled.")
#endif
#endif

//! Define one of the three setting for Burning's Video Software Rasterizer
/** So if we were marketing guys we could says Irrlicht has 4 Software-Rasterizers.
	In a Nutshell:
		All Burnings Rasterizers use 32 Bit Backbuffer, 32Bit Texture & 32 Bit Z or WBuffer,
		16 Bit/32 Bit can be adjusted on a global flag.

		BURNINGVIDEO_RENDERER_BEAUTIFUL
			32 Bit + Vertexcolor + Lighting + Per Pixel Perspective Correct + SubPixel/SubTexel Correct +
			Bilinear Texturefiltering + WBuffer

		BURNINGVIDEO_RENDERER_FAST
			32 Bit + Per Pixel Perspective Correct + SubPixel/SubTexel Correct + WBuffer +
			Bilinear Dithering TextureFiltering + WBuffer

		BURNINGVIDEO_RENDERER_ULTRA_FAST
			16Bit + SubPixel/SubTexel Correct + ZBuffer
*/

#define BURNINGVIDEO_RENDERER_BEAUTIFUL
//#define BURNINGVIDEO_RENDERER_FAST
//#define BURNINGVIDEO_RENDERER_ULTRA_FAST


//! Define _IRR_COMPILE_WITH_SKINNED_MESH_SUPPORT_ if you want to use bone based
/** animated meshes. If you compile without this, you will be unable to load
B3D, MS3D or X meshes */
#define _IRR_COMPILE_WITH_SKINNED_MESH_SUPPORT_

#ifdef _IRR_COMPILE_WITH_SKINNED_MESH_SUPPORT_
//! Define _IRR_COMPILE_WITH_B3D_LOADER_ if you want to use Blitz3D files
#define _IRR_COMPILE_WITH_B3D_LOADER_
//! Define _IRR_COMPILE_WITH_B3D_LOADER_ if you want to Milkshape files
#define _IRR_COMPILE_WITH_MS3D_LOADER_
//! Define _IRR_COMPILE_WITH_X_LOADER_ if you want to use Microsoft X files
#define _IRR_COMPILE_WITH_X_LOADER_
#endif

//! Define _IRR_COMPILE_WITH_IRR_MESH_LOADER_ if you want to load Irrlicht Engine .irrmesh files
#define _IRR_COMPILE_WITH_IRR_MESH_LOADER_

//! Define _IRR_COMPILE_WITH_LWO_LOADER_ if you want to load LightWave files
#define _IRR_COMPILE_WITH_LWO_LOADER_

//! Define _IRR_COMPILE_WITH_MD2_LOADER_ if you want to load Quake 2 animated files
#define _IRR_COMPILE_WITH_MD2_LOADER_
//! Define _IRR_COMPILE_WITH_MD3_LOADER_ if you want to load Quake 3 animated files
#define _IRR_COMPILE_WITH_MD3_LOADER_

//! Define _IRR_COMPILE_WITH_3DS_LOADER_ if you want to load 3D Studio Max files
#define _IRR_COMPILE_WITH_3DS_LOADER_
//! Define _IRR_COMPILE_WITH_COLLADA_LOADER_ if you want to load Collada files
#define _IRR_COMPILE_WITH_COLLADA_LOADER_
//! Define _IRR_COMPILE_WITH_CSM_LOADER_ if you want to load Cartography Shop files
#define _IRR_COMPILE_WITH_CSM_LOADER_
//! Define _IRR_COMPILE_WITH_BSP_LOADER_ if you want to load Quake 3 BSP files
#define _IRR_COMPILE_WITH_BSP_LOADER_
//! Define _IRR_COMPILE_WITH_DMF_LOADER_ if you want to load DeleD files
#define _IRR_COMPILE_WITH_DMF_LOADER_
//! Define _IRR_COMPILE_WITH_LMTS_LOADER_ if you want to load LMTools files
#define _IRR_COMPILE_WITH_LMTS_LOADER_
//! Define _IRR_COMPILE_WITH_MY3D_LOADER_ if you want to load MY3D files
#define _IRR_COMPILE_WITH_MY3D_LOADER_
//! Define _IRR_COMPILE_WITH_OBJ_LOADER_ if you want to load Wavefront OBJ files
#define _IRR_COMPILE_WITH_OBJ_LOADER_
//! Define _IRR_COMPILE_WITH_OCT_LOADER_ if you want to load FSRad OCT files
#define _IRR_COMPILE_WITH_OCT_LOADER_
//! Define _IRR_COMPILE_WITH_OGRE_LOADER_ if you want to load Ogre 3D files
#define _IRR_COMPILE_WITH_OGRE_LOADER_
//! Define _IRR_COMPILE_WITH_STL_LOADER_ if you want to load .stl files
#define _IRR_COMPILE_WITH_STL_LOADER_

//! Define _IRR_COMPILE_WITH_IRR_WRITER_ if you want to write static .irr files
#define _IRR_COMPILE_WITH_IRR_WRITER_
//! Define _IRR_COMPILE_WITH_COLLADA_WRITER_ if you want to write Collada files
#define _IRR_COMPILE_WITH_COLLADA_WRITER_
//! Define _IRR_COMPILE_WITH_STL_WRITER_ if you want to write .stl files
#define _IRR_COMPILE_WITH_STL_WRITER_

//! Define _IRR_COMPILE_WITH_BMP_LOADER_ if you want to load .bmp files
#define _IRR_COMPILE_WITH_BMP_LOADER_
//! Define _IRR_COMPILE_WITH_JPG_LOADER_ if you want to load .jpg files
#define _IRR_COMPILE_WITH_JPG_LOADER_
//! Define _IRR_COMPILE_WITH_PCX_LOADER_ if you want to load .pcx files
#define _IRR_COMPILE_WITH_PCX_LOADER_
//! Define _IRR_COMPILE_WITH_PNG_LOADER_ if you want to load .png files
#define _IRR_COMPILE_WITH_PNG_LOADER_
//! Define _IRR_COMPILE_WITH_PPM_LOADER_ if you want to load .ppm/.pgm/.pbm files
#define _IRR_COMPILE_WITH_PPM_LOADER_
//! Define _IRR_COMPILE_WITH_PSD_LOADER_ if you want to load .psd files
#define _IRR_COMPILE_WITH_PSD_LOADER_
//! Define _IRR_COMPILE_WITH_TGA_LOADER_ if you want to load .tga files
#define _IRR_COMPILE_WITH_TGA_LOADER_

//! Define _IRR_COMPILE_WITH_BMP_WRITER_ if you want to write .bmp files
#define _IRR_COMPILE_WITH_BMP_WRITER_
//! Define _IRR_COMPILE_WITH_JPG_WRITER_ if you want to write .jpg files
#define _IRR_COMPILE_WITH_JPG_WRITER_
//! Define _IRR_COMPILE_WITH_PCX_WRITER_ if you want to write .pcx files
#define _IRR_COMPILE_WITH_PCX_WRITER_
//! Define _IRR_COMPILE_WITH_PNG_WRITER_ if you want to write .png files
#define _IRR_COMPILE_WITH_PNG_WRITER_
//! Define _IRR_COMPILE_WITH_PPM_WRITER_ if you want to write .ppm files
#define _IRR_COMPILE_WITH_PPM_WRITER_
//! Define _IRR_COMPILE_WITH_PSD_WRITER_ if you want to write .psd files
#define _IRR_COMPILE_WITH_PSD_WRITER_
//! Define _IRR_COMPILE_WITH_TGA_WRITER_ if you want to write .tga files
#define _IRR_COMPILE_WITH_TGA_WRITER_

//! Set FPU settings
/** Irrlicht should use approximate float and integer fpu techniques
precision will be lower but speed higher. currently X86 only
*/
#if !defined(_IRR_OSX_PLATFORM_) && !defined(_IRR_SOLARIS_PLATFORM_)
	//#define IRRLICHT_FAST_MATH
#endif

// Some cleanup
// XBox does not have OpenGL or DirectX9
#if defined(_IRR_XBOX_PLATFORM_)
#undef _IRR_COMPILE_WITH_OPENGL_
#undef _IRR_COMPILE_WITH_DIRECT3D_9_
#endif

#endif // __IRR_COMPILE_CONFIG_H_INCLUDED__

