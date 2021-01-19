/**
 * Texture クラス
 */

#ifndef TextureIntfH
#define TextureIntfH

#include "tjsNative.h"
#include "LayerBitmapIntf.h"
#include "GLTexture.h"
#include "TextureInfo.h"
#include "GLVertexBufferObject.h"
#include "ComplexRect.h"
#include "LayerBitmapIntf.h"


enum class tTVPTextureColorFormat : tjs_int {
	RGBA = 0,
	Alpha = 1,
	// Luminance or Compressed texture
};


class tTJSNI_Texture : public tTJSNativeInstance, public iTVPTextureInfoIntrface
{
	GLTexture Texture;
	GLVertexBufferObject VertexBuffer;
	tjs_uint SrcWidth = 0;
	tjs_uint SrcHeight = 0;
	// 9patch描画用情報
	tTVPRect Scale9Patch = { -1, -1, -1, -1 };
	tTVPRect Margin9Patch = { -1, -1, -1, -1 };

	tTJSVariant MarginRectObject;
	class tTJSNI_Rect* MarginRectInstance = nullptr;

	void LoadTexture( const class tTVPBaseBitmap* bitmap, tTVPTextureColorFormat color, bool rbswap = false );
	tjs_error LoadMipmapTexture( const class tTVPBaseBitmap* bitmap, class tTJSArrayNI* sizeList, enum tTVPBBStretchType type, tjs_real typeopt );
	GLint ColorToGLColor( tTVPTextureColorFormat color );

	void SetMarginRectObject( const tTJSVariant & val );
public:
	tTJSNI_Texture();
	~tTJSNI_Texture() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	const tTJSVariant& GetMarginRectObject() const { return MarginRectObject; }

	void CopyBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );
	void CopyBitmap( const class tTVPBaseBitmap* bitmap );

	tjs_uint GetWidth() const override { return SrcWidth; }
	tjs_uint GetHeight() const override { return SrcHeight; }
	tjs_uint GetMemoryWidth() const { return Texture.width(); }
	tjs_uint GetMemoryHeight() const { return Texture.height(); }
	bool IsGray() const;
	bool IsPowerOfTwo() const;
	tjs_int64 GetNativeHandle() const override { return Texture.id(); }
	tjs_int64 GetVBOHandle() const override;
	// VBOに描画サイズを設定しておき、テクスチャサイズ以外で描画させる
	void SetDrawSize( tjs_uint width, tjs_uint height );
	tjs_int GetImageFormat() const override { return Texture.format(); }

	static inline bool IsPowerOfTwo( tjs_uint x ) { return (x & (x - 1)) == 0; }
	static inline tjs_uint ToPowerOfTwo( tjs_uint x ) {
		// 組み込み関数等でMSBを取得してシフトしてもいいが、32からシフトしてループで得ることにする。
		if( IsPowerOfTwo( x ) == false ) {
			tjs_uint r = 32;
			while( r < x ) r = r << 1;
			return r;
		}
		return x;
	}
	
	tjs_int GetStretchType() const;
	void SetStretchType( tjs_int v );
	tjs_int GetWrapModeHorizontal() const;
	void SetWrapModeHorizontal( tjs_int v );
	tjs_int GetWrapModeVertical() const;
	void SetWrapModeVertical( tjs_int v );


	const tTVPRect& GetScale9Patch() const { return Scale9Patch; }
	const tTVPRect& GetMargin9Patch() const { return Margin9Patch; }

	friend class tTJSNI_Offscreen;
};


//---------------------------------------------------------------------------
// tTJSNC_Texture : TJS Texture class
//---------------------------------------------------------------------------
class tTJSNC_Texture : public tTJSNativeClass
{
public:
	tTJSNC_Texture();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Texture(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Texture();
#endif
