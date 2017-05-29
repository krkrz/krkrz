
#include "tjsCommHead.h"

#include "OffscreenIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "BitmapIntf.h"

tTJSNI_Offscreen::tTJSNI_Offscreen() {
}
tTJSNI_Offscreen::~tTJSNI_Offscreen() {
}
tjs_error TJS_INTF_METHOD tTJSNI_Offscreen::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Offscreen::Invalidate() {
}
void TJS_INTF_METHOD tTJSNI_Offscreen::Destruct() {
}

void tTJSNI_Offscreen::CopyFromBitmap( class tTJSNI_Bitmap* bmp, tjs_int sleft, tjs_int stop, tjs_int width, tjs_int height, tjs_int left, tjs_int top ) {}
void tTJSNI_Offscreen::CopyToBitmap( class tTJSNI_Bitmap* bmp, tjs_int sleft, tjs_int stop, tjs_int width, tjs_int height, tjs_int dleft, tjs_int dtop ) {}
void tTJSNI_Offscreen::CopyToBitmap( class tTJSNI_Bitmap* bmp ) {}
void tTJSNI_Offscreen::Update() {}

tjs_uint tTJSNI_Offscreen::GetWidth() const {
	return 0;
}
tjs_uint tTJSNI_Offscreen::GetHeight() const {
	return 0;
}
tjs_int64 tTJSNI_Offscreen::GetNativeHandle() const {
	return 0;
}

//---------------------------------------------------------------------------
// tTJSNC_Offscreen : TJS Offscreen class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Offscreen::ClassID = -1;
tTJSNC_Offscreen::tTJSNC_Offscreen() : tTJSNativeClass(TJS_W("Offscreen"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Offscreen) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Offscreen, /*TJS class name*/Offscreen)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Offscreen)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/copyFromBitmap)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
	if( numparams < 7 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Bitmap* bmp = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp)))
			return TJS_E_INVALIDPARAM;
	}
	if(!bmp) TVPThrowExceptionMessage(TJS_W("Parameter require Bitmap class instance."));

	tjs_int sleft  = *param[1];
	tjs_int stop   = *param[2];
	tjs_int width  = *param[3];
	tjs_int height = *param[4];
	tjs_int left   = *param[5];
	tjs_int top    = *param[6];
	_this->CopyFromBitmap( bmp, sleft, stop, width, height, left, top );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/copyFromBitmap)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/copyToBitmap)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
	if( numparams < 7 || numparams != 1 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Bitmap* bmp = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp)))
			return TJS_E_INVALIDPARAM;
	}
	if(!bmp) TVPThrowExceptionMessage(TJS_W("Parameter require Bitmap class instance."));

	if( numparams > 1 ) {
		tjs_int sleft  = *param[1];
		tjs_int stop   = *param[2];
		tjs_int width  = *param[3];
		tjs_int height = *param[4];
		tjs_int dleft   = *param[5];
		tjs_int dtop    = *param[6];
		_this->CopyToBitmap( bmp, sleft, stop, width, height, dleft, dtop );
	} else {
		// 引数が1つだけの時は、全体コピーでBitmap側のサイズをOffscreenに合わせる
		_this->CopyToBitmap( bmp );
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/copyToBitmap)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/update)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
	// TODO: LayerTree 機能をデフォルトでOffscreenに持たせる方がいいか
	// その際自動更新か、手動更新か
	// 最初にレイヤーが追加されようとしたときに、マネージャ処々を生成するのがいいか
	_this->Update();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/update)
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
		*result = (tjs_int64)_this->GetWidth();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(width)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(height)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
		*result = (tjs_int64)_this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(nativeHandle)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Offscreen);
		*result = _this->GetNativeHandle();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(nativeHandle)
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Offscreen()
{
	tTJSNativeClass *cls = new tTJSNC_Offscreen();
	return cls;
}
//---------------------------------------------------------------------------
