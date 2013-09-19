//---------------------------------------------------------------------------

#ifndef rotatebaseH
#define rotatebaseH

#include "tp_stub.h"


//---------------------------------------------------------------------------
struct tPoint
{
	tjs_int x;
	tjs_int y;
};
//---------------------------------------------------------------------------
struct tRotateDrawLine
{
	tjs_int start; // destination start
	tjs_int sx; // source start x
	tjs_int sy; // source start y
	tjs_int stepx; // source step x
	tjs_int stepy; // source step y
};
//---------------------------------------------------------------------------
struct tRotateDrawRegionData
{
	tjs_int left; // left position of destination x
	tjs_int right; // right position of destination x
	tjs_int type; // 0 = bgcolor, 1 = src1, 2 = src2
};
//---------------------------------------------------------------------------
struct tRotateDrawData
{
	int count;
	tRotateDrawLine src1;
	tRotateDrawLine src2;
	tRotateDrawRegionData region[5];
};
//---------------------------------------------------------------------------
class tTVPBaseRotateTransHandler : public iTVPDivisibleTransHandler
{
	//	回転を行うトランジションハンドラ基底クラスの実装

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

	tRotateDrawData * DrawData; // 描画用データ

public:
	tTVPBaseRotateTransHandler(tjs_uint64 time,
		tjs_int width, tjs_int height, tjs_uint32 bgcolor);

	virtual ~tTVPBaseRotateTransHandler();

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

protected:

	void AddLine(tjs_int line, tjs_int left, tjs_int right, tjs_int type);
	void AddSource(const tPoint *points, tjs_int type);

	virtual void CalcPosition() = 0;
		// 矩形の位置を計算する
		// 下位クラスで実装すること
};
//---------------------------------------------------------------------------
#endif
