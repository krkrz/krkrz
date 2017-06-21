
#include "tjsCommHead.h"

#include "TextureIntf.h"
#include "BitmapIntf.h"
#include "GraphicsLoaderIntf.h"
#include "LayerBitmapIntf.h"
#include "LayerIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "TVPColor.h"	// clNone
#include "RectItf.h"
#include <memory>
#include <assert.h>

//----------------------------------------------------------------------
tTJSNI_Texture::tTJSNI_Texture() : SrcWidth(0), SrcHeight(0) {
	TVPTempBitmapHolderAddRef();
}
//----------------------------------------------------------------------
tTJSNI_Texture::~tTJSNI_Texture() {
	TVPTempBitmapHolderRelease();
}
//----------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_Texture::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	if( param[0]->Type() == tvtString ) {
		bool gray = false;
		if( numparams > 1 ) {
			gray = (tjs_int)*param[1] ? true : false;
		}
		bool powerof2 = false;
		if( numparams > 2 ) {
			powerof2 = (tjs_int)*param[2] ? true : false;
		}
		ttstr filename = *param[0];
		std::unique_ptr<tTVPBaseBitmap> bitmap( new tTVPBaseBitmap( TVPGetInitialBitmap() ) );
		// tTVPBaseBitmap経由して読み込む。キャッシュ機構などは共有される。
		TVPLoadGraphic( bitmap.get(), filename, clNone, 0, 0, gray ? glmGrayscale : glmNormalRGBA, nullptr, nullptr );
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
		LoadTexture( bitmap.get(), gray );
	} else if( param[0]->Type() == tvtObject ) {
		tTJSNI_Bitmap* bmp = nullptr;
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp)))
				return TJS_E_INVALIDPARAM;
		}
		if(!bmp) TVPThrowExceptionMessage(TJS_W("Parameter require Bitmap class instance."));
		bool gray = false;
		if( numparams > 1 ) {
			gray = (tjs_int)*param[1] ? true : false;
		}
		bool powerof2 = false;
		if( numparams > 2 ) {
			powerof2 = (tjs_int)*param[2] ? true : false;
		}
		bool isrecreate = false;
		const tTVPBaseBitmap* bitmap = bmp->GetBitmap();
		SrcWidth = bitmap->GetWidth();
		SrcHeight = bitmap->GetHeight();
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
			LoadTexture( bitmap, gray );
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
					std::unique_ptr<tTVPBaseBitmap> bitmap3( new tTVPBaseBitmap( dw, dh, 8 ) );
					tTVPBaseBitmap::CopyWithDifferentBitWidth( bitmap3.get(), bitmap2.get(), func );
					bitmap2.reset( bitmap3.release() );
				} else {
					// 8bit color to full color
					tTVPBaseBitmap::GrayToColorFunctor func;
					std::unique_ptr<tTVPBaseBitmap> bitmap3( new tTVPBaseBitmap( dw, dh, 32 ) );
					tTVPBaseBitmap::CopyWithDifferentBitWidth( bitmap3.get(), bitmap2.get(), func );
					bitmap2.reset( bitmap3.release() );
				}
			} else {
				assert( powerof2 );
				bitmap2->SetSizeWithFill( dw, dh, 0 );
			}
			LoadTexture( bitmap2.get(), gray );
		}
	} else if( numparams >= 2 && param[0]->Type() == tvtInteger && param[1]->Type() == tvtInteger ) {
		tjs_int width = *param[0];
		tjs_int height = *param[1];
		bool alpha = false;
		if( numparams > 2 ) {
			alpha = (tjs_int)*param[2] ? true : false;
		}
		// 未初期化データでテクスチャを作る。後でコピーする前提。
		Texture.create( width, height, nullptr, alpha ? GL_ALPHA : GL_RGBA );
	} else {
		return TJS_E_INVALIDPARAM;
	}
	return TJS_S_OK;
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Texture::Invalidate() {
	Texture.destory();
}
//----------------------------------------------------------------------
void tTJSNI_Texture::LoadTexture( const class tTVPBaseBitmap* bitmap, bool alpha ) {
	// Bitmap の内部表現が正順(上下反転されていない)ことを前提としているので注意
	Texture.create( bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetScanLine(0), alpha ? GL_ALPHA : GL_RGBA );
}
//----------------------------------------------------------------------
void tTJSNI_Texture::CopyBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect ) {
	tTVPRect clip( srcRect );
	if( clip.right > (tjs_int)bitmap->GetWidth() ) clip.right = bitmap->GetWidth();
	if( clip.bottom > (tjs_int)bitmap->GetHeight() ) clip.bottom = bitmap->GetHeight();
	if( clip.left < 0 ) clip.left = 0;
	if( clip.top < 0 ) clip.top = 0;
	if( (tjs_int)Texture.width() < (left+clip.get_width()) ) clip.set_width( Texture.width() - left );
	if( (tjs_int)Texture.height() < (top+clip.get_height()) ) clip.set_height( Texture.height() - top );

	std::unique_ptr<tjs_uint32[]> buffer(new tjs_uint32[clip.get_width()*clip.get_height()]);
	for( tjs_int y = clip.top, line = 0; y < clip.bottom; y++, line++ ) {
		tjs_uint32* sl = ((tjs_uint32*)bitmap->GetScanLine(y)) + clip.left;
		TVPRedBlueSwapCopy( &buffer[clip.get_width()*line], sl, clip.get_width() );
	}
	Texture.copyImage( left, top, clip.get_width(), clip.get_height(), (const void*)buffer.get() );
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
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Bitmap* bitmap = (tTJSNI_Bitmap*)TJSGetNativeInstance( tTJSNC_Bitmap::ClassID, param[2] );
	if( !bitmap ) return TJS_E_INVALIDPARAM;
	tTJSNI_Rect* rect = (tTJSNI_Rect*)TJSGetNativeInstance( tTJSNC_Rect::ClassID, param[3] );
	if( !rect ) return TJS_E_INVALIDPARAM;

	tjs_int left  = *param[0];
	tjs_int top   = *param[1];
	_this->CopyBitmap( left, top, bitmap->GetBitmap(), rect->Get() );

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
