
#include "tjsCommHead.h"

#include "CanvasIntf.h"
#include "Matrix44Intf.h"
#include "Mesh2DIntf.h"
#include "OffscreenIntf.h"
#include "TextureIntf.h"
#include "BitmapIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "tjsArray.h"
#include "LayerIntf.h"
#include "RectItf.h"
#include "OpenGLHeader.h"
#include "OpenGLScreen.h"
#include "WindowIntf.h"

tTJSNI_Canvas::tTJSNI_Canvas() : GLScreen(nullptr) {
	TVPInitializeOpenGLPlatform();
}
tTJSNI_Canvas::~tTJSNI_Canvas() {
}
tjs_error TJS_INTF_METHOD tTJSNI_Canvas::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	// get the window native instance
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	tTJSNI_Window *win = nullptr;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Window::ClassID, (iTJSNativeInstance**)&win)))
			TVPThrowExceptionMessage( TJS_W("Parameter require Window class instance.") );
#ifdef WIN32
		// GetNativeHandle() とか言う名前にするべきよね
		HWND hWnd = win->GetWindowHandle();
		if( GLScreen ) delete GLScreen;
		GLScreen = new tTVPOpenGLScreen( (void*)hWnd );
#endif
	}
	if( !GLScreen ) TVPThrowExceptionMessage( TJS_W("Cannot initialize low level graphics system.") );
	if( !GLScreen->Initialize() ) TVPThrowExceptionMessage( TJS_W("Cannot initialize low level graphics system.") );


	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Canvas::Invalidate() {
}
void TJS_INTF_METHOD tTJSNI_Canvas::Destruct() {
}

// method
void tTJSNI_Canvas::Capture( class tTJSNI_Bitmap* bmp ) {}
void tTJSNI_Canvas::Clear( tjs_uint32 color ) {}
iTJSDispatch2* tTJSNI_Canvas::CreateTexture( class tTJSNI_Bitmap* bmp, bool gray ) { return nullptr; }
iTJSDispatch2* tTJSNI_Canvas::CreateTexture( const ttstr &filename, bool gray ) { return nullptr; }
void tTJSNI_Canvas::DrawScreen( class tTJSNI_Offscreen* screen, tjs_real opacity ) {}
void tTJSNI_Canvas::DrawScreenUT( class tTJSNI_Offscreen* screen, class tTJSNI_Texture* texture, tjs_int vague, tjs_real opacity ) {}
void tTJSNI_Canvas::SetClipMask( class tTJSNI_Texture* texture, tjs_int left, tjs_int top ) { /* TODO: 二期実装  */}
void tTJSNI_Canvas::Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 color ) {}
void tTJSNI_Canvas::Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 colors[4] ) {}
void tTJSNI_Canvas::DrawTexture( class tTJSNI_Texture* texture, tjs_int left, tjs_int top ) {}
void tTJSNI_Canvas::DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color ) {}

// prop
void tTJSNI_Canvas::SetTargetScreen( class tTJSNI_Offscreen* screen ) {}
iTJSDispatch2* tTJSNI_Canvas::GetTargetScreenNoAddRef() { return nullptr; }
void tTJSNI_Canvas::SetClipRect( class tTJSNI_Rect* rect ) {}
iTJSDispatch2* tTJSNI_Canvas::GetClipRectNoAddRef() { return nullptr; }
void tTJSNI_Canvas::SetBlendMode( tTVPBlendMode bm ) {}
tTVPBlendMode tTJSNI_Canvas::GetBlendMode() const { return tTVPBlendMode::bmAlpha; }
void tTJSNI_Canvas::SetStretchType( tTVPStretchType st ) {}
tTVPStretchType tTJSNI_Canvas::GetStretchType() const { return tTVPStretchType::stLinear; }
void tTJSNI_Canvas::SetMatrix( class tTJSNI_Matrix44* matrix ) {}
iTJSDispatch2* tTJSNI_Canvas::GetMatrixNoAddRef() { return nullptr; }
tjs_uint tTJSNI_Canvas::GetWidth() const { return 0;  }
tjs_uint tTJSNI_Canvas::GetHeight() const { return 0;  }

//---------------------------------------------------------------------------
// tTJSNC_Canvas : TJS Canvas class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Canvas::ClassID = -1;
tTJSNC_Canvas::tTJSNC_Canvas() : tTJSNativeClass(TJS_W("Canvas"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Canvas) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Canvas, /*TJS class name*/Canvas)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Canvas)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/clear)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	// パラメータ未指定時はprop::clearColorが使われる
	tjs_uint32 color = _this->GetClearColor();
	if( numparams > 0 ) {
		color = static_cast<tjs_uint32>((tjs_int64)*param[0]);
	}
	_this->Clear( color );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/clear)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/capture)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Bitmap* dstbmp = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&dstbmp)))
			return TJS_E_INVALIDPARAM;
		_this->Capture( dstbmp );
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/capture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Offscreen* screen = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&screen)))
			return TJS_E_INVALIDPARAM;
	}
	if(!screen) TVPThrowExceptionMessage(TJS_W("Parameter require Offscreen class instance."));
	tjs_real opacity = *param[1];
	_this->DrawScreen( screen, opacity );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawScreenUT )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Offscreen* screen = nullptr;
	{
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&screen ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !screen ) TVPThrowExceptionMessage( TJS_W( "Parameter require Offscreen class instance." ) );
	}

	tTJSNI_Texture* texture = nullptr;
	{
		tTJSVariantClosure clo = param[1]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !texture ) TVPThrowExceptionMessage( TJS_W( "Parameter require Texture class instance." ) );
	}

	tjs_int vague = *param[2];
	tjs_real opacity = *param[3];
	_this->DrawScreenUT( screen, texture, vague, opacity );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setClipMask)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture)))
			return TJS_E_INVALIDPARAM;
	}
	if(!texture) TVPThrowExceptionMessage(TJS_W("Parameter require Texture class instance."));

	tjs_int left = *param[1];
	tjs_int top = *param[2];
	_this->SetClipMask( texture, left, top );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setClipMask)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/fill)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 5 ) return TJS_E_BADPARAMCOUNT;

	tjs_int left = *param[0];
	tjs_int top = *param[1];
	tjs_int width = *param[2];
	tjs_int height = *param[3];

	tTJSVariantType vt = param[4]->Type();
	tjs_uint32 color = 0xffffffff;
	if( vt == tvtInteger ) {
		tjs_uint32 color = (tjs_uint32)(tjs_int64)*param[4];
	} else if( vt == tvtObject ) {
		tTJSVariantClosure clo = param[4]->AsObjectClosureNoAddRef();
		tjs_int count = TJSGetArrayElementCount(clo.Object);
		tjs_uint32 colors[4];
		if( count >= 4 ) {
			tTJSVariant tmp;
			for( tjs_int i = 0; i < 4; i++ ) {
				if(TJS_FAILED(clo.Object->PropGetByNum(TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis)))
					TVPThrowExceptionMessage( TJS_W("Insufficient number of arrays.") );
				colors[i] = (tjs_uint32)(tjs_int64)tmp;
			}
			_this->Fill( left, top, width, height, colors );
			return TJS_S_OK;
		}
	}
	_this->Fill( left, top, width, height, color );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/fill)
//----------------------------------------------------------------------
#if 0
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawMesh2D)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNI_Texture::ClassID, (iTJSNativeInstance**)&texture)))
			return TJS_E_INVALIDPARAM;
	}
	if(!texture) TVPThrowExceptionMessage(TJS_W("Parameter require Texture class instance."));

	// Mesh にしてしまうと3Dやる時に被るよな……
	// TODO もうちょっと名前考える
	tTJSNI_Mesh* mesh = nullptr;
	tTJSVariantClosure clo = param[1]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNI_Mesh::ClassID, (iTJSNativeInstance**)&mesh)))
			return TJS_E_INVALIDPARAM;
	}
	if(!mesh) TVPThrowExceptionMessage(TJS_W("Parameter require Mesh class instance."));

	_this->DrawMesh( texture, mesh );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawMesh2D)
#endif
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawTexture)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture)))
			return TJS_E_INVALIDPARAM;
	}
	if(!texture) TVPThrowExceptionMessage(TJS_W("Parameter require Texture class instance."));

	tjs_int left = *param[1];
	tjs_int top = *param[2];

	_this->DrawTexture( texture, left, top );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawTexture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawText)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 5 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Font* font = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Font::ClassID, (iTJSNativeInstance**)&font)))
			return TJS_E_INVALIDPARAM;
	}
	if(!font) TVPThrowExceptionMessage(TJS_W("Parameter require Font class instance."));

	tjs_int x = *param[1];
	tjs_int y = *param[2];
	ttstr text = *param[3];
	tjs_uint32 color = (tjs_uint32)(tjs_int64)*param[4];
	_this->DrawText( font, x, y, text, color );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawText)
//----------------------------------------------------------------------
#if 0
// Texture クラスのコンストラクタに移動する
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/createTexture)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	bool doGray = false;
	if( numparams > 1 ) {
		doGray = ((tjs_int)*(param[0])) != 0;
	}
	iTJSDispatch2* texture = nullptr;
	if( param[0]->Type() == tvtString ) {
		texture = _this->CreateTexture( *param[0], doGray );
	} else {
		tTJSNI_Bitmap* srcbmp = nullptr;
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
			tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&srcbmp)))
			return TJS_E_INVALIDPARAM;
		texture = _this->CreateTexture( srcbmp, doGray );
	}
	if( !texture ) {
		return TJS_E_INVALIDPARAM;
	}

	try {
		if(result) *result = texture;
	} catch(...) {
		if(texture) texture->Release();
		throw;
	}
	if(texture) texture->Release();

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/createTexture)
#endif 
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(clearColor)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int64)_this->GetClearColor();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetClearColor( static_cast<tjs_uint32>((tjs_int64)*param) );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(clearColor)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(targetScreen)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		iTJSDispatch2 *dsp = _this->GetTargetScreenNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		// void or null

		tTJSNI_Offscreen* screen = nullptr;
		tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
		if( param->Type() == tvtObject && clo.Object != nullptr ) {
			tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
			if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&screen)))
				return TJS_E_INVALIDPARAM;
		} else if( param->Type() != tvtVoid ) {
			return TJS_E_INVALIDPARAM;
		}
		_this->SetTargetScreen( screen );

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(targetScreen)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(clipRect)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		iTJSDispatch2 *dsp = _this->GetClipRectNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);

		tTJSNI_Rect* rect = nullptr;
		tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Rect::ClassID, (iTJSNativeInstance**)&rect)))
			return TJS_E_INVALIDPARAM;
		if( rect != nullptr ) _this->SetClipRect( rect );

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(clipRect)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(blendMode)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int)_this->GetBlendMode();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetBlendMode( (tTVPBlendMode)(tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(blendMode)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stretchType)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int)_this->GetStretchType();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetStretchType( (tTVPStretchType)(tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(stretchType)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(matrix)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		iTJSDispatch2 *dsp = _this->GetMatrixNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);

		tTJSNI_Matrix44* matrix = nullptr;
		tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Matrix44::ClassID, (iTJSNativeInstance**)&matrix)))
			return TJS_E_INVALIDPARAM;
		if( matrix != nullptr ) _this->SetMatrix( matrix );

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(matrix)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
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
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int64)_this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
