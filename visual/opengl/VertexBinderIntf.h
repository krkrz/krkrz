/**
 * VertexBinder クラス
 */

#ifndef VertexBinderIntfH
#define VertexBinderIntfH

#include "tjsNative.h"

class tTJSNI_VertexBinder : public tTJSNativeInstance {

	tTJSVariant VertexBufferObject;
	class tTJSNI_VertexBuffer* VertexBufferInstance = nullptr;

	tjs_int Stride = 0;
	tjs_uint ComponentCount = 4;
	tjs_uint Offset = 0;
	bool Normalize = false;

private:
	void SetVertexBufferObject( const tTJSVariant & val );
public:
	const tTJSVariant& GetVertexBufferObject() const { return VertexBufferObject; }

public:
	tTJSNI_VertexBinder();
	~tTJSNI_VertexBinder() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	const class tTJSNI_VertexBuffer* GetVertexBuffer() const { return VertexBufferInstance; }

	tjs_int GetStride() const { return Stride; }
	void SetStride( tjs_int stride ) { Stride = stride; }
	tjs_uint GetComponentCount() const { return ComponentCount; }
	void SetComponentCount( tjs_uint count ) { ComponentCount = count; }
	tjs_uint GetOffset() const { return Offset; }
	void SetOffset( tjs_uint offset ) { Offset = offset; }
	bool GetNormalize() const { return Normalize; }
	void SetNormalize( bool norm ) { Normalize = norm; }
};


//---------------------------------------------------------------------------
// tTJSNC_VertexBinder : TJS VertexBinder class
//---------------------------------------------------------------------------
class tTJSNC_VertexBinder : public tTJSNativeClass
{
public:
	tTJSNC_VertexBinder();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_VertexBinder(); }
};

//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_VertexBinder();
#endif
