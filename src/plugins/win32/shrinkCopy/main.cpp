#include "ncbind.hpp"

struct LayerUtils
{
	// １ピクセルの型
	typedef unsigned long PixelT;

	// バッファ参照用の型
	typedef unsigned char UnitT;
	typedef UnitT const *BufRefT;
	typedef UnitT       *WrtRefT;
	typedef tTVReal              real;

	static bool IsValidLayer(iTJSDispatch2 *lay)
	{
		// レイヤインスタンス以外ではエラー
		if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))) return false;

		// レイヤイメージは在るか？
		tTJSVariant val;
		if (TJS_FAILED(lay->PropGet(0, TJS_W("hasImage"), 0, &val, lay)) || (val.AsInteger() == 0)) return false;

		return true;
	}

	/**
	 * レイヤのサイズとバッファを取得する
	 */
	static bool GetLayerSize(iTJSDispatch2 *lay, long &w, long &h, long &pitch)
	{
		if (!IsValidLayer(lay)) return false;

		// レイヤサイズを取得
		tTJSVariant val;
		if (TJS_FAILED(lay->PropGet(0, TJS_W("imageWidth"), 0, &val, lay))) return false;
		w = (long)val.AsInteger();

		val.Clear();
		if (TJS_FAILED(lay->PropGet(0, TJS_W("imageHeight"), 0, &val, lay))) return false;
		h = (long)val.AsInteger();

		// ピッチ取得
		val.Clear();
		if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferPitch"), 0, &val, lay))) return false;
		pitch = (long)val.AsInteger();

		// 正常な値かどうか
		return (w > 0 && h > 0 && pitch != 0);
	}

	// 読み込み用
	static bool GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, BufRefT &ptr, long &pitch)
	{
		if (!GetLayerSize(lay, w, h, pitch)) return false;

		// バッファ取得
		tTJSVariant val;
		if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBuffer"), 0, &val, lay))) return false;
		ptr = reinterpret_cast<BufRefT>(val.AsInteger());
		return  (ptr != 0);
	}

	// 書き込み用
	static bool GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, WrtRefT &ptr, long &pitch)
	{
		if (!GetLayerSize(lay, w, h, pitch)) return false;

		// バッファ取得
		tTJSVariant val;
		if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferForWrite"), 0, &val, lay))) return false;
		ptr = reinterpret_cast<WrtRefT>(val.AsInteger());
		return  (ptr != 0);
	}
};

struct ShrinkCopy : public LayerUtils
{
	// TJS Method
	static tjs_error (TJS_INTF_METHOD layerShrinkCopy)(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *dst)
	{
		if (numparams < 9) return TJS_E_BADPARAMCOUNT;
		ShrinkCopy inst(
			dst,
			param[0]->AsReal(),
			param[1]->AsReal(),
			param[2]->AsReal(),
			param[3]->AsReal(),
			param[4]->AsObjectNoAddRef(),
			(long)param[5]->AsInteger(),
			(long)param[6]->AsInteger(),
			(long)param[7]->AsInteger(),
			(long)param[8]->AsInteger());
		if (!inst.check()) return TJS_E_INVALIDPARAM;
		if (!inst.clip())  return TJS_S_OK;
		inst.copy();
		return TJS_S_OK;
	}

	ShrinkCopy(iTJSDispatch2 *_dst, real _dx, real _dy, real _dw, real _dh,
			   iTJSDispatch2 *_src, long _sx, long _sy, long _sw, long _sh)
		:   dst(_dst), dx(_dx), dy(_dy), dw(_dw), dh(_dh),
			src(_src), sx(_sx), sy(_sy), sw(_sw), sh(_sh),
			ps(0), siw(0), sih(0), spch(0),
			pd(0), diw(0), dih(0), dpch(0)
	{
	}

	bool check()
	{
		if (sw <= 0|| sh <= 0 || dw <= 0 || dh <= 0 ||
			sw < (long)dw || sh < (long)dh) return false;

		// サイズ取得
		if (!GetLayerBufferAndSize(src, siw, sih, ps, spch) ||
			!GetLayerBufferAndSize(dst, diw, dih, pd, dpch)) return false;

		return true;
	}

	static inline long RtoL(real const r) { return ((r) < 0) ? -(long)(-(r)) : (long)(r); }

	bool clip()
	{
		real zx = dw / (real)sw;
		real zy = dh / (real)sh;
		real dcut;
		long scut;

		// srcクリッピング
		if (sx  + sw <= 0 || sy  + sh <= 0 ||
			sx >= siw     || sy >= sih) return false;

		if (sx < 0) {
			sw += sx;
			dw -= (dcut = zx * (real)(-sx));
			sx  = 0;
			dx += dcut;
		}
		if (sy < 0) {
			sh += sy;
			dh -= (dcut = zy * (real)(-sy));
			sy  = 0;
			dy += dcut;
		}
		if ((scut = sx + sw - siw) > 0) (sw -= scut), (dw -= zx * (real)(scut));
		if ((scut = sy + sh - sih) > 0) (sh -= scut), (dh -= zy * (real)(scut));

		// dstの整数位置
		dtx = RtoL(dx);
		dty = RtoL(dy);
		dtw = RtoL(dx + dw) - dtx;
		dth = RtoL(dy + dh) - dty;
		if ((dx + dw) > (real)(dtx + dtw)) dtw++;
		if ((dy + dh) > (real)(dty + dth)) dth++;

		// dstクリッピング
		if (dtx  + dtw <= 0 || dty  + dth <= 0 ||
			dtx >= diw      || dty >= dih) return false;

		// dstクリッピング範囲
		dsx = (dtx < 0) ? -dtx : 0;
		dsy = (dty < 0) ? -dty : 0;
		dex = (dtx+dtw > diw) ? (diw-dtx) : dtw;
		dey = (dty+dth > dih) ? (dih-dty) : dth;

#if 0
		// デバッグ用
		tjs_char tmp[64];
		TVPAddLog(ttstr(TJS_W("shrinkInfo: dtx="))
				  + ttstr(TJS_int_to_str(dtx, tmp)).c_str() + TJS_W(", dty=") +
				  + ttstr(TJS_int_to_str(dty, tmp)).c_str() + TJS_W(", dtw=") +
				  + ttstr(TJS_int_to_str(dtw, tmp)).c_str() + TJS_W(", dth=") +
				  + ttstr(TJS_int_to_str(dth, tmp)).c_str() + TJS_W(" / dsx=") +
				  + ttstr(TJS_int_to_str(dsx, tmp)).c_str() + TJS_W(", dsy=") +
				  + ttstr(TJS_int_to_str(dsy, tmp)).c_str() + TJS_W(", dex=") +
				  + ttstr(TJS_int_to_str(dex, tmp)).c_str() + TJS_W(", dey=") +
				  + ttstr(TJS_int_to_str(dey, tmp)).c_str() );
#endif
		return true;
	}

	typedef unsigned long AvgT;
	typedef unsigned char uchar;
	struct AvgInfoT { int offset, step; AvgT ta, tc, ba, bc, total; };
	struct ElementT { AvgT r, g, b, a; };

	void copy() {
		AvgInfoT *horz=0, *vert=0;
		void *buf = allocAvgBuffer(horz, vert, dex-dsx, dey-dsy);
		if (!horz || !vert) return;

		// 縦横別に平均化テーブル作成
		AvgT hu = makeAvgTable(horz, true);
		AvgT vu = makeAvgTable(vert, false);
		AvgT unit = hu * vu;

		WrtRefT p, pl = pd + ((dtx+dsx)<<2) + ((dty+dsy)*dpch);
		AvgInfoT const *vi = vert;
		for (long x,   y=dsy; y < dey; y++,vi++,pl+=dpch) {
			BufRefT rl = ps + vi->offset;
			AvgInfoT const *hi = horz;
			for (p=pl, x=dsx; x < dex; x++,hi++,p+=4) {
				ElementT sum = { 0, 0, 0, 0 };
				BufRefT r = rl + hi->offset;
				const int w = hi->step;
				const int h = vi->step;
				const int ox = w << 2;
				const int oy = h * spch;
				addPoint(sum, r- 4-spch,       hi->ta * vi->ta,  hi->tc * vi->tc); // 左上
				addHorz (sum, r   -spch, w,    hu     * vi->ta,  hu     * vi->tc); //   上
				addPoint(sum, r+ox-spch,       hi->ba * vi->ta,  hi->bc * vi->tc); // 右上
				addVert (sum, r- 4,         h, hi->ta * vu,      hi->tc * vu    ); // 左
				addRect (sum, r,         w, h, unit                             ); // 中央
				addVert (sum, r+ox,         h, hi->ba * vu,      hi->bc * vu    ); // 右
				addPoint(sum, r- 4+oy,         hi->ta * vi->ba,  hi->tc * vi->bc); // 左下
				addHorz (sum, r   +oy,   w,    hu     * vi->ba,  hu     * vi->bc); //   下
				addPoint(sum, r+ox+oy,         hi->ba * vi->ba,  hi->bc * vi->bc); // 右下
				AvgT div = hi->total * vi->total;
				p[0] = (uchar)(sum.r / div);
				p[1] = (uchar)(sum.g / div);
				p[2] = (uchar)(sum.b / div);
				p[3] = (uchar)(sum.a / div);
			}
		}
		freeAvgBuffer(buf);
	}

	struct MakeAvgWorkT {
		AvgT unit;
		real max, ratio, diff;
		long stop, ofmul;
	};
	AvgT makeAvgTable(AvgInfoT *tbl, bool isHorz) {
		MakeAvgWorkT work;
		long pos, end;
		if (isHorz) {
			pos = dsx;
			end = dex-1;
			work.max   = (real)sw;
			work.ratio = work.max / dw;
			work.diff  = dx - (real)dtx;
			work.stop  = sx;
			work.ofmul = 4;
		} else {
			pos = dsy;
			end = dey-1;
			work.max   = (real)sh;
			work.ratio = work.max / dh;
			work.diff  = dy - (real)dty;
			work.stop  = sy;
			work.ofmul = spch;
		}
		work.unit = 256; // 1dotの解像度
		if (work.ratio <= 1.0/16) {
			// 1/16以下で桁あふれの可能性があるのでunitを小さくする
			work.unit /= (int)((2.0/16.0)/work.ratio);
			if (work.unit <= 0) work.unit = 1;
		}
		if (pos == end) {
			// 縮小幅が１ドットの場合
			setAvgInfoEdge(work, tbl,   pos);
		} else {
			setAvgInfoEdge(work, tbl++, pos++);
			while (pos < end)
				setAvgInfo(work, tbl++, pos++);
			setAvgInfoEdge(work, tbl++, pos++);
		}
		return work.unit;
	}
	// AvgInfo設定(端以外:ta==tc,ba==bc)
	inline void setAvgInfo(MakeAvgWorkT const &wk, AvgInfoT *tbl, long pos) {
		AvgT const unit = wk.unit;
		real r1, r2;
		r1 = ((real)pos - wk.diff) * (r2 = wk.ratio);
		r2 += r1;
		long t1 = RtoL(r1), t2 = RtoL(r2);
		tbl->offset = (wk.stop + t1 + 1) * wk.ofmul;
		tbl->total = (
			(tbl->ta = tbl->tc = (unit - RtoL((r1 - (real)t1) * unit))) +
			(tbl->ba = tbl->bc = (       RtoL((r2 - (real)t2) * unit))) +
			(tbl->step = t2-t1-1) * unit);
	}
	// AvgInfo設定(端例外:ta!=tc,ba!=bc)
	inline void setAvgInfoEdge(MakeAvgWorkT const &wk, AvgInfoT *tbl, long pos) {
		AvgT const unit = wk.unit;
		real r1, r2, f1, f2;
		r1 = ((real)pos - wk.diff) * (r2 = wk.ratio);
		r2 += r1;
		if ((f1 = r1) < 0) f1 = 0;
		if ((f2 = r2) > wk.max) f2 = wk.max;
		long t1 = RtoL(r1), t2 = RtoL(r2);
		long u1 = RtoL(f1), u2 = RtoL(f2);
		tbl->offset = (wk.stop + u1 + 1) * wk.ofmul;
		tbl->total = (
			(          tbl->tc = (unit - RtoL((r1 - (real)t1) * unit))) +
			(          tbl->bc = (       RtoL((r2 - (real)t2) * unit))) +
			(            t2-t1-1) * unit);
		/**/ tbl->ta =           (unit - RtoL((f1 - (real)u1) * unit));
		/**/ tbl->ba =           (       RtoL((f2 - (real)u2) * unit));
		/**/ tbl->step = u2-u1-1;
	}

	void* allocAvgBuffer(AvgInfoT* &hi, AvgInfoT* &vi, long w, long h) {
		void *ret = malloc(sizeof(AvgInfoT)*w*h);
		hi = (AvgInfoT*)ret;
		vi = hi + w;
		return ret;
	}
	void  freeAvgBuffer(void *buf) {
		if (buf) free(buf);
	}

	inline void addRect(ElementT &sum, BufRefT p, int w, int h, AvgT mul) {
		if (mul) for (; h > 0; h--, p+=spch) {
			PixelT *r = (PixelT*)p;
			for (int n = w; n > 0; n--,r++) {
				PixelT col = *r;
				sum.r += ( col      & 0xFF) *mul;
				sum.g += ((col>> 8) & 0xFF) *mul;
				sum.b += ((col>>16) & 0xFF) *mul;
				sum.a += ((col>>24)       ) *mul;
			}
		}
	}
	inline void addVert(ElementT &sum, BufRefT p, int h, AvgT mul1, AvgT mul2) {
		if (mul1) for (; h > 0; h--, p+=spch) {
			PixelT col = *(PixelT*)p;
			sum.r += ( col      & 0xFF) *mul2;
			sum.g += ((col>> 8) & 0xFF) *mul2;
			sum.b += ((col>>16) & 0xFF) *mul2;
			sum.a += ((col>>24)       ) *mul1;
		}
	}
	inline void addHorz(ElementT &sum, BufRefT p, int w, AvgT mul1, AvgT mul2) {
		if (mul1) for (PixelT *r = (PixelT*)p; w > 0; w--,r++) {
			PixelT col = *r;
			sum.r += ( col      & 0xFF) *mul2;
			sum.g += ((col>> 8) & 0xFF) *mul2;
			sum.b += ((col>>16) & 0xFF) *mul2;
			sum.a += ((col>>24)       ) *mul1;
		}
	}
	inline void addPoint(ElementT &sum, BufRefT p, AvgT mul1, AvgT mul2) {
		if (mul1) {
			PixelT col = *(PixelT*)p;
			sum.r += ( col      & 0xFF) *mul2;
			sum.g += ((col>> 8) & 0xFF) *mul2;
			sum.b += ((col>>16) & 0xFF) *mul2;
			sum.a += ((col>>24)       ) *mul1;
		}
	}

protected:
	iTJSDispatch2 *dst;
	real dx, dy, dw, dh;
	long dtx, dty, dtw, dth;
	long dsx, dsy, dex, dey;

	iTJSDispatch2 *src;
	long sx, sy, sw, sh;

	BufRefT ps;
	long siw, sih, spch;

	WrtRefT pd;
	long diw, dih, dpch;

};
NCB_ATTACH_FUNCTION(shrinkCopy, Layer, ShrinkCopy::layerShrinkCopy);


struct LimitedShrink : public LayerUtils
{
	// TJS Method
	static tjs_error (TJS_INTF_METHOD layerShrinkCopy)(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *dst)
	{
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		LimitedShrink inst( dst, param[0]->AsObjectNoAddRef(), (long)param[1]->AsInteger(),
							(numparams >= 3) ?                 (long)param[2]->AsInteger() : 0);
		if (!inst.check())  return TJS_E_INVALIDPARAM;
		if (!inst.resize()) return TJS_E_FAIL;
		inst.copy();
		return TJS_S_OK;
	}

	LimitedShrink(iTJSDispatch2 *_dst, iTJSDispatch2 *_src, long _stepx, long _stepy)
		: dst(_dst), src(_src), stepx(_stepx), stepy(_stepy), xdiv(0), xrem(false)
	{
		if (!stepy) stepy = stepx;
	}

	bool check()
	{
		return (stepx > 0 && stepy > 0 &&
				GetLayerBufferAndSize(src, siw, sih, ps, spch) &&
				IsValidLayer(dst));
	}
	bool resize() {
		tTJSVariant nw((tjs_int)((siw + stepx - 1) / stepx));
		tTJSVariant nh((tjs_int)((sih + stepy - 1) / stepy));
		tTJSVariant *param[] = { &nw, &nh };
		return (TJS_SUCCEEDED(dst->FuncCall(0, TJS_W("setImageSize"), 0, NULL, 2, param, dst)) &&
				GetLayerBufferAndSize(dst, diw, dih, pd, dpch));
	}
	void copy() {
		xdiv = siw/stepx;
		xrem = siw - xdiv*stepx;
		if (stepy <= 1) {
			for (long y = 0; y < sih; y++, pd+=dpch, ps+=spch)
				shrinkLineX(pd, ps);
		} else {
			long bpch = diw * 4;
			WrtRefT buf = new UnitT[bpch * stepy];
			try {
				long div = sih/stepy;
				for (long len = div; len > 0; len--, pd+=dpch) {
					for (long sub = stepy, ofs = 0; sub > 0; sub--, ofs+=bpch, ps+=spch)
						shrinkLineX(buf + ofs, ps);
					shrinkLineY(pd, buf, bpch, stepy);
				}
				div *= stepy;
				if (div < sih) {
					long yrem = sih - div;
					for (long sub = yrem, ofs = 0; sub > 0; sub--, ofs+=bpch, ps+=spch)
						shrinkLineX(buf + ofs, ps);
					shrinkLineY(pd, buf, bpch, yrem);
				}
			} catch(...) {
				delete[] buf;
				throw;
			}
			delete[] buf;
		}
	}
	static inline void ShrinkLine(WrtRefT &w, BufRefT &r, long len, long shrink, long step, long tstep) {
		BufRefT tr = r;
		switch (shrink) {
		case 1:
			for (; len > 0; len--, r+=step) {
				*w++ = r[0];
				*w++ = r[1];
				*w++ = r[2];
				*w++ = 255;
			}
			break;
		case 2:
		case 3:
		case 4:
			/* 専用処理を書く */
		default:
			for (PixelT sr, sg, sb; len > 0; len--, r+=step) {
				sr = sg = sb = 0;
				tr = r;
				for (long sub = shrink; sub > 0; sub--, tr+=tstep) {
					sr += (PixelT)tr[0];
					sg += (PixelT)tr[1];
					sb += (PixelT)tr[2];
				}
				*w++ = (UnitT)(sr/shrink);
				*w++ = (UnitT)(sg/shrink);
				*w++ = (UnitT)(sb/shrink);
				*w++ = 255;
			}
		}
	}
	
	void shrinkLineX(WrtRefT w, BufRefT r) {
		ShrinkLine(w, r, xdiv, stepx, stepx*4, 4);
		if (xrem > 0) ShrinkLine(w, r, 1, xrem, 0, 4);
	}
	inline void shrinkLineY(WrtRefT w, BufRefT r, long pch, long shrink) {
		ShrinkLine(w, r, diw, shrink, 4, pch);
	}
protected:
	iTJSDispatch2 *dst;
	iTJSDispatch2 *src;
	long stepx, stepy, xdiv, xrem;

	BufRefT ps;
	long siw, sih, spch;

	WrtRefT pd;
	long diw, dih, dpch;
};
NCB_ATTACH_FUNCTION(shrinkCopyFast, Layer, LimitedShrink::layerShrinkCopy);
