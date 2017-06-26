/**
 * Offscreen クラス
 */

#ifndef OffscreenIntfH
#define OffscreenIntfH

#include "tjsNative.h"
#include "GLFrameBufferObject.h"
#include "GLVertexBufferObject.h"
#include "TextureInfo.h"

class tTJSNI_Offscreen : public tTJSNativeInstance, public iTVPTextureInfoIntrface
{
	GLFrameBufferObject	FrameBuffer;
	GLVertexBufferObject VertexBuffer;

public:
	tTJSNI_Offscreen();
	~tTJSNI_Offscreen() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	// 引数が1つだけの時は、全体コピーでBitmap側のサイズをOffscreenに合わせる
	void CopyToBitmap( class tTVPBaseBitmap* bmp );
	void CopyToBitmap( class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int dleft, tjs_int dtop );
	void CopyFromBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );
	void ExchangeTexture( class tTJSNI_Texture* texture );

	tjs_uint GetWidth() const override { return FrameBuffer.width(); }
	tjs_uint GetHeight() const override { return FrameBuffer.height(); }
	tjs_int64 GetNativeHandle() const override { return FrameBuffer.textureId(); }
	tjs_int64 GetVBOHandle() const override;
	tjs_int GetImageFormat() const override { return FrameBuffer.format(); }

	/**
	 * 描画対象に設定する
	 */
	void BindFrameBuffer();
};


//---------------------------------------------------------------------------
// tTJSNC_Offscreen : TJS Offscreen class
//---------------------------------------------------------------------------
class tTJSNC_Offscreen : public tTJSNativeClass
{
public:
	tTJSNC_Offscreen();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Offscreen(); }
};

//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Offscreen();
#endif
