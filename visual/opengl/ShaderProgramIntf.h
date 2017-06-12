/**
 * ShaderProgram クラス
 */

#ifndef ShaderProgramIntfH
#define ShaderProgramIntfH

#include "tjsNative.h"
#include "OpenGLHeader.h"
#include <string>
#include <vector>

struct tTVPShaderParameter {
	enum class TypeName : tjs_int {
		Uniform,
		Attribute,
	};
	std::string Name;
	ttstr		TjsName;
	GLint		Id;
	TypeName	Type;
	GLenum		DataType;
	GLint		ArrayCount;
	bool		IsArray;

	void Set( const tTJSVariant *param );
};

class tTJSNI_ShaderProgram : public tTJSNativeInstance {
	GLuint Program;
	std::vector<tTVPShaderParameter> Parameters;

public:
	tTJSNI_ShaderProgram();
	~tTJSNI_ShaderProgram() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	bool HasShaderMemeber( const ttstr& name ) const;
	tjs_error SetShaderParam( const ttstr& name, const tTJSVariant *param );
	GLint FindLocation( const std::string name ) const;

	void SetProgram();
};


//---------------------------------------------------------------------------
// tTJSNC_ShaderProgram : TJS ShaderProgram class
//---------------------------------------------------------------------------
class tTJSNC_ShaderProgram : public tTJSNativeClass
{
public:
	tTJSNC_ShaderProgram();
	static tjs_uint32 ClassID;

protected:
	iTJSDispatch2 *CreateBaseTJSObject() override;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_ShaderProgram(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_ShaderProgram();
#endif
