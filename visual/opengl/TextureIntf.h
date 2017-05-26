/**
 * Texture クラス
 */

#ifndef TextureIntfH
#define TextureIntfH

#include "tjsNative.h"
#include "GLTexture.h"

class tTJSNI_Texture : public tTJSNativeInstance
{
	GLTexture Texture;

	void LoadTexture( class tTVPBaseBitmap* bitmap, bool gray, bool powerOfTwo );

public:
	tTJSNI_Texture();
	~tTJSNI_Texture() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;
	void TJS_INTF_METHOD Destruct() override;

	tjs_uint GetWidth() const;
	tjs_uint GetHeight() const;
	tjs_uint GetMemoryWidth() const;
	tjs_uint GetMemoryHeight() const;
	bool IsGray() const;
	bool IsPowerOfTwo() const;
	tjs_int64 GetNativeHandle() const;
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
#endif
