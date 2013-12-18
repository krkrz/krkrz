// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// This file was originally written by William Finlayson.  I (Nikolaus
// Gebhardt) did some minor modifications and changes to it and integrated it
// into Irrlicht. Thanks a lot to William for his work on this and that he gave
// me his permission to add it into Irrlicht using the zlib license.

// After Irrlicht 0.12, Michael Zoech did some improvements to this renderer, I
// merged this into Irrlicht 0.14, thanks to him for his work.

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "COpenGLSLMaterialRenderer.h"
#include "IGPUProgrammingServices.h"
#include "IShaderConstantSetCallBack.h"
#include "IMaterialRendererServices.h"
#include "IVideoDriver.h"
#include "os.h"
#include "COpenGLDriver.h"

namespace irr
{
namespace video
{


//! Constructor
COpenGLSLMaterialRenderer::COpenGLSLMaterialRenderer(video::COpenGLDriver* driver,
		s32& outMaterialTypeNr, const c8* vertexShaderProgram,
		const c8* vertexShaderEntryPointName,
		E_VERTEX_SHADER_TYPE vsCompileTarget,
		const c8* pixelShaderProgram,
		const c8* pixelShaderEntryPointName,
		E_PIXEL_SHADER_TYPE psCompileTarget,
		IShaderConstantSetCallBack* callback,
		video::IMaterialRenderer* baseMaterial,
		s32 userData)
	: Driver(driver), CallBack(callback), BaseMaterial(baseMaterial),
		Program(0), UserData(userData)
{
	//entry points must always be main, and the compile target isn't selectable
	//it is fine to ignore what has been asked for, as the compiler should spot anything wrong
	//just check that GLSL is available

	if (BaseMaterial)
		BaseMaterial->grab();

	if (CallBack)
		CallBack->grab();

	if (!Driver->queryFeature(EVDF_ARB_GLSL))
		return;

	init(outMaterialTypeNr, vertexShaderProgram, pixelShaderProgram);
}


//! constructor only for use by derived classes who want to
//! create a fall back material for example.
COpenGLSLMaterialRenderer::COpenGLSLMaterialRenderer(COpenGLDriver* driver,
					IShaderConstantSetCallBack* callback,
					IMaterialRenderer* baseMaterial, s32 userData)
: Driver(driver), CallBack(callback), BaseMaterial(baseMaterial),
		Program(0), UserData(userData)
{
	if (BaseMaterial)
		BaseMaterial->grab();

	if (CallBack)
		CallBack->grab();
}


//! Destructor
COpenGLSLMaterialRenderer::~COpenGLSLMaterialRenderer()
{
	if (CallBack)
		CallBack->drop();

	if(Program)
	{
		Driver->extGlDeleteObject(Program);
		Program = 0;
	}

	UniformInfo.clear();

	if (BaseMaterial)
		BaseMaterial->drop();
}

void COpenGLSLMaterialRenderer::init(s32& outMaterialTypeNr,
	const c8* vertexShaderProgram,
	const c8* pixelShaderProgram)
{
	outMaterialTypeNr = -1;

	if (!createProgram())
		return;

#if defined(GL_ARB_vertex_shader) && defined (GL_ARB_fragment_shader)
	if (!createShader(GL_VERTEX_SHADER_ARB, vertexShaderProgram))
		return;

	if (!createShader(GL_FRAGMENT_SHADER_ARB, pixelShaderProgram))
		return;
#endif

	if (!linkProgram())
		return;

	// register myself as new material
	outMaterialTypeNr = Driver->addMaterialRenderer(this);
}

bool COpenGLSLMaterialRenderer::OnRender(IMaterialRendererServices* service,
					E_VERTEX_TYPE vtxtype)
{
	// call callback to set shader constants
	if (CallBack && (Program))
		CallBack->OnSetConstants(this, UserData);

	return true;
}


void COpenGLSLMaterialRenderer::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
	if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
	{
		if(Program)
			Driver->extGlUseProgramObject(Program);

		if (BaseMaterial)
			BaseMaterial->OnSetMaterial(material, material, true, this);
	}

	for (u32 i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		Driver->setTexture(i, material.getTexture(i));
	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}


void COpenGLSLMaterialRenderer::OnUnsetMaterial()
{
	Driver->extGlUseProgramObject(0);

	if (BaseMaterial)
		BaseMaterial->OnUnsetMaterial();
}

//! Returns if the material is transparent.
bool COpenGLSLMaterialRenderer::isTransparent() const
{
	return BaseMaterial ? BaseMaterial->isTransparent() : false;
}

bool COpenGLSLMaterialRenderer::createProgram()
{
	Program = Driver->extGlCreateProgramObject();
	return true;
}

bool COpenGLSLMaterialRenderer::createShader(GLenum shaderType, const char* shader)
{
	GLhandleARB shaderHandle = Driver->extGlCreateShaderObject(shaderType);

	Driver->extGlShaderSource(shaderHandle, 1, &shader, NULL);
	Driver->extGlCompileShader(shaderHandle);

	int status = 0;

#ifdef GL_ARB_shader_objects
	Driver->extGlGetObjectParameteriv(shaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
#endif

	if (!status)
	{
		os::Printer::log("GLSL shader failed to compile");
		// check error message and log it
		int maxLength=0;
		GLsizei length;
#ifdef GL_ARB_shader_objects
		Driver->extGlGetObjectParameteriv(shaderHandle,
				GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);
#endif
		GLcharARB *pInfoLog = new GLcharARB[maxLength];
		Driver->extGlGetInfoLog(shaderHandle, maxLength, &length, pInfoLog);
		os::Printer::log(reinterpret_cast<const c8*>(pInfoLog));
		delete [] pInfoLog;

		return false;
	}

	Driver->extGlAttachObject(Program, shaderHandle);

	return true;
}

bool COpenGLSLMaterialRenderer::linkProgram()
{
	Driver->extGlLinkProgram(Program);

	int status = 0;

#ifdef GL_ARB_shader_objects
	Driver->extGlGetObjectParameteriv(Program, GL_OBJECT_LINK_STATUS_ARB, &status);
#endif

	if (!status)
	{
		os::Printer::log("GLSL shader program failed to link");
		// check error message and log it
		int maxLength=0;
		GLsizei length;
#ifdef GL_ARB_shader_objects
		Driver->extGlGetObjectParameteriv(Program,
				GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);
#endif
		GLcharARB *pInfoLog = new GLcharARB[maxLength];
		Driver->extGlGetInfoLog(Program, maxLength, &length, pInfoLog);
		os::Printer::log(reinterpret_cast<const c8*>(pInfoLog));
		delete [] pInfoLog;

		return false;
	}

	// get uniforms information

	int num = 0;
#ifdef GL_ARB_shader_objects
	Driver->extGlGetObjectParameteriv(Program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &num);
#endif

	if (num == 0)
	{
		// no uniforms
		return true;
	}

	int maxlen = 0;
#ifdef GL_ARB_shader_objects
	Driver->extGlGetObjectParameteriv(Program, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &maxlen);
#endif

	if (maxlen == 0)
	{
		os::Printer::log("GLSL: failed to retrieve uniform information");
		return false;
	}

	c8 *buf = new c8[maxlen];

	UniformInfo.clear();
	UniformInfo.reallocate(num);

	for (int i=0; i < num; ++i)
	{
		SUniformInfo ui;
		memset(buf, 0, maxlen);

		GLint size;
		Driver->extGlGetActiveUniform(Program, i, maxlen, 0, &size, &ui.type, reinterpret_cast<GLcharARB*>(buf));
		ui.name = buf;

		UniformInfo.push_back(ui);
	}

	delete [] buf;

	return true;
}



void COpenGLSLMaterialRenderer::setBasicRenderStates(const SMaterial& material,
						const SMaterial& lastMaterial,
						bool resetAllRenderstates)
{
	// forward
	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}


bool COpenGLSLMaterialRenderer::setVertexShaderConstant(const c8* name, const f32* floats, int count)
{
	return setPixelShaderConstant(name, floats, count);
}


void COpenGLSLMaterialRenderer::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	os::Printer::log("Cannot set constant, please use high level shader call instead.");
}

bool COpenGLSLMaterialRenderer::setPixelShaderConstant(const c8* name, const f32* floats, int count)
{
	int i, num = static_cast<int>(UniformInfo.size());

	for (i=0; i < num; ++i)
	{
		if (UniformInfo[i].name == name)
			break;
	}

	if (i == num)
		return false;

#ifdef GL_ARB_shader_objects
	switch (UniformInfo[i].type)
	{
		case GL_FLOAT:
			Driver->extGlUniform1fv(i, count, floats);
			break;
		case GL_FLOAT_VEC2_ARB:
			Driver->extGlUniform2fv(i, count/2, floats);
			break;
		case GL_FLOAT_VEC3_ARB:
			Driver->extGlUniform3fv(i, count/3, floats);
			break;
		case GL_FLOAT_VEC4_ARB:
			Driver->extGlUniform4fv(i, count/4, floats);
			break;
		case GL_FLOAT_MAT2_ARB:
			Driver->extGlUniformMatrix2fv(i, count/4, false, floats);
			break;
		case GL_FLOAT_MAT3_ARB:
			Driver->extGlUniformMatrix3fv(i, count/9, false, floats);
			break;
		case GL_FLOAT_MAT4_ARB:
			Driver->extGlUniformMatrix4fv(i, count/16, false, floats);
			break;
		default:
			Driver->extGlUniform1iv(i, count, reinterpret_cast<const GLint*>(floats));
			break;
	}
#endif

	return true;
}

void COpenGLSLMaterialRenderer::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	os::Printer::log("Cannot set constant, use high level shader call.");
}

IVideoDriver* COpenGLSLMaterialRenderer::getVideoDriver()
{
	return Driver;
}

} // end namespace video
} // end namespace irr


#endif

