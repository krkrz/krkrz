//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
	#pragma inline
#endif

#include <windows.h>
#include "tp_stub.h"
#include <math.h>
#include "ripple.h"
#include "common.h"

//---------------------------------------------------------------------------
/*
	'波紋' トランジション
	置換マップによる、波紋が広がっていくような感じのトランジション
	このトランジションは転送先がαを持っていると(要するにトランジションを行う
	レイヤの type が ltOpaque 以外の場合)、正常に透過情報を処理できないので
	注意
*/
//---------------------------------------------------------------------------


// 2003/12/15 W.Dee  M_PI が未定義エラーになるのを修正とSSE命令を_emitに置き換え

//---------------------------------------------------------------------------
// #define TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	// 定義するとトランジション中に画面を更新した回数を表示する
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define TVP_RIPPLE_DIR_PREC 32
	// テーブル内で１象限中(90°)の方向をいくつに分割するか
	// (2 の累乗で 256 まで。大きくするとメモリを食う)
#define TVP_RIPPLE_DRIFT_PREC 4
	// drift 1 ピクセルをいくつに分割するか
//---------------------------------------------------------------------------
#ifndef M_PI
	#define M_PI (3.14159263589793238462)
#endif
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/*
	いくつか テーブルを管理するクラス/関数群
	テーブルは、中心座標、トランジション画像のサイズ、
	波の幅、波紋の縦/横比、揺れの幅が前回と変わらない限り再生成はされない。
	再生成にはすこし時間がかかるため、4つまでキャッシュを行うことができる。
*/
//---------------------------------------------------------------------------
class tTVPRippleTable
{
	tjs_int RefCount; // 参照カウンタ

	tjs_int Width; // トランジション画像の幅
	tjs_int Height; // トランジション画像の高さ

	tjs_int CenterX; // 波紋の中心 X 座標
	tjs_int CenterY; // 波紋の中心 Y 座標

	tjs_int RippleWidth; // 波紋の幅
	float Roundness; // 波紋の縦/横比
	tjs_int MaxDrift; // 揺れの最大幅

	tjs_int MapWidth; // 置換マップの幅
	tjs_int MapHeight; // 置換マップの高さ

	tjs_uint16 *DisplaceMap; // [位置]->[方向,距離] 置換マップ
	tjs_uint16 *DriftMap; // [揺れの大きさ,方向,距離]->[ずれ] 置換マップ

public:
	tjs_int GetWidth() const { return Width; }
	tjs_int GetHeight() const { return Height; }

	tjs_int GetCenterX() const { return CenterX; }
	tjs_int GetCenterY() const { return CenterY; }

	tjs_int GetRippleWidth() const { return RippleWidth; }
	float GetRoundness() const { return Roundness; }
	tjs_int GetMaxDrift() const { return MaxDrift; }

	tjs_int GetMapWidth() const { return MapWidth; }
	tjs_int GetMapHeight() const { return MapHeight; }


public:
	tTVPRippleTable(tjs_int width, tjs_int height, tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth, float roundness, tjs_int maxdrift)
	{
		RefCount = 1;

		DisplaceMap = NULL;
		DriftMap = NULL;

		Width = width;
		Height = height;
		CenterX = centerx;
		CenterY = centery;
		RippleWidth = ripplewidth;
		Roundness = roundness;
		MaxDrift = maxdrift;

		MakeTable();
	}

protected:
	~tTVPRippleTable()
	{
		Clear();
	}

public:
	void AddRef()
	{
		RefCount ++;
	}

	void Release()
	{
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
	}

public:
	const tjs_uint16 * GetDisplaceMap(tjs_int x, tjs_int y) const
	{
		return DisplaceMap + x + y*MapWidth;
	}


	const tjs_uint16 * GetDriftMap(tjs_int drift, tjs_int phase)
	{
		return DriftMap + drift * RippleWidth * (2 * TVP_RIPPLE_DIR_PREC) +
			phase * TVP_RIPPLE_DIR_PREC;
	}

private:
	void MakeTable();
	void Clear();

};
//---------------------------------------------------------------------------
float inline TVPRippleWaveForm(float rad)
{
	// 波を生成する関数
	// 適当に。s は正にしかならないが見た目が良いのでこれでいく
	float s = (sin(rad) + sin(rad*2-2) * 0.2) / 1.19;
	s *= s;
	return s;
}
//---------------------------------------------------------------------------
void tTVPRippleTable::MakeTable()
{
	tjs_int32 *rippleform = NULL;
	tjs_int32 *cos_table = NULL;
	tjs_int32 *sin_table = NULL;

	try
	{
		// MapWidth, MapHeight の計算
		// Width, Height を CenterX, CenterY で分割する４つの象限のうち
		// もっとも大きい物のサイズを MapWidth, MapHeight とする
		MapWidth = CenterX < (Width >> 1) ?
			Width - CenterX : CenterX;
		MapHeight = CenterY < (Height >> 1) ?
			Height - CenterY : CenterY;

		// DisplaceMap メモリ確保
		DisplaceMap = new tjs_uint16[MapWidth * MapHeight];

		// DisplaceMap 計算
		// 置換マップは１象限についてのみ計算する(他の象限は対称だから)
		tjs_uint16 *rmp = DisplaceMap;
		tjs_int ripplemask = RippleWidth - 1;
		tjs_int x, y;
		for(y = 0; y < MapHeight; y++)
		{
			float yy = ((float)y + 0.5) * Roundness;
			float fac = 1.0 / yy;
			for(x = 0; x < MapWidth; x++)
			{
				float xx =  (float)x + 0.5;

				tjs_int dir = atan(xx*fac) * ((1.0/(M_PI/2.0)) * TVP_RIPPLE_DIR_PREC);
					// dir = 方向コード

				tjs_int dist = (int)sqrt(xx*xx + yy*yy) & ripplemask;
					// dist = 中心からの距離

				*(rmp++) = (tjs_uint16)((dist * TVP_RIPPLE_DIR_PREC) + dir);
			}
		}

		// DriftMap メモリ確保
		// DriftMap に使用するメモリ量は
		// MaxDrift*TVP_RIPPLE_DRIFT_PREC * RippleWidth * 2 * TVP_RIPPLE_DIR_PREC    *sizeof(tjs_uint16)
		// tjs_uint32 [MaxDrift*TVP_RIPPLE_DRIFT_PREC][RippleWidth*2][TVP_RIPPLE_DIR_PREC]
		// *2 が入っているのは 画像演算中に & でマスクをかける必要がないように
		DriftMap = new tjs_uint16[MaxDrift * TVP_RIPPLE_DRIFT_PREC * RippleWidth *
			2 * TVP_RIPPLE_DIR_PREC];


		// 波形の計算
		float rcp_rw = 1.0 / (float)RippleWidth;
		rippleform = new tjs_int32[RippleWidth];
		tjs_int w;
		for(w = 0; w < RippleWidth; w++)
		{
			// 適当に波っぽく見える波形(単純なsin波でもよい)
			float rad = (float)w * rcp_rw * (M_PI * -2.0);
			
			float s = TVPRippleWaveForm(rad);
			
			if(s < -1.0) s = -1.0;
			if(s > 1.0) s = 1.0;
			s *= 2048.0;
			rippleform[w] = (tjs_int32)(s < 0 ? s - 0.5 : s + 0.5); // 1.11
		}

		// sin/cos テーブルの生成
		cos_table = new tjs_int32[TVP_RIPPLE_DIR_PREC];
		sin_table = new tjs_int32[TVP_RIPPLE_DIR_PREC];
		for(w = 0; w < TVP_RIPPLE_DIR_PREC; w++)
		{
			float fdir = M_PI*0.5 - (((float)w + 0.5) *
				((1.0 / (float)TVP_RIPPLE_DIR_PREC) * (M_PI / 2.0)));
			float v;
			v = cos(fdir) * 2048.0;
			cos_table[w] = (tjs_int32)(v < 0 ? v - 0.5 : v + 0.5); // 1.11
			v = sin(fdir) * 2048.0;
			sin_table[w] = (tjs_int32)(v < 0 ? v - 0.5 : v + 0.5); // 1.11
		}

		// DriftMap 計算
		// float で計算するとエラく遅いので固定小数点で計算する
		tjs_int drift, dir;
		tjs_int ripplewidth_step = RippleWidth * TVP_RIPPLE_DIR_PREC;
		for(drift = 0; drift < MaxDrift*TVP_RIPPLE_DRIFT_PREC; drift ++)
		{
			tjs_int32 fdrift = (drift << 10) / TVP_RIPPLE_DRIFT_PREC; // 8.10
			tjs_uint16 *dmp = DriftMap + drift * RippleWidth * (2 * TVP_RIPPLE_DIR_PREC);
			for(w = 0; w < RippleWidth; w++)
			{
				tjs_int32 fd = rippleform[w] * fdrift >> 10; // 8.11
				for(dir = 0; dir < TVP_RIPPLE_DIR_PREC; dir++)
				{
					tjs_int32 xd = cos_table[dir] * fd >> 11; // 8.11
					tjs_int32 yd = sin_table[dir] * fd >> 11; // 8.11
					
					tjs_uint16 val = (tjs_uint16)(
						( (int)(char)(int)(xd >>11)<< 8) +
						  (int)(char)(int)(yd >>11) );

					dmp[w * TVP_RIPPLE_DIR_PREC +                    dir] =
					dmp[w * TVP_RIPPLE_DIR_PREC + ripplewidth_step + dir] = val;
				}
			}
		}

	}
	catch(...)
	{
		Clear();
		if(rippleform) delete [] rippleform;
		if(sin_table) delete [] sin_table;
		if(cos_table) delete [] cos_table;
		throw;
	}
	if(rippleform) delete [] rippleform;
	if(sin_table) delete [] sin_table;
	if(cos_table) delete [] cos_table;
}
//---------------------------------------------------------------------------
void tTVPRippleTable::Clear()
{
	if(DisplaceMap) delete [] DisplaceMap, DisplaceMap = NULL;
	if(DriftMap) delete [] DriftMap, DriftMap = NULL;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// キャッシュ管理
//---------------------------------------------------------------------------
#define TVP_RIPPLE_TABLE_MAX_CACHE 4
//---------------------------------------------------------------------------
static tTVPRippleTable *TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE] =
	{NULL};
//---------------------------------------------------------------------------
static tTVPRippleTable *TVPGetRippleTable
	(tjs_int width, tjs_int height, tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth, float roundness, tjs_int maxdrift)
{
	// キャッシュの中から指定された条件のデータを取ってくる
	// あればキャッシュ中での優先順位を最上位にして返し、
	// そうでなければデータを作成してキャッシュの最後のデータを削除し、
	// 優先順位の先頭に挿入して返す	

	// キャッシュ中にあるか
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		tTVPRippleTable * table = TVPRippleTableCache[i];
		if(!table) continue;

		if(
			table->GetWidth() == width &&
			table->GetHeight() == height &&
			table->GetCenterX() == centerx &&
			table->GetCenterY() == centery &&
			table->GetRippleWidth() == ripplewidth &&
			table->GetRoundness() == roundness &&
			table->GetMaxDrift() == maxdrift)
		{
			// キャッシュ中に見つかった

			// リストの先頭にもってくる
			if(i != 0)
			{
				memmove(TVPRippleTableCache + 1, TVPRippleTableCache,
					i * sizeof(tTVPRippleTable *));
				TVPRippleTableCache[0] = table;
			}

			// 参照カウンタをインクリメントして返す
			table->AddRef();
			return table;
		}
	}

	// キャッシュ中には見つからなかった

	// 最後の要素を削除
	if(TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1] != NULL)
	{
		tTVPRippleTable * table =
			TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1];
		TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1] = NULL;
		table->Release();
	}

	// データを作成
	tTVPRippleTable * table =
		new tTVPRippleTable
		(width, height, centerx, centery, ripplewidth, roundness, maxdrift);

	// リストの先頭に挿入
	memmove(TVPRippleTableCache + 1, TVPRippleTableCache,
		(TVP_RIPPLE_TABLE_MAX_CACHE -1) * sizeof(tTVPRippleTable *));
	TVPRippleTableCache[0] = table;
	table->AddRef();

	// 返す
	return table;
}
//---------------------------------------------------------------------------
static void TVPInitRippleTableCache()
{
	// キャッシュの初期化
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		TVPRippleTableCache[i] = NULL;
	}
}
//---------------------------------------------------------------------------
static void TVPClearRippleTableCache()
{
	// キャッシュをクリア
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		tTVPRippleTable * table =
			TVPRippleTableCache[i];
		TVPRippleTableCache[i] = NULL;
		if(table) table->Release();
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// 演算関数群 (TVPRippleTransform_????) は、
// ・置換マップテーブルを正方向に見ていくか逆方向に見ていくか (_f _b サフィックス)
// ・C バージョンと MMX/EMMX アセンブラバージョン (_c _mmx _emmx サフィックス)
// の 6 個と、上下左右を折り返しながら画面外を参照しないように慎重に
// 転送する C 関数 (_e サフィックス) 4 個からなる
// ・置換マップの y を正にとるか負にとるか (_a _d サフィックス)
//---------------------------------------------------------------------------
#define TVP_RIPPLE_BLEND 	{ \
		tjs_uint32 s1, s2, s1_; \
		s1 = *(const tjs_uint32*)(src1 + ofs); s2 = *(const tjs_uint32*)(src2 + ofs); \
		s1_ = s1 & 0xff00ff; s1_ = (s1_ + (((s2 & 0xff00ff) - s1_) * ratio >> 8)) & 0xff00ff; \
		s2 &= 0xff00; s1 &= 0xff00; \
		dest[i] = s1_ | ((s1 + ((s2 - s1) * ratio >> 8)) & 0xff00); \
	}
//---------------------------------------------------------------------------
static void TVPRippleTransform_c_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int ofs = (int)((i - (int)(char)(n>>8))*sizeof(tjs_uint32)) +
			(int)(char)(n)*pitch;
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_c_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int ofs = (int)((i + (int)(char)(n>>8))*sizeof(tjs_uint32)) +
			(int)(char)(n)*pitch;
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
static void TVPRippleTransform_mmx_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1
																		// ↓スレッド
		cmp			num,			ebx
		jng			pexit_mmx_f

	ploop_mmx_f_1:	// このループでは２つのスレッドを適当にインターリーブしている
		movzx		eax,			word ptr [ecx + ebx*2]				// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		neg			edx													// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			ecx,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[ecx + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			ecx,			displacemap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx + ebx*2 + 2]			// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		neg			edx													// 2
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			ecx,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[ecx + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			ecx,			displacemap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_mmx_f_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_f

	ploop_mmx_f_2:
		movzx		eax,			word ptr [ecx + ebx*2]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		neg			edx
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			ecx,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[ecx + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			ecx,			displacemap
		movd		[edi+ebx*4],	mm2

		inc			ebx

		cmp			num,			ebx
		jg			ploop_mmx_f_2

	pexit_mmx_f:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_mmx_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_b

	ploop_mmx_b_1:
		movzx		eax,			word ptr [ecx]						// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			esi,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[esi + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			esi,			driftmap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx - 2]					// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			esi,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[esi + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		sub			ecx,			4
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			esi,			driftmap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_mmx_b_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_b

	ploop_mmx_b_2:
		movzx		eax,			word ptr [ecx]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			esi,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[esi + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			esi,			driftmap
		movd		[edi+ebx*4],	mm2

		inc			ebx
		sub			ecx,			2

		cmp			num,			ebx
		jg			ploop_mmx_b_2

	pexit_mmx_b:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_emmx_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1
																		// ↓スレッド
		cmp			num,			ebx
		jng			pexit_emmx_f

	ploop_emmx_f_1:
		movzx		eax,			word ptr [ecx + ebx*2]				// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		neg			edx													// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			ecx,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[ecx + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			ecx,			displacemap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx + ebx*2 + 2]			// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
#ifdef __BORLANDC__
		} __emit__ (0x0f, 0x18, 0x4c, 0x59, 0x10); _asm{ // prefetcht0	[ecx + ebx*2 + 16]
#else
		} _asm _emit 0x0f _asm _emit 0x18 _asm _emit 0x4c _asm _emit 0x59 _asm _emit 0x10 _asm{ // prefetcht0	[ecx + ebx*2 + 16]
#endif
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		neg			edx													// 2
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			ecx,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[ecx + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			ecx,			displacemap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_emmx_f_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_f

	ploop_emmx_f_2:
		movzx		eax,			word ptr [ecx + ebx*2]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		neg			edx
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			ecx,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[ecx + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			ecx,			displacemap
		movd		[edi+ebx*4],	mm2

		inc			ebx

		cmp			num,			ebx
		jg			ploop_emmx_f_2

	pexit_emmx_f:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_emmx_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_b

	ploop_emmx_b_1:
		movzx		eax,			word ptr [ecx]						// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			esi,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[esi + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			esi,			driftmap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx - 2]					// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
#ifdef __BORLANDC__
		} __emit__ (0x0f, 0x18, 0x49, 0xf4); _asm{ // prefetcht0	[ecx - 12]
#else
		} _asm _emit 0x0f _asm _emit 0x18 _asm _emit 0x49 _asm _emit 0xf4 _asm{ // prefetcht0	[ecx - 12]
#endif
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			esi,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[esi + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		sub			ecx,			4
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			esi,			driftmap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_emmx_b_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_b

	ploop_emmx_b_2:
		movzx		eax,			word ptr [ecx]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			esi,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[esi + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			esi,			driftmap
		movd		[edi+ebx*4],	mm2

		inc			ebx
		sub			ecx,			2

		cmp			num,			ebx
		jg			ploop_emmx_b_2

	pexit_emmx_b:

		emms
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
typedef void (*tTVPRippleTransformFunc)(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int ratio);
static tTVPRippleTransformFunc TVPRippleTransform_f = TVPRippleTransform_c_f;
static tTVPRippleTransformFunc TVPRippleTransform_b = TVPRippleTransform_c_b;
//---------------------------------------------------------------------------
static void TVPInitRippleTransformFuncs()
{
	tjs_uint32 cputype = TVPGetCPUType();
	if(cputype & TVP_CPU_HAS_MMX)
	{
		// MMX が使用可能な場合
		TVPRippleTransform_f = TVPRippleTransform_mmx_f;
		TVPRippleTransform_b = TVPRippleTransform_mmx_b;
	}
	if((cputype & TVP_CPU_HAS_MMX) && (cputype & TVP_CPU_HAS_EMMX))
	{
		// MMX/EMMX が使用可能な場合
		// EMMX バージョンは MMX バージョンに prefetch 命令を追加しただけだが
		// 微妙に速い
		TVPRippleTransform_f = TVPRippleTransform_emmx_f;
		TVPRippleTransform_b = TVPRippleTransform_emmx_b;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#define TVP_RIPPLE_TURN_BORDER { \
		if(x<0) x = -x; \
		if(y<0) y = -y; \
		if(x>=srcwidth) x = srcwidth - 1 - (x - srcwidth); \
		if(y>=srcheight) y = srcheight - 1 - (y - srcheight); \
	}
#define TVP_RIPPLE_CALC_OFS tjs_uint ofs = \
		x*sizeof(tjs_uint32) + y*pitch;
//---------------------------------------------------------------------------
static void TVPRippleTransform_f_a_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int x = srcx + i - (int)(char)(n>>8);
		tjs_int y = srcy + (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_f_d_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int x = srcx + i - (int)(char)(n>>8);
		tjs_int y = srcy - (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_b_a_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int x = srcx + i + (int)(char)(n>>8);
		tjs_int y = srcy + (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_b_d_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int x = srcx + i + (int)(char)(n>>8);
		tjs_int y = srcy - (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
#undef TVP_RIPPLE_CALC_OFS
#undef TVP_RIPPLE_TURN_BORDER
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#undef TVP_RIPPLE_BLEND
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// tTVPRippleTransHandler
//---------------------------------------------------------------------------
class tTVPRippleTransHandler : public iTVPDivisibleTransHandler
{
	//	'波紋' トランジションハンドラクラスの実装

	tjs_int RefCount; // 参照カウンタ
		/*
			iTVPDivisibleTransHandler は 参照カウンタによる管理を行う
		*/

protected:
	tjs_uint64 StartTick; // トランジションを開始した tick count
	tjs_uint64 Time; // トランジションに要する時間
	tTVPLayerType LayerType; // レイヤのタイプ
	tjs_int Width; // 処理する画像の幅
	tjs_int Height; // 処理する画像の高さ
	tjs_int64 CurTime; // 現在の tick count
	tjs_int BlendRatio; // ブレンド比
	tjs_int Phase; // 位相
	tjs_int Drift; // 揺れ
	bool First; // 一番最初の呼び出しかどうか

	tjs_int DriftCarePixels; // 周囲の折り返しに注意しなければならないピクセル数

	tjs_int CenterX; // 中心 X 座標
	tjs_int CenterY; // 中心 Y 座標
	tjs_int RippleWidth; // 波紋の幅 (16, 32, 64, 128 のいずれか)
	float Roundness; // 波紋の縦/横比
	float Speed; // 波紋の動く角速度
	tjs_int MaxDrift; // 揺れの最大幅(ピクセル単位) (127まで)

	const tjs_uint16 *CurDriftMap; // 現在描画中の DirftMap

	tTVPRippleTable *Table; // 置換マップなどのテーブル

#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	tjs_int UpdateCount;
#endif

public:
	tTVPRippleTransHandler(tjs_uint64 time, tTVPLayerType layertype,
		tjs_int width, tjs_int height,
		tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth,
		float roundness, float speed, tjs_int maxdrift)
	{
		RefCount = 1;

		LayerType = layertype;
		Width = width;
		Height = height;
		Time = time;

		CenterX = centerx;
		CenterY = centery;

		RippleWidth = ripplewidth;

		Roundness = roundness;
		Speed = speed;

		First = true;

		MaxDrift = maxdrift;

		Table =
			TVPGetRippleTable(Width, Height, CenterX, CenterY,
				RippleWidth, Roundness, MaxDrift);
	}

	virtual ~tTVPRippleTransHandler()
	{
#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
		TVPAddLog(TJS_W("ripple update count : ") + ttstr(UpdateCount));
#endif
		Table->Release();
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
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::StartProcess(tjs_uint64 tick)
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
#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
		UpdateCount = 0;
#endif
	}

	// 画像演算に必要な各パラメータを計算
	CurTime = (tick - StartTick);

	// BlendRatio
	BlendRatio = CurTime * 255 / Time;
	if(BlendRatio > 255) BlendRatio = 255;

	// Phase
	// 角速度が Speed (rad/sec) で与えられている
	Phase = (int)(Speed * ((1.0/(M_PI*2))*(1.0/1000.0)) * CurTime * RippleWidth) % RippleWidth;
	if(Phase < 0) Phase = 0;
	Phase = RippleWidth - Phase - 1;

	// Drift
	float s = sin(M_PI * CurTime / Time);
	Drift = (int)(s * MaxDrift * TVP_RIPPLE_DRIFT_PREC);
	if(Drift < 0) Drift = 0;
	if(Drift >= MaxDrift * TVP_RIPPLE_DRIFT_PREC) Drift = MaxDrift * TVP_RIPPLE_DRIFT_PREC - 1;

	DriftCarePixels = (int)(Drift / TVP_RIPPLE_DRIFT_PREC) + 1;
	if(DriftCarePixels&1) DriftCarePixels ++; // 一応偶数にアライン

	// CurDriftMap
	CurDriftMap = Table->GetDriftMap(Drift, Phase);

#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	UpdateCount ++;
#endif

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::EndProcess()
{
	// トランジションの画面更新一回分が終わるごとに呼ばれる

	if(BlendRatio == 255) return TJS_S_FALSE; // トランジション終了

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::Process(
			tTVPDivisibleData *data)
{
	// トランジションの各領域ごとに呼ばれる
	// 吉里吉里は画面を更新するときにいくつかの領域に分割しながら処理を行うので
	// このメソッドは通常、画面更新一回につき複数回呼ばれる

	// data には領域や画像に関する情報が入っている

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

	if(src1pitch != src2pitch) return TJS_E_FAIL; // 両方のpitchが一致していないと駄目

	// ラインごとに処理
	tjs_int h = data->Height;
	tjs_int y = data->Top;
	while(h--)
	{
		tjs_int l, r;

		if(y < DriftCarePixels || y >= Height - DriftCarePixels)
		{
			// 上下のすみではみ出す可能性があるので
			// 折り返し転送を行う

			// 左端 ～ CenterX
			l = 0;
			r = CenterX;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				if(y < CenterY)
				{
					TVPRippleTransform_b_a_e(
						Table->GetDisplaceMap(CenterX - l - 1, CenterY - y - 1),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
				else
				{
					TVPRippleTransform_b_d_e(
						Table->GetDisplaceMap(CenterX - l - 1, y - CenterY),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
			}

			// CenterX ～ 右端
			l = CenterX;
			r = Width;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				if(y < CenterY)
				{
					TVPRippleTransform_f_a_e(
						Table->GetDisplaceMap(l - CenterX, CenterY - y - 1),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
				else
				{
					TVPRippleTransform_f_d_e(
						Table->GetDisplaceMap(l - CenterX, y - CenterY),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
			}

		}
		else
		{
			// 左端 ～ CenterX
			l = 0;
			r = CenterX;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				int ll, rr;
				ll = 0, rr = DriftCarePixels;
				if(Clip(ll, rr, l, r))
				{
					// この ll ～ rr で表される左端は 左端にはみ出す可能性がある
					// ので折り返し転送をさせる
					if(y < CenterY)
					{
						TVPRippleTransform_b_a_e(
							Table->GetDisplaceMap(CenterX - ll - 1, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
					else
					{
						TVPRippleTransform_b_d_e(
							Table->GetDisplaceMap(CenterX - ll - 1, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
				}

				ll = DriftCarePixels; rr = r;
				if(Clip(ll, rr, l, r))
				{
					// ここははみ出さない
					if(y < CenterY)
					{
						TVPRippleTransform_b(
							Table->GetDisplaceMap(CenterX - ll - 1, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
					else
					{
						TVPRippleTransform_b(
							Table->GetDisplaceMap(CenterX - ll - 1, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							-src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
				}
			}

			// CenterX ～ 右端
			l = CenterX;
			r = Width;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				int ll, rr;
				ll = l, rr = Width - DriftCarePixels;
				if(Clip(ll, rr, l, r))
				{
					// ここははみ出さない
					if(y < CenterY)
					{
						TVPRippleTransform_f(
							Table->GetDisplaceMap(ll - CenterX, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
					else
					{
						TVPRippleTransform_f(
							Table->GetDisplaceMap(ll - CenterX, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							-src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
				}

				ll = Width - DriftCarePixels, rr = r;
				if(Clip(ll, rr, l, r))
				{
					// この ll ～ rr で表される右端は 右端にはみ出す可能性がある
					// ので折り返し転送をさせる
					if(y < CenterY)
					{
						TVPRippleTransform_f_a_e(
							Table->GetDisplaceMap(ll - CenterX, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
					else
					{
						TVPRippleTransform_f_d_e(
							Table->GetDisplaceMap(ll - CenterX, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
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





//---------------------------------------------------------------------------
class tTVPRippleTransHandlerProvider : public iTVPTransHandlerProvider
{
	tjs_uint RefCount; // 参照カウンタ
public:
	tTVPRippleTransHandlerProvider() { RefCount = 1; }
	~tTVPRippleTransHandlerProvider() {; }

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
		if(name) *name = TJS_W("ripple");
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

		tjs_int centerx = src1w >> 1, centery = src1h >> 1;
		tjs_int ripplewidth = 128;
		float roundness = 1.0;
		float speed = 6;
		tjs_int maxdrift = 24;
//		tjs_int rippletype = 0; // タイプ


		if(TJS_FAILED(options->GetValue(TJS_W("time"), &tmp)))
			return TJS_E_FAIL; // time 属性が指定されていない
		if(tmp.Type() == tvtVoid) return TJS_E_FAIL;
		time = (tjs_int64)tmp;
		if(time < 2) time = 2; // あまり小さな数値を指定すると問題が起きるので

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("centerx"), &tmp)))
			if(tmp.Type() != tvtVoid) centerx = (tjs_int)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("centery"), &tmp)))
			if(tmp.Type() != tvtVoid) centery = (tjs_int)tmp;

		if(centerx < 0 || centery < 0 ||
			(tjs_uint)centerx >= src1w || (tjs_uint)centery >= src1h)
			TVPThrowExceptionMessage(TJS_W("centerx and centery cannot be out of the image"));


		if(TJS_SUCCEEDED(options->GetValue(TJS_W("rwidth"), &tmp)))
			if(tmp.Type() != tvtVoid) ripplewidth = (tjs_int)tmp;

		if(ripplewidth != 16 && ripplewidth != 32 && ripplewidth != 64 &&
			ripplewidth != 128)
			TVPThrowExceptionMessage(TJS_W("rwidth must be 16, 32, 64 or 128"));


		if(TJS_SUCCEEDED(options->GetValue(TJS_W("roundness"), &tmp)))
			if(tmp.Type() != tvtVoid) roundness = (float)(double)tmp;

		if(roundness <= 0.0)
			TVPThrowExceptionMessage(TJS_W("roundness cannot be nagative or equal to 0"));

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("speed"), &tmp)))
			if(tmp.Type() != tvtVoid) speed = (float)(double)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("maxdrift"), &tmp)))
			if(tmp.Type() != tvtVoid) maxdrift = (tjs_int)tmp;
		if(maxdrift < 0 || maxdrift >= 128)
			TVPThrowExceptionMessage(TJS_W("maxdrift cannot be nagative or larger than 127"));

		if((tjs_uint)maxdrift >= src1w || (tjs_uint)maxdrift >= src1h)
			TVPThrowExceptionMessage(TJS_W("maxdrift must be lesser than both image width and height"));

		// オブジェクトを作成
		*handler = new tTVPRippleTransHandler(time, layertype,
			src1w, src1h, centerx, centery,
			ripplewidth, roundness, speed, maxdrift);

		return TJS_S_OK;
	}

} static * RippleTransHandlerProvider;
//---------------------------------------------------------------------------
void RegisterRippleTransHandlerProvider()
{
	TVPInitRippleTableCache(); // テーブルのキャッシュの初期化

	TVPInitRippleTransformFuncs(); // 演算関数の初期化

	// TVPAddTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録する
	RippleTransHandlerProvider = new tTVPRippleTransHandlerProvider();
	TVPAddTransHandlerProvider(RippleTransHandlerProvider);
}
//---------------------------------------------------------------------------
void UnregisterRippleTransHandlerProvider()
{
	// TVPRemoveTransHandlerProvider を使ってトランジションハンドラプロバイダを
	// 登録抹消する
	TVPRemoveTransHandlerProvider(RippleTransHandlerProvider);
	RippleTransHandlerProvider->Release();

	TVPClearRippleTableCache(); // 置換マップなどのテーブルのキャッシュのクリア
}
//---------------------------------------------------------------------------
