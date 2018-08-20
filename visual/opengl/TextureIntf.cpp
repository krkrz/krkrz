
#include "tjsCommHead.h"

#include "tjsArray.h"
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
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp ) ) ) {
				tTJSArrayNI* sizeList = nullptr;
				if( ( numparams >= 2 ) && TJS_SUCCEEDED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, TJSGetArrayClassID(), (iTJSNativeInstance**)&sizeList ) ) ) {
					// 第一引数が配列の時、Mipmap生成する
					if( param[1]->Type() == tvtString ) {
						ttstr filename = *param[1];
						std::unique_ptr<tTVPBaseBitmap> bitmap( new tTVPBaseBitmap( TVPGetInitialBitmap() ) );
						TVPLoadGraphic( bitmap.get(), filename, clNone, 0, 0, glmNormalRGBA, nullptr, nullptr, true );
						tTVPBBStretchType type = stFastAreaAvg;
						if( numparams >= 3 && param[2]->Type() != tvtVoid )
							type = (tTVPBBStretchType)(tjs_int)*param[2];
						tjs_real typeopt = 0.0;
						if( numparams >= 4 )
							typeopt = (tjs_real)*param[3];
						else if( type == stFastCubic || type == stCubic )
							typeopt = -1.0;
						LoadMipmapTexture( bitmap.get(), sizeList, type, typeopt );
						return TJS_S_OK;
					}
#if 0	// Bitmap 渡しのケースは実装していない
					else {
						clo = param[1]->AsObjectClosureNoAddRef();
						if( clo.Object ) {
							// 第二引数がBitmap
							if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&bmp ) ) ) {
								return TJS_E_INVALIDPARAM;
							}
							tTVPBBStretchType type = stFastAreaAvg;
							if( numparams >= 3 && param[2]->Type() != tvtVoid )
								type = (tTVPBBStretchType)(tjs_int)*param[2];

							tjs_real typeopt = 0.0;
							if( numparams >= 4 )
								typeopt = (tjs_real)*param[3];
							else if( type == stFastCubic || type == stCubic )
								typeopt = -1.0;
							tjs_int h = bmp->GetHeight();
							tjs_int w = bmp->GetWidth();
							std::unique_ptr<tTVPBaseBitmap> bitmap( new tTVPBaseBitmap( w, h ) );
							const tTVPBaseBitmap* srcBmp = bmp->GetBitmap();
							for( tjs_int y = 0; y < h; y++ ) {
								const tjs_uint32* sl = static_cast<const tjs_uint32*>( srcBmp->GetScanLine( h - y - 1 ) );
								tjs_uint32* dl = static_cast<tjs_uint32*>( bitmap->GetScanLineForWrite( h - y - 1 ) );
								TVPRedBlueSwapCopy( dl, sl, w );
							}
							LoadMipmapTexture( bitmap.get(), tTVPTextureColorFormat::RGBA, sizeList, type, typeopt, true );
						}
						return TJS_S_OK;
					}
#endif
				}
				return TJS_E_INVALIDPARAM;
			}
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
			LoadTexture( bitmap, color, true );
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
			LoadTexture( bitmap2.get(), color, true );
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
void tTJSNI_Texture::LoadTexture( const class tTVPBaseBitmap* bitmap, tTVPTextureColorFormat color, bool rbswap ) {
	tjs_int bpp = color == tTVPTextureColorFormat::RGBA ? 4 : 1;
	tjs_int w = bitmap->GetWidth();
	tjs_int h = bitmap->GetHeight();
	tjs_int pitch = w * bpp;
	if( std::abs(bitmap->GetPitchBytes()) != pitch || ( bpp == 4 && rbswap) ) {
		// パディングされているので、そのままではコピーできない(OpenGL ES2.0の場合)
		// もしくは赤と青をスワップする必要がある
		if( bpp == 4 ) {
			std::unique_ptr<tjs_uint32[]> buffer( new tjs_uint32[w*h] );
			if( rbswap ) {
				for( tjs_int y = 0; y < h; y++ ) {
					tjs_uint32* sl = (tjs_uint32*)bitmap->GetScanLine( h-y-1 );
					TVPRedBlueSwapCopy( &buffer[w*y], sl, w );
				}
			} else {
				for( tjs_int y = 0; y < h; y++ ) {
					tjs_uint32* sl = (tjs_uint32*)bitmap->GetScanLine( h-y-1 );
					memcpy( &buffer[w*y], sl, pitch );
				}
			}
			Texture.create( w, h, buffer.get(), ColorToGLColor( color ) );
		} else {
			std::unique_ptr<tjs_uint8[]> buffer( new tjs_uint8[w*h] );
			for( tjs_int y = 0; y < h; y++ ) {
				tjs_uint8* sl = (tjs_uint8*)bitmap->GetScanLine( h-y-1 );
				memcpy( &buffer[w*y], sl, pitch );
			}
			Texture.create( w, h, buffer.get(), ColorToGLColor( color ) );
		}
	} else {
		// 上下反転のままコピーする
		//Texture.create( bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetScanLine( 0 ), ColorToGLColor( color ) );
		Texture.create( bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetScanLine( bitmap->GetHeight() - 1 ), ColorToGLColor( color ) );
	}
}
//----------------------------------------------------------------------
/**
 * 色は RGBA のみで、赤青入れ替えも行わないケースのみ実装する
 */
tjs_error tTJSNI_Texture::LoadMipmapTexture( const tTVPBaseBitmap* bitmap, class tTJSArrayNI* sizeList, enum tTVPBBStretchType type, tjs_real typeopt ) {
	tjs_uint count = static_cast<tjs_uint>(sizeList->Items.size() / 2);
	if( count < 1 ) return TJS_E_INVALIDPARAM;	// mipmapが生成できない

	std::vector<GLTextreImageSet> mipmap;
	mipmap.reserve( count + 1 );
	std::vector<std::unique_ptr<tjs_uint32[]> > buffers;

	// まずはオリジナルサイズのBitmapを格納する
	tjs_int w = bitmap->GetWidth();
	tjs_int h = bitmap->GetHeight();
	tjs_int pitch = w * 4;
	if( std::abs(bitmap->GetPitchBytes()) != pitch ) {
		buffers.push_back( std::unique_ptr<tjs_uint32[]>( new tjs_uint32[w*h] ) );
		tjs_uint32* buffer = buffers[0].get();
		for( tjs_int y = 0; y < h; y++ ) {
			tjs_uint32* sl = (tjs_uint32*)bitmap->GetScanLine( h - y - 1 );
			memcpy( &buffer[w*y], sl, pitch );
		}
		mipmap.emplace_back( GLTextreImageSet( w, h, buffer ) );
	} else {
		mipmap.emplace_back( GLTextreImageSet( w, h, bitmap->GetScanLine( h - 1 ) ) );
	}

	// ミップマップ生成、自前の縮小関数で縮小する
	tjs_int prew = w;
	tjs_int preh = h;
	std::vector<std::unique_ptr<tTVPBaseBitmap> > bmps;
	tTVPRect srcRect( 0, 0, w, h );
	for( tjs_uint i = 0; i < count; i++ ) {
		tjs_int sw = sizeList->Items[i * 2 + 0];
		tjs_int sh = sizeList->Items[i * 2 + 1];
		if( sw > prew || sh > preh ) return TJS_E_INVALIDPARAM;	// 順に小さいサイズにしていく必要がある
		tjs_int spitch = sw * 4;
		bmps.push_back( std::unique_ptr<tTVPBaseBitmap>( new tTVPBaseBitmap( sw, sh, 32, true ) ) );
		tTVPRect dstRect( 0, 0, sw, sh );
		tTVPRect clipRect( dstRect );
		tTVPBaseBitmap* dstBmp = bmps.back().get();
		dstBmp->StretchBlt( clipRect, dstRect, bitmap, srcRect, bmCopy, 255, false, type, typeopt );
		if( std::abs(dstBmp->GetPitchBytes()) != spitch ) {
			buffers.push_back( std::unique_ptr<tjs_uint32[]>( new tjs_uint32[sw*sh] ) );
			tjs_uint32* buffer = buffers.back().get();
			for( tjs_int y = 0; y < sh; y++ ) {
				tjs_uint32* sl = (tjs_uint32*)dstBmp->GetScanLine( sh - y - 1 );
				memcpy( &buffer[sw*y], sl, pitch );
			}
			mipmap.emplace_back( GLTextreImageSet( sw, sh, buffer ) );
			// バッファにコピーされたのでBitmapは不要、ここで削除してしまう
			auto bmpend = bmps.end();
			bmpend--;
			bmps.erase( bmpend );
		} else {
			mipmap.emplace_back( GLTextreImageSet( sw, sh, dstBmp->GetScanLine( sh - 1 ) ) );
		}
		prew = sw;
		preh = sh;
	}
	Texture.createMipmapTexture( mipmap );
	return TJS_S_OK;
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
void tTJSNI_Texture::SetDrawSize( tjs_uint width, tjs_uint height ) {
	const float w = (float)width;
	const float h = (float)height;
	const GLfloat vertices[] = {
		0.0f, 0.0f,	// 左上
		0.0f,    h,	// 左下
		   w, 0.0f,	// 右上
		   w,    h,	// 右下
	};
	GLVertexBufferObject& vbo = const_cast<GLVertexBufferObject&>( VertexBuffer );
	if( VertexBuffer.isCreated() ) {
		vbo.copyBuffer( 0, sizeof( vertices ), vertices );
	} else {
		vbo.createStaticVertex( vertices, sizeof( vertices ) );
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
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setDrawSize ) {
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Texture );

	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tjs_int64 width = *param[0];
	tjs_int64 height= *param[1];
	_this->SetDrawSize( (tjs_uint)width, (tjs_uint)height );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setDrawSize )
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
	tjs_int bottom = top+clip.get_height();

	// copy image for each format
	if( texture->GetImageFormat() == GL_RGBA && bitmap->Is32BPP() ) {
		std::unique_ptr<tjs_uint32[]> buffer(new tjs_uint32[clip.get_width()*clip.get_height()]);
		for( tjs_int y = clip.bottom-1, line = 0; y >= clip.top; y--, line++ ) {
			tjs_uint32* sl = ((tjs_uint32*)bitmap->GetScanLine(y)) + clip.left;
			TVPRedBlueSwapCopy( &buffer[clip.get_width()*line], sl, clip.get_width() );
		}
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
		glTexSubImage2D( GL_TEXTURE_2D, 0, left, texture->GetHeight()-bottom, clip.get_width(), clip.get_height(), texture->GetImageFormat(), GL_UNSIGNED_BYTE, (const void*)buffer.get() );
	} else if( texture->GetImageFormat() == GL_ALPHA && bitmap->Is8BPP() ) {
		std::unique_ptr<tjs_uint8[]> buffer(new tjs_uint8[clip.get_width()*clip.get_height()]);
		for( tjs_int y = clip.bottom-1, line = 0; y >= clip.top; y--, line++ ) {
			tjs_uint8* sl = ((tjs_uint8*)bitmap->GetScanLine(y)) + clip.left;
			memcpy( &buffer[clip.get_width()*line], sl, clip.get_width() );
		}
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
		glTexSubImage2D( GL_TEXTURE_2D, 0, left, texture->GetHeight()-bottom, clip.get_width(), clip.get_height(), texture->GetImageFormat(), GL_UNSIGNED_BYTE, (const void*)buffer.get() );
	} else {
		TVPAddLog(TJS_W("unsupported format"));
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------

