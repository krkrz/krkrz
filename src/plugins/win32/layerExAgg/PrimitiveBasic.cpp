#include "Primitive.hpp"

/**
 * 基本プリミティブ保持用
 */
class AGGBasic : public AGGPrimitive
{
public:
	static const tjs_char *getTypeName() { return L"Basic"; }

public:
	/**
	 * 描画処理
	 * @param rb ベースレンダラ
	 * @param mtx 基本アフィン変形
	 */
	void paint(renderer_base &rb, agg::trans_affine &mtx) {

		rasterizer_scanline ras;
		scanline sl;
		renderer_scanline ren(rb);

		// 変形処理
		agg::trans_affine selfMtx;
		selfMtx *= agg::trans_affine_scaling(_scale);
		selfMtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
		selfMtx *= agg::trans_affine_translation(_x, _y);

		// 全体変形
		selfMtx *= mtx;

		// 線の拡張
		_path.expand(_expand);

		// 描画
		_path.render(ras, sl, ren, selfMtx, rb.clip_box(), 1.0);
	}


public:
	/// コンストラクタ
	AGGBasic(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	
};

// --------------------------------------------------------------------
// プリミティブ固有メソッドの定義
// --------------------------------------------------------------------

PRIMFUNC(tLoadMethod,1,AGGBasic)
PRIMFUNCEND
tLoadMethod loadMethod;

/**
 * コンストラクタ
 * @param owner AGGPrimitive のネイティブオブジェクト
 * @param numparams コンストラクタパラメータ数
 * @param param コンストラクタパラメータ
 * @param tjs_obj 生成インスタンス
 */
AGGBasic::AGGBasic(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) : AGGPrimitive(owner)
{
	// tjs_obj にメソッド追加 

	addMember(tjs_obj, L"load", &loadMethod);
}
