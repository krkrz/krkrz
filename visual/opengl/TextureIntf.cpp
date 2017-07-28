
#include "tjsCommHead.h"

#include "TextureIntf.h"
#include "BitmapIntf.h"
#include "GraphicsLoaderIntf.h"
#include "LayerBitmapIntf.h"
#include "LayerIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "TVPColor.h"	// clNone
#include "RectItf.h"
#include "DebugIntf.h"
#include <memory>
#include <assert.h>

bool TVPCopyBitmapToTexture( const iTVPTextureInfoIntrface* texture, tjs_int left, tjs_int top, const tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );
//----------------------------------------------------------------------
tTJSNI_Texture::tTJSNI_Texture() {
	TVPTempBitmapHolderAddRef();
}
//----------------------------------------------------------------------
tTJSNI_Texture::~tTJSNI_Texture() {
	TVPTempBitmapHolderRelease();
}
//----------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_Texture::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	/**
	* 画像読み込みする場合以下のパラメータが指定可能であるが、p2パラメータは非公開(undocument)としておく
	* @param bitmap/filename テクスチャの元となるBitmapクラスのインスタンス
	* @param format カラーフォーマット(tcfRGBA or tcfAlpha), tcfAlpha選択時に色情報はグレイスケール化される
	* @param is9patch 9patch情報を読み込むかどうか
	* @param p2 サイズ2の累乗化(必須ではないが高速化する環境もある、一応指定可能に) : undocument
	*/
	if( param[0]->Type() == tvtString ) {
		tTVPTextureColorFormat color = tTVPTextureColorFormat::RGBA;
		if( numparams > 1 ) {
			color = (tTVPTextureColorFormat)(tjs_int)*param[1];
		}
		bool is9patch = false;
		if( numparams > 2 ) {
			is9patch = (tjs_int)*param[2] ? true : false;
		}
		bool powerof2 = false;
		if( numparams > 3 ) {
			powerof2 = (tjs_int)*param[3] ? true : false;
		}
		ttstr filename = *param[0];
		std::unique_ptr<tTVPBaseBitmap> bitmap( new tTVPBaseBitmap( TVPGetInitialBitmap() ) );
		// tTVPBaseBitmap経由して読み込む。キャッシュ機構などは共有される。
		TVPLoadGraphic( bitmap.get(), filename, clNone, 0, 0, color == tTVPTextureColorFormat::Alpha ? glmGrayscale : glmNormalRGBA, nullptr, nullptr, true );
		if( is9patch && bitmap->Is32BPP() ) {
			bitmap->Read9PatchInfo( Scale9Patch, Margin9Patch );
			Scale9Patch.add_offsets( -1, -1 );
			Margin9Patch.add_offsets( -1, -1 );
			if( Margin9Patch.top >= 0 ) {
				if( Margin9Patch.bottom < 0 ) Margin9Patch.bottom = 0;
				if( Margin9Patch.right < 0 ) Margin9Patch.right = 0;
			}
			// 周囲1ピクセル削る
			std::unique_ptr<tTVPBaseBitmap> bitmap2( new tTVPBaseBitmap( bitmap->GetWidth()-2, bitmap->GetHeight()-2, 32, true ) );
			if( bitmap2->CopyRect( 0, 0, bitmap.get(), tTVPRect( 1, 1, bitmap->GetWidth()-1, bitmap->GetHeight()-1 ) ) ) {
				bitmap.reset( bitmap2.release() );
			}
			if( Margin9Patch.left >= 0 && Margin9Patch.top >= 0 && Margin9Patch.right >= 0 && Margin9Patch.bottom >= 0 ) {
				iTJSDispatch2 *ret = TVPCreateRectObject( Margin9Patch.left, Margin9Patch.top, Margin9Patch.right, Margin9Patch.bottom );
				SetMarginRectObject( tTJSVariant( ret, ret ) );
				if( ret ) ret->Release();
			}
		}
		SrcWidth = bitmap->GetWidth();
		SrcHeight = bitmap->GetHeight();
		if( powerof2 ) {
			// 2のべき乗化を行う
			tjs_uint w = bitmap->GetWidth();
			tjs_uint dw = ToPowerOfTwo(w);
			tjs_uint h = bitmap->GetHeight();
			tjs_uint dh = ToPowerOfTwo(h);
			if( w != dw || h != dh ) {
				bitmap->SetSizeWithFill( dw, dh, 0 );
			}
		}
		LoadTexture( bitmap.get(), color );
	} else if( param[0]->Type() == tvtObject ) {
		tTJSNI_Bitmap* bmp = nullptr;
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp)))
				return TJS_E_INVALIDPARAM;
		}
		if(!bmp) TVPThrowExceptionMessage(TJS_W("Parameter require Bitmap class instance."));
		tTVPTextureColorFormat color = tTVPTextureColorFormat::RGBA;
		if( numparams > 1 ) {
			color = (tTVPTextureColorFormat)(tjs_int)*param[1];
		}
		bool is9patch = false;
		if( numparams > 2 ) {
			is9patch = (tjs_int)*param[2] ? true : false;
		}
		bool powerof2 = false;
		if( numparams > 3 ) {
			powerof2 = (tjs_int)*param[3] ? true : false;
		}
		bool isrecreate = false;
		const tTVPBaseBitmap* bitmap = bmp->GetBitmap();
		std::unique_ptr<tTVPBaseBitmap> bitmapHolder9;
		if( is9patch && bitmap->Is32BPP() ) {
			bitmap->Read9PatchInfo( Scale9Patch, Margin9Patch );
			Scale9Patch.add_offsets( -1, -1 );
			Margin9Patch.add_offsets( -1, -1 );
			if( Margin9Patch.top >= 0 ) {
				if( Margin9Patch.bottom < 0 ) Margin9Patch.bottom = 0;
				if( Margin9Patch.right < 0 ) Margin9Patch.right = 0;
			}
			// 周囲1ピクセル削る
			std::unique_ptr<tTVPBaseBitmap> bitmap2( new tTVPBaseBitmap( bitmap->GetWidth() - 2, bitmap->GetHeight() - 2, 32, true ) );
			if( bitmap2->CopyRect( 0, 0, bitmap, tTVPRect( 1, 1, bitmap->GetWidth() - 1, bitmap->GetHeight() - 1 ) ) ) {
				bitmapHolder9.reset( bitmap2.release() );
				bitmap = bitmapHolder9.get();
			}
			if( Margin9Patch.left >= 0 && Margin9Patch.top >= 0 && Margin9Patch.right >= 0 && Margin9Patch.bottom >= 0 ) {
				iTJSDispatch2 *ret = TVPCreateRectObject( Margin9Patch.left, Margin9Patch.top, Margin9Patch.right, Margin9Patch.bottom );
				SetMarginRectObject( tTJSVariant( ret, ret ) );
				if( ret ) ret->Release();
			}
		}
		SrcWidth = bitmap->GetWidth();
		SrcHeight = bitmap->GetHeight();
		bool gray = color == tTVPTextureColorFormat::Alpha;
		if( gray == bitmap->Is8BPP() ) {
			if( powerof2 ) {
				if( IsPowerOfTwo( bitmap->GetWidth() ) == false || IsPowerOfTwo( bitmap->GetHeight() ) == false ) {
					isrecreate = true;
				}
			}
		} else {
			isrecreate = true;
		}
		if( isrecreate == false ) {
			// そのまま生成して問題ない
			LoadTexture( bitmap, color );
		} else {
			// 変更が入るので、コピーを作る
			std::unique_ptr<tTVPBaseBitmap> bitmap2( new tTVPBaseBitmap( *bitmap ) );
			tjs_uint dw = bitmap->GetWidth(), dh = bitmap->GetHeight();
			if( powerof2 ) {
				// 2のべき乗化を行う
				dw = ToPowerOfTwo( bitmap->GetWidth() );
				dh = ToPowerOfTwo( bitmap->GetHeight() );
			}
			if( gray != bitmap->Is8BPP() ) {
				if( bitmap->Is32BPP() ) {
					// full color to 8bit color
					assert( gray );
					tTVPRect r( 0, 0, bitmap->GetWidth(), bitmap->GetHeight() );
					bitmap2->DoGrayScale( r );
					tTVPBaseBitmap::GrayToAlphaFunctor func;
					std::unique_ptr<tTVPBaseBitmap> bitmap3( new tTVPBaseBitmap( dw, dh, 8, true ) );
					tTVPBaseBitmap::CopyWithDifferentBitWidth( bitmap3.get(), bitmap2.get(), func );
					bitmap2.reset( bitmap3.release() );
				} else {
					// 8bit color to full color
					tTVPBaseBitmap::GrayToColorFunctor func;
					std::unique_ptr<tTVPBaseBitmap> bitmap3( new tTVPBaseBitmap( dw, dh, 32, true ) );
					tTVPBaseBitmap::CopyWithDifferentBitWidth( bitmap3.get(), bitmap2.get(), func );
					bitmap2.reset( bitmap3.release() );
				}
			} else {
				assert( powerof2 );
				bitmap2->SetSizeWithFill( dw, dh, 0 );
			}
			LoadTexture( bitmap2.get(), color );
		}
	} else if( numparams >= 2 && param[0]->Type() == tvtInteger && param[1]->Type() == tvtInteger ) {
		tjs_int width = *param[0];
		tjs_int height = *param[1];
		bool alpha = false;
		tTVPTextureColorFormat color = tTVPTextureColorFormat::RGBA;
		if( numparams > 2 ) {
			color = (tTVPTextureColorFormat)(tjs_int)*param[2];
		}
		// 未初期化データでテクスチャを作る。後でコピーする前提。
		Texture.create( width, height, nullptr, ColorToGLColor(( tTVPTextureColorFormat)color) );
		// 全体が使用される前提
		SrcWidth = width;
		SrcHeight = height;
	} else {
		return TJS_E_INVALIDPARAM;
	}
	return TJS_S_OK;
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Texture::Invalidate() {
	Texture.destory();

	// release clip rect
	if( MarginRectObject.Type() == tvtObject )
		MarginRectObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, MarginRectObject.AsObjectNoAddRef() );
}
//----------------------------------------------------------------------
void tTJSNI_Texture::SetMarginRectObject( const tTJSVariant & val ) {
	// assign new rect
	MarginRectObject = val;
	MarginRectInstance = nullptr;

	// extract interface
	if( MarginRectObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = MarginRectObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Rect::ClassID, (iTJSNativeInstance**)&MarginRectInstance ) ) ) {
				MarginRectInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive rect instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
GLint tTJSNI_Texture::ColorToGLColor( tTVPTextureColorFormat color ) {
	switch( color ) {
	case tTVPTextureColorFormat::RGBA:
		return GL_RGBA;
	case tTVPTextureColorFormat::Alpha:
		return GL_ALPHA;
	default:
		return GL_RGBA;
	}
}
//----------------------------------------------------------------------
void tTJSNI_Texture::LoadTexture( const class tTVPBaseBitmap* bitmap, tTVPTextureColorFormat color ) {
	tjs_int bpp = color == tTVPTextureColorFormat::RGBA ? 4 : 1;
	tjs_int w = bitmap->GetWidth();
	tjs_int h = bitmap->GetHeight();
	tjs_int pitch = w * bpp;
	if( bitmap->GetPitchBytes() != pitch ) {
		// パディングされているので、そのままではコピーできない(OpenGL ES2.0の場合)
		if( bpp == 4 ) {
			std::unique_ptr<tjs_uint32[]> buffer( new tjs_uint32[w*h] );
			for( tjs_int y = 0; y < h; y++ ) {
				tjs_uint32* sl = (tjs_uint32*)bitmap->GetScanLine( y );
				memcpy( &buffer[w*y], sl, pitch );
			}
			Texture.create( w, h, buffer.get(), ColorToGLColor( color ) );
		} else {
			std::unique_ptr<tjs_uint8[]> buffer( new tjs_uint8[w*h] );
			for( tjs_int y = 0; y < h; y++ ) {
				tjs_uint8* sl = (tjs_uint8*)bitmap->GetScanLine( y );
				memcpy( &buffer[w*y], sl, pitch );
			}
			Texture.create( w, h, buffer.get(), ColorToGLColor( color ) );
		}
	} else {
		// Bitmap の内部表現が正順(上下反転されていない)ことを前提としているので注意
		Texture.create( bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetScanLine( 0 ), ColorToGLColor( color ) );
	}
}
//----------------------------------------------------------------------
void tTJSNI_Texture::CopyBitmap( tjs_int left, tjs_int top, const tTVPBaseBitmap* bitmap, const tTVPRect& srcRect ) {
	TVPCopyBitmapToTexture( this, left, top, bitmap, srcRect );
}
//----------------------------------------------------------------------
void  tTJSNI_Texture::CopyBitmap( const class tTVPBaseBitmap* bitmap ) {
	tTVPRect rect( 0, 0, bitmap->GetWidth(), bitmap->GetHeight() );
	TVPCopyBitmapToTexture( this, 0, 0, bitmap, rect );
}
//----------------------------------------------------------------------
bool tTJSNI_Texture::IsGray() const {
	return Texture.format() == GL_ALPHA;
}
//----------------------------------------------------------------------
bool tTJSNI_Texture::IsPowerOfTwo() const {
	return IsPowerOfTwo( Texture.width() ) && IsPowerOfTwo( Texture.height() );
}
//----------------------------------------------------------------------
tjs_int64 tTJSNI_Texture::GetVBOHandle() const {
	if( VertexBuffer.isCreated() ) {
		return VertexBuffer.id();
	} else {
		const float w = (float)Texture.width();
		const float h = (float)Texture.height();
		const GLfloat vertices[] = {
			0.0f, 0.0f,	// 左上
			0.0f,    h,	// 左下
			   w, 0.0f,	// 右上
			   w,    h,	// 右下
		};
		GLVertexBufferObject& vbo = const_cast<GLVertexBufferObject&>( VertexBuffer );
		vbo.createStaticVertex( vertices, sizeof(vertices) );
		return VertexBuffer.id();
	}
}
//----------------------------------------------------------------------
tjs_int tTJSNI_Texture::GetStretchType() const {
	return static_cast<tjs_int>(Texture.stretchType());
}
//----------------------------------------------------------------------
void tTJSNI_Texture::SetStretchType( tjs_int v ) {
	Texture.setStretchType( static_cast<GLenum>(v) );
}
//----------------------------------------------------------------------
tjs_int tTJSNI_Texture::GetWrapModeHorizontal() const {
	return static_cast<tjs_int>(Texture.wrapS());
}
//----------------------------------------------------------------------
void tTJSNI_Texture::SetWrapModeHorizontal( tjs_int v ) {
	Texture.setWrapS( static_cast<GLenum>(v) );
}
//----------------------------------------------------------------------
tjs_int tTJSNI_Texture::GetWrapModeVertical() const {
	return static_cast<tjs_int>(Texture.wrapT());
}
//----------------------------------------------------------------------
void tTJSNI_Texture::SetWrapModeVertical( tjs_int v ) {
	Texture.setWrapT( static_cast<GLenum>(v) );
}
//----------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTJSNC_Texture : TJS Texture class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Texture::ClassID = -1;
tTJSNC_Texture::tTJSNC_Texture() : tTJSNativeClass(TJS_W("Texture"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Texture) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Texture, /*TJS class name*/Texture)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Texture)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/copyRect)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
	if( numparams == 1 ) {
		tTJSNI_Bitmap* bitmap = (tTJSNI_Bitmap*)TJSGetNativeInstance( tTJSNC_Bitmap::ClassID, param[0] );
		if( !bitmap ) return TJS_E_INVALIDPARAM;
		_this->CopyBitmap( bitmap->GetBitmap() );
		return TJS_S_OK;
	}
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Bitmap* bitmap = (tTJSNI_Bitmap*)TJSGetNativeInstance( tTJSNC_Bitmap::ClassID, param[2] );
	if( !bitmap ) return TJS_E_INVALIDPARAM;
	tTJSNI_Rect* rect = (tTJSNI_Rect*)TJSGetNativeInstance( tTJSNC_Rect::ClassID, param[3] );
	tTVPRect srcRect;
	if( !rect ) {
		if( numparams >= 7 ) {
			srcRect.left = *param[3];
			srcRect.top = *param[4];
			srcRect.right = (tjs_int)*param[5] + srcRect.left;
			srcRect.bottom = (tjs_int)*param[6] + srcRect.top;
		} else {
			return TJS_E_INVALIDPARAM;
		}
	} else {
		srcRect = rect->Get();
	}

	tjs_int left  = *param[0];
	tjs_int top   = *param[1];
	_this->CopyBitmap( left, top, bitmap->GetBitmap(), srcRect );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/copyRect)
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
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
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int64)_this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(memoryWidth)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int64)_this->GetMemoryWidth();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(memoryWidth)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(memoryHeight)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int64)_this->GetMemoryHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(memoryHeight)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(isGray)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = _this->IsGray() ? (tjs_int)1 : (tjs_int)0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(isGray)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(isPowerOfTwo)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = _this->IsPowerOfTwo() ? (tjs_int)1 : (tjs_int)0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(isPowerOfTwo)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(nativeHandle)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = _this->GetNativeHandle();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(nativeHandle)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stretchType)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int)_this->GetStretchType();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		_this->SetStretchType( (tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(stretchType)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(wrapModeHorizontal)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int)_this->GetWrapModeHorizontal();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		_this->SetWrapModeHorizontal( (tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(wrapModeHorizontal)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(wrapModeVertical)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		*result = (tjs_int)_this->GetWrapModeVertical();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture);
		_this->SetWrapModeVertical( (tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(wrapModeVertical)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL( margin9Patch )
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture );
		*result = _this->GetMarginRectObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL( margin9Patch )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stNearest) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_NEAREST;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(stNearest)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stLinear) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_LINEAR;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(stLinear)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(wmRepeat) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_REPEAT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(wmRepeat)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(wmClampToEdge) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_CLAMP_TO_EDGE;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(wmClampToEdge)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(wmMirror) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_MIRRORED_REPEAT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(wmMirror)
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Texture()
{
	tTJSNativeClass *cls = new tTJSNC_Texture();
	return cls;
}
//---------------------------------------------------------------------------
// utility function
//---------------------------------------------------------------------------
bool TVPCopyBitmapToTexture( const iTVPTextureInfoIntrface* texture, tjs_int left, tjs_int top, const tTVPBaseBitmap* bitmap, const tTVPRect& srcRect ) {
	if( texture == nullptr || bitmap == nullptr ) return false;

	// clip src bitmap area
	tTVPRect clip( srcRect );
	if( clip.right > (tjs_int)bitmap->GetWidth() ) clip.right = bitmap->GetWidth();
	if( clip.bottom > (tjs_int)bitmap->GetHeight() ) clip.bottom = bitmap->GetHeight();
	if( clip.left < 0 ) clip.left = 0;
	if( clip.top < 0 ) clip.top = 0;

	// clip dest texture area
	if( left < 0 ) {
		clip.left += -left;
		left = 0;
	}
	if( top < 0 ) {
		clip.top += -top;
		top = 0;
	}
	if( (tjs_int)texture->GetWidth() < (left+clip.get_width()) ) clip.set_width( texture->GetWidth() - left );
	if( (tjs_int)texture->GetHeight() < (top+clip.get_height()) ) clip.set_height( texture->GetHeight() - top );

	// has copy area?
	if( clip.get_width() <= 0 || clip.get_height() <= 0 ) {
		TVPAddLog(TJS_W("out of area"));
		return false;
	}
	if( clip.left >= (tjs_int)bitmap->GetWidth() || clip.top >= (tjs_int)bitmap->GetHeight() ) {
		TVPAddLog(TJS_W("out of area"));
		return false;
	}

	// copy image for each format
	if( texture->GetImageFormat() == GL_RGBA && bitmap->Is32BPP() ) {
		std::unique_ptr<tjs_uint32[]> buffer(new tjs_uint32[clip.get_width()*clip.get_height()]);
		for( tjs_int y = clip.top, line = 0; y < clip.bottom; y++, line++ ) {
			tjs_uint32* sl = ((tjs_uint32*)bitmap->GetScanLine(y)) + clip.left;
			TVPRedBlueSwapCopy( &buffer[clip.get_width()*line], sl, clip.get_width() );
		}
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
		glTexSubImage2D( GL_TEXTURE_2D, 0, left, top, clip.get_width(), clip.get_height(), texture->GetImageFormat(), GL_UNSIGNED_BYTE, (const void*)buffer.get() );
	} else if( texture->GetImageFormat() == GL_ALPHA && bitmap->Is8BPP() ) {
		std::unique_ptr<tjs_uint8[]> buffer(new tjs_uint8[clip.get_width()*clip.get_height()]);
		for( tjs_int y = clip.top, line = 0; y < clip.bottom; y++, line++ ) {
			tjs_uint8* sl = ((tjs_uint8*)bitmap->GetScanLine(y)) + clip.left;
			memcpy( &buffer[clip.get_width()*line], sl, clip.get_width() );
		}
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
		glTexSubImage2D( GL_TEXTURE_2D, 0, left, top, clip.get_width(), clip.get_height(), texture->GetImageFormat(), GL_UNSIGNED_BYTE, (const void*)buffer.get() );
	} else {
		TVPAddLog(TJS_W("unsupported format"));
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------

