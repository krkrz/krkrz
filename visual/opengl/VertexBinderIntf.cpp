
#include "tjsCommHead.h"

#include "VertexBinderIntf.h"
#include "VertexBufferIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "DebugIntf.h"
#include "tjsArray.h"
#include <memory>


//---------------------------------------------------------------------------
tTJSNI_VertexBinder::tTJSNI_VertexBinder() {
}
//---------------------------------------------------------------------------
tTJSNI_VertexBinder::~tTJSNI_VertexBinder() {
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_VertexBinder::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	if( param[0]->Type() == tvtObject ) {
		SetVertexBufferObject( *param[0] );
	} else {
		return TJS_E_INVALIDPARAM;
	}
	if( numparams >= 2 ) {
		Stride = *param[1];
	}
	if( numparams >= 3 ) {
		ComponentCount = (tjs_int)*param[2];
	}
	if( numparams >= 4 ) {
		Offset = (tjs_int)*param[3];
	}
	if( numparams >= 5 ) {
		Normalize = (tjs_int)*param[4] ? true : false;
	}
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_VertexBinder::Invalidate() {
	SetVertexBufferObject( tTJSVariant() );
}
//---------------------------------------------------------------------------
void tTJSNI_VertexBinder::SetVertexBufferObject( const tTJSVariant & val ) {
	// assign new vertex buffer
	VertexBufferObject = val;
	VertexBufferInstance = nullptr;

	// extract interface
	if( VertexBufferObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = VertexBufferObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_VertexBuffer::ClassID, (iTJSNativeInstance**)&VertexBufferInstance ) ) ) {
				VertexBufferInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive vertex buffer instance." ) );
			}
		}
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTJSNC_VertexBinder : TJS VertexBinder class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_VertexBinder::ClassID = -1;
tTJSNC_VertexBinder::tTJSNC_VertexBinder() : tTJSNativeClass(TJS_W("VertexBinder"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(VertexBinder) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_VertexBinder, /*TJS class name*/VertexBinder)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/VertexBinder)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(vertexBuffer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		*result = _this->GetVertexBufferObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(vertexBuffer)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stride)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		*result = _this->GetStride();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		_this->SetStride( *param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(stride)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(componentCount)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		*result = (tjs_int64)_this->GetComponentCount();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		_this->SetComponentCount( (tjs_uint)(tjs_int64)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(componentCount)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(offset)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		*result = (tjs_int64)_this->GetOffset();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		_this->SetOffset( (tjs_uint)(tjs_int64)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(offset)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(normalize)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		*result = _this->GetNormalize() ? (tjs_int)1 : (tjs_int)0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBinder);
		_this->SetNormalize( (tjs_int)*param ? true : false );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(normalize)
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_VertexBinder()
{
	tTJSNativeClass *cls = new tTJSNC_VertexBinder();
	return cls;
}
//---------------------------------------------------------------------------
