/**
 * Texture クラス
 */

#ifndef TextureIntfH
#define TextureIntfH

#include "tjsNative.h"
#include "GLTexture.h"
#include "TextureInfo.h"
#include "GLVertexBufferObject.h"
#include "ComplexRect.h"

class tTJSNI_Texture : public tTJSNativeInstance, public iTVPTextureInfoIntrface
{
	GLTexture Texture;
	GLVertexBufferObject VertexBuffer;
	tjs_uint SrcWidth;
	tjs_uint SrcHeight;

	void LoadTexture( const class tTVPBaseBitmap* bitmap, bool alpha );

public:
	tTJSNI_Texture();
	~tTJSNI_Texture() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	void CopyBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );

	tjs_uint GetWidth() const override { return SrcWidth; }
	tjs_uint GetHeight() const override { return SrcHeight; }
	tjs_uint GetMemoryWidth() const { return Texture.width(); }
	tjs_uint GetMemoryHeight() const { return Texture.height(); }
	bool IsGray() const;
	bool IsPowerOfTwo() const;
	tjs_int64 GetNativeHandle() const override { return Texture.id(); }
	tjs_int64 GetVBOHandle() const override;
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
