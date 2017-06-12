
#define NOMINMAX
#include "tjsCommHead.h"

#include "tjsArray.h"
#include "ShaderProgramIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "tjsArray.h"
#include "GLShaderUtil.h"
#include "CharacterSet.h"
#include "StorageIntf.h"
#include "UtilStreams.h"
#include <string.h>
#include <vector>

tTJSNI_ShaderProgram::tTJSNI_ShaderProgram() : Program(0) {
}
tTJSNI_ShaderProgram::~tTJSNI_ShaderProgram() {
}
/*
shader type
ES3.0
GL_FLOAT 	float
GL_FLOAT_VEC2 	vec2
GL_FLOAT_VEC3 	vec3
GL_FLOAT_VEC4 	vec4
GL_INT 	int
GL_INT_VEC2 	ivec2
GL_INT_VEC3 	ivec3
GL_INT_VEC4 	ivec4
GL_UNSIGNED_INT 	unsigned int
GL_UNSIGNED_INT_VEC2 	uvec2
GL_UNSIGNED_INT_VEC3 	uvec3
GL_UNSIGNED_INT_VEC4 	uvec4
GL_BOOL 	bool
GL_BOOL_VEC2 	bvec2
GL_BOOL_VEC3 	bvec3
GL_BOOL_VEC4 	bvec4
GL_FLOAT_MAT2 	mat2
GL_FLOAT_MAT3 	mat3
GL_FLOAT_MAT4 	mat4
GL_FLOAT_MAT2x3 	mat2x3
GL_FLOAT_MAT2x4 	mat2x4
GL_FLOAT_MAT3x2 	mat3x2
GL_FLOAT_MAT3x4 	mat3x4
GL_FLOAT_MAT4x2 	mat4x2
GL_FLOAT_MAT4x3 	mat4x3
GL_SAMPLER_2D 	sampler2D
GL_SAMPLER_3D 	sampler3D
GL_SAMPLER_CUBE 	samplerCube
GL_SAMPLER_2D_SHADOW 	sampler2DShadow
GL_SAMPLER_2D_ARRAY 	sampler2DArray
GL_SAMPLER_2D_ARRAY_SHADOW 	sampler2DArrayShadow
GL_SAMPLER_CUBE_SHADOW 	samplerCubeShadow
GL_INT_SAMPLER_2D 	isampler2D
GL_INT_SAMPLER_3D 	isampler3D
GL_INT_SAMPLER_CUBE 	isamplerCube
GL_INT_SAMPLER_2D_ARRAY 	isampler2DArray
GL_UNSIGNED_INT_SAMPLER_2D 	usampler2D
GL_UNSIGNED_INT_SAMPLER_3D 	usampler3D
GL_UNSIGNED_INT_SAMPLER_CUBE 	usamplerCube
GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 	usampler2DArray

ES2.0
GL_FLOAT
GL_FLOAT_VEC2
GL_FLOAT_VEC3
GL_FLOAT_VEC4
GL_INT
GL_INT_VEC2
GL_INT_VEC3
GL_INT_VEC4
GL_BOOL
GL_BOOL_VEC2
GL_BOOL_VEC3
GL_BOOL_VEC4
GL_FLOAT_MAT2
GL_FLOAT_MAT3
GL_FLOAT_MAT4
GL_SAMPLER_2D
GL_SAMPLER_CUBE
*/

void tTVPShaderParameter::Set( const tTJSVariant *param ) {
	switch( DataType ) {
	case GL_FLOAT:	// float
		if( Type == TypeName::Uniform ) {
			glUniform1f( Id, (float)(tjs_real)*param );
		}
		break;
	case GL_FLOAT_VEC2: 	// vec2
		break;
	case GL_FLOAT_VEC3: 	// vec3
		break;
	case GL_FLOAT_VEC4: 	// vec4
		break;
	case GL_INT: 	// int
		break;
	case GL_INT_VEC2: 	// ivec2
		break;
	case GL_INT_VEC3: 	// ivec3
		break;
	case GL_INT_VEC4: 	// ivec4
		break;
	case GL_UNSIGNED_INT: 	// unsigned int
		break;
	case GL_UNSIGNED_INT_VEC2: 	// uvec2
		break;
	case GL_UNSIGNED_INT_VEC3: 	// uvec3
		break;
	case GL_UNSIGNED_INT_VEC4: 	// uvec4
		break;
	case GL_BOOL: 	// bool
		break;
	case GL_BOOL_VEC2: 	// bvec2
		break;
	case GL_BOOL_VEC3: 	// bvec3
		break;
	case GL_BOOL_VEC4: 	// bvec4
		break;
	case GL_FLOAT_MAT2: 	// mat2
		break;
	case GL_FLOAT_MAT3: 	// mat3
		break;
	case GL_FLOAT_MAT4: 	// mat4
		break;
	case GL_FLOAT_MAT2x3: 	// mat2x3
		break;
	case GL_FLOAT_MAT2x4: 	// mat2x4
		break;
	case GL_FLOAT_MAT3x2: 	// mat3x2
		break;
	case GL_FLOAT_MAT3x4: 	// mat3x4
		break;
	case GL_FLOAT_MAT4x2: 	// mat4x2
		break;
	case GL_FLOAT_MAT4x3: 	// mat4x3
		break;
	case GL_SAMPLER_2D: 	// sampler2D
		break;
	case GL_SAMPLER_3D: 	// sampler3D
		break;
	case GL_SAMPLER_CUBE: 	// samplerCube
		break;
	case GL_SAMPLER_2D_SHADOW: 	// sampler2DShadow
		break;
	case GL_SAMPLER_2D_ARRAY: 	// sampler2DArray
		break;
	case GL_SAMPLER_2D_ARRAY_SHADOW: 	// sampler2DArrayShadow
		break;
	case GL_SAMPLER_CUBE_SHADOW: 	// samplerCubeShadow
		break;
	case GL_INT_SAMPLER_2D: 	// isampler2D
		break;
	case GL_INT_SAMPLER_3D: 	// isampler3D
		break;
	case GL_INT_SAMPLER_CUBE: 	// isamplerCube
		break;
	case GL_INT_SAMPLER_2D_ARRAY: 	// isampler2DArray
		break;
	case GL_UNSIGNED_INT_SAMPLER_2D: 	// usampler2D
		break;
	case GL_UNSIGNED_INT_SAMPLER_3D: 	// usampler3D
		break;
	case GL_UNSIGNED_INT_SAMPLER_CUBE: 	// usamplerCube
		break;
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: 	// usampler2DArray
		break;
	}
}
tjs_error TJS_INTF_METHOD tTJSNI_ShaderProgram::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	ttstr vertex = *param[0];
	ttstr fragment = *param[1];
	bool isFile = true;
	if( numparams > 2 ) {
		isFile = ((tjs_int)(*param[2])) != 0;
	}
	if( isFile ) {
		ttstr vertexFile = TVPNormalizeStorageName(vertex);
		tTVPStreamHolder vertexStream( vertexFile, TJS_BS_READ );
		tjs_uint size = (tjs_uint)vertexStream->GetSize();
		std::unique_ptr<tjs_uint8[]> vertexScript(new tjs_uint8[size+1]);
		tjs_uint read = vertexStream->Read( vertexScript.get(), size );
		if( read != size ) TJS_eTJSError( TJSReadError );
		vertexScript[size] = '\0';
		vertexStream.Close();

		ttstr fragmentFile = TVPNormalizeStorageName(fragment);
		tTVPStreamHolder fragmentStream( fragmentFile, TJS_BS_READ );
		size = (tjs_uint)fragmentStream->GetSize();
		std::unique_ptr<tjs_uint8[]> fragmentScript(new tjs_uint8[size+1]);
		read = fragmentStream->Read( fragmentScript.get(), size );
		if( read != size ) TJS_eTJSError( TJSReadError );
		fragmentScript[size] = '\0';
		fragmentStream.Close();
		std::string vs((char*)vertexScript.get());
		std::string fs( (char*)fragmentScript.get());
		Program = CompileProgram( vs, fs );
		if( Program == 0 ) {
			TVPThrowExceptionMessage(TJS_W("Shader compile error."));
			return TJS_E_FAIL;
		}
		GLint attrCount = 0, unifCount = 0;
		glGetProgramiv( Program, GL_ACTIVE_ATTRIBUTES, &attrCount );
		glGetProgramiv( Program, GL_ACTIVE_UNIFORMS, &unifCount );
		if( attrCount > 0 ) {
			GLint attrNameMaxLen = 0;
			glGetProgramiv( Program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attrNameMaxLen );
			std::vector<GLchar> attrName( attrNameMaxLen );
			std::vector<tjs_char> ttAttrName( attrNameMaxLen );
			for( GLint i = 0; i < attrCount; i++ ) {
				GLsizei len = 0;
				GLint size = 0;
				GLenum type = 0;
				glGetActiveAttrib( Program, i, attrNameMaxLen, &len, &size, &type, &( attrName[0] ) );

				tTVPShaderParameter param;
				param.Name = std::string( &( attrName[0] ) );
				tjs_int count = TVPUtf8ToWideCharString( &( attrName[0] ), &( ttAttrName[0] ) );
				ttAttrName[count] = TJS_W( '\0' );
				param.TjsName = ttstr( &( ttAttrName[0] ) );
				param.Id = glGetAttribLocation( Program, &( attrName[0] ) );
				param.Type = tTVPShaderParameter::TypeName::Attribute;
				param.DataType = type;
				param.ArrayCount = size;
				param.IsArray = attrName[len - 1] == ']';
				Parameters.push_back( param );
			}
		}
		if( unifCount > 0 ) {
			GLint unifNameMaxLen = 0;
			glGetProgramiv( Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &unifNameMaxLen );
			std::vector<GLchar> unifName( unifNameMaxLen );
			std::vector<tjs_char> ttUnifName( unifNameMaxLen );
			for( GLint i = 0; i < unifCount; i++ ) {
				GLsizei len = 0;
				GLint size = 0;
				GLenum type = 0;
				glGetActiveUniform( Program, i, unifNameMaxLen, &len, &size, &type, &( unifName[0] ) );
				tTVPShaderParameter param;
				param.Name = std::string( &( unifName[0] ) );
				tjs_int count = TVPUtf8ToWideCharString( &( unifName[0] ), &( ttUnifName[0] ) );
				ttUnifName[count] = TJS_W('\0');
				param.TjsName = ttstr( &( ttUnifName[0] ) );
				param.Id = glGetUniformLocation( Program, &( unifName[0] ) );
				param.Type = tTVPShaderParameter::TypeName::Uniform;
				param.DataType = type;
				param.ArrayCount = size;
				param.IsArray = unifName[len - 1] == ']';
				Parameters.push_back( param );
			}
		}
#if 0
		tTVPShaderInputs vsparam;
		tTVPShaderInputs fsparam;
		if( parser->ParseVertexShader( vs, vsparam ) && parser->ParseFragmentShader( fs, fsparam ) ) {
			std::vector<tTVPShaderParameter> params;
			ShaderParamConvert( Program, vsparam, params );
			ShaderParamConvert( Program, fsparam, params );
		}
		parser->ClearParseVertexShaderResults();
		parser->ClearParseFragmentShaderResults();
#endif
	} else {
		std::string vs, fs;
		if( TVPUtf16ToUtf8( vs, vertex.AsStdString() ) == false )
			TVPThrowExceptionMessage( TVPInvalidUTF16ToUTF8 );
		if( TVPUtf16ToUtf8( fs, fragment.AsStdString() ) == false )
			TVPThrowExceptionMessage( TVPInvalidUTF16ToUTF8 );
		Program = CompileProgram( vs, fs );
		if( Program == 0 ) {
			TVPThrowExceptionMessage(TJS_W("Shader compile error."));
			return TJS_E_FAIL;
		}
	}

	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_ShaderProgram::Invalidate() {
	if( Program ) {
		glDeleteProgram( Program );
		Program = 0;
	}
}
bool tTJSNI_ShaderProgram::HasShaderMemeber( const ttstr& name ) const {
	for( auto i = Parameters.begin(); i != Parameters.end(); i++ ) {
		if( ( *i ).TjsName == name ) return true;
	}
	return false;
}
tjs_error tTJSNI_ShaderProgram::SetShaderParam( const ttstr& name, const tTJSVariant *param ) {
	for( auto i = Parameters.begin(); i != Parameters.end(); i++ ) {
		if( ( *i ).TjsName == name ) {
			tTVPShaderParameter& sp = *i;
			sp.Set( param );
			return TJS_S_OK;
		}
	}
	return TJS_E_MEMBERNOTFOUND;
}
GLint tTJSNI_ShaderProgram::FindLocation( const std::string name ) const {
	for( auto i = Parameters.begin(); i != Parameters.end(); i++ ) {
		if( ( *i ).Name == name ) return ( *i ).Id;
	}
	return -1;
}
void tTJSNI_ShaderProgram::SetProgram() {
	glUseProgram( Program );
}
//---------------------------------------------------------------------------
// tTJSNC_ShaderProgram : TJS ShaderProgram class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_ShaderProgram::ClassID = -1;
tTJSNC_ShaderProgram::tTJSNC_ShaderProgram() : tTJSNativeClass(TJS_W("ShaderProgram"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(ShaderProgram) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_ShaderProgram, /*TJS class name*/ShaderProgram)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ShaderProgram)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
//----------------------------------------------------------------------
class tTSJNC_ShaderProgramObject: public tTJSCustomObject {
public:
	tjs_error TJS_INTF_METHOD PropSet( tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, const tTJSVariant *param, iTJSDispatch2 *objthis ) override {
		if( objthis && membername ) {
			tTJSNI_ShaderProgram* ni;
			tjs_error hr = objthis->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&ni );
			if( TJS_SUCCEEDED( hr ) ) {
				ttstr name( membername );
				tjs_error hr = ni->SetShaderParam( name, param );
				if( hr != TJS_E_MEMBERNOTFOUND && hr != TJS_S_OK ) {
					return hr;
				}
			}
		}
		return tTJSCustomObject::PropSet( flag, membername, hint, param, objthis );
	}
	tjs_error TJS_INTF_METHOD PropSetByVS( tjs_uint32 flag, tTJSVariantString *membername, const tTJSVariant *param, iTJSDispatch2 *objthis ) override  {
		if( membername && param && objthis ) {
			tTJSNI_ShaderProgram* ni;
			tjs_error hr = objthis->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&ni );
			if( TJS_SUCCEEDED( hr ) ) {
				ttstr name( membername );
				tjs_error hr = ni->SetShaderParam( name, param );
				if( hr != TJS_E_MEMBERNOTFOUND && hr != TJS_S_OK ) {
					return hr;
				}
			}
		}
		return tTJSCustomObject::PropSetByVS(flag,membername,param,objthis);
	}

	tjs_error TJS_INTF_METHOD PropGet( tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, tTJSVariant *result, iTJSDispatch2 *objthis ) override {
		return tTJSCustomObject::PropGet( flag, membername, hint, result, objthis );
	}
};

iTJSDispatch2 *tTJSNC_ShaderProgram::CreateBaseTJSObject() {
	return new tTSJNC_ShaderProgramObject();
}
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_ShaderProgram()
{
	tTJSNativeClass *cls = new tTJSNC_ShaderProgram();
	return cls;
}
//---------------------------------------------------------------------------

