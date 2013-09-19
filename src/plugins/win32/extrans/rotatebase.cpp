//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <math.h>
#include "rotatebase.h"
#include "common.h"

#include <stdio.h>

//---------------------------------------------------------------------------
/*
	切り替え元、切り替え先の画像をクルクル回す系のトランジション用の基底クラス
	の実装
*/
//---------------------------------------------------------------------------
tTVPBaseRotateTransHandler::tTVPBaseRotateTransHandler(tjs_uint64 time,
	tjs_int width, tjs_int height, tjs_uint32 bgcolor)
{
	RefCount = 1;
	Width = width;
	Height = height;
	Time = time;
	BGColor = bgcolor;

	DrawData = new tRotateDrawData[height];

	First = true;
}
//---------------------------------------------------------------------------
tTVPBaseRotateTransHandler::~tTVPBaseRotateTransHandler()
{
	delete [] DrawData;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPBaseRotateTransHandler::StartProcess(tjs_uint64 tick)
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
	CurTime = (tick - StartTick);
	if(CurTime > Time) CurTime = Time;


	// データをクリア
	for(tjs_int i = 0; i < Height; i++)
	{
		// 背景でクリア
		DrawData[i].count = 1;
		DrawData[i].region[0].left = 0;
		DrawData[i].region[0].right = Width;
		DrawData[i].region[0].type = 0; // 0 = 背景
	}

	CalcPosition(); // 下位クラスの CalcPosition メソッドを呼ぶ

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPBaseRotateTransHandler::EndProcess()
{
	// トランジションの画面更新一回分が終わるごとに呼ばれる

	if(CurTime == Time) return TJS_S_FALSE; // トランジション終了

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPBaseRotateTransHandler::Process(
			tTVPDivisibleData *data)
{
	// トランジションの各領域ごとに呼ばれる
	// 吉里吉里は画面を更新するときにいくつかの領域に分割しながら処理を行うので
	// このメソッドは通常、画面更新一回につき複数回呼ばれる

	// data には領域や画像に関する情報が入っている

	// data->Left, data->Top, data->Width, data->Height で示される矩形に
	// のみ転送する必要がある。

	// 変数の準備
	tjs_int destxofs = data->DestLeft - data->Left;
//	tjs_int destyofs = data->DestTop - data->Top;

	tjs_uint8 *dest;
	tjs_int destpitch;
	const tjs_uint8 *src1;
	tjs_int src1pitch;
	const tjs_uint8 *src2;
	tjs_int src2pitch;
	if(TJS_FAILED(data->Dest->GetScanLineForWrite(data->DestTop, (void**)&dest)))
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

	// 各ラインごとに転送
	tjs_int h = data->Height;
	tjs_int y = data->Top;
	while(h--)
	{
		tRotateDrawData *line = DrawData + y;

		for(tjs_int i = 0; i < line->count; i++)
		{
			// 各領域ごとに
			if(line->region[i].left == line->region[i].right) continue;
			tjs_int l = line->region[i].left;
			tjs_int r = line->region[i].right;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				// l, r は data->Left, data->Width でクリップされた結果
				// 残った
				tjs_int type = line->region[i].type;
				if(type == 0)
				{
					// 背景
					TVPFillARGB((tjs_uint32*)dest + destxofs + l, r - l, BGColor);
				}
				else
				{
					// src1 または src2
					const tRotateDrawLine & drawline
						= (type == 1) ? line->src1 : line->src2;

					// 転送
					if(drawline.stepx == 65536 && drawline.stepy == 0)
					{
						// そのまま転送
						memcpy((tjs_uint32*)dest + destxofs + l,
							(const tjs_uint32*)((type == 1 ? src1 : src2) +
							((drawline.sy + (l - drawline.start) * drawline.stepy) >> 16)*
							(type == 1 ? src1pitch : src2pitch)) +
							((drawline.sx + (l - drawline.start) * drawline.stepx) >> 16),
							(r - l) * sizeof(tjs_uint32));
					}
					else if(drawline.stepy == 0)
					{
						// 拡大縮小転送
						TVPStretchCopy((tjs_uint32*)dest + destxofs + l,
							r - l,
                            (const tjs_uint32*)((type == 1 ? src1 : src2) +
							((drawline.sy + (l - drawline.start) * drawline.stepy) >> 16)*
							(type == 1 ? src1pitch : src2pitch)),
							(drawline.sx + (l - drawline.start) * drawline.stepx),
							drawline.stepx);
					}
					else
					{
						// 線形変形転送
						TVPLinTransCopy((tjs_uint32*)dest + destxofs + l,
							r - l,
							(const tjs_uint32*)(type == 1 ? src1 : src2),
							drawline.sx + (l - drawline.start) * drawline.stepx,
							drawline.sy + (l - drawline.start) * drawline.stepy,
							drawline.stepx,
							drawline.stepy,
							type == 1 ? src1pitch : src2pitch);
					}
				}
			}
		}

		dest += destpitch;
		y++;
	}

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void tTVPBaseRotateTransHandler::AddLine(tjs_int line,
	tjs_int left, tjs_int right, tjs_int type)
{
	// line で示したラインに left と right で示した領域を type で上書きする
	// 領域と left, right によっていろいろ処理が変わる
	// 領域が left, right よりも左右にはみ出ている場合
	//    →その領域を２つに分割し、真ん中に left, right を挿入
	// 領域が left, right の左右のどちらかにはみ出ている場合
	//    →その領域をクリップし、右か左に left, right を挿入
	// left, right が領域を完全に内包する場合
	//    →その領域を削除

	if(Clip(left, right, 0, Width))
	{
		tRotateDrawData *data = DrawData + line;
		tjs_int i;
		for(i = 0; i < data->count; i++)
		{
			if(data->region[i].left == data->region[i].right) continue;

			if(data->region[i].left >= left && data->region[i].right <= right)
			{
				// left, right が領域を完全に内包する場合
				//    →その領域を削除
				data->region[i].right = data->region[i].left;
					// 一時的に長さを 0 にする ( あとでここに新しく挿入されるか、
					// あるいはそのまま放置される )

				continue;
			}

			if(data->region[i].left < left && data->region[i].right > right)
			{
				// 領域が left, right よりも左右にはみ出ている場合
				//    →その領域を２つに分割し、真ん中に left, right を挿入

				// 新しい領域に分割される右側を作成
				data->region[data->count].left = right;
				data->region[data->count].right = data->region[i].right;
				data->region[data->count].type = data->region[i].type;

				data->count++;

				// 分割される左側の領域の right をカット
				data->region[i].right = left;


				// これ以上処理は必要ないのでループから抜ける
				break;
			}

			if(data->region[i].left < left && data->region[i].right > left)
			{
				// 領域が left, right の左右のどちらかにはみ出ている場合
				//    →その領域をクリップし、右か左に left, right を挿入

				// 領域の右側をカット
				data->region[i].right = left;

				continue;
			}

			if(data->region[i].left < right && data->region[i].right > right)
			{
				// 領域が left, right の左右のどちらかにはみ出ている場合
				//    →その領域をクリップし、右か左に left, right を挿入

				// 領域の左側をカット
				data->region[i].left = right;

				continue;
			}

		}

		// left, right を挿入するために空きを探す
		for(i = 0; i < data->count; i++)
		{
			if(data->region[i].left == data->region[i].right) break; // 空き
		}	// 空きが見つからなかった場合は i == data->count

		// データを作成
		data->region[i].left = left;
		data->region[i].right = right;
		data->region[i].type = type;

		// 新規に追加された場合は カウントを増やす
		if(i == data->count) data->count++;
	}
}
//---------------------------------------------------------------------------
void tTVPBaseRotateTransHandler::AddSource(const tPoint *points, tjs_int type)
{
	// type ( 1 = src1, 2 = src2 ) で表されたソースを、points の３点で示された
	// 位置に変形転送するように設定する。

	// 吉里吉里本体のソースの LayerBitmapIntf.cpp から引っ張ってきた

	// vertex points
	tjs_int points_x[4];
	tjs_int points_y[4];

	// check each vertex and find most-top/most-bottom/most-left/most-right points
	tjs_int scanlinestart, scanlineend; // most-top/most-bottom
	tjs_int leftlimit, rightlimit; // most-left/most-right
	tjs_int toppoint, bottompoint;

	// - upper-left
	points_x[0] = points[0].x;
	points_y[0] = points[0].y;
	leftlimit = points_x[0]; //, leftpoint = 0;
	rightlimit = points_x[0]; //, rightpoint = 0;
	scanlinestart = points_y[0], toppoint = 0;
	scanlineend = points_y[0], bottompoint = 0;

	// - upper-right
	points_x[1] = points[1].x;
	points_y[1] = points[1].y;
	if(leftlimit > points_x[1]) leftlimit = points_x[1];
	if(rightlimit < points_x[1]) rightlimit = points_x[1];
	if(scanlinestart > points_y[1]) scanlinestart = points_y[1], toppoint = 1;
	if(scanlineend < points_y[1]) scanlineend = points_y[1], bottompoint = 1;

	// - bottom-right
	points_x[2] = points[1].x - points[0].x + points[2].x;
	points_y[2] = points[1].y - points[0].y + points[2].y;
	if(leftlimit > points_x[2]) leftlimit = points_x[2];
	if(rightlimit < points_x[2]) rightlimit = points_x[2];
	if(scanlinestart > points_y[2]) scanlinestart = points_y[2], toppoint = 2;
	if(scanlineend < points_y[2]) scanlineend = points_y[2], bottompoint = 2;

	// - bottom-left
	points_x[3] = points[2].x;
	points_y[3] = points[2].y;
	if(leftlimit > points_x[3]) leftlimit = points_x[3];
	if(rightlimit < points_x[3]) rightlimit = points_x[3]; 
	if(scanlinestart > points_y[3]) scanlinestart = points_y[3], toppoint = 3;
	if(scanlineend < points_y[3]) scanlineend = points_y[3], bottompoint = 3;

	// check destrect intersections
	if(leftlimit >= Width) return;
	if(rightlimit < 0) return;
	if(scanlinestart >= Height) return;
	if(scanlineend < 0) return;

	// prepare to transform...
	tjs_int pd, pa, pdnext, panext;
	tjs_int pdstepx, pastepx;
	tjs_int pdx, pax;
	tjs_int sdstep, sastep;
	tjs_int sd, sa;
	tjs_int y = 0 < scanlinestart ? scanlinestart : 0;
	tjs_int ylim = (Height-1) < scanlineend ? (Height-1) : scanlineend;

	// skip to the first scanline

	// - for descent
	pd = pdnext = toppoint;
	pdnext --;
	pdnext &= 3;

	while(pdnext != bottompoint && points_y[pdnext] < y)
	{
		pdnext--;
		pdnext &= 3; // because pd, pdnext, pa and panext take ring of 0..3
		pd--;
		pd &= 3;
	}

	while(pdnext != bottompoint && points_y[pdnext] == y)
	{
		pd--;
		pdnext--;
		pd &= 3;
		pdnext &= 3;
	}

	// - for ascent
	pa = panext = toppoint;
	panext ++;
	panext &= 3;

	while(panext != bottompoint && points_y[panext] < y)
	{
		panext++;
		panext &= 3;
		pa++;
		pa &= 3;
	}

	while(panext != bottompoint && points_y[panext] == y)
	{
		pa++;
		panext++;
		pa &= 3;
		panext &= 3;
	}

	// compute initial horizontal step per a line

	// - for descent
	if(points_y[pdnext] - points_y[pd] + 1)
		pdstepx = 65536 * (points_x[pdnext] - points_x[pd]) /
			(points_y[pdnext] - points_y[pd] + 1);
	else
		pdstepx = 65536;

	// - for ascent
	if(points_y[panext] - points_y[pa] + 1)
		pastepx = 65536 * (points_x[panext] - points_x[pa]) /
			(points_y[panext] - points_y[pa] + 1);
	else
		pastepx = 65536;


	// compute initial source step

	// - for descent
	if(points_y[pdnext] - points_y[pd] + 1)
	{
		switch(pd)
		{
		case 0: sdstep = 65536 * (Height) /
				(points_y[pdnext] - points_y[pd] + 1); break;
		case 1: sdstep = 65536 * (-Width) /
				(points_y[pdnext] - points_y[pd] + 1); break;
		case 2: sdstep = 65536 * (-Height) /
				(points_y[pdnext] - points_y[pd] + 1); break;
		case 3: sdstep = 65536 * (Width) /
				(points_y[pdnext] - points_y[pd] + 1); break;
		}
	}
	else
	{
		sdstep = 65536;
	}

	// - for ascent
	if(points_y[panext] - points_y[pa] + 1)
	{
		switch(pa)
		{
		case 0: sastep = 65536 * (Width) /
				(points_y[panext] - points_y[pa] + 1); break;
		case 1: sastep = 65536 * (Height) /
				(points_y[panext] - points_y[pa] + 1); break;
		case 2: sastep = 65536 * (-Width) /
				(points_y[panext] - points_y[pa] + 1); break;
		case 3: sastep = 65536 * (-Height) /
				(points_y[panext] - points_y[pa] + 1); break;
		}
	}
	else
	{
		sastep = 65536;
	}

	// compute initial horizontal position

	// - for descent
	pdx = points_x[pd] * 65536;
	if(points_y[pd] < y) pdx += pdstepx * (y - points_y[pd]);

	// - for ascent
	pax = points_x[pa] * 65536;
	if(points_y[pa] < y) pax += pastepx * (y - points_y[pa]);

	// compute initial source position

	// - for descent
	switch(pd)
	{
	case 0: sd = 0; break;
	case 1: sd = Width * 65536 - 1; break;
	case 2: sd = Height * 65536 - 1; break;
	case 3: sd = 0; break;
	}
	if(points_y[pd] < y) sd += sdstep * (y - points_y[pd]);

	// - for ascent
	switch(pa)
	{
	case 0: sa = 0; break;
	case 1: sa = 0; break;
	case 2: sa = Width * 65536 - 1; break;
	case 3: sa = Height * 65536 - 1; break;
	}
	if(points_y[pa] < y) sa += sastep * (y - points_y[pa]);

	// process per a line
	for(; y <= ylim; y++)
	{
		// transfer a line

		// - compute descent x and ascent x
		tjs_int ddx = pdx >> 16;
		tjs_int adx = pax >> 16;

		// - compute descent source position
		tjs_int dx, dy, ax, ay;
		switch(pd)
		{
		case 0: dx = 0; dy = sd; break;
		case 1: dx = sd; dy = 0; break;
		case 2: dx = Width * 65536 - 1; dy = sd; break;
		case 3: dx = sd; dy = Height * 65536 - 1; break;
		}

		// - compute ascent source position
		switch(pa)
		{
		case 0: ax = sa; ay = 0; break;
		case 1: ax = Width * 65536 - 1; ay = sa; break;
		case 2: ax = sa; ay = Height * 65536 - 1; break;
		case 3: ax = 0; ay = sa; break;
		}

		// - swap dx/dy ax/ay dax/ddx if dax < ddx
		if(adx < ddx) Swap_tjs_int(dx, ax), Swap_tjs_int(dy, ay), Swap_tjs_int(adx, ddx);

		// - compute source step
		tjs_int sxstep, systep;
		if(adx != ddx)
		{
			sxstep = /*65536 * */ (ax - dx + 1) / (adx - ddx + 1);
			systep = /*65536 * */ (ay - dy + 1) / (adx - ddx + 1);
		}
		else
		{
			sxstep = systep = 65536;
		}

		// add line
		AddLine(y, ddx, adx + 1, type);

		// write transfer information
		tRotateDrawLine & drawline
			= (type == 1) ? DrawData[y].src1 : DrawData[y].src2;

		drawline.start = ddx;
		drawline.sx = dx;
		drawline.sy = dy;
		drawline.stepx = sxstep;
		drawline.stepy = systep;

		// check descent point
		if(points_y[pdnext] == y)
		{
			do
			{
				pd--;
				pdnext--;
				pd &= 3;
				pdnext &= 3;
			} while(pdnext != bottompoint && points_y[pdnext] == y);


			if(points_y[pdnext] - points_y[pd] + 1)
				pdstepx = 65536 * (points_x[pdnext] - points_x[pd]) /
					(points_y[pdnext] - points_y[pd] + 1);
			else
				pdstepx = 65536;

			if(points_y[pdnext] - points_y[pd] + 1)
			{
				switch(pd)
				{
				case 0: sdstep = 65536 * (Height) /
						(points_y[pdnext] - points_y[pd] + 1); break;
				case 1: sdstep = 65536 * (-Width) /
						(points_y[pdnext] - points_y[pd] + 1); break;
				case 2: sdstep = 65536 * (-Height) /
						(points_y[pdnext] - points_y[pd] + 1); break;
				case 3: sdstep = 65536 * (Width) /
						(points_y[pdnext] - points_y[pd] + 1); break;
				}
			}
			else
			{
				sdstep = 65536;
			}

			switch(pd)
			{
			case 0: sd = 0; break;
			case 1: sd = Width * 65536 - 1; break;
			case 2: sd = Height * 65536 - 1; break;
			case 3: sd = 0; break;
			}

			pdx = points_x[pd] * 65536;
		}

		// check ascent point
		if(points_y[panext] == y)
		{
			do
			{
				pa++;
				panext++;
				pa &= 3;
				panext &= 3;
			} while(panext != bottompoint && points_y[panext] == y);

			if(points_y[panext] - points_y[pa] + 1)
				pastepx = 65536 * (points_x[panext] - points_x[pa]) /
					(points_y[panext] - points_y[pa] + 1);
			else
				pastepx = 65536;

			if(points_y[panext] - points_y[pa] + 1)
			{
				switch(pa)
				{
				case 0: sastep = 65536 * (Width) /
						(points_y[panext] - points_y[pa] + 1); break;
				case 1: sastep = 65536 * (Height) /
						(points_y[panext] - points_y[pa] + 1); break;
				case 2: sastep = 65536 * (-Width) /
						(points_y[panext] - points_y[pa] + 1); break;
				case 3: sastep = 65536 * (-Height) /
						(points_y[panext] - points_y[pa] + 1); break;
				}
			}
			else
			{
				sastep = 65536;
			}

			switch(pa)
			{
			case 0: sa = 0; break;
			case 1: sa = 0; break;
			case 2: sa = Width * 65536 - 1; break;
			case 3: sa = Height * 65536 - 1; break;
			}

			pax = points_x[pa] * 65536;
		}

		// to next ...
		pdx += pdstepx;
		pax += pastepx;
		sd += sdstep;
		sa += sastep;
	}
}
//---------------------------------------------------------------------------

