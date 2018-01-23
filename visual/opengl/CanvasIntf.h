/**
 * Canvas クラス
 * Intf/Impl で分ける方法ではなく、共通化して、ifdefかラッパーメソッドでの環境切り替えを
 * 前提に継承は避けたデザインとする。
 */

#ifndef CanvasIntfH
#define CanvasIntfH

#include "tjsNative.h"
#include "drawable.h"
#include "tjsHashSearch.h"
#include "ComplexRect.h"
#include "GLVertexBufferObject.h"
#include <vector>
#include <memory>

enum class tTVPBlendMode : tjs_int {
	bmDisable = 0,
	bmOpaque = 1,
	bmAlpha = 2,
	bmAdd = 3,
	bmAddWithAlpha = 4,
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
struct tTVPCanvasState {
	static const tjs_uint32 FLAG_CLIP_RECT = 0x01 << 0;
	static const tjs_uint32 FLAG_CULLING = 0x01 << 1;

	float Matrix[6];
	tjs_int ClipRect[4];
	tjs_uint32 Flag;

	tTVPCanvasState( class tTJSNI_Matrix32* mat, class tTJSNI_Rect* clip, bool enableClip, bool enableCulling );
};

class tTJSNI_Canvas : public tTJSNativeInstance
{
	static const ttstr DefaultVertexShaderText;
	static const ttstr DefaultFragmentShaderText;
	static const ttstr DefaultFillVertexShaderText;
	static const ttstr DefaultFillFragmentShaderText;
	static const float DefaultUVs[];

	bool IsFirst;
	bool InDrawing;
	bool EnableClipRect;
	bool EnableCulling = false;
	tjs_uint32 ClearColor;
	tTVPBlendMode BlendMode;
	tTVPRect CurrentScissorRect;

	class tTVPOpenGLScreen* GLScreen;

	std::vector<std::unique_ptr<tTVPCanvasState> > StateStack;

	GLVertexBufferObject TextureVertexBuffer;

	// 直前のBeginDrawingで設定したViewportの幅と高さ
	tjs_int PrevViewportWidth;
	tjs_int PrevViewportHeight;

	tTJSVariant RenterTaretObject;
	class tTJSNI_Offscreen* RenderTargetInstance;

	tTJSVariant ClipRectObject;
	class tTJSNI_Rect* ClipRectInstance;

	tTJSVariant Matrix32Object;
	class tTJSNI_Matrix32* Matrix32Instance;

	tTJSVariant EmbeddedDefaultShaderObject;
	tTJSVariant DefaultShaderObject;
	class tTJSNI_ShaderProgram* DefaultShaderInstance;

	tTJSVariant EmbeddedDefaultFillShaderObject;
	tTJSVariant DefaultFillShaderObject;
	class tTJSNI_ShaderProgram* DefaultFillShaderInstance;
public:
	void SetRenterTargetObject( const tTJSVariant & val );
	const tTJSVariant& GetRenderTargetObject() const { return RenterTaretObject; }

	void SetClipRectObject( const tTJSVariant & val );
	const tTJSVariant& GetClipRectObject() const { return ClipRectObject; }

	void SetMatrix32Object( const tTJSVariant & val );
	const tTJSVariant& GetMatrix32Object() const { return Matrix32Object; }

	void SetDefaultShader( const tTJSVariant & val );
	const tTJSVariant& GetDefaultShader() const { return DefaultShaderObject; }

	void SetDefaultFillShader( const tTJSVariant & val );
	const tTJSVariant& GetDefaultFillShader() const { return DefaultFillShaderObject; }

	void UpdateWindowSurface();
	void ReleaseWindowSurface();

private:
	void ApplyBlendMode();
	void ApplyClipRect();
	void DisableClipRect();
	static void SetCulling( bool b );
	void CreateDefaultShader();
	void CreateDefaultMatrix();
	void SetupEachDrawing();

	// 描画に必要な設定と1個目のテクスチャまで設定する
	void SetupTextureDrawing( class tTJSNI_ShaderProgram* shader, const class iTVPTextureInfoIntrface* tex, class tTJSNI_Matrix32* mat, const tTVPPoint& vpSize );

	// 描画領域の幅/高さ。レンダーターゲット指定している場合はそのサイズ、そうでない場合はクライアント領域(サーフェイス)のサイズ
	tjs_int GetCanvasWidth() const;
	tjs_int GetCanvasHeight() const;

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
	void Capture( const class iTVPTextureInfoIntrface* texture, bool front = true );
	void Clear( tjs_uint32 color );

	void Fill( tjs_int width, tjs_int height, tjs_uint32 colors[4], class tTJSNI_ShaderProgram* shader = nullptr );
	void DrawTexture( const class iTVPTextureInfoIntrface* texture, class tTJSNI_ShaderProgram* shader = nullptr );
	void DrawTexture( const class iTVPTextureInfoIntrface* texture0, const class iTVPTextureInfoIntrface* texture1, class tTJSNI_ShaderProgram* shader );
	void DrawTexture( const class iTVPTextureInfoIntrface* texture0, const class iTVPTextureInfoIntrface* texture1, const class iTVPTextureInfoIntrface* texture2, class tTJSNI_ShaderProgram* shader );
	void DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color );
	void DrawTextureAtlas( const class tTJSNI_Rect* rect, const class iTVPTextureInfoIntrface* texture, class tTJSNI_ShaderProgram* shader = nullptr );

	void DrawMesh( class tTJSNI_ShaderProgram* shader, tjs_int primitiveType, tjs_int offset, tjs_int count );
	void DrawMesh( class tTJSNI_ShaderProgram* shader, tjs_int primitiveType, const class tTJSNI_VertexBinder* index, tjs_int count );

	/**
	 * 9patchを利用した描画
	 */
	void Draw9PatchTexture( class tTJSNI_Texture* tex, tjs_int width, tjs_int height, tTVPRect& margin, class tTJSNI_ShaderProgram* shader = nullptr );

	// 状態をセーブする
	void Save();
	// 状態を元に戻す
	void Restore();

	// prop
	void SetClearColor(tjs_uint32 color) { ClearColor = color; }
	tjs_uint32 GetClearColor() const { return ClearColor; }
	void SetBlendMode( tTVPBlendMode bm );
	tTVPBlendMode GetBlendMode() const { return BlendMode; }
	tjs_uint GetWidth() const;
	tjs_uint GetHeight() const;
	void SetEnableClipRect( bool b );
	bool GetEnableClipRect() const { return EnableClipRect; }
	void SetEnableCulling( bool b );
	bool GetEnableCulling() const { return EnableCulling; }

	void SetWaitVSync( bool b );
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
