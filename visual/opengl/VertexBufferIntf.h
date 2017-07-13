/**
 * VertexBuffer クラス
 */

#ifndef VertexBufferIntfH
#define VertexBufferIntfH

#include "tjsNative.h"
#include "GLVertexBufferObject.h"

class tTJSNI_VertexBuffer : public tTJSNativeInstance {

	GLVertexBufferObject VertexBufferObject;
	GLenum DataType;

public:
	tTJSNI_VertexBuffer();
	~tTJSNI_VertexBuffer() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	tjs_int64 GetNativeHandle() const { return (tjs_int64)VertexBufferObject.id(); }
	bool IsIndex() const { return VertexBufferObject.isIndex(); }
	tjs_int GetUpdateType() const { return (tjs_int)VertexBufferObject.usage(); }
	tjs_int GetDataType() const { return (tjs_int)DataType; }
	tjs_int GetBufferSize() const { return (tjs_int)VertexBufferObject.size(); }
	void* Lock() { return VertexBufferObject.mapBuffer(); }
	void Unlock() { return VertexBufferObject.unmapBuffer(); }
	void SetVertex( const tTJSVariant *param, tjs_int offset );

	void BindFrameBuffer();
};


//---------------------------------------------------------------------------
// tTJSNC_VertexBuffer : TJS VertexBuffer class
//---------------------------------------------------------------------------
class tTJSNC_VertexBuffer : public tTJSNativeClass
{
public:
	tTJSNC_VertexBuffer();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_VertexBuffer(); }
};

//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_VertexBuffer();
#endif
