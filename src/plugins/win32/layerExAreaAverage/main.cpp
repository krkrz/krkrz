#include <windows.h>
#include "ncbind.hpp"

typedef tjs_int	fixdot;
#define DOTBASE			12
#define	INT2FIXDOT(a)	(fixdot)((a) << DOTBASE)
#define	REAL2FIXDOT(a)	(fixdot)((a) * INT2FIXDOT(1))
#define	FIXDOT2INT(a)	((a) >> DOTBASE)
//	↓結果はfixdot
#define	MULFIXDOT(a, b)	(((a) * (b)) >> DOTBASE)
//	↓結果はint
#define	DIVFIXDOT(a, b)	((a) / (b))
#define	ROUNDFIXDOT(a)	FIXDOT2INT((a) + REAL2FIXDOT(0.5))

struct layerExAreaAverage
{
	static tjs_error TJS_INTF_METHOD stretchCopyAA(tTJSVariant *result,
														tjs_int numparams,
														tTJSVariant **param,
														iTJSDispatch2 *objthis)
	{
		if(numparams < 9) return TJS_E_BADPARAMCOUNT;

		tjs_uint64	tick = TVPGetTickCount();	//	開始時刻

		//	転送先レイヤーの情報を取得
		tjs_int	dImageWidth, dImageHeight, dPitch;
		tjs_uint8 * dBuffer;
		tTJSVariant	val;
		objthis->PropGet(0, L"imageWidth", NULL, &val, objthis);
		dImageWidth = (tjs_int)val;
		objthis->PropGet(0, L"imageHeight", NULL, &val, objthis);
		dImageHeight = (tjs_int)val;
		objthis->PropGet(0, L"mainImageBufferPitch", NULL, &val, objthis);
		dPitch = (tjs_int)val;
		objthis->PropGet(0, L"mainImageBufferForWrite", NULL, &val, objthis);
		dBuffer = (tjs_uint8*)(tjs_int)val;

		//	転送先の位置、サイズを取得
		tjs_int dLeft, dTop, dWidth, dHeight;
		dLeft	= param[0]->AsInteger();
		dTop	= param[1]->AsInteger();
		dWidth	= param[2]->AsInteger();
		dHeight	= param[3]->AsInteger();

		//	転送元レイヤーの情報を取得
		tjs_int sImageWidth, sImageHeight, sPitch;
		tjs_uint8 * sBuffer;
		iTJSDispatch2* srcobj	= param[4]->AsObjectNoAddRef();
		srcobj->PropGet(0, L"imageWidth", NULL, &val, srcobj);
		sImageWidth = (tjs_int)val;
		srcobj->PropGet(0, L"imageHeight", NULL, &val, srcobj);
		sImageHeight = (tjs_int)val;
		srcobj->PropGet(0, L"mainImageBufferPitch", NULL, &val, srcobj);
		sPitch = (tjs_int)val;
		srcobj->PropGet(0, L"mainImageBuffer", NULL, &val, srcobj);
		sBuffer = (tjs_uint8*)(tjs_int)val;

		//	転送元の位置、サイズを取得
		tjs_int sLeft, sTop, sWidth, sHeight;
		sLeft	= param[5]->AsInteger();
		sTop	= param[6]->AsInteger();
		sWidth	= param[7]->AsInteger();
		sHeight	= param[8]->AsInteger();

		//	拡大処理は行なえません
		if(dWidth > sWidth || dHeight > sHeight)
		{
			TVPThrowExceptionMessage(L"stretchCopyAA は拡大処理を行なえません。");
			return TJS_E_FAIL;
		}

		//	クリッピング
		if(dLeft + dWidth > dImageWidth)
		{
			tjs_int	dw	= dImageWidth - dLeft;
			sWidth	= (tjs_int)((tjs_real)sWidth * ((tjs_real)dw / (tjs_real)dWidth));
			dWidth	= dw;
		}
		if(sLeft + sWidth > sImageWidth)
		{
			tjs_int	sw	= sImageWidth - sLeft;
			dWidth	= (tjs_int)((tjs_real)dWidth * ((tjs_real)sw / (tjs_real)sWidth));
			sWidth	= sw;
		}
		if(dTop + dHeight > dImageHeight)
		{
			tjs_int	dh	= dImageHeight - dTop;
			sHeight	= (tjs_int)((tjs_real)sHeight * ((tjs_real)dh / (tjs_real)dHeight));
			dHeight	= dh;
		}
		if(sTop + sHeight > sImageHeight)
		{
			tjs_int	sh	= sImageHeight - sTop;
			dHeight	= (tjs_int)((tjs_real)dHeight * ((tjs_real)sh / (tjs_real)sHeight));
			sHeight	= sh;
		}

		fixdot	sl	= INT2FIXDOT(sLeft);
		fixdot	st	= INT2FIXDOT(sTop);
		fixdot	rw	= REAL2FIXDOT((tjs_real)sWidth / dWidth);
		fixdot	rh	= REAL2FIXDOT((tjs_real)sHeight / dHeight);

		for(tjs_int y=0; y<dHeight; y++)
		{
			tjs_uint32*	outpixel	= (tjs_uint32*)(dBuffer + (y + dTop) * dPitch);
			outpixel	+= dLeft;
			for(tjs_int x=0; x<dWidth; x++)
			{
				//	縮小後画像の(x, y)座標に対応する領域
				fixdot		x1	= sl + x * rw;	//	int * fixdot = fixdot
				fixdot		y1	= st + y * rh;
				fixdot		x2	= x1 + rw;
				fixdot		y2	= y1 + rh;

				//	整数領域に割り当て
				tjs_int		sx	= FIXDOT2INT(x1);
				tjs_int		sy	= FIXDOT2INT(y1);
				tjs_int		ex	= FIXDOT2INT(x2 + INT2FIXDOT(1) - 1);
				tjs_int		ey	= FIXDOT2INT(y2 + INT2FIXDOT(1) - 1);
				if(ex >= sImageWidth)	ex	= sImageWidth - 1;
				if(ey >= sImageHeight)	ey	= sImageHeight - 1;

				fixdot		totalarea_a = 0, a = 0;
				fixdot		totalarea_rgb = 0, r = 0, g = 0, b = 0;
				for(tjs_int ay=sy; ay<ey; ay++)
				{
					tjs_uint32*	inpixel	= (tjs_uint32*)(sBuffer + ay * sPitch);
					inpixel	+= sx;

					fixdot		e1	= INT2FIXDOT(ay);
					fixdot		e2	= INT2FIXDOT(ay + 1);
					if(e1 < y1)	e1	= y1;	//	上端
					if(e2 > y2)	e2	= y2;	//	下端
					fixdot		ah	= e2 - e1;
					for(tjs_int ax=sx; ax<ex; ax++)
					{
						e1	= INT2FIXDOT(ax);
						e2	= INT2FIXDOT(ax + 1);
						if(e1 < x1)	e1	= x1;	//	左端
						if(e2 > x2)	e2	= x2;	//	右端
						fixdot		aw	= e2 - e1;
						fixdot		area	= MULFIXDOT(aw, ah);
						totalarea_a	+= area;
						tjs_uint32	alpha = (*inpixel >> 24) & 0xFF;
						a	+= alpha * area;
						area	= (area * alpha) >> 8;	// α値が低いピクセルは、色への影響を小さくする
						r	+= ((*inpixel >> 16) & 0xFF) * area;
						g	+= ((*inpixel >>  8) & 0xFF) * area;
						b	+= ((*inpixel      ) & 0xFF) * area;
						totalarea_rgb	+= area;

						inpixel++;
					}
				}

				if(totalarea_a == 0) continue;

				//	平均値を計算し、設定
				a	= DIVFIXDOT(a, totalarea_a);
				if(totalarea_rgb == 0)
				{
					r = g = b = 0;
				}
				else
				{
					r	= DIVFIXDOT(r, totalarea_rgb);
					g	= DIVFIXDOT(g, totalarea_rgb);
					b	= DIVFIXDOT(b, totalarea_rgb);
				}
				*outpixel	= ((((tjs_int)a)&0xFF) << 24) |((((tjs_int)r)&0xFF) << 16) | ((((tjs_int)g)&0xFF) << 8) | (((tjs_int)b)&0xFF);// | 0xFF000000;

				outpixel++;
			}
		}

		//	領域を更新させる
		{
			tTJSVariant val[4];
			tTJSVariant *pval[4] = { val, val +1, val +2, val +3 };
			val[0] = (tjs_int64)dLeft;
			val[1] = (tjs_int64)dTop;
			val[2] = (tjs_int64)dWidth;
			val[3] = (tjs_int64)dHeight;
			static tjs_uint32 update_hint = 0;
			objthis->FuncCall(0, L"update", &update_hint, NULL, 4, pval, objthis);
		}

		TVPAddLog(TJS_W("stretch copy by area average:RESULT (")+ttstr((int)sWidth)+L","+ttstr((int)sHeight)+L")->("+ttstr((int)dWidth)+L","+ttstr((int)dHeight)+L"), time = "+ttstr((int)(TVPGetTickCount() - tick))+L"(ms)");

		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(layerExAreaAverage, Layer)
{
	RawCallback("stretchCopyAA", &Class::stretchCopyAA, 0);
}
