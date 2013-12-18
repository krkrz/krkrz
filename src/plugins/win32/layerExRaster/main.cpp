#include "ncbind/ncbind.hpp"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

/**
 * ログ出力用
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "../layerExDraw/LayerExBase.hpp"

/*
 * アウトラインベースのテキスト描画メソッドの追加
 */
struct layerExRaster : public layerExBase
{
public:
	// コンストラクタ
	layerExRaster(DispatchT obj) : layerExBase(obj) {}

	/**
	 * ラスターコピー処理
	 * @param layer 描画元レイヤ
	 * @param maxh  最大振幅(pixel)
	 * @param lines １周期あたりライン数
	 * @param cycle 周期指定(msec)
	 * @param time 現在時刻
	 */
	void copyRaster(tTJSVariant layer, int maxh, int lines, int cycle, tjs_int64 time) {
		
		// レイヤ画像情報
		tjs_int width, height, pitch;
		unsigned char* buffer;
		{
			iTJSDispatch2 *layerobj = layer.AsObjectNoAddRef();
			tTJSVariant var;
			layerobj->PropGet(0, L"imageWidth", NULL, &var, layerobj);
			width = (tjs_int)var;
			layerobj->PropGet(0, L"imageHeight", NULL, &var, layerobj);
			height = (tjs_int)var;
			layerobj->PropGet(0, L"mainImageBuffer", NULL, &var, layerobj);
			buffer = (unsigned char*)(tjs_int)var;
			layerobj->PropGet(0, L"mainImageBufferPitch", NULL, &var, layerobj);
			pitch = (tjs_int)var;
		}

		if (_width != width || _height != height) {
			return;
		}

		// 角速度計算
		double omega = 2 * M_PI / lines;
		
		//double tt = sin((3.14159265358979/2.0) * time / cycle);
		//tjs_int CurH = (tjs_int)(tt * maxh);
		tjs_int CurH = (tjs_int)maxh;
		
		// 初期パラメータを計算
		double rad = - omega * time / cycle * (height/2);

		// クリップ処理
		rad += omega * _clipTop;
		_buffer += _pitch * _clipTop + _clipLeft * 4;
		buffer  +=  pitch * _clipTop + _clipLeft * 4;

		// ラインごとに処理
		tjs_int n;
		for (n = 0; n < _clipHeight; n++, rad += omega) {
			tjs_int d = (tjs_int)(sin(rad) * CurH);
			if (d >= 0) {
				int w = _clipWidth - d;
				const tjs_uint32 *src = (const tjs_uint32*)(buffer + n * pitch);
				tjs_uint32 *dest = (tjs_uint32 *)(_buffer + n * _pitch) + d;
				for (tjs_int i=0;i<w;i++) {
					*dest++ = *src++;
				}
			} else {
				int w = _clipWidth + d;
				const tjs_uint32 *src = (const tjs_uint32*)(buffer + n * pitch) - d;
				tjs_uint32 *dest = (tjs_uint32 *)(_buffer + n * _pitch);
				for (tjs_int i=0;i<w;i++) {
					*dest++ = *src++;
				}
			}
		}

		redraw();
	}
};

// ----------------------------------- クラスの登録

NCB_GET_INSTANCE_HOOK(layerExRaster)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		obj->reset();
		return obj;
	}
	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(layerExRaster, Layer) {
	NCB_METHOD(copyRaster);
}
