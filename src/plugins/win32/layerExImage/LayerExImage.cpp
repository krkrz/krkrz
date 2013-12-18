
/** original code header */
/*
 * 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 5.99c 17/Oct/2004
 */

// xImaDsp.cpp : DSP functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include <windows.h>
#include <math.h>
#include "LayerExImage.h"

void
layerExImage::reset()
{
	layerExBase::reset();
	// バッファ位置をクリッピングにあわせて変更する
	_buffer += _clipTop * _pitch + _clipLeft * 4;
	_width  = _clipWidth;
	_height = _clipHeight;
}

void
layerExImage::lut(BYTE* pLut)
{
	BYTE *src = (BYTE*)_buffer;
	for (int i=0; i < _height ; i++){
		BYTE *p = src;
		for (int j=0; j< _width; j++) {
			*p++ = pLut[*p]; // B
			*p++ = pLut[*p]; // G
			*p++ = pLut[*p]; // R
			p++;             // A
		}
		src += _pitch;
	}
}

/**
 * 明度とコントラスト
 * @param brightness 明度 -255 ～ 255, 負数の場合は暗くなる
 * @param contrast コントラスト -100 ～100, 0 の場合変化しない
 */
void
layerExImage::light(int brightness, int contrast)
{
	float c = (100 + contrast)/100.0f;
	brightness +=128;
	BYTE cTable[256];
	for (int i=0;i<256;i++)	{
		cTable[i] = (BYTE)max(0,min(255,(int)((i-128)*c + brightness)));
	}
	lut(cTable);
	redraw();
}


////////////////////////////////////////////////////////////////////////////////
#define  HSLMAX   255	/* H,L, and S vary over 0-HSLMAX */
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */
                        /* HSLMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HSLMAX must each fit in a BYTE. */
/* Hue is undefined if Saturation is 0 (grey-scale) */
/* This value determines where the Hue scrollbar is */
/* initially set for achromatic colors */
#define HSLUNDEFINED (HSLMAX*2/3)
////////////////////////////////////////////////////////////////////////////////

static RGBQUAD
RGBtoHSL(RGBQUAD lRGBColor)
{
	BYTE R,G,B;					/* input RGB values */
	BYTE H,L,S;					/* output HSL values */
	BYTE cMax,cMin;				/* max and min RGB values */
	WORD Rdelta,Gdelta,Bdelta;	/* intermediate value: % of spread from max*/

	R = lRGBColor.rgbRed;	/* get R, G, and B out of DWORD */
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

	cMax = max( max(R,G), B);	/* calculate lightness */
	cMin = min( min(R,G), B);
	L = (BYTE)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	if (cMax==cMin){			/* r=g=b --> achromatic case */
		S = 0;					/* saturation */
		H = HSLUNDEFINED;		/* hue */
	} else {					/* chromatic case */
		if (L <= (HSLMAX/2))	/* saturation */
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
		/* hue */
		Rdelta = (WORD)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (WORD)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (WORD)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (BYTE)(Bdelta - Gdelta);
		else if (G == cMax)
			H = (BYTE)((HSLMAX/3) + Rdelta - Bdelta);
		else /* B == cMax */
			H = (BYTE)(((2*HSLMAX)/3) + Gdelta - Rdelta);

//		if (H < 0) H += HSLMAX;     //always false
		if (H > HSLMAX) H -= HSLMAX;
	}
	RGBQUAD hsl={L,S,H,0};
	return hsl;
}

static float
HueToRGB(float n1,float n2, float hue)
{
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float rValue;

	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		rValue = n1 + (n2-n1)*hue/60.0f;
	else if (hue < 180)
		rValue = n2;
	else if (hue < 240)
		rValue = n1+(n2-n1)*(240-hue)/60;
	else
		rValue = n1;

	return rValue;
}

static RGBQUAD
HSLtoRGB(RGBQUAD lHSLColor)
{ 
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float h,s,l;
	float m1,m2;
	BYTE r,g,b;

	h = (float)lHSLColor.rgbRed * 360.0f/255.0f;
	s = (float)lHSLColor.rgbGreen/255.0f;
	l = (float)lHSLColor.rgbBlue/255.0f;

	if (l <= 0.5)	m2 = l * (1+s);
	else			m2 = l + s - l*s;

	m1 = 2 * l - m2;

	if (s == 0) {
		r=g=b=(BYTE)(l*255.0f);
	} else {
		r = (BYTE)(HueToRGB(m1,m2,h+120) * 255.0f);
		g = (BYTE)(HueToRGB(m1,m2,h) * 255.0f);
		b = (BYTE)(HueToRGB(m1,m2,h-120) * 255.0f);
	}

	RGBQUAD rgb = {b,g,r,0};
	return rgb;
}

/**
 * 色相と彩度
 * @param hue 色相
 * @param sat 彩度
 * @param blend ブレンド 0 (効果なし) ～ 1 (full effect)
 */
void
layerExImage::colorize(int hue, int sat, double blend)
{
	if (blend < 0.0f) blend = 0.0f;
	if (blend > 1.0f) blend = 1.0f;
	int a0 = (int)(256*blend);
	int a1 = 256 - a0;

	bool bFullBlend = false;
	if (blend > 0.999f)	bFullBlend = true;
	
	RGBQUAD color,hsl;

	BYTE *src = (BYTE*)_buffer;
	for (int y=0; y<_height; y++){
 		BYTE *p = src;
		for (int x=0; x<_width; x++){
			color.rgbBlue = p[0];
			color.rgbGreen= p[1];
			color.rgbRed  = p[2];
			if (bFullBlend){
				color = RGBtoHSL(color);
				color.rgbRed=hue;
				color.rgbGreen=sat;
				color = HSLtoRGB(color);
			} else {
				hsl = RGBtoHSL(color);
				hsl.rgbRed=hue;
				hsl.rgbGreen=sat;
				hsl = HSLtoRGB(hsl);
				//BlendPixelColor(x,y,hsl,blend);
				//color.rgbRed = (BYTE)(hsl.rgbRed * blend + color.rgbRed * (1.0f - blend));
				//color.rgbBlue = (BYTE)(hsl.rgbBlue * blend + color.rgbBlue * (1.0f - blend));
				//color.rgbGreen = (BYTE)(hsl.rgbGreen * blend + color.rgbGreen * (1.0f - blend));
				color.rgbRed = (BYTE)((hsl.rgbRed * a0 + color.rgbRed * a1)>>8);
				color.rgbBlue = (BYTE)((hsl.rgbBlue * a0 + color.rgbBlue * a1)>>8);
				color.rgbGreen = (BYTE)((hsl.rgbGreen * a0 + color.rgbGreen * a1)>>8);
			}
			*p++ = color.rgbBlue;  // B
			*p++ = color.rgbGreen; // G
			*p++ = color.rgbRed;   // R
			p++;                   // A
		}
		src += _pitch;
	}
	redraw();
}

static int
hue2rgb(double n1,double n2, double hue)
{
	double color;
	if (hue < 0) { hue += 1.0; }
	else if (hue > 1.0) {	hue -= 1.0; };
	if (hue < 1.0/6.0)
		color = n1 + (n2-n1)*hue*6.0;
	else if (hue < 1.0/2.0)
		color = n2;
	else if (hue < 2.0/3.0)
		color = n1+(n2-n1)*(2.0/3.0-hue)*6.0;
	else
		color = n1;
	return (int)(color * 255.0);
}

static void
modulate(int &b, int &g, int &r, double h, double s, double l)
{
	// RGB正規化
	double red   = r / 255.0;
	double green = g / 255.0;
	double blue  = b / 255.0;

	// RGBからHSLに変換
	double cMax = max(max(red,green), blue);
	double cMin = min(min(red,green), blue);
	double delta = cMax - cMin;
	double add   = cMax + cMin;
	double luminance = add/2.0;
	double hue;
	double saturation;
	if (delta == 0) {
		saturation = 0;
		hue = 0;
	} else {
		if (luminance < 0.5) {
			saturation = delta/add;
		} else {
			saturation = delta/(2.0-add);
		}
		if (red == cMax) {
			hue = (green - blue)/delta;
		} else if (green == cMax) {
			hue = 2.0 + (blue - red)/delta;
		} else {
			hue = 4.0 + (red - green)/delta;
		}
		hue /= 6.0;
	}
	// 色変換処理
	// %処理はこれでいいんだろうか…
	hue += h;
	while (hue < 0) { hue += 1.0; };
	while (hue > 1.0) { hue -= 1.0; };
	if (s > 0) {
		saturation += (1.0 - saturation) * s;
	} else {
		saturation += saturation * s;
	}
	if (l > 0) {
		luminance += (1.0 - luminance) * l;
	} else {
		luminance += luminance * l;
	}

	// HSLからRGBに戻す
	if (saturation == 0.0) {
		r = g = b = (int)(luminance * 255.0);
	} else {
		double m2;
		if (luminance <= 0.5f) {
			m2 = luminance * (1+saturation);
		} else {
			m2 = luminance + saturation - luminance * saturation;
		}
		double m1 = 2.0 * luminance - m2;
		r = hue2rgb(m1,m2,hue+1.0/3.0);
		g = hue2rgb(m1,m2,hue);
		b = hue2rgb(m1,m2,hue-1.0/3.0);
	}
}


/**
 * 色相と彩度と輝度調整
 * @param hue 色相 -180～180 (度)
 * @param saturation 彩度 -100～100 (%)
 * @param luminance 輝度 -100～100 (%)
 */
void
layerExImage::modulate(int hue, int saturation, int luminance)
{
	double h = hue / 360.0f;
	double s = saturation / 100.0f;
	double l = luminance / 100.0f;

	BYTE *src = (BYTE*)_buffer;
	for (int y=0; y<_height; y++){
		BYTE *p = src;
		for (int x=0; x<_width; x++){
			int b = p[0];
			int g = p[1];
			int r = p[2];
			::modulate(b,g,r,h,s,l);
			*p++ = b;
			*p++ = g;
			*p++ = r;
			p++;
		}
		src += _pitch;
	}
	redraw();
}

/**
 * ノイズ追加
 * @param level ノイズレベル 0 (no noise) ～ 255 (lot of noise).
 */
void
layerExImage::noise(int level)
{
	BYTE *src = (BYTE*)_buffer;
	for (int y=0; y<_height; y++){
		BYTE *p = src;
		for (int x=0; x<_width; x++){
			int n = (int)((rand()/(float)RAND_MAX - 0.5)*level);
			*p++ = (BYTE)max(0,min(255,(int)(*p + n)));
			n = (int)((rand()/(float)RAND_MAX - 0.5)*level);
			*p++ = (BYTE)max(0,min(255,(int)(*p + n)));
			n = (int)((rand()/(float)RAND_MAX - 0.5)*level);
			*p++ = (BYTE)max(0,min(255,(int)(*p + n)));
			p++;
		}
		src += _pitch;
	}
	redraw();
}

/**
 * ノイズ生成（元の画像を無視してグレースケールのホワイトノイズを描画／α情報は維持）
 */
void
layerExImage::generateWhiteNoise()
{
	BYTE *src = (BYTE*)_buffer;
	for (int y=0; y<_height; y++){
		BYTE *p = src;
		for (int x=0; x<_width; x++,p+=4){
			BYTE n = (BYTE)(rand()/(RAND_MAX/255));
			p[2] = p[1] = p[0] = n;
		}
		src += _pitch;
	}
	redraw();
}


typedef _int32 int32_t;
typedef unsigned char uint8_t;

////////////////////////////////////////////////////////////////////////////////
/** 
 * generates a 1-D convolution matrix to be used for each pass of 
 * a two-pass gaussian blur.  Returns the length of the matrix.
 * \author [nipper]
 */
static int32_t
gen_convolve_matrix (float radius, float **cmatrix_p)
{
	int32_t matrix_length;
	int32_t matrix_midpoint;
	float* cmatrix;
	int32_t i,j;
	float std_dev;
	float sum;
	
	/* we want to generate a matrix that goes out a certain radius
	* from the center, so we have to go out ceil(rad-0.5) pixels,
	* inlcuding the center pixel.  Of course, that's only in one direction,
	* so we have to go the same amount in the other direction, but not count
	* the center pixel again.  So we double the previous result and subtract
	* one.
	* The radius parameter that is passed to this function is used as
	* the standard deviation, and the radius of effect is the
	* standard deviation * 2.  It's a little confusing.
	* <DP> modified scaling, so that matrix_lenght = 1+2*radius parameter
	*/
	radius = (float)fabs(0.5*radius) + 0.25f;
	
	std_dev = radius;
	radius = std_dev * 2;
	
	/* go out 'radius' in each direction */
	matrix_length = int32_t (2 * ceil(radius-0.5) + 1);
	if (matrix_length <= 0) matrix_length = 1;
	matrix_midpoint = matrix_length/2 + 1;
	*cmatrix_p = new float[matrix_length];
	cmatrix = *cmatrix_p;
	
	/*  Now we fill the matrix by doing a numeric integration approximation
	* from -2*std_dev to 2*std_dev, sampling 50 points per pixel.
	* We do the bottom half, mirror it to the top half, then compute the
	* center point.  Otherwise asymmetric quantization errors will occur.
	*  The formula to integrate is e^-(x^2/2s^2).
	*/
	
	/* first we do the top (right) half of matrix */
	for (i = matrix_length/2 + 1; i < matrix_length; i++)
    {
		float base_x = i - (float)floor((float)(matrix_length/2)) - 0.5f;
		sum = 0;
		for (j = 1; j <= 50; j++)
		{
			if ( base_x+0.02*j <= radius ) 
				sum += (float)exp (-(base_x+0.02*j)*(base_x+0.02*j) / 
				(2*std_dev*std_dev));
		}
		cmatrix[i] = sum/50;
    }
	
	/* mirror the thing to the bottom half */
	for (i=0; i<=matrix_length/2; i++) {
		cmatrix[i] = cmatrix[matrix_length-1-i];
	}
	
	/* find center val -- calculate an odd number of quanta to make it symmetric,
	* even if the center point is weighted slightly higher than others. */
	sum = 0;
	for (j=0; j<=50; j++)
    {
		sum += (float)exp (-(0.5+0.02*j)*(0.5+0.02*j) /
			(2*std_dev*std_dev));
    }
	cmatrix[matrix_length/2] = sum/51;
	
	/* normalize the distribution by scaling the total sum to one */
	sum=0;
	for (i=0; i<matrix_length; i++) sum += cmatrix[i];
	for (i=0; i<matrix_length; i++) cmatrix[i] = cmatrix[i] / sum;
	
	return matrix_length;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * generates a lookup table for every possible product of 0-255 and
 * each value in the convolution matrix.  The returned array is
 * indexed first by matrix position, then by input multiplicand (?)
 * value.
 * \author [nipper]
 */
static float*
gen_lookup_table (float *cmatrix, int32_t cmatrix_length)
{
	float* lookup_table = new float[cmatrix_length * 256];
	float* lookup_table_p = lookup_table;
	float* cmatrix_p      = cmatrix;
	
	for (int32_t i=0; i<cmatrix_length; i++)
    {
		for (int32_t j=0; j<256; j++)
		{
			*(lookup_table_p++) = *cmatrix_p * (float)j;
		}
		cmatrix_p++;
    }
	
	return lookup_table;
}

/**
 * this function is written as if it is blurring a column at a time,
 * even though it can operate on rows, too.  There is no difference
 * in the processing of the lines, at least to the blur_line function.
 * \author [nipper]
 */
static void
blur_line (float *ctable, float *cmatrix, int32_t cmatrix_length, uint8_t* cur_col, uint8_t* dest_col, int32_t y, int32_t bytes)
{
	float scale;
	float sum;
	int32_t i=0, j=0;
	int32_t row;
	int32_t cmatrix_middle = cmatrix_length/2;
	
	float *cmatrix_p;
	uint8_t  *cur_col_p;
	uint8_t  *cur_col_p1;
	uint8_t  *dest_col_p;
	float *ctable_p;
	
	/* this first block is the same as the non-optimized version --
	* it is only used for very small pictures, so speed isn't a
	* big concern.
	*/
	if (cmatrix_length > y)
    {
		for (row = 0; row < y ; row++)
		{
			scale=0;
			/* find the scale factor */
			for (j = 0; j < y ; j++)
			{
				/* if the index is in bounds, add it to the scale counter */
				if ((j + cmatrix_middle - row >= 0) &&
					(j + cmatrix_middle - row < cmatrix_length))
					scale += cmatrix[j + cmatrix_middle - row];
			}
			for (i = 0; i<bytes; i++)
			{
				sum = 0;
				for (j = 0; j < y; j++)
				{
					if ((j >= row - cmatrix_middle) &&
						(j <= row + cmatrix_middle))
						sum += cur_col[j*bytes + i] * cmatrix[j];
				}
				dest_col[row*bytes + i] = (uint8_t)(0.5f + sum / scale);
			}
		}
    }
	else
    {
		/* for the edge condition, we only use available info and scale to one */
		for (row = 0; row < cmatrix_middle; row++)
		{
			/* find scale factor */
			scale=0;
			for (j = cmatrix_middle - row; j<cmatrix_length; j++)
				scale += cmatrix[j];
			for (i = 0; i<bytes; i++)
			{
				sum = 0;
				for (j = cmatrix_middle - row; j<cmatrix_length; j++)
				{
					sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
				}
				dest_col[row*bytes + i] = (uint8_t)(0.5f + sum / scale);
			}
		}
		/* go through each pixel in each col */
		dest_col_p = dest_col + row*bytes;
		for (; row < y-cmatrix_middle; row++)
		{
			cur_col_p = (row - cmatrix_middle) * bytes + cur_col;
			for (i = 0; i<bytes; i++)
			{
				sum = 0;
				cmatrix_p = cmatrix;
				cur_col_p1 = cur_col_p;
				ctable_p = ctable;
				for (j = cmatrix_length; j>0; j--)
				{
					sum += *(ctable_p + *cur_col_p1);
					cur_col_p1 += bytes;
					ctable_p += 256;
				}
				cur_col_p++;
				*(dest_col_p++) = (uint8_t)(0.5f + sum);
			}
		}
		
		/* for the edge condition , we only use available info, and scale to one */
		for (; row < y; row++)
		{
			/* find scale factor */
			scale=0;
			for (j = 0; j< y-row + cmatrix_middle; j++)
				scale += cmatrix[j];
			for (i = 0; i<bytes; i++)
			{
				sum = 0;
				for (j = 0; j<y-row + cmatrix_middle; j++)
				{
					sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
				}
				dest_col[row*bytes + i] = (uint8_t) (0.5f + sum / scale);
			}
		}
    }
}

// src -> dest にカラム情報を取得
static void
getCol(BYTE *src, BYTE *dest, int height, int pitch)
{
	pitch -= 4;
	for (int i=0;i<height;i++) {
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		src += pitch;
	}
}

// dest -> srct にカラム情報を復帰
static void
setCol(BYTE *src, BYTE *dest, int height, int pitch)
{
	pitch -= 4;
	for (int i=0;i<height;i++) {
		*src++ = *dest++;
		*src++ = *dest++;
		*src++ = *dest++;
		*src++ = *dest++;
		src += pitch;
	}
}

void
layerExImage::gaussianBlur(float radius /*= 1.0f*/)
{
	int tmppitch = _width * 4;
	BYTE *tmpbuf = new BYTE[tmppitch * _height];
	
	// generate convolution matrix and make sure it's smaller than each dimension
	float *cmatrix = NULL;
	int32_t cmatrix_length = gen_convolve_matrix(radius, &cmatrix);
	// generate lookup table
	float *ctable = gen_lookup_table(cmatrix, cmatrix_length);
	
	int32_t x,y;
	int32_t bypp = 4;
	
	// blur the rows
    for (y=0;y<_height;y++)
	{
		blur_line(ctable, cmatrix, cmatrix_length, _buffer + _pitch * y, tmpbuf + tmppitch * y, _width, bypp);
	}

	// blur the cols
	BYTE* cur_col  = new BYTE[_height*4];
	BYTE* dest_col = new BYTE[_height*4];
	for (x=0;x<_width;x++)
	{
		getCol(tmpbuf+x*4, cur_col, _height, tmppitch);
		//getCol(_buffer+x*4, dest_col, _height, _pitch);
		blur_line(ctable, cmatrix, cmatrix_length, cur_col, dest_col, _height, bypp);
		setCol(_buffer+x*4, dest_col, _height, _pitch);
	}
	delete[] cur_col;
	delete[] dest_col;

	delete[] cmatrix;
	delete[] ctable;
	delete[] tmpbuf;
	redraw();
}
