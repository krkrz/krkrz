#include "Primitive.hpp"
#include "agg_svg_parser.h"

/**
 * SVG 保持用
 */
class AGGSVG : public AGGPrimitive
{
public:
	static const tjs_char *getTypeName() { return L"SVG"; }

protected:
	/// SVG 用パス情報
	agg::svg::path_renderer _path;

	// バウンディング
	double _min_x;
	double _min_y;
    double _max_x;
    double _max_y;

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
		selfMtx *= agg::trans_affine_translation((_min_x + _max_x) * -0.5, (_min_y + _max_y) * -0.5);
		selfMtx *= agg::trans_affine_scaling(_scale);
		selfMtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
		selfMtx *= agg::trans_affine_translation((_min_x + _max_x) * 0.5 + _x, (_min_y + _max_y) * 0.5 + _y);

		// 全体変形
		selfMtx *= mtx;

		// 線の拡張
		_path.expand(_expand);

		// 描画
		_path.render(ras, sl, ren, selfMtx, rb.clip_box(), 1.0);
	}
	
public:
	
	/**
	 * 画像のパース
	 */
	void parse(const ttstr &name) {

		// 画像読み込み
		IStream *in = TVPCreateIStream(name, TJS_BS_READ);
		if(!in) {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + ttstr(name)).c_str());
		}
		try	{
			// レンダリング処理
			agg::svg::parser p(_path);
			p.parse(in);
			_path.arrange_orientations();
			_path.bounding_rect(&_min_x, &_min_y, &_max_x, &_max_y);
		} catch(agg::svg::exception& e) {
			in->Release();
			TVPThrowExceptionMessage(ttstr(e.msg()).c_str());
		} catch(...) {
			in->Release();
			throw;
		}
		in->Release();

		redraw();
	}

public:
	/// コンストラクタ
	AGGSVG(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) : AGGPrimitive(owner) {
		_min_x = 0.0;
		_min_y = 0.0;
		_max_x = 0.0;
		_max_y = 0.0;

		if (numparams > 0) {
			parse(*param[0]);
		}

		// tjs_obj にメソッド追加 XXX
	}
};

static RegistTypeFactory<AGGSVG> regist;
