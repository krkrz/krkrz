/**
 * Canvas クラス
 * Intf/Impl で分ける方法ではなく、共通化して、ifdefかラッパーメソッドでの環境切り替えを
 * 前提に継承は避けたデザインとする。
 */

#ifndef CanvasIntfH
#define CanvasIntfH

#include "tjsNative.h"
#include "drawable.h"
#include "GLTextureDrawing.h"
#include "tjsHashSearch.h"

enum class tTVPBlendMode : tjs_int {
	bmOpaque = 0,
	bmAlpha = 1,
	bmAdd = 2,
	bmAddWithAlpha = 3,
/*
	bmBinder = ltBinder,
	bmCoverRect = ltCoverRect,
	bmOpaque = ltOpaque, // the same as ltCoverRect
	bmTransparent = ltTransparent, // alpha blend
	bmAlpha = ltAlpha, // the same as ltTransparent
	bmAdditive = ltAdditive,
	bmSubtractive = ltSubtractive,
	bmMultiplicative = ltMultiplicative,
	bmEffect = ltEffect,
	bmFilter = ltFilter,
	bmDodge = ltDodge,
	bmDarken = ltDarken,
	bmLighten = ltLighten,
	bmScreen = ltScreen,
	bmAddAlpha = ltAddAlpha, // additive alpha blend
	bmPsNormal = ltPsNormal,
	bmPsAdditive = ltPsAdditive,
	bmPsSubtractive = ltPsSubtractive,
	bmPsMultiplicative = ltPsMultiplicative,
	bmPsScreen = ltPsScreen,
	bmPsOverlay = ltPsOverlay,
	bmPsHardLight = ltPsHardLight,
	bmPsSoftLight = ltPsSoftLight,
	bmPsColorDodge = ltPsColorDodge,
	bmPsColorDodge5 = ltPsColorDodge5,
	bmPsColorBurn = ltPsColorBurn,
	bmPsLighten = ltPsLighten,
	bmPsDarken = ltPsDarken,
	bmPsDifference = ltPsDifference,
	bmPsDifference5 = ltPsDifference5,
	bmPsExclusion = ltPsExclusion
*/
};

enum class tTVPStretchType : tjs_int {
	stNearest,
	stLinear
	// cubic とかはシェーダー使う必要あり
};

class tTJSNI_Canvas : public tTJSNativeInstance
{
	bool IsFirst;
	bool InDrawing;
	tjs_uint32 ClearColor;
	tTVPBlendMode BlendMode;
	tTVPStretchType StretchType;

	class tTVPOpenGLScreen* GLScreen;
	tTVPGLTextureDrawing GLDrawer;

	// 直前のBeginDrawingで設定したViewportの幅と高さ
	tjs_int PrevViewportWidth;
	tjs_int PrevViewportHeight;

	tTJSHashTable<ttstr, tjs_uint> ShaderList;

	tTJSVariant RenterTaretObject;
	class tTJSNI_Offscreen* RenderTargetInstance;

	tTJSVariant ClipRectObject;
	class tTJSNI_Rect* ClipRectInstance;
public:
	void SetRenterTargetObject( const tTJSVariant & val );
	const tTJSVariant& GetRenderTargetObject() const { return RenterTaretObject; }

	void SetClipRectObject( const tTJSVariant & val );
	const tTJSVariant& GetClipRectObject() const { return ClipRectObject; }
	void ApplyClipRect();
	void DisableClipRect();

private:
	void ApplyBlendMode();

public:
	tTJSNI_Canvas();
	~tTJSNI_Canvas() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;
	void TJS_INTF_METHOD Destruct() override;

	void BeginDrawing();
	void EndDrawing();

	// method
	void Capture( class tTJSNI_Bitmap* bmp, bool front = true );
	void Clear();
	void Clear( tjs_uint32 color );
	iTJSDispatch2* CreateTexture( class tTJSNI_Bitmap* bmp, bool gray );
	iTJSDispatch2* CreateTexture( const ttstr &filename, bool gray );
	void DrawScreen( class tTJSNI_Offscreen* screen, tjs_real opacity );
	void DrawScreenUT( class tTJSNI_Offscreen* screen, class tTJSNI_Texture* texture, tjs_int vague, tjs_real opacity );
	void SetClipMask( class tTJSNI_Texture* texture, tjs_int left, tjs_int top );
	//void Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 color );
	void Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 colors[4] );
	void DrawTexture( class tTJSNI_Texture* texture, tjs_int left, tjs_int top );
	void DrawTexture( class tTJSNI_Texture* texture, class tTJSNI_ShaderProgram* shader );
	void DrawTexture2( class tTJSNI_Texture* texture0, class tTJSNI_Texture* texture1, class tTJSNI_ShaderProgram* shader );
	void DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color );
	tjs_uint RegisterShader( const ttstr& name, const ttstr& vertex, const ttstr& fragment );
	tjs_uint FindShader( const ttstr& name ) const;
	void UseShader( class tTJSNI_ShaderProgram* shader );

	// prop
	void SetClearColor(tjs_uint32 color) { ClearColor = color; }
	tjs_uint32 GetClearColor() const { return ClearColor; }
	void SetTargetScreen( class tTJSNI_Offscreen* screen );
	iTJSDispatch2* GetTargetScreenNoAddRef();
	void SetBlendMode( tTVPBlendMode bm );	// --> 直接値設定だけじゃなく、OpenGL ESにも設定してしまった方がいいか
	tTVPBlendMode GetBlendMode() const { return BlendMode; }
	void SetStretchType( tTVPStretchType st );
	tTVPStretchType GetStretchType() const;
	void SetMatrix( class tTJSNI_Matrix44* matrix );
	iTJSDispatch2* GetMatrixNoAddRef();
	tjs_uint GetWidth() const;
	tjs_uint GetHeight() const;
	tjs_uint GetCurrentShader() const;
	void SetCurrentShader( tjs_uint index );
};


//---------------------------------------------------------------------------
// tTJSNC_Canvas : TJS Canvas class
//---------------------------------------------------------------------------
class tTJSNC_Canvas : public tTJSNativeClass
{
public:
	tTJSNC_Canvas();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Canvas(); }
};

extern tTJSNativeClass * TVPCreateNativeClass_Canvas();
#endif
