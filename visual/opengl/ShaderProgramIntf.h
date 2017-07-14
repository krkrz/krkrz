/**
 * ShaderProgram クラス
 */

#ifndef ShaderProgramIntfH
#define ShaderProgramIntfH

#include "tjsNative.h"
#include "OpenGLHeader.h"
#include <string>
#include <vector>
#include <memory>

/**
 * シェーダーに設定する値を管理する。
 */
struct tTVPShaderVariant {
	enum class ValueType: tjs_int {
		Integer,
		Float,
		UnsingedInteger,
		IntegerArray,
		FloatArray,
		UnsingedIntegerArray,
		Texture,
		Vertex,
	};
	union ShaderValue {
		GLint		i;
		GLfloat		f;
		GLuint		ui;	// ui or bool
		GLint		*ai;
		GLfloat		*af;
		GLuint		*aui;
		tTJSVariant	*v;
		ShaderValue( GLint v ) : i( v ) {}
		ShaderValue( GLfloat v ) : f( v ) {}
		ShaderValue( GLuint v ) : ui( v ) {}
		ShaderValue( GLint v[] ) : ai( v ) {}
		ShaderValue( GLfloat v[] ) : af( v ) {}
		ShaderValue( GLuint v[] ) : aui( v ) {}
		ShaderValue( tTJSVariant *v ) : v( v ) {}
	} Value;
	ValueType Type;

	tTVPShaderVariant( const tTVPShaderVariant& ) = delete;
	tTVPShaderVariant& operator=( const tTVPShaderVariant& ) = delete;
	tTVPShaderVariant() = delete;

	tTVPShaderVariant( GLint v ) : Value( v ), Type( ValueType::Integer) {}
	tTVPShaderVariant( GLfloat v ) : Value( v ), Type( ValueType::Float ) {}
	tTVPShaderVariant( GLuint v ) : Value( v ), Type( ValueType::UnsingedInteger ) {}
	tTVPShaderVariant( GLint *v ) : Value( v ), Type( ValueType::IntegerArray ) {}
	tTVPShaderVariant( GLfloat *v ) : Value( v ), Type( ValueType::FloatArray ) {}
	tTVPShaderVariant( GLuint *v ) : Value( v ), Type( ValueType::UnsingedIntegerArray ) {}
	tTVPShaderVariant( tTJSVariant *v, ValueType t ) : Value( v ), Type( t ) {}
	~tTVPShaderVariant() {
		switch( Type ) {
		case ValueType::IntegerArray:
			delete[] Value.ai;
			break;
		case ValueType::FloatArray:
			delete[] Value.af;
			break;
		case ValueType::UnsingedIntegerArray:
			delete[] Value.aui;
			break;
		case ValueType::Texture:
			delete Value.v;
			break;
		case ValueType::Vertex:
			delete Value.v;
			break;
		}
	}
	operator GLint() const { return Value.i; }
	operator GLfloat() const { return Value.f; }
	operator GLuint() const { return Value.ui; }
	operator GLint*() const { return Value.ai; }
	operator GLfloat*( ) const { return Value.af; }
	operator GLuint*( ) const { return Value.aui; }
	operator tTJSVariant*() const { return Value.v; }
};

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
	std::unique_ptr<tTVPShaderVariant> Value;

	void Set( const tTJSVariant *param );
	void SetToShader( tjs_int* texCount=nullptr, tjs_int* vertexCount=nullptr );

	static GLint* ArrayToInt( const tTJSVariant *param, tjs_int rqCount );
	static GLfloat* ArrayToFloat( const tTJSVariant *param, tjs_int rqCount );
	static GLuint* ArrayToUInt( const tTJSVariant *param, tjs_int rqCount );
	static GLuint* ArrayToBool( const tTJSVariant *param, tjs_int rqCount );
};

class tTJSNI_ShaderProgram : public tTJSNativeInstance {
	GLuint Program;
	std::vector<std::unique_ptr<tTVPShaderParameter> > Parameters;
	tjs_int BindTextureCount = 0;
	tjs_int BindVertexCount = 0;

private:
	/**
	 * シェーダーからパラメータを取得し、保持する。
	 */
	void RetrieveShaderParameter();

public:
	tTJSNI_ShaderProgram();
	~tTJSNI_ShaderProgram() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	bool HasShaderMemeber( const ttstr& name ) const;
	tjs_error SetShaderParam( const ttstr& name, const tTJSVariant *param );
	GLint FindLocation( const std::string name ) const;

	/**
	 * 固定メンバを除くUniformを設定する
	 */
	void SetupProgram();

	/**
	 * Uniformだけでなく、プロパティに設定されたAttributeな要素、Textureも設定する。
	 */
	void SetupProgramFull();

	/**
	 * SetupProgramFullで設定されたテクスチャと頂点情報を解除する
	 */
	void UnbindParam();
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
