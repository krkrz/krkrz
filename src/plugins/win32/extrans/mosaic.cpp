//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <math.h>
#include "mosaic.h"
#include "common.h"

#include <stdio.h>

//---------------------------------------------------------------------------
/*
	'モザイク' トランジション
	矩形モザイクによるトランジション
	このトランジションは転送先がαを持っていると(要するにトランジションを行う
	レイヤの type が ltOpaque 以外の場合)、正常に透過情報を処理できないので
	注意
*/
//---------------------------------------------------------------------------
class tTVPMosaicTransHandler : public iTVPDivisibleTransHandler
{
	//	'モザイク' トランジションハンドラクラスの実装

	tjs_int RefCount; // 参照カウンタ
		/*
			iTVPDivisibleTransHandler は 参照カウンタによる管理を行う
		*/

protected:
	tjs_uint64 StartTick; // トランジションを開始した tick count
	tjs_uint64 Time; // トランジションに要する時間
	tjs_uint64 HalfTime; // トランジションに要する時間 / 2
	tjs_uint64 CurTime; // 現在の時間
	tjs_int Width; // 処理する画像の幅
	tjs_int Height; // 処理する画像の高さ
	tjs_int CurOfsX; // ブロックオフセット X
	tjs_int CurOfsY; // ブロックオフセット Y
	tjs_int MaxBlockSize; // 最大のブロック幅
	tjs_int CurBlockSize; // 現在のブロック幅
	tjs_int BlendRatio; // ブレンド比
	bool First; // 一番最初の呼び出しかどうか

public:
	tTVPMosaicTransHandler(tjs_uint64 time, tjs_int width, tjs_int height,
		tjs_int maxblocksize)
	{
		RefCount = 1;

		Width = width;
		Height = height;
		Time = time;
		HalfTime = time / 2;
		MaxBlockSize = maxblocksize;

		First = true;
	}

	virtual ~tTVPMosaicTransHandler()
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
tjs_error TJS_INTF_METHOD tTVPMosaicTransHandler::StartProcess(tjs_uint64 tick)
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

	// 画像演算に必要なパラメータを計算
	tjs_int64 t = CurTime = (tick - StartTick);
	if(CurTime > Time) CurTime = Time;
	if(t >= HalfTime) t = Time - t;
	if(t < 0) t = 0;
	CurBlockSize = (MaxBlockSize-2) * t / HalfTime + 2;

	// BlendRatio
	BlendRatio = CurTime * 255 / Time;
	if(BlendRatio > 255) BlendRatio = 255;

	// 中心のブロックを本当に中心に持ってこられるように CurOfsX と CurOfsY を調整
	int x = Width / 2;
	int y = Height / 2;
	x /= CurBlockSize;
	y /= CurBlockSize;
	x *= CurBlockSize;
	y *= CurBlockSize;
	CurOfsX = (Width - CurBlockSize) / 2 - x;
	CurOfsY = (Height - CurBlockSize) / 2 - y;
	if(CurOfsX > 0) CurOfsX -= CurBlockSize;
	if(CurOfsY > 0) CurOfsY -= CurBlockSize;

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPMosaicTransHandler::EndProcess()
{
	// トランジションの画面更新一回分が終わるごとに呼ばれる

	if(BlendRatio == 255) return TJS_S_FALSE; // トランジション終了

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPMosaicTransHandler::Process(
			tTVPDivisibleData *data)
{
	// トランジションの各領域ごとに呼ばれる
	// 吉里吉里は画面を更新するときにいくつかの領域に分割しながら処理を行うので
	// このメソッドは通常、画面更新一回につき複数回呼ばれる

	// data には領域や画像に関する情報が入っている

	// data->Left, data->Top, data->Width, data->Height で示される矩形に
	// のみ転送する必要がある。ここで行う処理は以下の通り。
	// 1: その転送矩形に含まれるモザイクのブロックの範囲を判定する
	// 2: まず辺境のブロックに対して転送矩形との積矩形を得てそこに色を塗りつぶす
	// 3: 残りのブロックははみ出しについて注意する必要がないので心おきなく色を塗りつぶす


	// 変数の準備
	tjs_uint8 *dest;
	tjs_int destpitch;
	const tjs_uint8 *src1;
	tjs_int src1pitch;
	const tjs_uint8 *src2;
	tjs_int src2pitch;
	if(TJS_FAILED(data->Dest->GetScanLineForWrite(0, (void**)&dest)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src1->GetScanLine(0, (const void**)&src1)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src2->GetScanLine(0, (const void**)&src2)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Dest->GetPitchBytes(&destpitch)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src1->GetPitchBytes(&src1pitch)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src2->GetPitchBytes(&src2pitch)))
		return TJS_E_FAIL;

	tjs_int destxofs = data->DestLeft - data->Left;
	tjs_int destyofs = data->DestTop - data->Top;

	// 1: その転送矩形に含まれるモザイクのブロックの範囲を判定する
	int startx, starty;
	int endx, endy;
	int bs = CurBlockSize;

	startx = (data->Left - CurOfsX) / bs;
	starty = (data->Top - CurOfsY) / bs;
	endx = (data->Left + data->Width - 1 - CurOfsX) / bs;
	endy = (data->Top + data->Height - 1 - CurOfsY) / bs;

	// 塗りつぶしマクロ
#define FILL_LINE(dp, xlen, ylen, d) { \
			tjs_uint8 *__destp = (tjs_uint8*)(dp); \
			int __count = ylen; \
			tjs_uint32 color = (d); \
			switch(xlen)                   \
			{                              \
			case 2:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			case 3:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] =       \
					((tjs_uint32*)__destp)[2] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			case 4:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] =       \
					((tjs_uint32*)__destp)[2] =       \
					((tjs_uint32*)__destp)[3] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			case 5:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] =       \
					((tjs_uint32*)__destp)[2] =       \
					((tjs_uint32*)__destp)[3] =       \
					((tjs_uint32*)__destp)[4] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			case 6:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] =       \
					((tjs_uint32*)__destp)[2] =       \
					((tjs_uint32*)__destp)[3] =       \
					((tjs_uint32*)__destp)[4] =       \
					((tjs_uint32*)__destp)[5] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			case 7:                        \
				do           \
				{                          \
					((tjs_uint32*)__destp)[0] =       \
					((tjs_uint32*)__destp)[1] =       \
					((tjs_uint32*)__destp)[2] =       \
					((tjs_uint32*)__destp)[3] =       \
					((tjs_uint32*)__destp)[4] =       \
					((tjs_uint32*)__destp)[5] =       \
					((tjs_uint32*)__destp)[6] = color;      \
					__destp += destpitch;  \
				} while(--__count);                          \
				break;                     \
			default:                       \
				while(__count--) \
				{ \
					TVPFillARGB((tjs_uint32*)__destp, (xlen), color); \
					__destp += destpitch; \
				} \
			} \
		}


	// 注意しながらの塗りつぶしマクロ
	int bs2 = bs >> 1;
#define FILL_ONE(x, y) { \
		tjs_int l = (x) * bs + CurOfsX; \
		tjs_int t = (y) * bs + CurOfsY; \
		tjs_int r = l + bs; \
		tjs_int b = t + bs; \
		tjs_int mx = l + bs2, my = t + bs2; \
		if(Clip(l, r, data->Left, data->Left + data->Width) && \
			Clip(t, b, data->Top, data->Top + data->Height)) \
		{ \
			if(mx < 0) mx = 0; \
			if(my < 0) my = 0; \
			if(mx >= Width) mx = Width - 1; \
			if(my >= Height) my = Height - 1; \
			tjs_uint32 d = Blend( \
				*((const tjs_uint32 *)(src1 + my*src1pitch) + mx), \
				*((const tjs_uint32 *)(src2 + my*src2pitch) + mx), \
				BlendRatio); \
			tjs_uint8 *destp = (tjs_uint8*) \
				((tjs_uint32*)(dest + (t + destyofs)*destpitch) + l + destxofs); \
			tjs_int xlen = r - l; \
			tjs_int ylen = b - t; \
			FILL_LINE(destp, xlen, ylen, d); \
		} \
	}
		/* 本来は転送元ブロックの範囲内にあるピクセルの色の平均を取ると綺麗だけど
		   重くなるのでやらない */

	// 2: まず辺境のブロックに対して転送矩形との積矩形を得てそこに色を塗りつぶす
	// 3: 残りのブロックははみ出しについて注意する必要がないので心おきなく色を塗りつぶす

	// 一番上の行
	int y = starty;
	for(int x = startx; x <= endx; x++)
	{
		FILL_ONE(x, y);
	}
	y++;

	// なかほどの行
	for(; y < endy; y++)
	{
		// 左端
		FILL_ONE(startx, y);

		// なかほど
		tjs_int x = startx + 1;
		tjs_int l = x * bs + CurOfsX;
		tjs_int t = y * bs + CurOfsY;
		const tjs_uint32 *src1p =
			(const tjs_uint32*)(src1 + (t + bs2)*src1pitch) + (l + bs2);
		const tjs_uint32 *src2p =
			(const tjs_uint32*)(src2 + (t + bs2)*src2pitch) + (l + bs2);
		tjs_uint32 *destp =
			((tjs_uint32*)(dest + (t + destyofs)*destpitch) + l + destxofs);

		for(; x < endx; x++)
		{
			// ここの塗りつぶしは(はみ出ているかどうかを)ノーチェックでいける
			tjs_uint32 d = Blend(*src1p, *src2p, BlendRatio);
			FILL_LINE(destp, bs, bs, d);

			src1p += bs;
			src2p += bs;
			destp += bs;
		}

		// 右端
		FILL_ONE(endx, y);
	}


	// 一番下の行
	if(y <= endy)
	{
		for(int x = startx; x <= endx; x++)
		{
			FILL_ONE(x, y);
		}
	}

#undef FILL_ONE
#undef FILL_LINE

	return TJS_S_OK;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class tTVPMosaicTransHandlerProvider : public iTVPTransHandlerProvider
{
	tjs_uint RefCount; // 参照カウンタ
public:
	tTVPMosaicTransHandlerProvider() { RefCount = 1; }
	~tTVPMosaicTransHandlerProvider() {; }

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
		if(name) *name = TJS_W("mosaic");
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
		tjs_int maxblocksize = 30;

		if(TJS_FAILED(options->GetValue(TJS_W("time"), &tmp)))
			return TJS_E_FAIL; // time 属性が指定されていない
		if(tmp.Type() == tvtVoid) return TJS_E_FAIL;
		time = (tjs_int64)tmp;
		if(time < 2) time = 2; // あまり小さな数値を指定すると問題が起きるので

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("maxsize"), &tmp)))
			if(tmp.Type() != tvtVoid) maxblocksize = (tjs_int)tmp;

		// オブジェクトを作成
		*handler = new tTVPMosaicTransHandler(time, src1w, src1h, maxblocksize);

		return TJS_S_OK;
	}

} static * MosaicTransHandlerProvider;
//---------------------------------------------------------------------------
void RegisterMosaicTransHandlerProvider()
{
	// TVPAddTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録する
	MosaicTransHandlerProvider = new tTVPMosaicTransHandlerProvider();
	TVPAddTransHandlerProvider(MosaicTransHandlerProvider);
}
//---------------------------------------------------------------------------
void UnregisterMosaicTransHandlerProvider()
{
	// TVPRemoveTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録抹消する
	TVPRemoveTransHandlerProvider(MosaicTransHandlerProvider);
	MosaicTransHandlerProvider->Release();
}
//---------------------------------------------------------------------------
