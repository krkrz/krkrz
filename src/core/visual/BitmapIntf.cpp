
#include "tjsCommHead.h"
#include "BitmapIntf.h"
#include "MsgIntf.h"
#include "LayerBitmapIntf.h"
#include "GraphicsLoaderIntf.h"
#include "TColor.h"
#include "LayerIntf.h"

tTJSNI_Bitmap::tTJSNI_Bitmap() : Bitmap(NULL), Loading(false) {
}
// string, [uint] ファイル名、カラーキーの順で指定
// uint, uint, [bpp] 幅、高さ、bppの順で指定
tjs_error TJS_INTF_METHOD tTJSNI_Bitmap::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams > 0 ) {
		if( param[0]->Type() == tvtString ) {
			tjs_uint32 colorkey = clNone;
			if( numparams > 1 ) {
				colorkey = (tjs_uint32)(tjs_int64)*param[1];
			}
			// Bitmap = new tTVPBaseBitmap( TVPGetInitialBitmap() );
			iTJSDispatch2 *ret = Load( *param[0], colorkey );
			if( ret ) ret->Release(); // ignore
		} else if( numparams > 1 ) {
			tjs_uint32 width = (tjs_uint32)(tjs_int64)*param[0];
			tjs_uint32 height = (tjs_uint32)(tjs_int64)*param[1];
			tjs_uint32 bpp = 32;
			if( numparams > 2 ) {
				bpp = (tjs_int)param[2];
			}
			Bitmap = new tTVPBaseBitmap( width, height, bpp );
		}
	} else {
		Bitmap = new tTVPBaseBitmap( TVPGetInitialBitmap() );
	}
	return TJS_S_OK;
}

void TJS_INTF_METHOD tTJSNI_Bitmap::Invalidate() {
	if(Bitmap) delete Bitmap, Bitmap = NULL;
}

tjs_uint32 tTJSNI_Bitmap::GetPixel(tjs_int x, tjs_int y) const {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	return TVPFromActualColor(Bitmap->GetPoint(x, y) & 0xffffff);
}
void tTJSNI_Bitmap::SetPixel(tjs_int x, tjs_int y, tjs_uint32 color) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	Bitmap->SetPointMain(x, y, TVPToActualColor(color));
}

tjs_int tTJSNI_Bitmap::GetMaskPixel(tjs_int x, tjs_int y) const {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	return (Bitmap->GetPoint(x, y) & 0xff000000) >> 24;
}
void tTJSNI_Bitmap::SetMaskPixel(tjs_int x, tjs_int y, tjs_int mask) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	Bitmap->SetPointMask(x, y, mask);
}

void tTJSNI_Bitmap::Independ(bool copy) {
	if(Bitmap) {
		if(copy)
			Bitmap->Independ();
		else
			Bitmap->IndependNoCopy();
	}
}

iTJSDispatch2 * tTJSNI_Bitmap::Load(const ttstr &name, tjs_uint32 colorkey) {
	if( !Bitmap ) Bitmap = new tTVPBaseBitmap( TVPGetInitialBitmap() );

	iTJSDispatch2* metainfo = NULL;
	TVPLoadGraphic( Bitmap, name, colorkey, 0, 0, glmNormal, NULL, &metainfo);
	return metainfo;
}
void tTJSNI_Bitmap::LoadAsync(const ttstr &name, tjs_uint32 colorkey) {
	Loading = true;
}
void tTJSNI_Bitmap::Save(const ttstr &name, const ttstr &type) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	if( type.StartsWith(TJS_W("bmp")) )
		TVPSaveAsBMP(name, type, Bitmap);
}

void tTJSNI_Bitmap::SetSize(tjs_uint width, tjs_uint height) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	if(width == Bitmap->GetWidth() && height == Bitmap->GetHeight()) return;

	// be called from geographical management
	if(!width || !height)
		TVPThrowExceptionMessage(TVPCannotCreateEmptyLayerImage);

	Bitmap->SetSizeWithFill(width, height, 0);
}
void tTJSNI_Bitmap::SetWidth(tjs_uint width) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	if(width == Bitmap->GetWidth()) return;

	Bitmap->SetSizeWithFill(width, Bitmap->GetHeight(), 0);
}
tjs_uint tTJSNI_Bitmap::GetWidth() const {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);
	return Bitmap->GetWidth();
}
void tTJSNI_Bitmap::SetHeight(tjs_uint height) {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);

	if(height == Bitmap->GetHeight()) return;
	Bitmap->SetSizeWithFill(Bitmap->GetWidth(), height, 0);
}
tjs_uint tTJSNI_Bitmap::GetHeight() const {
	if(!Bitmap) TVPThrowExceptionMessage(TVPNotDrawableLayerType);
	return Bitmap->GetHeight();
}

const void * tTJSNI_Bitmap::GetPixelBuffer() const {
	if(!Bitmap) return NULL;
	return Bitmap->GetScanLine(0);
}
void * tTJSNI_Bitmap::GetPixelBufferForWrite() {
	if(!Bitmap) return NULL;
	return Bitmap->GetScanLineForWrite(0);
}
tjs_int tTJSNI_Bitmap::GetPixelBufferPitch() const {
	if(!Bitmap) return 0;
	return Bitmap->GetPitchBytes();
}

void tTJSNI_Bitmap::CopyFrom( const tTJSNI_Bitmap* src ) {
	Bitmap->Assign(*src->GetBitmap());
}

tjs_uint32 tTJSNC_Bitmap::ClassID = -1;

tTJSNC_Bitmap::tTJSNC_Bitmap() : inherited(TJS_W("Bitmap") ) {
	
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Bitmap) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Bitmap,
	/*TJS class name*/Bitmap)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Bitmap)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getPixel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;
	if(result) *result = (tjs_int64)_this->GetPixel(*param[0], *param[1]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/getPixel)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setPixel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 3) return TJS_E_BADPARAMCOUNT;
	_this->SetPixel(*param[0], *param[1], (tjs_int)*param[2]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setPixel)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getMaskPixel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;
	if(result) *result = _this->GetMaskPixel(*param[0], *param[1]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/getMaskPixel)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setMaskPixel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 3) return TJS_E_BADPARAMCOUNT;
	_this->SetMaskPixel(*param[0], *param[1], *param[2]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setMaskPixel)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/independ)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	bool copy = true;
	if(numparams >= 1 && param[0]->Type() != tvtVoid)
		copy = param[0]->operator bool();
	_this->Independ(copy);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/independ)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setSize)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;
	_this->SetSize((tjs_int)*param[0], (tjs_int)*param[1]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setSize)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/copyFrom)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	tTJSNI_Bitmap* srcbmp = NULL;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
			tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&srcbmp)))
			return TJS_E_INVALIDPARAM;
		_this->CopyFrom( srcbmp );
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/copyFrom)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/save)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	ttstr name(*param[0]);
	ttstr type(TJS_W("bmp"));
	if(numparams >=2 && param[1]->Type() != tvtVoid)
		type = *param[1];
	_this->Save(name, type); // TODO TLG6 の時のパラメータ配列も受けられるようにしたいところ
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/save)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/load)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	ttstr name(*param[0]);
	tjs_uint32 key = clNone;
	if(numparams >=2 && param[1]->Type() != tvtVoid)
		key = (tjs_uint32)param[1]->AsInteger();
	iTJSDispatch2 * metainfo = _this->Load(name, key);
	try {
		if(result) *result = metainfo;
	} catch(...) {
		if(metainfo) metainfo->Release();
		throw;
	}
	if(metainfo) metainfo->Release();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/load)
//----------------------------------------------------------------------


//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = (tjs_int64)_this->GetWidth();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		_this->SetWidth(static_cast<tjs_uint>((tjs_int64)*param));
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(width)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(height)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = (tjs_int64)_this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		_this->SetHeight(static_cast<tjs_uint>((tjs_int64)*param));
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(buffer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = reinterpret_cast<tjs_int>(_this->GetPixelBuffer());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(buffer)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(bufferForWrite)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = reinterpret_cast<tjs_int>(_this->GetPixelBufferForWrite());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(bufferForWrite)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(bufferPitch)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = _this->GetPixelBufferPitch();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(bufferPitch)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(loading)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Bitmap);
		*result = (tjs_int)( _this->IsLoading() ? 1 : 0 );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(loading)
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

tTJSNativeInstance *tTJSNC_Bitmap::CreateNativeInstance() {
	return new tTJSNI_Bitmap();
}


tTJSNativeClass * TVPCreateNativeClass_Bitmap()
{
	tTJSNativeClass *cls = new tTJSNC_Bitmap();
	return cls;
}
