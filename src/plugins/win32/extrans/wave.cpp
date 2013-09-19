//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <math.h>
#include "wave.h"
#include "common.h"

//---------------------------------------------------------------------------
/*
	'波' トランジション
	ラスタスクロールによるトランジション
*/
//---------------------------------------------------------------------------
class tTVPWaveTransHandler : public iTVPDivisibleTransHandler
{
	//	'波' トランジションハンドラクラスの実装

	tjs_int RefCount; // 参照カウンタ
		/*
			iTVPDivisibleTransHandler は 参照カウンタによる管理を行う
		*/

protected:
	tjs_uint64 StartTick; // トランジションを開始した tick count
	tjs_uint64 HalfTime; // トランジションに要する時間 / 2
	tjs_uint64 Time; // トランジションに要する時間
	tTVPLayerType LayerType; // レイヤタイプ
	tjs_int Width; // 処理する画像の幅
	tjs_int Height; // 処理する画像の高さ
	tjs_int MaxH; // 最大振幅
	double MaxOmega; // 最大角速度
	tjs_int CurH; // 現在の振幅
	double CurOmega; // 現在の角速度
	double CurRadStart; // 角開始位置
	tjs_int64 CurTime; // 現在の tick count
	tjs_int BlendRatio; // ブレンド比
	tjs_uint32 BGColor1; // 背景色その１
	tjs_uint32 BGColor2; // 背景色その２
	tjs_uint32 CurBGColor; // 現在の背景色
	tjs_int WaveType; // 0 = 最初と最後 1 = 最初 2 = 最後 が波が細かい
	bool First; // 一番最初の呼び出しかどうか

public:
	tTVPWaveTransHandler(tjs_uint64 time, tTVPLayerType layertype,
		tjs_int width, tjs_int height,
		tjs_int maxh, double maxomega,
		tjs_uint32 bgcolor1, tjs_uint32 bgcolor2, tjs_int wavetype)
	{
		RefCount = 1;

		LayerType = layertype;
		Width = width;
		Height = height;
		Time = time;
		HalfTime = time / 2;
		MaxH = maxh;
		MaxOmega = maxomega;
		BGColor1 = bgcolor1;
		BGColor2 = bgcolor2;
		WaveType = wavetype;

		First = true;
	}

	virtual ~tTVPWaveTransHandler()
	{
	}

	tjs_error TJS_INTF_METHOD AddRef()
	{
		// iTVPBaseTransHandler の AddRef
		// 参照カウンタをインクリメント
		RefCount ++;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD Release()
	{
		// iTVPBaseTransHandler の Release
		// 参照カウンタをデクリメントし、0 になるならば delete this
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
		return TJS_S_OK;
	}


	tjs_error TJS_INTF_METHOD SetOption(
			/*in*/iTVPSimpleOptionProvider *options // option provider
		)
	{
		// iTVPBaseTransHandler の SetOption
		// とくにやることなし
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD StartProcess(tjs_uint64 tick);

	tjs_error TJS_INTF_METHOD EndProcess();

	tjs_error TJS_INTF_METHOD Process(
			tTVPDivisibleData *data);

	tjs_error TJS_INTF_METHOD MakeFinalImage(
			iTVPScanLineProvider ** dest,
			iTVPScanLineProvider * src1,
			iTVPScanLineProvider * src2)
	{
		*dest = src2; // 常に最終画像は src2
		return TJS_S_OK;
	}
};
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPWaveTransHandler::StartProcess(tjs_uint64 tick)
{
	// トランジションの画面更新一回ごとに呼ばれる

	// トランジションの画面更新一回につき、まず最初に StartProcess が呼ばれる
	// そのあと Process が複数回呼ばれる ( 領域を分割処理している場合 )
	// 最後に EndProcess が呼ばれる

	if(First)
	{
		// 最初の実行
		First = false;
		StartTick = tick;
	}

	// 画像演算に必要な各パラメータを計算
	tjs_int64 t = CurTime = (tick - StartTick);
	if(CurTime > Time) CurTime = Time;
	if(t >= HalfTime) t = Time - t;
	if(t < 0) t = 0;

	double tt = sin((3.14159265358979/2.0) * t / (tjs_int64)HalfTime);

	// CurH, CurOmega, CurRadStart
	CurH = tt * MaxH;
	switch(WaveType)
	{
	case 0: // 最初と最後が波が細かい
		CurOmega = MaxOmega * tt;
		break;
	case 1: // 最初が波が細かい
		CurOmega = MaxOmega * ((tjs_int64)Time - CurTime) / (tjs_int64)Time;
		break;
	case 2: // 最後が波が細かい
		CurOmega = MaxOmega * CurTime / (tjs_int64)Time;
		break;
	}
	CurRadStart = -CurOmega * (Height / 2);

	// BlendRatio
	BlendRatio = CurTime * 255 / Time;
	if(BlendRatio > 255) BlendRatio = 255;

	// 背景色のブレンド
	CurBGColor = Blend(BGColor1, BGColor2, BlendRatio);

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPWaveTransHandler::EndProcess()
{
	// トランジションの画面更新一回分が終わるごとに呼ばれる

	if(BlendRatio == 255) return TJS_S_FALSE; // トランジション終了

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPWaveTransHandler::Process(
			tTVPDivisibleData *data)
{
	// トランジションの各領域ごとに呼ばれる
	// 吉里吉里は画面を更新するときにいくつかの領域に分割しながら処理を行うので
	// このメソッドは通常、画面更新一回につき複数回呼ばれる

	// data には領域や画像に関する情報が入っている

	// 初期パラメータを計算
	double rad = data->Top * CurOmega + CurRadStart; // 角度

	// ラインごとに処理
	tjs_int n;
	for(n = 0; n < data->Height; n++, rad += CurOmega)
	{
		// ズレ位置
		tjs_int d = (tjs_int)(sin(rad) * CurH);

		// 転送
		tjs_int l, r;

		// ここでやるべきことは、data->Src1 と data->Src2 の (0, data->Top + n) から
		// 始まる１ラインを BlendRatio によってブレンドし、(d, data->Top + n) に
		// 転送する。はみ出て描画されない部分は CurBGColor で塗りつぶす。
		// ただし、左右は data->Left と data->Width によってクリッピングされる。
		// また、data->Dest に転送するときは、そのオフセットは (data->Left, data->Top)
		// ではなくて(data->DestLeft, data->DestTop) になるので補正する。

		// スキャンライン
		tjs_uint32 *dest;
		const tjs_uint32 *src1;
		const tjs_uint32 *src2;
		if(TJS_FAILED(data->Dest->GetScanLineForWrite(data->DestTop + n, (void**)&dest)))
			return TJS_E_FAIL;
		if(TJS_FAILED(data->Src1->GetScanLine(data->Top + n, (const void**)&src1)))
			return TJS_E_FAIL;
		if(TJS_FAILED(data->Src2->GetScanLine(data->Top + n, (const void**)&src2)))
			return TJS_E_FAIL;

		// 左側のずれる部分に背景色を転送
		if(d > 0)
		{
			l = 0;
			r = d;
			if(Clip(l, r, data->Left, data->Left + data->Width))
				TVPFillARGB(dest + l + data->DestLeft - data->Left, r - l, CurBGColor);
		}

		// 左端のずれる部分に背景色を転送
		if(d < 0)
		{
			l = d + Width;
			r = Width;
			if(Clip(l, r, data->Left, data->Left + data->Width))
				TVPFillARGB(dest + l + data->DestLeft - data->Left, r - l, CurBGColor);
		}

		// ブレンドしながら転送
		// TVPConstAlphaBlend_SD(dest, src1, src2, len, opa)
		// は dest に src1 と src2 を opa で指定した混合比で混合して転送する
		l = d;
		r = Width + d;
		if(Clip(l, r, data->Left, data->Left + data->Width))
		{
			if(LayerType == ltAlpha)
				TVPConstAlphaBlend_SD_d(dest + l + data->DestLeft - data->Left,
					src1 + l - d, src2 + l - d, r - l, BlendRatio);
			else if(LayerType == ltAddAlpha)
				TVPConstAlphaBlend_SD_a(dest + l + data->DestLeft - data->Left,
					src1 + l - d, src2 + l - d, r - l, BlendRatio);
			else
				TVPConstAlphaBlend_SD(dest + l + data->DestLeft - data->Left,
					src1 + l - d, src2 + l - d, r - l, BlendRatio);
				/*
					転送先がαを持っている場合はブレンドアルゴリズムが違うので
					注意する必要がある。
					_d のサフィックスを持つブレンド関数はすべて通常のαブレンドで、
					α値を考慮したブレンドを行う。同様に _a のサフィックスを持つ
					ブレンド関数は加算αブレンドである。_a や _d サフィックスを持
					たないブレンド関数に比べて低速。_d や _a サフィックスを持たな
					いブレンド関数はα値は扱わない ( 常に完全に不透明であると扱われる )。
				*/
		}
	}


	return TJS_S_OK;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class tTVPWaveTransHandlerProvider : public iTVPTransHandlerProvider
{
	tjs_uint RefCount; // 参照カウンタ
public:
	tTVPWaveTransHandlerProvider() { RefCount = 1; }
	~tTVPWaveTransHandlerProvider() {; }

	tjs_error TJS_INTF_METHOD AddRef()
	{
		// iTVPBaseTransHandler の AddRef
		// 参照カウンタをインクリメント
		RefCount ++;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD Release()
	{
		// iTVPBaseTransHandler の Release
		// 参照カウンタをデクリメントし、0 になるならば delete this
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD GetName(
			/*out*/const tjs_char ** name)
	{
		// このトランジションの名前を返す
		if(name) *name = TJS_W("wave");
		return TJS_S_OK;
	}


	tjs_error TJS_INTF_METHOD StartTransition(
			/*in*/iTVPSimpleOptionProvider *options, // option provider
			/*in*/iTVPSimpleImageProvider *imagepro, // image provider
			/*in*/tTVPLayerType layertype, // destination layer type
			/*in*/tjs_uint src1w, tjs_uint src1h, // source 1 size
			/*in*/tjs_uint src2w, tjs_uint src2h, // source 2 size
			/*out*/tTVPTransType *type, // transition type
			/*out*/tTVPTransUpdateType * updatetype, // update typwe
			/*out*/iTVPBaseTransHandler ** handler // transition handler
			)
	{
		if(type) *type = ttExchange; // transition type : exchange
		if(updatetype) *updatetype = tutDivisible;
			// update type : divisible
		if(!handler) return TJS_E_FAIL;
		if(!options) return TJS_E_FAIL;

		if(src1w != src2w || src1h != src2h)
			return TJS_E_FAIL; // src1 と src2 のサイズが一致していないと駄目


		// オプションを得る
		tTJSVariant tmp;
		tjs_uint64 time;
		tjs_int maxh = 50;
		double maxomega = 0.2;
		tjs_uint32 bgcolor1 = 0;
		tjs_uint32 bgcolor2 = 0;
		tjs_int wavetype = 0;

		if(TJS_FAILED(options->GetValue(TJS_W("time"), &tmp)))
			return TJS_E_FAIL; // time 属性が指定されていない
		if(tmp.Type() == tvtVoid) return TJS_E_FAIL;
		time = (tjs_int64)tmp;
		if(time < 2) time = 2; // あまり小さな数値を指定すると問題が起きるので

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("maxh"), &tmp)))
			if(tmp.Type() != tvtVoid) maxh = (tjs_int)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("maxomega"), &tmp)))
			if(tmp.Type() != tvtVoid) maxomega = (double)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("bgcolor1"), &tmp)))
			if(tmp.Type() != tvtVoid) bgcolor1 = (tjs_int)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("bgcolor2"), &tmp)))
			if(tmp.Type() != tvtVoid) bgcolor2 = (tjs_int)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("wavetype"), &tmp)))
			if(tmp.Type() != tvtVoid) wavetype = (tjs_int)tmp;


		// オブジェクトを作成
		*handler = new tTVPWaveTransHandler(time, layertype,
			src1w, src1h, maxh, maxomega,
			bgcolor1, bgcolor2, wavetype);

		return TJS_S_OK;
	}

} static * WaveTransHandlerProvider;
//---------------------------------------------------------------------------
void RegisterWaveTransHandlerProvider()
{
	// TVPAddTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録する
	WaveTransHandlerProvider = new tTVPWaveTransHandlerProvider();
	TVPAddTransHandlerProvider(WaveTransHandlerProvider);
}
//---------------------------------------------------------------------------
void UnregisterWaveTransHandlerProvider()
{
	// TVPRemoveTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録抹消する
	TVPRemoveTransHandlerProvider(WaveTransHandlerProvider);
	WaveTransHandlerProvider->Release();
}
//---------------------------------------------------------------------------
