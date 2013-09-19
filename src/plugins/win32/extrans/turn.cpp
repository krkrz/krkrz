//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <math.h>
#include "turn.h"
#include "turntrans_table.h"
#include "common.h"

#include <stdio.h>

//---------------------------------------------------------------------------
/*
	'turn' トランジション
	正方形の小さなタイルをひっくり返すようにして切り替わるトランジション
	いろいろがんばってみたがいまいち回転している雰囲気が出ていない

	このトランジションは転送先がαを持っていると(要するにトランジションを行う
	レイヤの type が ltCoverRect 以外の場合)、正常に透過情報を処理できないので
	注意
*/
//---------------------------------------------------------------------------
// テカり
static const tjs_int gloss[64] =
	{
	   0,   0,   0,   0,  16,  48,  80, 128,
	 192, 128,  80,  48,  16,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,
	   };
#define TURN_WIDTH_FACTOR 2
	// 1 を設定すると 2 を設定したときよりも一度に回転するブロックの数が多くなる
//---------------------------------------------------------------------------
class tTVPTurnTransHandler : public iTVPDivisibleTransHandler
{
	//	'モザイク' トランジションハンドラクラスの実装

	tjs_int RefCount; // 参照カウンタ
		/*
			iTVPDivisibleTransHandler は 参照カウンタによる管理を行う
		*/

protected:
	tjs_uint64 StartTick; // トランジションを開始した tick count
	tjs_uint64 Time; // トランジションに要する時間
	tjs_uint64 CurTime; // 現在の時間
	tjs_int Width; // 処理する画像の幅
	tjs_int Height; // 処理する画像の高さ
	tjs_int BGColor; // 背景色
	tjs_int Phase; // アニメーションのフェーズ
	bool First; // 一番最初の呼び出しかどうか

public:
	tTVPTurnTransHandler(tjs_uint64 time, tjs_int width, tjs_int height, tjs_uint32 bgcolor)
	{
		RefCount = 1;

		Width = width;
		Height = height;
		Time = time;
		BGColor = bgcolor;

		First = true;
	}

	virtual ~tTVPTurnTransHandler()
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
tjs_error TJS_INTF_METHOD tTVPTurnTransHandler::StartProcess(tjs_uint64 tick)
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

	// 左下から回転し始め、最後に右上が回転を終えるまで処理をする


	CurTime = (tick - StartTick);
	if(CurTime > Time) CurTime = Time;
	int xcount = (Width-1) / 64 + 1;
	int ycount = (Height-1) / 64 + 1;
	Phase = CurTime * (64 + (xcount +  ycount) *TURN_WIDTH_FACTOR) / Time -
		ycount *TURN_WIDTH_FACTOR;


	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPTurnTransHandler::EndProcess()
{
	// トランジションの画面更新一回分が終わるごとに呼ばれる

	if(CurTime == Time) return TJS_S_FALSE; // トランジション終了

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPTurnTransHandler::Process(
			tTVPDivisibleData *data)
{
	// トランジションの各領域ごとに呼ばれる
	// 吉里吉里は画面を更新するときにいくつかの領域に分割しながら処理を行うので
	// このメソッドは通常、画面更新一回につき複数回呼ばれる

	// data には領域や画像に関する情報が入っている

	// data->Left, data->Top, data->Width, data->Height で示される矩形に
	// のみ転送する必要がある。ここで行う処理は 'モザイク' のトランジション
	// の処理に似ていて、以下の通り。
	// 1: その転送矩形に含まれるブロックの範囲を判定する
	// 2: 画面一番下のブロックはアクセスオーバーランに気をつけて転送する
	// 3: その範囲の左端と右端のブロックは、上下のクリッピングに加え、
	//    左右のクリッピングを行いながら転送する
	// 4: それ以外のブロックは上下のクリッピングのみを行いながら転送する
	// ちなみに吉里吉里は通常 8 ラインごとの横に細長い領域を上から順に
	// 指定してくる。
	// ブロックサイズは 64x64 固定。

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

	startx = data->Left / 64;
	starty = data->Top / 64;
	endx = (data->Left + data->Width - 1) / 64;
	endy = (data->Top + data->Height - 1) / 64;

	// 2: 画面一番下のブロックはアクセスオーバーランに気をつけて転送する
	// 3: その範囲の左端と右端のブロックは、上下のクリッピングに加え、
	//    左右のクリッピングを行いながら転送する
	// 4: それ以外のブロックは上下のクリッピングのみを行いながら転送する

	for(int y = starty; y <= endy; y++)
	{
		// 同じようなのが何回も出てきて汚いけど、勘弁

		for(int x = startx; x <= endx; x++)
		{
			tjs_int phase = Phase - (x - y) * TURN_WIDTH_FACTOR;
			if(phase < 0) phase = 0;
			if(phase > 63) phase = 63;
			tjs_int gl = gloss[phase];
			if(y * 64 + 64 >= Height || x == startx || x == endx)
			{
				// 下側がアクセスオーバーランの可能性がある
				// あるいは 左端 右端のブロック
				tjs_int l = (x) * 64;
				tjs_int t = (y) * 64;
				tjs_int r = l + 64;
				tjs_int b = t + 64;
				if(Clip(l, r, data->Left, data->Left + data->Width) &&
					Clip(t, b, data->Top, data->Top + data->Height))
				{
					// l, t, r, b は既にクリップされた領域を表している

					// phase を決定

					if(phase == 0)
					{
						// 完全に src1
						tjs_uint8 * dp = dest + (t + destyofs) * destpitch +
							(l + destxofs) * sizeof(tjs_uint32);
						const tjs_uint8 * sp = src1 + t * src1pitch +
							l * sizeof(tjs_uint32);
						tjs_int count = b - t;
						tjs_int len = (r - l) * sizeof(tjs_uint32);
						while(count--)
							memcpy(dp, sp, len), dp += destpitch, sp += src1pitch;
					}
					else if(phase == 63)
					{
						// 完全に src2
						tjs_uint8 * dp = dest + (t + destyofs) * destpitch +
							(l + destxofs) * sizeof(tjs_uint32);
						const tjs_uint8 * sp = src2 + t * src2pitch +
							l * sizeof(tjs_uint32);
						tjs_int count = b - t;
						tjs_int len = (r - l) * sizeof(tjs_uint32);
						while(count--)
							memcpy(dp, sp, len), dp += destpitch, sp += src2pitch;
					}
					else
					{
						// 転送パラメータとソースを決定
						const tTurnTransParams *params = TurnTransParams[phase];
						const tjs_uint8 * src;
						tjs_int srcpitch;
						if(phase < 32)  
						{
							src = src1;
							srcpitch = src1pitch;
						}
						else
						{
							src = src2;
							srcpitch = src2pitch;
						}

						tjs_int line = t - y * 64;  // 開始ライン ( 0 .. 63 )
						tjs_int start = l - x * 64; // 左端の切り取られる部分 ( 0 .. 63 )
						tjs_int end = r - x * 64; // 右端

						params += line;

						src += x * 64 * sizeof(tjs_uint32);

						tjs_int count = b - t;
						tjs_uint8 *dp =
							(tjs_uint8*)
							((tjs_uint32*)(dest + (t + destyofs) * destpitch)
								+ x * 64 + destxofs);
						while(count --)
						{
							tjs_int fl, fr;

							// 左の背景
							fl = 0;
							fr = params->start;
							if(Clip(fl, fr, start, end))
							{
								// fl-fr を背景色で塗りつぶす
								TVPFillARGB((tjs_uint32*)dp + fl, fr - fl, BGColor);
							}

							// 右の背景
							fl = params->start + params->len;
							fr = 64;
							if(Clip(fl, fr, start, end))
							{
								// fl-fr を背景色で塗りつぶす
								TVPFillARGB((tjs_uint32*)dp + fl, fr - fl, BGColor);
							}

							// 変形転送
							fl = params->start;
							fr = params->start + params->len;
							if(Clip(fl, fr, start, end))
							{
								tjs_int sx = params->sx;
								tjs_int sy = params->sy;
								sx += params->stepx * (fl - params->start);
								sy += params->stepy * (fl - params->start);
								if(gl)
								{
									for(; fl < fr; fl++)
									{
										tjs_int yy = y * 64 + (sy >> 16);
										if(yy >= Height)
											((tjs_uint32*)dp)[fl] = BGColor;
										else
											((tjs_uint32*)dp)[fl] = Blend(
												*(const tjs_uint32*)
													(src + (sx >> 16) * sizeof(tjs_uint32) +
													yy * srcpitch),
													0xffffff, gl);
										sx += params->stepx;
										sy += params->stepy;
									}
								}
								else
								{
									for(; fl < fr; fl++)
									{
										tjs_int yy = y * 64 + (sy >> 16);
										if(yy >= Height)
											((tjs_uint32*)dp)[fl] = BGColor;
										else
											((tjs_uint32*)dp)[fl] =
												*(const tjs_uint32*)
													(src + (sx >> 16) * sizeof(tjs_uint32) +
													yy * srcpitch);
										sx += params->stepx;
										sy += params->stepy;
									}
								}
							}
							dp += destpitch;
							params ++;
						}
					}
				}
			}
			else
			{
				// 右端、左端、アクセスオーバーランには注意せずに転送
				tjs_int l = (x) * 64;
				tjs_int t = (y) * 64;
				tjs_int r = l + 64;
				tjs_int b = t + 64;
				if(Clip(t, b, data->Top, data->Top + data->Height))
				{
					// l, t, r, b は既にクリップされた領域を表している

					// phase を決定

					if(phase == 0)
					{
						// 完全に src1
						tjs_uint8 * dp = dest + (t + destyofs) * destpitch +
							(l + destxofs) * sizeof(tjs_uint32);
						const tjs_uint8 * sp = src1 + t * src1pitch +
							l * sizeof(tjs_uint32);
						tjs_int count = b - t;
						tjs_int len = (r - l) * sizeof(tjs_uint32);
						while(count--)
							memcpy(dp, sp, len), dp += destpitch, sp += src1pitch;
					}
					else if(phase == 63)
					{
						// 完全に src2
						tjs_uint8 * dp = dest + (t + destyofs) * destpitch +
							(l + destxofs) * sizeof(tjs_uint32);
						const tjs_uint8 * sp = src2 + t * src2pitch +
							l * sizeof(tjs_uint32);
						tjs_int count = b - t;
						tjs_int len = (r - l) * sizeof(tjs_uint32);
						while(count--)
							memcpy(dp, sp, len), dp += destpitch, sp += src2pitch;
					}
					else
					{
						// 転送パラメータとソースを決定
						const tTurnTransParams *params = TurnTransParams[phase];
						const tjs_uint8 * src;
						tjs_int srcpitch;
						if(phase < 32)  
						{
							src = src1;
							srcpitch = src1pitch;
						}
						else
						{
							src = src2;
							srcpitch = src2pitch;
						}

						tjs_int line = t - y * 64;  // 開始ライン ( 0 .. 63 )

						params += line;

						src += l * sizeof(tjs_uint32) + y * 64 * srcpitch;

						tjs_int count = b - t;
						tjs_uint8 *dp =
							(tjs_uint8*)
							((tjs_uint32*)(dest + (t + destyofs) * destpitch)
								+ l + destxofs);
						while(count --)
						{
							tjs_int fl, fr;

							// 左の背景
							// 0-params->start を背景色で塗りつぶす
							TVPFillARGB((tjs_uint32*)dp + 0, params->start, BGColor);

							// 右の背景
							fl = params->start + params->len;
							// fl-64 を背景色で塗りつぶす
							TVPFillARGB((tjs_uint32*)dp + fl, 64 - fl, BGColor);

							// 変形転送
							fl = params->start;
							fr = params->start + params->len;
							tjs_int sx = params->sx;
							tjs_int sy = params->sy;
							if(gl)
							{
								for(; fl < fr; fl++)
								{
									((tjs_uint32*)dp)[fl] = Blend(
										*(const tjs_uint32*)
											(src + (sx >> 16) * sizeof(tjs_uint32) +
											(sy >> 16) * srcpitch),
											0xffffff, gl);
									sx += params->stepx;
									sy += params->stepy;
								}
							}
							else
							{
								TVPLinTransCopy((tjs_uint32*)dp + fl, fr - fl,
									(const tjs_uint32*)src, sx, sy,
									params->stepx, params->stepy, srcpitch);
/*
								for(; fl < fr; fl++)
								{
									((tjs_uint32*)dp)[fl] =
										*(const tjs_uint32*)
											(src + (sx >> 16) * sizeof(tjs_uint32) +
											(sy >> 16) * srcpitch);
									sx += params->stepx;
									sy += params->stepy;
								}
*/
							}
							dp += destpitch;
							params ++;
						}
					}
				}
			}
		}
	}

	return TJS_S_OK;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class tTVPTurnTransHandlerProvider : public iTVPTransHandlerProvider
{
	tjs_uint RefCount; // 参照カウンタ
public:
	tTVPTurnTransHandlerProvider() { RefCount = 1; }
	~tTVPTurnTransHandlerProvider() {; }

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
		if(name) *name = TJS_W("turn");
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
		tjs_uint32 bgcolor = 0;

		if(TJS_FAILED(options->GetValue(TJS_W("time"), &tmp)))
			return TJS_E_FAIL; // time 属性が指定されていない
		if(tmp.Type() == tvtVoid) return TJS_E_FAIL;
		time = (tjs_int64)tmp;
		if(time < 2) time = 2; // あまり小さな数値を指定すると問題が起きるので

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("bgcolor"), &tmp)))
		if(tmp.Type() != tvtVoid) bgcolor = (tjs_int)tmp;

		// オブジェクトを作成
		*handler = new tTVPTurnTransHandler(time, src1w, src1h, bgcolor);

		return TJS_S_OK;
	}

} static * TurnTransHandlerProvider;
//---------------------------------------------------------------------------
void RegisterTurnTransHandlerProvider()
{
	// TVPAddTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録する
	TurnTransHandlerProvider = new tTVPTurnTransHandlerProvider();
	TVPAddTransHandlerProvider(TurnTransHandlerProvider);
}
//---------------------------------------------------------------------------
void UnregisterTurnTransHandlerProvider()
{
	// TVPRemoveTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録抹消する
	TVPRemoveTransHandlerProvider(TurnTransHandlerProvider);
	TurnTransHandlerProvider->Release();
}
//---------------------------------------------------------------------------


