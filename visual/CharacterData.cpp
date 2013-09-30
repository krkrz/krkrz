
#include "CharacterData.h"
#include "tvpgl.h"
#include "MsgIntf.h"
//---------------------------------------------------------------------------
void TVPChBlurMulCopy_c(tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level);
void TVPChBlurAddMulCopy_c(tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level);
void TVPChBlurCopy_c(tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel);
//---------------------------------------------------------------------------
tTVPCharacterData::tTVPCharacterData( const tjs_uint8 * indata,
	tjs_int inpitch,
	tjs_int originx, tjs_int originy,
	tjs_uint blackboxw, tjs_uint blackboxh,
	const tGlyphMetrics & metrics, bool fullcolor )
: Antialiased(false), Blured(false)
{

	// フィールドのクリア
	RefCount = 1; // 参照カウンタの初期値は 1
	Data = NULL;

	// Metrics やビットマップ情報のコピー
	Metrics = metrics;
	OriginX = originx;
	OriginY = originy;
	BlackBoxX = blackboxw;
	BlackBoxY = blackboxh;
	Gray = 65;
	FullColored = fullcolor;

	// サイズのチェック
	if( BlackBoxX != 0 && BlackBoxY != 0 ) {
		try {
			// ビットマップをコピー
			if( fullcolor ) {
				//- 横方向のピッチを計算
				// MMX 等の使用を考えて横方向は 8 バイトでアライン
				Pitch = (((blackboxw*4 - 1) >> 3) + 1) << 3;

				//- バイト数を計算してメモリを確保
				Data = new tjs_uint8 [Pitch * blackboxh];

				//- ビットマップをコピー
				inpitch *= 4;
				for(tjs_uint y = 0; y < blackboxh; y++) {
					memcpy( Data + Pitch * y, indata + inpitch * y, blackboxw*4);
				}
			} else {
				//- 横方向のピッチを計算
				// MMX 等の使用を考えて横方向は 8 バイトでアライン
				Pitch = (((blackboxw - 1) >> 3) + 1) << 3;

				//- バイト数を計算してメモリを確保
				Data = new tjs_uint8 [Pitch * blackboxh];

				//- ビットマップをコピー
				for(tjs_uint y = 0; y < blackboxh; y++) {
					memcpy( Data + Pitch * y, indata + inpitch * y, blackboxw);
				}
			}
		} catch(...) {
			if(Data) delete [] Data;
			throw;
		}
	}
}

//---------------------------------------------------------------------------
/**
 * コピーコンストラクタ
 * @param ref	参照オブジェクト
 */
tTVPCharacterData::tTVPCharacterData(const tTVPCharacterData & ref) {
	// コピーコンストラクタは未サポート
	TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::tTVPCharacterData(const tTVPCharacterData & ref)") );
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Expand()
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Expand for FullColored") );

	// expand the bitmap stored in 1bpp, to 8bpp
	tjs_int newpitch = (((BlackBoxX -1)>>2)+1)<<2;
	tjs_uint8 *nd;
	tjs_uint8 *newdata = nd = new tjs_uint8[newpitch * BlackBoxY];
	tjs_int h = BlackBoxY;
	tjs_uint8 *d = Data;

	tjs_int w = BlackBoxX;
	static tjs_uint32 pal[2] = {0, 64};
	while(h--)
	{
		TVPBLExpand1BitTo8BitPal(nd, d, w, pal);
		nd += newpitch, d += Pitch;
	}
	if(Data) delete [] Data;
	Data = newdata;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Blur(tjs_int blurlevel, tjs_int blurwidth)
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Blur for FullColored") );

	// blur the bitmap with given parameters
	// blur the bitmap
	if(!Data) return;
	if(blurlevel == 255 && blurwidth == 0) return; // no need to blur
	if(blurwidth == 0)
	{
		// no need to blur but must be transparent
		if( Gray == 256 )
			TVPChBlurMulCopy_c(Data, Data, Pitch*BlackBoxY, BlurLevel<<10);
		else
			TVPChBlurMulCopy65(Data, Data, Pitch*BlackBoxY, BlurLevel<<10);
		return;
	}

	// simple blur ( need to optimize )
	tjs_int bw = std::abs(blurwidth);
	tjs_int newwidth = BlackBoxX + bw*2;
	tjs_int newheight = BlackBoxY + bw*2;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;

	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	if( Gray == 256 )
		TVPChBlurCopy_c(newdata, newpitch, newwidth, newheight, Data, Pitch, BlackBoxX,
			BlackBoxY, bw, blurlevel);
	else
		TVPChBlurCopy65(newdata, newpitch, newwidth, newheight, Data, Pitch, BlackBoxX,
			BlackBoxY, bw, blurlevel);

	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	Pitch = newpitch;
	OriginX -= blurwidth;
	OriginY -= blurwidth;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Blur()
{
	// blur the bitmap
	Blur(BlurLevel, BlurWidth);
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Bold(tjs_int size)
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Bold for FullColored") );

	// enbold the bitmap for 65-level grayscale bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// apply bold
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		for(tjs_int i = 0; i<level; i++) destp[i] = srcp[i];
		destp[0] = srcp[0];
		for(tjs_int x = level; x<newwidth-level; x++)
		{
			tjs_uint largest = srcp[x];
			for(tjs_int xx = x-level; xx<x; xx++)
				if((tjs_uint)srcp[xx] > largest) largest = srcp[xx];
			destp[x] = largest;
		}
		for(tjs_int i = 0; i<level; i++) destp[newwidth-i-1] = srcp[BlackBoxX-1-i];

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Bold2(tjs_int size)
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Bold2 for FullColored") );

	// enbold the bitmap for black/white monochrome bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  (((newwidth -1)>>5)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// apply bold
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		memcpy(destp, srcp, Pitch);
		if(newpitch > Pitch) destp[Pitch] = 0;

		for(tjs_int i = 1; i<=level; i++)
		{
			tjs_uint8 bollow = 0;
			tjs_int bl = 8 - i;
			for(tjs_int x = 0; x < Pitch; x++)
			{
				destp[x] |= (srcp[x] >> i) + bollow;
				bollow = srcp[x] << bl;
			}
			if(newpitch > Pitch) destp[Pitch] |= bollow;
		}

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Resample4()
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Resample4 for FullColored") );

	// down-sampling 4x4

	static tjs_uint16 bitcounter[256] = {0xffff};
	if(bitcounter[0] == 0xffff)
	{
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++)
		{
			tjs_uint16 v;
			tjs_int n;
			n = i & 0x0f;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v = (n<<2);
			n = i >> 4;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v |= ((n<<2)) << 8;
			bitcounter[i] = v;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>2)+1;
	tjs_int newheight = ((BlackBoxY-1)>>2)+1;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// resampling
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &=
			((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*4;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 4 ? 4 : rem;

		tjs_uint8 *dp = destp;
		tjs_int lim = (newwidth+1) >> 1;
		for(tjs_int i = 0; i<lim; i++)
		{
			tjs_uint32 n = 0;
			tjs_uint8 *sp = srcp + i;
			switch(rem)
			{
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			dp[0] = n >> 8;
			dp[1] = n & 0xff;
			dp += 2;
		}

		srcp += Pitch * 4;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /4;
	OriginY = OriginY /4;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Resample8()
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::Resample8 for FullColored") );

	// down-sampling 8x8

	static tjs_uint8 bitcounter[256] = {0xff};
	if(bitcounter[0] == 0xff)
	{
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++)
		{
			tjs_int n;
			n = (i & 0x55) + ((i & 0xaa)>>1);
			n = (n & 0x33) + ((n & 0xcc)>>2);
			n = (n & 0x0f) + ((n & 0xf0)>>4);
			bitcounter[i] = (tjs_uint8)n;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>3)+1;
	tjs_int newheight = ((BlackBoxY-1)>>3)+1;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// resampling
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0;;)
	{
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &=
			((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*8;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 8 ? 8 : rem;

		for(tjs_int x = 0; x<newwidth; x++)
		{
			tjs_uint n = 0;
			tjs_uint8 *sp = srcp + x;
			switch(rem)
			{
			case 8:	n += bitcounter[*sp]; sp += Pitch;
			case 7:	n += bitcounter[*sp]; sp += Pitch;
			case 6:	n += bitcounter[*sp]; sp += Pitch;
			case 5:	n += bitcounter[*sp]; sp += Pitch;
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			destp[x] = n;
		}

		y++;
		if(y >= newheight) break;
		srcp += Pitch * 8;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /8;
	OriginY = OriginY /8;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::AddHorizontalLine( tjs_int liney, tjs_int thickness, tjs_uint8 val ) {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: tTVPCharacterData::AddHorizontalLine for FullColored") );

	tjs_int linetop = liney - thickness/2;
	if( linetop < 0 ) linetop = 0;
	tjs_int linebottom = linetop + thickness;

	tjs_int newwidth = BlackBoxX;
	tjs_int newheight = BlackBoxY;
	if( BlackBoxX != Metrics.CellIncX ) {
		newwidth = Metrics.CellIncX;
	}
	int top = OriginY;
	int bottom = top + BlackBoxY;
	if( linetop < top ) { // 上過ぎる
		top = linetop;
	} else if( linebottom >= bottom ) { // 下過ぎる
		bottom = linebottom;
	}
	newheight = bottom - top;
	// 大きさが変化する時は作り直す
	if( newwidth != BlackBoxX || newheight != BlackBoxY ) {
		tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
		tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];
		memset( newdata, 0, sizeof(tjs_uint8)*newpitch*newheight );
		// x は OriginX 分ずれる
		// y は OriginY - top分ずれる
		tjs_int offsetx = OriginX;
		tjs_int offsety = OriginY - top;
		tjs_uint8 *sp = Data;
		tjs_uint8 *dp = newdata + offsety*newpitch + offsetx;
		for( tjs_uint y = 0; y < BlackBoxY; y++ ) {
			for( tjs_uint x = 0; x < BlackBoxX; x++ ) {
				dp[x] = sp[x];
			}
			sp += Pitch;
			dp += newpitch;
		}
		delete [] Data;
		Data = newdata;
		BlackBoxX = newwidth;
		BlackBoxY = newheight;
		OriginX = 0;
		OriginY = top;
		Pitch = newpitch;
	}
	tjs_int end = linetop-OriginY+thickness;
	tjs_uint8 *dp = Data + (linetop-OriginY)*Pitch;
	for( tjs_int y = 0; y < thickness; y++ ) {
		for( tjs_uint x = 0; x < BlackBoxX; x++ ) {
			dp[x] = val;
		}
		dp += Pitch;
	}
}
//---------------------------------------------------------------------------
void TVPChBlurMulCopy_c(tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level)
{
	tjs_int a, b;
	{
		int ___index = 0;
		len -= (4-1);

		while(___index < len)
		{
			a = (src[(___index+(0*2))] * level >> 18);
			b = (src[(___index+(0*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(0*2))] = a;
			dest[(___index+(0*2+1))] = b;
			a = (src[(___index+(1*2))] * level >> 18);
			b = (src[(___index+(1*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(1*2))] = a;
			dest[(___index+(1*2+1))] = b;
			___index += 4;
		}

		len += (4-1);

		while(___index < len)
		{
			a = (src[___index] * level >> 18);;
			if(a>=255) a = 255;
			dest[___index] = a;;
			___index ++;
		}
	}
}
void TVPChBlurAddMulCopy_c(tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level)
{
	tjs_int a, b;
	{
		int ___index = 0;
		len -= (4-1);

		while(___index < len)
		{
			a = dest[(___index+(0*2))] +(src[(___index+(0*2))] * level >> 18);
			b = dest[(___index+(0*2+1))] +(src[(___index+(0*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(0*2))] = a;
			dest[(___index+(0*2+1))] = b;
			a = dest[(___index+(1*2))] +(src[(___index+(1*2))] * level >> 18);
			b = dest[(___index+(1*2+1))] +(src[(___index+(1*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(1*2))] = a;
			dest[(___index+(1*2+1))] = b;
			___index += 4;
		}

		len += (4-1);

		while(___index < len)
		{
			a = dest[___index] +(src[___index] * level >> 18);;
			if(a>=255) a = 255;;
			dest[___index] = a;;
			___index ++;
		}
	}
}
extern "C" tjs_uint fast_int_hypot(tjs_int lx, tjs_int ly);
void TVPChBlurCopy_c(tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel)
{
	tjs_int lvsum, x, y;

	/* clear destination */
	memset(dest, 0, destpitch*destheight);

	/* compute filter level */
	lvsum = 0;
	for(y = -blurwidth; y <= blurwidth; y++)
	{
		for(x = -blurwidth; x <= blurwidth; x++)
		{
			tjs_int len = fast_int_hypot(x, y);
			if(len <= blurwidth)
				lvsum += (blurwidth - len +1);
		}
	}

	if(lvsum) lvsum = (1<<18)/lvsum; else lvsum=(1<<18);

	/* apply */
	for(y = -blurwidth; y <= blurwidth; y++)
	{
		for(x = -blurwidth; x <= blurwidth; x++)
		{
			tjs_int len = fast_int_hypot(x, y);
			if(len <= blurwidth)
			{
				tjs_int sy;

				len = blurwidth - len +1;
				len *= lvsum;
				len *= blurlevel;
				len >>= 8;
				for(sy = 0; sy < srcheight; sy++)
				{
					TVPChBlurAddMulCopy_c(dest + (y + sy + blurwidth)*destpitch + x + blurwidth, 
						src + sy * srcpitch, srcwidth, len);
				}
			}
		}
	}
}
