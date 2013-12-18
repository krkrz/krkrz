// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COpenGLDriver.h"
// needed here also because of the create methods' parameters
#include "CNullDriver.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "COpenGLTexture.h"
#include "COpenGLMaterialRenderer.h"
#include "COpenGLShaderMaterialRenderer.h"
#include "COpenGLSLMaterialRenderer.h"
#include "COpenGLNormalMapRenderer.h"
#include "COpenGLParallaxMapRenderer.h"
#include "CImage.h"
#include "os.h"

#ifdef _IRR_USE_SDL_DEVICE_
#include <SDL/SDL.h>
#endif

namespace irr
{
namespace video
{

// -----------------------------------------------------------------------
// WINDOWS CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef _IRR_USE_WINDOWS_DEVICE_
//! Windows constructor and init code
COpenGLDriver::COpenGLDriver(const core::dimension2d<s32>& screenSize,
		HWND window, bool fullscreen, bool stencilBuffer,
		io::IFileSystem* io, bool antiAlias)
: CNullDriver(io, screenSize), COpenGLExtensionHandler(),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true), Transformation3DChanged(true),
	AntiAlias(antiAlias), RenderTargetTexture(0), LastSetLight(-1),
	CurrentRendertargetSize(0,0),
	HDc(0), Window(window), HRc(0)
{
	#ifdef _DEBUG
	setDebugName("COpenGLDriver");
	#endif
}

//! inits the open gl driver
bool COpenGLDriver::initDriver(const core::dimension2d<s32>& screenSize,
				HWND window, u32 bits, bool fullscreen, bool vsync, bool stencilBuffer)
{
	static	PIXELFORMATDESCRIPTOR pfd =	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size Of This Pixel Format Descriptor
		1,				// Version Number
		PFD_DRAW_TO_WINDOW |		// Format Must Support Window
		PFD_SUPPORT_OPENGL |		// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,		// Must Support Double Buffering
		PFD_TYPE_RGBA,			// Request An RGBA Format
		bits,				// Select Our Color Depth
		0, 0, 0, 0, 0, 0,		// Color Bits Ignored
		0,				// No Alpha Buffer
		0,				// Shift Bit Ignored
		0,				// No Accumulation Buffer
		0, 0, 0, 0,			// Accumulation Bits Ignored
		24,				// Z-Buffer (Depth Buffer)
		stencilBuffer ? 1 : 0,		// Stencil Buffer Depth
		0,				// No Auxiliary Buffer
		PFD_MAIN_PLANE,			// Main Drawing Layer
		0,				// Reserved
		0, 0, 0				// Layer Masks Ignored
	};

	for (u32 i=0; i<5; ++i)
	{
		if (i == 1)
		{
			if (stencilBuffer)
			{
				os::Printer::log("Cannot create a GL device with stencil buffer, disabling stencil shadows.", ELL_WARNING);
				stencilBuffer = false;
				pfd.cStencilBits = 0;
			}
			else
				continue;
		}
		else
		if (i == 2)
		{
			pfd.cDepthBits = 24;
		}
		if (i == 3)
		{
			if (bits!=16)
				pfd.cDepthBits = 16;
			else
				continue;
		}
		else
		if (i == 4)
		{
			os::Printer::log("Cannot create a GL device context.", ELL_ERROR);
			return false;
		}

		// get hdc
		if (!(HDc=GetDC(window)))
		{
			os::Printer::log("Cannot create a GL device context.", ELL_ERROR);
			continue;
		}

		GLuint PixelFormat;

		// choose pixelformat
		if (!(PixelFormat = ChoosePixelFormat(HDc, &pfd)))
		{
			os::Printer::log("Cannot find a suitable pixelformat.", ELL_ERROR);
			continue;
		}

		// set pixel format
		if(!SetPixelFormat(HDc, PixelFormat, &pfd))
		{
			os::Printer::log("Cannot set the pixel format.", ELL_ERROR);
			continue;
		}

		// create rendering context
		if (!(HRc=wglCreateContext(HDc)))
		{
			os::Printer::log("Cannot create a GL rendering context.", ELL_ERROR);
			continue;
		}

		// activate rendering context
		if(!wglMakeCurrent(HDc, HRc))
		{
			os::Printer::log("Cannot activate GL rendering context", ELL_ERROR);
			continue;
		}

		break;
	}

	genericDriverInit(screenSize, stencilBuffer);

	// set vsync
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(vsync ? 1 : 0);

	// set exposed data
	ExposedData.OpenGLWin32.HDc = reinterpret_cast<s32>(HDc);
	ExposedData.OpenGLWin32.HRc = reinterpret_cast<s32>(HRc);
	ExposedData.OpenGLWin32.HWnd = reinterpret_cast<s32>(Window);

	return true;
}

#endif //IRR_USE_WINDOWS_DEVICE_

// -----------------------------------------------------------------------
// MacOSX CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef _IRR_USE_OSX_DEVICE_
//! Windows constructor and init code
COpenGLDriver::COpenGLDriver(const core::dimension2d<s32>& screenSize,
		bool fullscreen, bool stencilBuffer, CIrrDeviceMacOSX *device,
		io::IFileSystem* io, bool vsync, bool antiAlias)
: CNullDriver(io, screenSize), COpenGLExtensionHandler(),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true), Transformation3DChanged(true),
	AntiAlias(antiAlias), RenderTargetTexture(0), LastSetLight(-1),
	CurrentRendertargetSize(0,0), _device(device)
{
	#ifdef _DEBUG
	setDebugName("COpenGLDriver");
	#endif
	genericDriverInit(screenSize, stencilBuffer);
}

#endif

// -----------------------------------------------------------------------
// LINUX CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef _IRR_USE_LINUX_DEVICE_
//! Linux constructor and init code
COpenGLDriver::COpenGLDriver(const core::dimension2d<s32>& screenSize,
		bool fullscreen, bool stencilBuffer, io::IFileSystem* io,
		bool vsync, bool antiAlias)
: CNullDriver(io, screenSize), COpenGLExtensionHandler(),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true),
	Transformation3DChanged(true), AntiAlias(antiAlias),
	RenderTargetTexture(0), LastSetLight(-1), CurrentRendertargetSize(0,0)
{
	#ifdef _DEBUG
	setDebugName("COpenGLDriver");
	#endif
	XWindow = glXGetCurrentDrawable();
	XDisplay = glXGetCurrentDisplay();
	ExposedData.OpenGLLinux.X11Display = XDisplay;
	ExposedData.OpenGLLinux.X11Window = XWindow;
	genericDriverInit(screenSize, stencilBuffer);

	// set vsync
#ifdef GLX_SGI_swap_control
#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	if (vsync && glxSwapIntervalSGI)
		glxSwapIntervalSGI(1);
#else
	if (vsync)
		glXSwapIntervalSGI(1);
#endif
#endif
}

#endif // _IRR_USE_LINUX_DEVICE_


// -----------------------------------------------------------------------
// SDL CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef _IRR_USE_SDL_DEVICE_
//! SDL constructor and init code
COpenGLDriver::COpenGLDriver(const core::dimension2d<s32>& screenSize,
		bool fullscreen, bool stencilBuffer, io::IFileSystem* io,
		bool vsync, bool antiAlias)
: CNullDriver(io, screenSize), COpenGLExtensionHandler(),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true),
	Transformation3DChanged(true), AntiAlias(antiAlias),
	RenderTargetTexture(0), LastSetLight(-1), CurrentRendertargetSize(0,0)
{
	#ifdef _DEBUG
	setDebugName("COpenGLDriver");
	#endif

	genericDriverInit(screenSize, stencilBuffer);
}

#endif // _IRR_USE_SDL_DEVICE_


//! destructor
COpenGLDriver::~COpenGLDriver()
{
	deleteMaterialRenders();

	// I get a blue screen on my laptop, when I do not delete the
	// textures manually before releasing the dc. Oh how I love this.

	deleteAllTextures();

#ifdef _IRR_USE_WINDOWS_DEVICE_
	if (HRc)
	{
		if (!wglMakeCurrent(0, 0))
			os::Printer::log("Release of dc and rc failed.", ELL_WARNING);

		if (!wglDeleteContext(HRc))
			os::Printer::log("Release of rendering context failed.", ELL_WARNING);
	}

	if (HDc)
		ReleaseDC(Window, HDc);
#endif
}

// -----------------------------------------------------------------------
// METHODS
// -----------------------------------------------------------------------

bool COpenGLDriver::genericDriverInit(const core::dimension2d<s32>& screenSize, bool stencilBuffer)
{
	Name=L"OpenGL ";
	Name.append(glGetString(GL_VERSION));
	s32 pos=Name.findNext(L' ', 7);
	if (pos != -1)
		Name=Name.subString(0, pos);
	printVersion();

	// print renderer information
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	if (renderer && vendor)
		os::Printer::log(reinterpret_cast<const c8*>(renderer), reinterpret_cast<const c8*>(vendor), ELL_INFORMATION);

	u32 i;
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		CurrentTexture[i]=0;
	// load extensions
	initExtensions(stencilBuffer);
	if (queryFeature(EVDF_ARB_GLSL))
	{
		char buf[32];
		const u32 maj = ShaderLanguageVersion/100;
		snprintf(buf, 32, "%u.%u", maj, ShaderLanguageVersion-maj*100);
		os::Printer::log("GLSL version", buf, ELL_INFORMATION);
	}
	else
		os::Printer::log("GLSL not available.", ELL_INFORMATION);

	glViewport(0, 0, screenSize.Width, screenSize.Height); // Reset The Current Viewport
	setAmbientLight(SColorf(0.0f,0.0f,0.0f,0.0f));
#ifdef GL_EXT_separate_specular_color
	if (FeatureAvailable[IRR_EXT_separate_specular_color])
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
// This is a fast replacement for NORMALIZE_NORMALS
//	if ((Version>101) || FeatureAvailable[IRR_EXT_rescale_normal])
//		glEnable(GL_RESCALE_NORMAL_EXT);
	glClearDepth(1.0);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace( GL_CW );
// currently disabled, because often in software, and thus very slow
//	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
//	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
//	glEnable(GL_POINT_SMOOTH);
//	glEnable(GL_LINE_SMOOTH);

	if (AntiAlias && MultiSamplingExtension)
		glEnable(GL_MULTISAMPLE_ARB);

	UserClipPlane.reallocate(MaxUserClipPlanes);
	UserClipPlaneEnabled.reallocate(MaxUserClipPlanes);
	for (i=0; i<MaxUserClipPlanes; ++i)
	{
		UserClipPlane.push_back(core::plane3df());
		UserClipPlaneEnabled.push_back(false);
	}

	// create material renderers
	createMaterialRenderers();

	// set the renderstates
	ResetRenderStates = true;
	setRenderStates3DMode();

	// set fog mode
	setFog(FogColor, LinearFog, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

	return true;
}


void COpenGLDriver::createMaterialRenderers()
{
	// create OpenGL material renderers

	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID_2_LAYER(this));

	// add the same renderer for all lightmap types
	COpenGLMaterialRenderer_LIGHTMAP* lmr = new COpenGLMaterialRenderer_LIGHTMAP(this);
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:
	lmr->drop();

	// add remaining material renderer
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_DETAIL_MAP(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SPHERE_MAP(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_REFLECTION_2_LAYER(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ADD_COLOR(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_VERTEX_ALPHA(this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(this));

	// add normal map renderers
	s32 tmp = 0;
	video::IMaterialRenderer* renderer = 0;
	renderer = new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_SOLID].Renderer);
	renderer->drop();
	renderer = new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);
	renderer->drop();
	renderer = new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_VERTEX_ALPHA].Renderer);
	renderer->drop();

	// add parallax map renderers
	renderer = new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_SOLID].Renderer);
	renderer->drop();
	renderer = new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);
	renderer->drop();
	renderer = new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_VERTEX_ALPHA].Renderer);
	renderer->drop();

	// add basic 1 texture blending
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_ONETEXTURE_BLEND(this));
}


//! presents the rendered scene on the screen, returns false if failed
bool COpenGLDriver::endScene( s32 windowId, core::rect<s32>* sourceRect, core::rect<s32>* destRect, void* destDC)
{
	CNullDriver::endScene();

	glFlush();

#ifdef _IRR_USE_WINDOWS_DEVICE_
	return SwapBuffers(HDc) == TRUE;
#elif defined(_IRR_USE_LINUX_DEVICE_)
	glXSwapBuffers(XDisplay, XWindow);
	return true;
#elif defined(_IRR_USE_OSX_DEVICE_)
	_device->flush();
	return true;
#elif defined(_IRR_USE_SDL_DEVICE_)
	SDL_GL_SwapBuffers();
	return true;
#else
	return false;
#endif
}


//! clears the zbuffer
bool COpenGLDriver::beginScene(bool backBuffer, bool zBuffer, SColor color)
{
	CNullDriver::beginScene(backBuffer, zBuffer, color);

	GLbitfield mask = 0;

	if (backBuffer)
	{
		const f32 inv = 1.0f / 255.0f;
		glClearColor(color.getRed() * inv, color.getGreen() * inv,
				color.getBlue() * inv, color.getAlpha() * inv);

		mask |= GL_COLOR_BUFFER_BIT;
	}

	if (zBuffer)
	{
		glDepthMask(GL_TRUE);
		LastMaterial.ZWriteEnable=true;
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	glClear(mask);
	return true;
}


//! Returns the transformation set by setTransform
const core::matrix4& COpenGLDriver::getTransform(E_TRANSFORMATION_STATE state) const
{
	return Matrices[state];
}


//! sets transformation
void COpenGLDriver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
	GLfloat glmat[16];
	Matrices[state] = mat;
	Transformation3DChanged = true;

	switch(state)
	{
	case ETS_VIEW:
	case ETS_WORLD:
		{
			// OpenGL only has a model matrix, view and world is not existent. so lets fake these two.
			createGLMatrix(glmat, Matrices[ETS_VIEW] * Matrices[ETS_WORLD]);
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(glmat);
			// we have to update the clip planes to the latest view matrix
			for (u32 i=0; i<MaxUserClipPlanes; ++i)
				if (UserClipPlaneEnabled[i])
					uploadClipPlane(i);
		}
		break;
	case ETS_PROJECTION:
		createGLMatrix(glmat, mat);
		// flip z to compensate OpenGLs right-hand coordinate system
		glmat[12] *= -1.0f;
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glmat);
		break;
	case ETS_TEXTURE_0:
	case ETS_TEXTURE_1:
	case ETS_TEXTURE_2:
	case ETS_TEXTURE_3:
	{
		const u32 i = state - ETS_TEXTURE_0;
		const bool isRTT = Material.getTexture(i) && Material.getTexture(i)->isRenderTarget();

		if (MultiTextureExtension)
			extGlActiveTexture(GL_TEXTURE0_ARB + i);

		glMatrixMode(GL_TEXTURE);
		if (mat.isIdentity() && !isRTT)
			glLoadIdentity();
		else
		{
			createGLTextureMatrix(glmat, mat);
			if (isRTT)
				glmat[5] *= -1.0f;
			glLoadMatrixf(glmat);
		}
		break;
	}
	default:
		break;
	}
}


//! draws a vertex primitive list
void COpenGLDriver::drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
		const u16* indexList, u32 primitiveCount,
		E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
	if (!primitiveCount || !vertexCount)
		return;

	if (!checkPrimitiveCount(primitiveCount))
		return;

	CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType);

	// convert colors to gl color format.
	vertexCount *= 4; //reused as color component count
	ColorBuffer.set_used(vertexCount);
	u32 i;

	switch (vType)
	{
		case EVT_STANDARD:
		{
			const S3DVertex* p = reinterpret_cast<const S3DVertex*>(vertices);
			for ( i=0; i<vertexCount; i+=4)
			{
				p->Color.toOpenGLColor(&ColorBuffer[i]);
				++p;
			}
		}
		break;
		case EVT_2TCOORDS:
		{
			const S3DVertex2TCoords* p = reinterpret_cast<const S3DVertex2TCoords*>(vertices);
			for ( i=0; i<vertexCount; i+=4)
			{
				p->Color.toOpenGLColor(&ColorBuffer[i]);
				++p;
			}
		}
		break;
		case EVT_TANGENTS:
		{
			const S3DVertexTangents* p = reinterpret_cast<const S3DVertexTangents*>(vertices);
			for ( i=0; i<vertexCount; i+=4)
			{
				p->Color.toOpenGLColor(&ColorBuffer[i]);
				++p;
			}
		}
		break;
	}

	// draw everything

	setRenderStates3DMode();

	if (MultiTextureExtension)
		extGlClientActiveTexture(GL_TEXTURE0_ARB);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	if ((pType!=scene::EPT_POINTS) && (pType!=scene::EPT_POINT_SPRITES))
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if ((pType!=scene::EPT_POINTS) && (pType!=scene::EPT_POINT_SPRITES))
		glEnableClientState(GL_NORMAL_ARRAY);

	glColorPointer(4, GL_UNSIGNED_BYTE, 0, &ColorBuffer[0]);
	switch (vType)
	{
		case EVT_STANDARD:
			glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex), &(reinterpret_cast<const S3DVertex*>(vertices))[0].Pos);
			glNormalPointer(GL_FLOAT, sizeof(S3DVertex), &(reinterpret_cast<const S3DVertex*>(vertices))[0].Normal);
			glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex), &(reinterpret_cast<const S3DVertex*>(vertices))[0].TCoords);
			if (MultiTextureExtension && CurrentTexture[1])
			{
				extGlClientActiveTexture(GL_TEXTURE1_ARB);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex), &(reinterpret_cast<const S3DVertex*>(vertices))[0].TCoords);
			}
			break;
		case EVT_2TCOORDS:
			glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex2TCoords), &(reinterpret_cast<const S3DVertex2TCoords*>(vertices))[0].Pos);
			glNormalPointer(GL_FLOAT, sizeof(S3DVertex2TCoords), &(reinterpret_cast<const S3DVertex2TCoords*>(vertices))[0].Normal);
			// texture coordinates
			glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &(reinterpret_cast<const S3DVertex2TCoords*>(vertices))[0].TCoords);
			if (MultiTextureExtension)
			{
				extGlClientActiveTexture(GL_TEXTURE1_ARB);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &(reinterpret_cast<const S3DVertex2TCoords*>(vertices))[0].TCoords2);
			}
			break;
		case EVT_TANGENTS:
			glVertexPointer(3, GL_FLOAT, sizeof(S3DVertexTangents), &(reinterpret_cast<const S3DVertexTangents*>(vertices))[0].Pos);
			glNormalPointer(GL_FLOAT, sizeof(S3DVertexTangents), &(reinterpret_cast<const S3DVertexTangents*>(vertices))[0].Normal);
			// texture coordinates
			glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertexTangents), &(reinterpret_cast<const S3DVertexTangents*>(vertices))[0].TCoords);
			if (MultiTextureExtension)
			{
				extGlClientActiveTexture(GL_TEXTURE1_ARB);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(3, GL_FLOAT, sizeof(S3DVertexTangents), &(reinterpret_cast<const S3DVertexTangents*>(vertices))[0].Tangent);

				extGlClientActiveTexture(GL_TEXTURE2_ARB);
				glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
				glTexCoordPointer(3, GL_FLOAT, sizeof(S3DVertexTangents), &(reinterpret_cast<const S3DVertexTangents*>(vertices))[0].Binormal);
			}
			break;
	}

	switch (pType)
	{
		case scene::EPT_POINTS:
		case scene::EPT_POINT_SPRITES:
		{
#ifdef GL_ARB_point_sprite
			if (pType==scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_ARB_point_sprite])
				glEnable(GL_POINT_SPRITE_ARB);
#endif
			float quadratic[] = {0.0f, 0.0f, 10.01f};
			extGlPointParameterfv(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
			float maxParticleSize=1.0f;
			glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxParticleSize);
//			maxParticleSize=maxParticleSize<Material.Thickness?maxParticleSize:Material.Thickness;
//			extGlPointParameterf(GL_POINT_SIZE_MAX_ARB,maxParticleSize);
//			extGlPointParameterf(GL_POINT_SIZE_MIN_ARB,Material.Thickness);
			extGlPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f);
			glPointSize(Material.Thickness);
#ifdef GL_ARB_point_sprite
			if (pType==scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_ARB_point_sprite])
				glTexEnvf(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE, GL_TRUE);
#endif
			glDrawArrays(GL_POINTS, 0, primitiveCount);
#ifdef GL_ARB_point_sprite
			if (pType==scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_ARB_point_sprite])
			{
				glDisable(GL_POINT_SPRITE_ARB);
				glTexEnvf(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE, GL_FALSE);
			}
#endif
		}
			break;
		case scene::EPT_LINE_STRIP:
			glDrawElements(GL_LINE_STRIP, primitiveCount+1, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_LINE_LOOP:
			glDrawElements(GL_LINE_LOOP, primitiveCount, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_LINES:
			glDrawElements(GL_LINES, primitiveCount*2, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_TRIANGLE_STRIP:
			glDrawElements(GL_TRIANGLE_STRIP, primitiveCount+2, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_TRIANGLE_FAN:
			glDrawElements(GL_TRIANGLE_FAN, primitiveCount+2, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_TRIANGLES:
			glDrawElements(GL_TRIANGLES, primitiveCount*3, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_QUAD_STRIP:
			glDrawElements(GL_QUAD_STRIP, primitiveCount*2+2, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_QUADS:
			glDrawElements(GL_QUADS, primitiveCount*4, GL_UNSIGNED_SHORT, indexList);
			break;
		case scene::EPT_POLYGON:
			glDrawElements(GL_POLYGON, primitiveCount, GL_UNSIGNED_SHORT, indexList);
			break;
	}

	if (MultiTextureExtension)
	{
		if (vType==EVT_TANGENTS)
		{
			extGlClientActiveTexture(GL_TEXTURE2_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if ((vType!=EVT_STANDARD) || CurrentTexture[1])
		{
			extGlClientActiveTexture(GL_TEXTURE1_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		extGlClientActiveTexture(GL_TEXTURE0_ARB);
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}



//! draws a 2d image, using a color and the alpha channel of the texture if
//! desired. The image is drawn at pos, clipped against clipRect (if != 0).
//! Only the subtexture defined by sourceRect is used.
void COpenGLDriver::draw2DImage(const video::ITexture* texture,
				const core::position2d<s32>& pos,
				const core::rect<s32>& sourceRect,
				const core::rect<s32>* clipRect, SColor color,
				bool useAlphaChannelOfTexture)
{
	if (!texture)
		return;

	if (!sourceRect.isValid())
		return;

	core::position2d<s32> targetPos(pos);
	core::position2d<s32> sourcePos(sourceRect.UpperLeftCorner);
	core::dimension2d<s32> sourceSize(sourceRect.getSize());
	if (clipRect)
	{
		if (targetPos.X < clipRect->UpperLeftCorner.X)
		{
			sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
			if (sourceSize.Width <= 0)
				return;

			sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
			targetPos.X = clipRect->UpperLeftCorner.X;
		}

		if (targetPos.X + sourceSize.Width > clipRect->LowerRightCorner.X)
		{
			sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
			if (sourceSize.Width <= 0)
				return;
		}

		if (targetPos.Y < clipRect->UpperLeftCorner.Y)
		{
			sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
			if (sourceSize.Height <= 0)
				return;

			sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
			targetPos.Y = clipRect->UpperLeftCorner.Y;
		}

		if (targetPos.Y + sourceSize.Height > clipRect->LowerRightCorner.Y)
		{
			sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
			if (sourceSize.Height <= 0)
				return;
		}
	}

	// clip these coordinates

	if (targetPos.X<0)
	{
		sourceSize.Width += targetPos.X;
		if (sourceSize.Width <= 0)
			return;

		sourcePos.X -= targetPos.X;
		targetPos.X = 0;
	}

	const core::dimension2d<s32>& renderTargetSize = getCurrentRenderTargetSize();

	if (targetPos.X + sourceSize.Width > renderTargetSize.Width)
	{
		sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
		if (sourceSize.Width <= 0)
			return;
	}

	if (targetPos.Y<0)
	{
		sourceSize.Height += targetPos.Y;
		if (sourceSize.Height <= 0)
			return;

		sourcePos.Y -= targetPos.Y;
		targetPos.Y = 0;
	}

	if (targetPos.Y + sourceSize.Height > renderTargetSize.Height)
	{
		sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
		if (sourceSize.Height <= 0)
			return;
	}

	// ok, we've clipped everything.
	// now draw it.

	const core::dimension2d<s32>& ss = texture->getOriginalSize();
	core::rect<f32> tcoords;

	tcoords.UpperLeftCorner.X = sourcePos.X / static_cast<f32>(ss.Width);
	tcoords.UpperLeftCorner.Y = sourcePos.Y / static_cast<f32>(ss.Height);
	tcoords.LowerRightCorner.X = (sourcePos.X + sourceSize.Width) / static_cast<f32>(ss.Width);
	tcoords.LowerRightCorner.Y = (sourcePos.Y + sourceSize.Height) / static_cast<f32>(ss.Height);

	const core::rect<s32> poss(targetPos, sourceSize);

	setRenderStates2DMode(color.getAlpha()<255, true, useAlphaChannelOfTexture);
	disableTextures(1);
	if (!setTexture(0, texture))
		return;

	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glBegin(GL_QUADS);

	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(GLfloat(poss.UpperLeftCorner.X), GLfloat(poss.UpperLeftCorner.Y));

	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(GLfloat(poss.LowerRightCorner.X), GLfloat(poss.UpperLeftCorner.Y));

	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(GLfloat(poss.LowerRightCorner.X), GLfloat(poss.LowerRightCorner.Y));

	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(GLfloat(poss.UpperLeftCorner.X), GLfloat(poss.LowerRightCorner.Y));

	glEnd();
}



//! The same, but with a four element array of colors, one for each vertex
void COpenGLDriver::draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
		const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
		video::SColor* colors, bool useAlphaChannelOfTexture)
{
	if (!texture)
		return;

	const core::dimension2d<s32>& ss = texture->getOriginalSize();
	core::rect<f32> tcoords;
	tcoords.UpperLeftCorner.X = sourceRect.UpperLeftCorner.X / static_cast<f32>(ss.Width);
	tcoords.UpperLeftCorner.Y = sourceRect.UpperLeftCorner.Y / static_cast<f32>(ss.Height);
	tcoords.LowerRightCorner.X = sourceRect.LowerRightCorner.X / static_cast<f32>(ss.Width);
	tcoords.LowerRightCorner.Y = sourceRect.LowerRightCorner.Y / static_cast<f32>(ss.Height);

	video::SColor temp[4] =
	{
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF
	};

	video::SColor* useColor = colors ? colors : temp;

	setRenderStates2DMode(useColor[0].getAlpha()<255 || useColor[1].getAlpha()<255 ||
			useColor[2].getAlpha()<255 || useColor[3].getAlpha()<255,
			true, useAlphaChannelOfTexture);

	disableTextures(1);
	setTexture(0, texture);

	if (clipRect)
	{
		if (!clipRect->isValid())
			return;

		glEnable(GL_SCISSOR_TEST);
		const core::dimension2d<s32>& renderTargetSize = getCurrentRenderTargetSize();
		glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height-clipRect->LowerRightCorner.Y,
			clipRect->getWidth(), clipRect->getHeight());
	}

	glBegin(GL_QUADS);

	glColor4ub(useColor[0].getRed(), useColor[0].getGreen(), useColor[0].getBlue(), useColor[0].getAlpha());
	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(GLfloat(destRect.UpperLeftCorner.X), GLfloat(destRect.UpperLeftCorner.Y));

	glColor4ub(useColor[3].getRed(), useColor[3].getGreen(), useColor[3].getBlue(), useColor[3].getAlpha());
	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(GLfloat(destRect.LowerRightCorner.X), GLfloat(destRect.UpperLeftCorner.Y));

	glColor4ub(useColor[2].getRed(), useColor[2].getGreen(), useColor[2].getBlue(), useColor[2].getAlpha());
	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(GLfloat(destRect.LowerRightCorner.X), GLfloat(destRect.LowerRightCorner.Y));

	glColor4ub(useColor[1].getRed(), useColor[1].getGreen(), useColor[1].getBlue(), useColor[1].getAlpha());
	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(GLfloat(destRect.UpperLeftCorner.X), GLfloat(destRect.LowerRightCorner.Y));

	glEnd();

	if (clipRect)
		glDisable(GL_SCISSOR_TEST);
}



//! draws a set of 2d images, using a color and the alpha channel of the
//! texture if desired. The images are drawn beginning at pos and concatenated
//! in one line. All drawings are clipped against clipRect (if != 0).
//! The subtextures are defined by the array of sourceRects and are chosen
//! by the indices given.
void COpenGLDriver::draw2DImage(const video::ITexture* texture,
				const core::position2d<s32>& pos,
				const core::array<core::rect<s32> >& sourceRects,
				const core::array<s32>& indices,
				const core::rect<s32>* clipRect, SColor color,
				bool useAlphaChannelOfTexture)
{
	if (!texture)
		return;

	setRenderStates2DMode(color.getAlpha()<255, true, useAlphaChannelOfTexture);
	disableTextures(1);
	if (!setTexture(0, texture))
		return;

	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	if (clipRect)
	{
		if (!clipRect->isValid())
			return;

		glEnable(GL_SCISSOR_TEST);
		const core::dimension2d<s32>& renderTargetSize = getCurrentRenderTargetSize();
		glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height-clipRect->LowerRightCorner.Y,
			clipRect->getWidth(),clipRect->getHeight());
	}

	const core::dimension2d<s32>& ss = texture->getOriginalSize();
	core::position2d<s32> targetPos(pos);
	core::position2d<s32> sourcePos;
	core::dimension2d<s32> sourceSize;
	core::rect<f32> tcoords;

	for (u32 i=0; i<indices.size(); ++i)
	{
		s32 currentIndex(indices[i]);
		if (!sourceRects[currentIndex].isValid())
			break;
		sourcePos=sourceRects[currentIndex].UpperLeftCorner;
		sourceSize=sourceRects[currentIndex].getSize();

		tcoords.UpperLeftCorner.X = sourceRects[currentIndex].UpperLeftCorner.X / static_cast<f32>(ss.Width);
		tcoords.UpperLeftCorner.Y = sourceRects[currentIndex].UpperLeftCorner.Y / static_cast<f32>(ss.Height);
		tcoords.LowerRightCorner.X = sourceRects[currentIndex].LowerRightCorner.X / static_cast<f32>(ss.Width);
		tcoords.LowerRightCorner.Y = sourceRects[currentIndex].LowerRightCorner.Y / static_cast<f32>(ss.Height);

		const core::rect<s32> poss(targetPos, sourceSize);

		glBegin(GL_QUADS);

		glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
		glVertex2f(GLfloat(poss.UpperLeftCorner.X), GLfloat(poss.UpperLeftCorner.Y));

		glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
		glVertex2f(GLfloat(poss.LowerRightCorner.X), GLfloat(poss.UpperLeftCorner.Y));

		glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
		glVertex2f(GLfloat(poss.LowerRightCorner.X), GLfloat(poss.LowerRightCorner.Y));

		glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
		glVertex2f(GLfloat(poss.UpperLeftCorner.X), GLfloat(poss.LowerRightCorner.Y));

		glEnd();
		targetPos.X += sourceRects[currentIndex].getWidth();
	}
	if (clipRect)
		glDisable(GL_SCISSOR_TEST);
}



//! draw a 2d rectangle
void COpenGLDriver::draw2DRectangle(SColor color, const core::rect<s32>& position,
		const core::rect<s32>* clip)
{
	setRenderStates2DMode(color.getAlpha() < 255, false, false);
	disableTextures();

	core::rect<s32> pos = position;

	if (clip)
		pos.clipAgainst(*clip);

	if (!pos.isValid())
		return;

	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glRectf(GLfloat(pos.UpperLeftCorner.X), GLfloat(pos.UpperLeftCorner.Y),
		GLfloat(pos.LowerRightCorner.X), GLfloat(pos.LowerRightCorner.Y));
}



//! draw an 2d rectangle
void COpenGLDriver::draw2DRectangle(const core::rect<s32>& position,
			SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
			const core::rect<s32>* clip)
{
	core::rect<s32> pos = position;

	if (clip)
		pos.clipAgainst(*clip);

	if (!pos.isValid())
		return;

	setRenderStates2DMode(colorLeftUp.getAlpha() < 255 ||
		colorRightUp.getAlpha() < 255 ||
		colorLeftDown.getAlpha() < 255 ||
		colorRightDown.getAlpha() < 255, false, false);

	disableTextures();

	glBegin(GL_QUADS);
	glColor4ub(colorLeftUp.getRed(), colorLeftUp.getGreen(),
		colorLeftUp.getBlue(), colorLeftUp.getAlpha());
	glVertex2f(GLfloat(pos.UpperLeftCorner.X), GLfloat(pos.UpperLeftCorner.Y));

	glColor4ub(colorRightUp.getRed(), colorRightUp.getGreen(),
		colorRightUp.getBlue(), colorRightUp.getAlpha());
	glVertex2f(GLfloat(pos.LowerRightCorner.X), GLfloat(pos.UpperLeftCorner.Y));

	glColor4ub(colorRightDown.getRed(), colorRightDown.getGreen(),
		colorRightDown.getBlue(), colorRightDown.getAlpha());
	glVertex2f(GLfloat(pos.LowerRightCorner.X), GLfloat(pos.LowerRightCorner.Y));

	glColor4ub(colorLeftDown.getRed(), colorLeftDown.getGreen(),
		colorLeftDown.getBlue(), colorLeftDown.getAlpha());
	glVertex2f(GLfloat(pos.UpperLeftCorner.X), GLfloat(pos.LowerRightCorner.Y));

	glEnd();
}



//! Draws a 2d line.
void COpenGLDriver::draw2DLine(const core::position2d<s32>& start,
				const core::position2d<s32>& end,
				SColor color)
{
	setRenderStates2DMode(color.getAlpha() < 255, false, false);
	disableTextures();

	glBegin(GL_LINES);
	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glVertex2f(GLfloat(start.X), GLfloat(start.Y));
	glVertex2f(GLfloat(end.X),   GLfloat(end.Y));
	glEnd();
}



bool COpenGLDriver::setTexture(u32 stage, const video::ITexture* texture)
{
	if (stage >= MaxTextureUnits)
		return false;

	if (CurrentTexture[stage]==texture)
		return true;

	if (MultiTextureExtension)
		extGlActiveTexture(GL_TEXTURE0_ARB + stage);

	CurrentTexture[stage]=texture;

	if (!texture)
	{
		glDisable(GL_TEXTURE_2D);
		return true;
	}
	else
	{
		if (texture->getDriverType() != EDT_OPENGL)
		{
			glDisable(GL_TEXTURE_2D);
			os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
			return false;
		}

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,
			static_cast<const COpenGLTexture*>(texture)->getOpenGLTextureName());
	}
	return true;
}



//! disables all textures beginning with the optional fromStage parameter. Otherwise all texture stages are disabled.
//! Returns whether disabling was successful or not.
bool COpenGLDriver::disableTextures(u32 fromStage)
{
	bool result=true;
	for (u32 i=fromStage; i<MaxTextureUnits; ++i)
		result &= setTexture(i, 0);
	return result;
}



//! creates a matrix in supplied GLfloat array to pass to OpenGL
inline void COpenGLDriver::createGLMatrix(GLfloat gl_matrix[16], const core::matrix4& m)
{
	memcpy ( gl_matrix, m.pointer(), 16 * sizeof(f32) );
}


//! creates a opengltexturematrix from a D3D style texture matrix
inline void COpenGLDriver::createGLTextureMatrix(GLfloat *o, const core::matrix4& m)
{
	o[0] = m[0];
	o[1] = m[1];
	o[2] = 0.f;
	o[3] = 0.f;

	o[4] = m[4];
	o[5] = m[5];
	o[6] = 0.f;
	o[7] = 0.f;

	o[8] = 0.f;
	o[9] = 0.f;
	o[10] = 1.f;
	o[11] = 0.f;

	o[12] = m[8];
	o[13] = m[9];
	o[14] = 0.f;
	o[15] = 1.f;
}


//! returns a device dependent texture from a software surface (IImage)
video::ITexture* COpenGLDriver::createDeviceDependentTexture(IImage* surface, const char* name)
{
	return new COpenGLTexture(surface, name, this);
}


//! Sets a material. All 3d drawing functions draw geometry now
//! using this material.
//! \param material: Material to be used from now on.
void COpenGLDriver::setMaterial(const SMaterial& material)
{
	Material = material;

	for (s32 i = MaxTextureUnits-1; i>= 0; --i)
	{
		setTransform ((E_TRANSFORMATION_STATE) ( ETS_TEXTURE_0 + i ),
				material.getTextureMatrix(i));
	}
}


//! prints error if an error happened.
bool COpenGLDriver::testGLError()
{
#ifdef _DEBUG
	GLenum g = glGetError();
	switch(g)
	{
	case GL_NO_ERROR:
		return false;
	case GL_INVALID_ENUM:
		os::Printer::log("GL_INVALID_ENUM", ELL_ERROR); break;
	case GL_INVALID_VALUE:
		os::Printer::log("GL_INVALID_VALUE", ELL_ERROR); break;
	case GL_INVALID_OPERATION:
		os::Printer::log("GL_INVALID_OPERATION", ELL_ERROR); break;
	case GL_STACK_OVERFLOW:
		os::Printer::log("GL_STACK_OVERFLOW", ELL_ERROR); break;
	case GL_STACK_UNDERFLOW:
		os::Printer::log("GL_STACK_UNDERFLOW", ELL_ERROR); break;
	case GL_OUT_OF_MEMORY:
		os::Printer::log("GL_OUT_OF_MEMORY", ELL_ERROR); break;
	case GL_TABLE_TOO_LARGE:
		os::Printer::log("GL_TABLE_TOO_LARGE", ELL_ERROR); break;
	};
	return true;
#else
	return false;
#endif
}


//! sets the needed renderstates
void COpenGLDriver::setRenderStates3DMode()
{
	if (CurrentRenderMode != ERM_3D)
	{
		// Reset Texture Stages
		if (FeatureAvailable[IRR_ARB_texture_env_combine])
			glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE );
		glDisable(GL_ALPHA_TEST);
		glDisable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );

		// switch back the matrices
		GLfloat glmat[16];

		createGLMatrix(glmat, Matrices[ETS_VIEW] * Matrices[ETS_WORLD]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glmat);

		createGLMatrix(glmat, Matrices[ETS_PROJECTION]);
		glmat[12] *= -1.0f;
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glmat);

		ResetRenderStates = true;
	}

	if ( ResetRenderStates || LastMaterial != Material)
	{
		// unset old material

		if (LastMaterial.MaterialType != Material.MaterialType &&
				static_cast<u32>(LastMaterial.MaterialType) < MaterialRenderers.size())
			MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();

		// set new material.
		if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
			MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(
				Material, LastMaterial, ResetRenderStates, this);

		LastMaterial = Material;
		ResetRenderStates = false;
	}

	if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
		MaterialRenderers[Material.MaterialType].Renderer->OnRender(this, video::EVT_STANDARD);

	CurrentRenderMode = ERM_3D;
}


//! Can be called by an IMaterialRenderer to make its work easier.
void COpenGLDriver::setBasicRenderStates(const SMaterial& material, const SMaterial& lastmaterial,
	bool resetAllRenderStates)
{
	if (resetAllRenderStates ||
		lastmaterial.AmbientColor != material.AmbientColor ||
		lastmaterial.DiffuseColor != material.DiffuseColor ||
		lastmaterial.SpecularColor != material.SpecularColor ||
		lastmaterial.EmissiveColor != material.EmissiveColor ||
		lastmaterial.Shininess != material.Shininess)
	{
		GLfloat color[4];

		const f32 inv = 1.0f / 255.0f;

		color[0] = material.AmbientColor.getRed() * inv;
		color[1] = material.AmbientColor.getGreen() * inv;
		color[2] = material.AmbientColor.getBlue() * inv;
		color[3] = material.AmbientColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

		color[0] = material.DiffuseColor.getRed() * inv;
		color[1] = material.DiffuseColor.getGreen() * inv;
		color[2] = material.DiffuseColor.getBlue() * inv;
		color[3] = material.DiffuseColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

		// disable Specular colors if no shininess is set
		if (material.Shininess != 0.0f)
		{
#ifdef GL_EXT_separate_specular_color
			if (FeatureAvailable[IRR_EXT_separate_specular_color])
				glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.Shininess);
			color[0] = material.SpecularColor.getRed() * inv;
			color[1] = material.SpecularColor.getGreen() * inv;
			color[2] = material.SpecularColor.getBlue() * inv;
			color[3] = material.SpecularColor.getAlpha() * inv;
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
		}
#ifdef GL_EXT_separate_specular_color
		else
			if (FeatureAvailable[IRR_EXT_separate_specular_color])
				glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
#endif

		color[0] = material.EmissiveColor.getRed() * inv;
		color[1] = material.EmissiveColor.getGreen() * inv;
		color[2] = material.EmissiveColor.getBlue() * inv;
		color[3] = material.EmissiveColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
	}

	// Texture filter
	// Has to be checked always because it depends on the textures
	// Filtering has to be set for each texture layer
	for (u32 i=0; i<MaxTextureUnits; ++i)
	{
		if (!material.getTexture(i))
			continue;
		if (MultiTextureExtension)
			extGlActiveTexture(GL_TEXTURE0_ARB + i);
		else if (i>0)
			break;

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			(material.TextureLayer[i].BilinearFilter || material.TextureLayer[i].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);

		if (material.getTexture(i) && material.getTexture(i)->hasMipMaps())
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				material.TextureLayer[i].TrilinearFilter ? GL_LINEAR_MIPMAP_LINEAR :
				material.TextureLayer[i].BilinearFilter ? GL_LINEAR_MIPMAP_NEAREST :
				GL_NEAREST_MIPMAP_NEAREST );
		else
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				(material.TextureLayer[i].BilinearFilter || material.TextureLayer[i].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);

#ifdef GL_EXT_texture_filter_anisotropic
		if (AnisotropyExtension)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				material.TextureLayer[i].AnisotropicFilter ? MaxAnisotropy : 1.0f );
#endif
	}

	// fillmode
	if (resetAllRenderStates || (lastmaterial.Wireframe != material.Wireframe) || (lastmaterial.PointCloud != material.PointCloud))
		glPolygonMode(GL_FRONT_AND_BACK, material.Wireframe ? GL_LINE : material.PointCloud? GL_POINT : GL_FILL);

	// shademode
	if (resetAllRenderStates || (lastmaterial.GouraudShading != material.GouraudShading))
	{
		if (material.GouraudShading)
			glShadeModel(GL_SMOOTH);
		else
			glShadeModel(GL_FLAT);
	}

	// lighting
	if (resetAllRenderStates || (lastmaterial.Lighting != material.Lighting))
	{
		if (material.Lighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
	}

	// zbuffer
	if (resetAllRenderStates || lastmaterial.ZBuffer != material.ZBuffer)
	{
		switch (material.ZBuffer)
		{
			case 0:
				glDisable(GL_DEPTH_TEST);
				break;
			case 1:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc ( GL_LEQUAL );
				break;
			case 2:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc ( GL_EQUAL );
				break;
		}
	}

	// zwrite
//	if (resetAllRenderStates || lastmaterial.ZWriteEnable != material.ZWriteEnable)
	{
		if (material.ZWriteEnable && (AllowZWriteOnTransparent || !material.isTransparent()))
		{
			glDepthMask(GL_TRUE);
		}
		else
			glDepthMask(GL_FALSE);
	}

	// back face culling
	if (resetAllRenderStates || lastmaterial.BackfaceCulling != material.BackfaceCulling)
	{
		if (material.BackfaceCulling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	// fog
	if (resetAllRenderStates || lastmaterial.FogEnable != material.FogEnable)
	{
		if (material.FogEnable)
			glEnable(GL_FOG);
		else
			glDisable(GL_FOG);
	}

	// normalization
	if (resetAllRenderStates || lastmaterial.NormalizeNormals != material.NormalizeNormals)
	{
		if (material.NormalizeNormals)
			glEnable(GL_NORMALIZE);
		else
			glDisable(GL_NORMALIZE);
	}

	// thickness
	if (resetAllRenderStates || lastmaterial.Thickness != material.Thickness)
	{
		glPointSize(material.Thickness);
		glLineWidth(material.Thickness);
	}

	// texture address mode
	for (u32 u=0; u<MaxTextureUnits; ++u)
	{
		if (resetAllRenderStates || lastmaterial.TextureLayer[u].TextureWrap != material.TextureLayer[u].TextureWrap)
		{
			if (MultiTextureExtension)
				extGlActiveTexture(GL_TEXTURE0_ARB + u);
			else if (u>0)
				break; // stop loop

			GLint mode=GL_REPEAT;
			switch (material.TextureLayer[u].TextureWrap)
			{
				case ETC_REPEAT:
					mode=GL_REPEAT;
					break;
				case ETC_CLAMP:
					mode=GL_CLAMP;
					break;
				case ETC_CLAMP_TO_EDGE:
#ifdef GL_VERSION_1_2
					if (Version>101)
						mode=GL_CLAMP_TO_EDGE;
					else
#endif
#ifdef GL_SGIS_texture_edge_clamp
					if (FeatureAvailable[IRR_SGIS_texture_edge_clamp])
						mode=GL_CLAMP_TO_EDGE_SGIS;
					else
#endif
						// fallback
						mode=GL_CLAMP;
					break;
				case ETC_CLAMP_TO_BORDER:
#ifdef GL_VERSION_1_3
					if (Version>102)
						mode=GL_CLAMP_TO_BORDER;
					else
#endif
#ifdef GL_ARB_texture_border_clamp
					if (FeatureAvailable[IRR_ARB_texture_border_clamp])
						mode=GL_CLAMP_TO_BORDER_ARB;
					else
#endif
#ifdef GL_SGIS_texture_border_clamp
					if (FeatureAvailable[IRR_SGIS_texture_border_clamp])
						mode=GL_CLAMP_TO_BORDER_SGIS;
					else
#endif
						// fallback
						mode=GL_CLAMP;
					break;
				case ETC_MIRROR:
#ifdef GL_VERSION_1_4
					if (Version>103)
						mode=GL_MIRRORED_REPEAT;
					else
#endif
#ifdef GL_ARB_texture_border_clamp
					if (FeatureAvailable[IRR_ARB_texture_mirrored_repeat])
						mode=GL_MIRRORED_REPEAT_ARB;
					else
#endif
#ifdef GL_IBM_texture_mirrored_repeat
					if (FeatureAvailable[IRR_IBM_texture_mirrored_repeat])
						mode=GL_MIRRORED_REPEAT_IBM;
					else
#endif
						mode=GL_REPEAT;
					break;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
		}
	}
	// be sure to leave in texture stage 0
	if (MultiTextureExtension)
		extGlActiveTexture(GL_TEXTURE0_ARB);
}


//! sets the needed renderstates
void COpenGLDriver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
{
	if (CurrentRenderMode != ERM_2D || Transformation3DChanged)
	{
		// unset last 3d material
		if (CurrentRenderMode == ERM_3D)
		{
			if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
				MaterialRenderers[Material.MaterialType].Renderer->OnUnsetMaterial();
			SMaterial mat;
			mat.Lighting=false;
			mat.ZBuffer=false;
			setBasicRenderStates(mat, SMaterial(), true);
		}

		GLfloat glmat[16];
		core::matrix4 m;

		glMatrixMode(GL_PROJECTION);

		const core::dimension2d<s32>& renderTargetSize = getCurrentRenderTargetSize();
		m.buildProjectionMatrixOrthoLH(f32(renderTargetSize.Width), f32(-renderTargetSize.Height), -1.0, 1.0);
		m.setTranslation(core::vector3df(-1,1,0));

		createGLMatrix(glmat, m);
		glLoadMatrixf(glmat);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef (0.375, 0.375, 0.0);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		Transformation3DChanged = false;
	}

	if (texture)
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (alphaChannel)
		{
			if (alpha)
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
			}
			else
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
			}

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
		}
		else
		{
			if (alpha)
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
				glDisable(GL_ALPHA_TEST);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glDisable(GL_ALPHA_TEST);
				glDisable(GL_BLEND);
			}
		}
	}
	else
	{
		if (alpha)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_ALPHA_TEST);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}
	}

	CurrentRenderMode = ERM_2D;
}


//! \return Returns the name of the video driver. Example: In case of the Direct3D8
//! driver, it would return "Direct3D8.1".
const wchar_t* COpenGLDriver::getName() const
{
	return Name.c_str();
}


//! deletes all dynamic lights there are
void COpenGLDriver::deleteAllDynamicLights()
{
	for (s32 i=0; i<LastSetLight+1; ++i)
		glDisable(GL_LIGHT0 + i);

	LastSetLight = -1;

	CNullDriver::deleteAllDynamicLights();
}


//! adds a dynamic light
void COpenGLDriver::addDynamicLight(const SLight& light)
{
	if (LastSetLight == MaxLights-1)
		return;

	setTransform(ETS_WORLD, core::matrix4());

	++LastSetLight;
	CNullDriver::addDynamicLight(light);

	s32 lidx = GL_LIGHT0 + LastSetLight;
	GLfloat data[4];

	switch (light.Type)
	{
	case video::ELT_SPOT:
		data[0] = light.Direction.X;
		data[1] = light.Direction.Y;
		data[2] = light.Direction.Z;
		data[3] = 0.0f;
		glLightfv(lidx, GL_SPOT_DIRECTION, data);

		// set position
		data[0] = light.Position.X;
		data[1] = light.Position.Y;
		data[2] = light.Position.Z;
		data[3] = 1.0f; // 1.0f for positional light
		glLightfv(lidx, GL_POSITION, data);

		glLightf(lidx, GL_SPOT_EXPONENT, light.Falloff);
		glLightf(lidx, GL_SPOT_CUTOFF, light.OuterCone);
	break;
	case video::ELT_POINT:
		// set position
		data[0] = light.Position.X;
		data[1] = light.Position.Y;
		data[2] = light.Position.Z;
		data[3] = 1.0f; // 1.0f for positional light
		glLightfv(lidx, GL_POSITION, data);

		glLightf(lidx, GL_SPOT_EXPONENT, 0.0f);
		glLightf(lidx, GL_SPOT_CUTOFF, 180.0f);
	break;
	case video::ELT_DIRECTIONAL:
		// set direction
		data[0] = -light.Direction.X;
		data[1] = -light.Direction.Y;
		data[2] = -light.Direction.Z;
		data[3] = 0.0f; // 0.0f for directional light
		glLightfv(lidx, GL_POSITION, data);

		glLightf(lidx, GL_SPOT_EXPONENT, 0.0f);
		glLightf(lidx, GL_SPOT_CUTOFF, 180.0f);
	break;
	}

	// set diffuse color
	data[0] = light.DiffuseColor.r;
	data[1] = light.DiffuseColor.g;
	data[2] = light.DiffuseColor.b;
	data[3] = light.DiffuseColor.a;
	glLightfv(lidx, GL_DIFFUSE, data);

	// set specular color
	data[0] = light.SpecularColor.r;
	data[1] = light.SpecularColor.g;
	data[2] = light.SpecularColor.b;
	data[3] = light.SpecularColor.a;
	glLightfv(lidx, GL_SPECULAR, data);

	// set ambient color
	data[0] = light.AmbientColor.r;
	data[1] = light.AmbientColor.g;
	data[2] = light.AmbientColor.b;
	data[3] = light.AmbientColor.a;
	glLightfv(lidx, GL_AMBIENT, data);

	// 1.0f / (constant + linear * d + quadratic*(d*d);

	// set attenuation
	glLightf(lidx, GL_CONSTANT_ATTENUATION, light.Attenuation.X);
	glLightf(lidx, GL_LINEAR_ATTENUATION, light.Attenuation.Y);
	glLightf(lidx, GL_QUADRATIC_ATTENUATION, light.Attenuation.Z);

	glEnable(lidx);
}


//! returns the maximal amount of dynamic lights the device can handle
u32 COpenGLDriver::getMaximalDynamicLightAmount() const
{
	return MaxLights;
}


//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void COpenGLDriver::setAmbientLight(const SColorf& color)
{
	GLfloat data[4] = {color.r, color.g, color.b, color.a};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, data);
}


// this code was sent in by Oliver Klems, thank you! (I modified the glViewport
// method just a bit.
void COpenGLDriver::setViewPort(const core::rect<s32>& area)
{
	core::rect<s32> vp = area;
	core::rect<s32> rendert(0,0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);
	vp.clipAgainst(rendert);

	if (vp.getHeight()>0 && vp.getWidth()>0)
		glViewport(vp.UpperLeftCorner.X,
				getCurrentRenderTargetSize().Height - vp.UpperLeftCorner.Y - vp.getHeight(),
				vp.getWidth(), vp.getHeight());

	ViewPort = vp;
}

//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
//! this: First, draw all geometry. Then use this method, to draw the shadow
//! volume. Next use IVideoDriver::drawStencilShadow() to visualize the shadow.
void COpenGLDriver::drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail)
{
	if (!StencilBuffer || !count)
		return;

	// unset last 3d material
	if (CurrentRenderMode == ERM_3D &&
		static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
	{
		MaterialRenderers[Material.MaterialType].Renderer->OnUnsetMaterial();
		ResetRenderStates = true;
	}

	// store current OpenGL state
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
		GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE); // no depth buffer writing
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ); // no color buffer drawing
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f, 1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(core::vector3df),&triangles[0]);
	glStencilMask(~0);
	glStencilFunc(GL_ALWAYS, 0, ~0);

	// The first parts are not correctly working, yet.
#if 0
#ifdef GL_EXT_stencil_two_side
	if (FeatureAvailable[IRR_EXT_stencil_two_side])
	{
		glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
#ifdef GL_NV_depth_clamp
		if (FeatureAvailable[IRR_NV_depth_clamp])
			glEnable(GL_DEPTH_CLAMP_NV);
#endif
		glDisable(GL_CULL_FACE);
		if (!zfail)
		{
			// ZPASS Method

			extGlActiveStencilFace(GL_BACK);
			if (FeatureAvailable[IRR_EXT_stencil_wrap])
				glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
			else
				glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
			glStencilMask(~0);
			glStencilFunc(GL_ALWAYS, 0, ~0);

			extGlActiveStencilFace(GL_FRONT);
			if (FeatureAvailable[IRR_EXT_stencil_wrap])
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
			else
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			glStencilMask(~0);
			glStencilFunc(GL_ALWAYS, 0, ~0);

			glDrawArrays(GL_TRIANGLES,0,count);
		}
		else
		{
			// ZFAIL Method

			extGlActiveStencilFace(GL_BACK);
			if (FeatureAvailable[IRR_EXT_stencil_wrap])
				glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
			else
				glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
			glStencilMask(~0);
			glStencilFunc(GL_ALWAYS, 0, ~0);

			extGlActiveStencilFace(GL_FRONT);
			if (FeatureAvailable[IRR_EXT_stencil_wrap])
				glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
			else
				glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
			glStencilMask(~0);
			glStencilFunc(GL_ALWAYS, 0, ~0);

			glDrawArrays(GL_TRIANGLES,0,count);
		}
	}
	else
#endif
	if (FeatureAvailable[IRR_ATI_separate_stencil])
	{
		glDisable(GL_CULL_FACE);
		if (!zfail)
		{
			// ZPASS Method

			extGlStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR);
			extGlStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR);
			extGlStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 0, ~0);
			glStencilMask(~0);

			glDrawArrays(GL_TRIANGLES,0,count);
		}
		else
		{
			// ZFAIL Method

			extGlStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
			extGlStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
			extGlStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 0, ~0);

			glDrawArrays(GL_TRIANGLES,0,count);
		}
	}
	else
#endif
	{
		glEnable(GL_CULL_FACE);
		if (!zfail)
		{
			// ZPASS Method

			glCullFace(GL_BACK);
			glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			glDrawArrays(GL_TRIANGLES,0,count);

			glCullFace(GL_FRONT);
			glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
			glDrawArrays(GL_TRIANGLES,0,count);
		}
		else
		{
			// ZFAIL Method

			glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
			glCullFace(GL_FRONT);
			glDrawArrays(GL_TRIANGLES,0,count);

			glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
			glCullFace(GL_BACK);
			glDrawArrays(GL_TRIANGLES,0,count);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY); //not stored on stack
	glPopAttrib();
}



void COpenGLDriver::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge,
	video::SColor rightUpEdge, video::SColor leftDownEdge, video::SColor rightDownEdge)
{
	if (!StencilBuffer)
		return;

	disableTextures();

	// store attributes
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT );

	glDisable( GL_LIGHTING );
	glDisable(GL_FOG);
	glDepthMask(GL_FALSE);

	glShadeModel( GL_FLAT );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable( GL_STENCIL_TEST );
	glStencilFunc(GL_NOTEQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// draw a shadow rectangle covering the entire screen using stencil buffer
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);

	glColor4ub (leftDownEdge.getRed(), leftDownEdge.getGreen(), leftDownEdge.getBlue(), leftDownEdge.getAlpha() );
	glVertex3f(-1.1f,-1.1f,0.9f);

	glColor4ub (leftUpEdge.getRed(), leftUpEdge.getGreen(), leftUpEdge.getBlue(), leftUpEdge.getAlpha() );
	glVertex3f(-1.1f, 1.1f,0.9f);

	glColor4ub (rightUpEdge.getRed(), rightUpEdge.getGreen(), rightUpEdge.getBlue(), rightUpEdge.getAlpha() );
	glVertex3f( 1.1f, 1.1f,0.9f);

	glColor4ub (rightDownEdge.getRed(), rightDownEdge.getGreen(), rightDownEdge.getBlue(), rightDownEdge.getAlpha() );
	glVertex3f( 1.1f,-1.1f,0.9f);

	glEnd();

	if (clearStencilBuffer)
		glClear(GL_STENCIL_BUFFER_BIT);

	// restore settings
	glPopMatrix();
	glPopAttrib();
}


//! Sets the fog mode.
void COpenGLDriver::setFog(SColor c, bool linearFog, f32 start,
			f32 end, f32 density, bool pixelFog, bool rangeFog)
{
	CNullDriver::setFog(c, linearFog, start, end, density, pixelFog, rangeFog);

	glFogf(GL_FOG_MODE, GLfloat(linearFog ? GL_LINEAR : GL_EXP));
#ifdef GL_EXT_fog_coord
	if (FeatureAvailable[IRR_EXT_fog_coord])
		glFogi(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
#endif

	if(linearFog)
	{
		glFogf(GL_FOG_START, start);
		glFogf(GL_FOG_END, end);
	}
	else
		glFogf(GL_FOG_DENSITY, density);

	if (pixelFog)
		glHint(GL_FOG_HINT, GL_NICEST);
	else
		glHint(GL_FOG_HINT, GL_FASTEST);

	SColorf color(c);
	GLfloat data[4] = {color.r, color.g, color.b, color.a};
	glFogfv(GL_FOG_COLOR, data);
}



//! Draws a 3d line.
void COpenGLDriver::draw3DLine(const core::vector3df& start,
				const core::vector3df& end, SColor color)
{
	setRenderStates3DMode();

	glBegin(GL_LINES);
	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glVertex3f(start.X, start.Y, start.Z);

	glVertex3f(end.X, end.Y, end.Z);
	glEnd();
}



//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void COpenGLDriver::OnResize(const core::dimension2d<s32>& size)
{
	CNullDriver::OnResize(size);
	glViewport(0, 0, size.Width, size.Height);
}



//! Returns type of video driver
E_DRIVER_TYPE COpenGLDriver::getDriverType() const
{
	return EDT_OPENGL;
}



//! Sets a vertex shader constant.
void COpenGLDriver::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
#ifdef GL_ARB_vertex_program
	for (s32 i=0; i<constantAmount; ++i)
		extGlProgramLocalParameter4fv(GL_VERTEX_PROGRAM_ARB, startRegister+i, &data[i*4]);
#endif
}

//! Sets a pixel shader constant.
void COpenGLDriver::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
#ifdef GL_ARB_fragment_program
	for (s32 i=0; i<constantAmount; ++i)
		extGlProgramLocalParameter4fv(GL_FRAGMENT_PROGRAM_ARB, startRegister+i, &data[i*4]);
#endif
}

//! Sets a constant for the vertex shader based on a name.
bool COpenGLDriver::setVertexShaderConstant(const c8* name, const f32* floats, int count)
{
	//pass this along, as in GLSL the same routine is used for both vertex and fragment shaders
	return setPixelShaderConstant(name, floats, count);
}

//! Sets a constant for the pixel shader based on a name.
bool COpenGLDriver::setPixelShaderConstant(const c8* name, const f32* floats, int count)
{
	os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
	return false;
}


//! Adds a new material renderer to the VideoDriver, using pixel and/or
//! vertex shaders to render geometry.
s32 COpenGLDriver::addShaderMaterial(const c8* vertexShaderProgram,
	const c8* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial, s32 userData)
{
	s32 nr = -1;
	COpenGLShaderMaterialRenderer* r = new COpenGLShaderMaterialRenderer(
		this, nr, vertexShaderProgram, pixelShaderProgram,
		callback, getMaterialRenderer(baseMaterial), userData);

	r->drop();
	return nr;
}


//! Adds a new material renderer to the VideoDriver, using GLSL to render geometry.
s32 COpenGLDriver::addHighLevelShaderMaterial(
	const c8* vertexShaderProgram,
	const c8* vertexShaderEntryPointName,
	E_VERTEX_SHADER_TYPE vsCompileTarget,
	const c8* pixelShaderProgram,
	const c8* pixelShaderEntryPointName,
	E_PIXEL_SHADER_TYPE psCompileTarget,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	s32 nr = -1;

	COpenGLSLMaterialRenderer* r = new COpenGLSLMaterialRenderer(
		this, nr, vertexShaderProgram, vertexShaderEntryPointName,
		vsCompileTarget, pixelShaderProgram, pixelShaderEntryPointName, psCompileTarget,
		callback,getMaterialRenderer(baseMaterial), userData);

	r->drop();
	return nr;
}

//! Returns a pointer to the IVideoDriver interface. (Implementation for
//! IMaterialRendererServices)
IVideoDriver* COpenGLDriver::getVideoDriver()
{
	return this;
}


//! Returns pointer to the IGPUProgrammingServices interface.
IGPUProgrammingServices* COpenGLDriver::getGPUProgrammingServices()
{
	return this;
}


ITexture* COpenGLDriver::createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name)
{
	//disable mip-mapping
	bool generateMipLevels = getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);

	video::ITexture* rtt = 0;
	if (name==0)
		name="rt";
#if defined(GL_EXT_framebuffer_object)
	// if driver supports FrameBufferObjects, use them
	if (queryFeature(EVDF_FRAMEBUFFER_OBJECT))
		rtt = new COpenGLTexture(size, PackedDepthStencilExtension, name, this);
	else
#endif
	{
		rtt = addTexture(size, name);
		if (rtt)
		{
			rtt->grab();
			static_cast<video::COpenGLTexture*>(rtt)->setRenderTarget(true);
		}
	}

	//restore mip-mapping
	setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, generateMipLevels);

	return rtt;
}


//! Returns the maximum amount of primitives (mostly vertices) which
//! the device is able to render with one drawIndexedTriangleList
//! call.
u32 COpenGLDriver::getMaximalPrimitiveCount() const
{
	return 65535;// TODO: Fix all loaders to auto-split and then return the correct value: MaxIndices;
}


//! checks triangle count and print warning if wrong
bool COpenGLDriver::setRenderTarget(video::ITexture* texture, bool clearBackBuffer,
					bool clearZBuffer, SColor color)
{
	// check for right driver type

	if (texture && texture->getDriverType() != EDT_OPENGL)
	{
		os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
		return false;
	}

	// check if we should set the previous RT back

	bool ret = true;

	setTexture(0, 0);
	ResetRenderStates=true;
	if (RenderTargetTexture!=0)
	{
		if (RenderTargetTexture->isFrameBufferObject())
		{
			RenderTargetTexture->unbindFrameBufferObject();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, RenderTargetTexture->getOpenGLTextureName());

			// Copy Our ViewPort To The Texture
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
			RenderTargetTexture->getSize().Width, RenderTargetTexture->getSize().Height);
		}
	}

	if (texture)
	{
		// we want to set a new target. so do this.
		glViewport(0, 0, texture->getSize().Width, texture->getSize().Height);
		RenderTargetTexture = static_cast<COpenGLTexture*>(texture);
		CurrentRendertargetSize = texture->getSize();

		if (RenderTargetTexture->isFrameBufferObject())
		{
			RenderTargetTexture->bindFrameBufferObject();
		}
	}
	else
	{
		glViewport(0,0,ScreenSize.Width,ScreenSize.Height);
		RenderTargetTexture = 0;
		CurrentRendertargetSize = core::dimension2d<s32>(0,0);
	}

	GLbitfield mask = 0;
	if (clearBackBuffer)
	{
		const f32 inv = 1.0f / 255.0f;
		glClearColor(color.getRed() * inv, color.getGreen() * inv,
				color.getBlue() * inv, color.getAlpha() * inv);

		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (clearZBuffer)
	{
		glDepthMask(GL_TRUE);
		LastMaterial.ZWriteEnable=true;
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	glClear(mask);

	return ret;
}


// returns the current size of the screen or rendertarget
const core::dimension2d<s32>& COpenGLDriver::getCurrentRenderTargetSize() const
{
	if ( CurrentRendertargetSize.Width == 0 )
		return ScreenSize;
	else
		return CurrentRendertargetSize;
}


//! Clears the ZBuffer.
void COpenGLDriver::clearZBuffer()
{
	GLboolean enabled = GL_TRUE;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &enabled);

	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);

	glDepthMask(enabled);
}


//! Returns an image created from the last rendered frame.
IImage* COpenGLDriver::createScreenShot()
{
	IImage* newImage = new CImage(ECF_R8G8B8, ScreenSize);

	u8* pPixels = reinterpret_cast<u8*>(newImage->lock());
	if (!pPixels)
	{
		newImage->drop();
		return 0;
	}

	glReadPixels(0, 0, ScreenSize.Width, ScreenSize.Height, GL_RGB, GL_UNSIGNED_BYTE, pPixels);

	// opengl images are horizontally flipped, so we have to fix that here.
	s32 pitch=newImage->getPitch();
	u8* p2 = pPixels + (ScreenSize.Height - 1) * pitch;
	u8* tmpBuffer = new u8[pitch];
	for (s32 i=0; i < ScreenSize.Height; i += 2)
	{
		memcpy(tmpBuffer, pPixels, pitch);
		memcpy(pPixels, p2, pitch);
		memcpy(p2, tmpBuffer, pitch);
		pPixels += pitch;
		p2 -= pitch;
	}
	delete [] tmpBuffer;

	newImage->unlock();

	if (testGLError())
	{
		newImage->drop();
		return 0;
	}

	return newImage;
}


//! Set/unset a clipping plane.
//! There are at least 6 clipping planes available for the user to set at will.
//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
//! \param plane: The plane itself.
//! \param enable: If true, enable the clipping plane else disable it.
bool COpenGLDriver::setClipPlane(u32 index, const core::plane3df& plane, bool enable)
{
	if (index >= MaxUserClipPlanes)
		return false;

	UserClipPlane[index]=plane;
	enableClipPlane(index, enable);
	return true;
}


void COpenGLDriver::uploadClipPlane(u32 index)
{
	// opengl needs an array of doubles for the plane equation
	double clip_plane[4];
	clip_plane[0] = UserClipPlane[index].Normal.X;
	clip_plane[1] = UserClipPlane[index].Normal.Y;
	clip_plane[2] = UserClipPlane[index].Normal.Z;
	clip_plane[3] = UserClipPlane[index].D;
	glClipPlane(GL_CLIP_PLANE0 + index, clip_plane);
}


//! Enable/disable a clipping plane.
//! There are at least 6 clipping planes available for the user to set at will.
//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
//! \param enable: If true, enable the clipping plane else disable it.
void COpenGLDriver::enableClipPlane(u32 index, bool enable)
{
	if (index >= MaxUserClipPlanes)
		return;
	if (enable)
	{
		if (!UserClipPlaneEnabled[index])
		{
			uploadClipPlane(index);
			glEnable(GL_CLIP_PLANE0 + index);
		}
	}
	else
		glDisable(GL_CLIP_PLANE0 + index);

	UserClipPlaneEnabled[index]=enable;
}


} // end namespace
} // end namespace


namespace irr
{
namespace video
{


// -----------------------------------
// WINDOWS VERSION
// -----------------------------------
#ifdef _IRR_USE_WINDOWS_DEVICE_
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
	HWND window, u32 bits, bool fullscreen, bool stencilBuffer, io::IFileSystem* io, bool vsync, bool antiAlias)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	COpenGLDriver* ogl =  new COpenGLDriver(screenSize, window, fullscreen, stencilBuffer, io, antiAlias);
	if (!ogl->initDriver(screenSize, window, bits, fullscreen, vsync, stencilBuffer))
	{
		ogl->drop();
		ogl = 0;
	}
	return ogl;
#else
	return 0;
#endif // _IRR_COMPILE_WITH_OPENGL_
}
#endif // _IRR_USE_WINDOWS_DEVICE_

// -----------------------------------
// MACOSX VERSION
// -----------------------------------
#if defined(_IRR_USE_OSX_DEVICE_)
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
	CIrrDeviceMacOSX *device, bool fullscreen, bool stencilBuffer,
	io::IFileSystem* io, bool vsync, bool antiAlias)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	return new COpenGLDriver(screenSize, fullscreen, stencilBuffer,
		device, io, vsync, antiAlias);
#else
	return 0;
#endif //  _IRR_COMPILE_WITH_OPENGL_
}
#endif // _IRR_USE_OSX_DEVICE_

// -----------------------------------
// LINUX VERSION
// -----------------------------------
#ifdef _IRR_USE_LINUX_DEVICE_
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
		bool fullscreen, bool stencilBuffer, io::IFileSystem* io, bool vsync, bool antiAlias)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	return new COpenGLDriver(screenSize, fullscreen, stencilBuffer,
		io, vsync, antiAlias);
#else
	return 0;
#endif //  _IRR_COMPILE_WITH_OPENGL_
}
#endif // _IRR_USE_LINUX_DEVICE_

// -----------------------------------
// SDL VERSION
// -----------------------------------
#ifdef _IRR_USE_SDL_DEVICE_
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
		bool fullscreen, bool stencilBuffer, io::IFileSystem* io, bool vsync, bool antiAlias)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	return new COpenGLDriver(screenSize, fullscreen, stencilBuffer,
		io, vsync, antiAlias);
#else
	return 0;
#endif //  _IRR_COMPILE_WITH_OPENGL_
}
#endif // _IRR_USE_SDL_DEVICE_


} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_OPENGL_
