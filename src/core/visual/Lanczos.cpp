/**
 * Lanczos ����������ɂ�����Q�l�ɂ����T�C�g
 * 
 * Java �ŏ�����Ă��邪�A�\�[�X�R�[�h��������Ă���̂ł킩��₷���B
 * �������A�k�����̏����͐������Ȃ��悤�ɓǂ߂�̂ł��̂܂܂͎g���Ȃ��B
 * HexeRein / �摜���� / Lanczos�i�����c�H�V���j��Ԗ@
 * http://www7a.biglobe.ne.jp/~fairytale/article/program/graphics.html#lanczos
 *
 * �e�[�u�����ASIMD���A�c�������������s���Ă��đ��x�ł����ł��邪�A���ʂ��������̂ŁA������x�Q�l�ɂ������x�B
 * AviUtl �v���O�C�� / Lanczos 3-lobed �g��k��
 * http://www.marumo.ne.jp/auf/
 *
 * �e�L�X�g�ɂ����������A�킩��₷����������Ă���̂ŁA�����͂����̐�������ɍs�����B
 * Lanczos�֐��ɂ��摜�̊g��k��
 * http://www.maroon.dti.ne.jp/twist/4C616E637A6F73B4D8BFF4A4CBA4E8A4EBB2E8C1FCA4CEB3C8C2E7BDCCBEAE.html
 *
 *
 * Spline���T�C�Y�̌W�����Ƃ��̓��o�@
 * http://www.geocities.jp/w_bean17/spline.html
 */


#define _USE_MATH_DEFINES

#include "tjsCommHead.h"
#include "LayerBitmapIntf.h"
#include "LayerBitmapImpl.h"

#include <float.h>
#include <math.h>
#include <cmath>
#include <vector>
#include "Lanczos.h"

template<int TTap>
struct LanczosWeight {
	double operator()( double phase ) {
		if( std::abs(phase) < DBL_EPSILON ) return 1.0;
		if( std::abs(phase) >= (double)TTap ) return 0.0;
		return std::sin(M_PI*phase)*std::sin(M_PI*phase/TTap)/(M_PI*M_PI*phase*phase/TTap);
	}
};

/**
 * Spline36 �p�E�F�C�g�֐�
 * ���̌v�Z���ł͂��܂������Ȃ��B
 * �d�ˍ��킹���@���Ԉ���Ă���̂��ȁB
 */
struct Spline36Weight {
	double operator()( double phase ) {
		double x = std::abs(phase);
		if( x <= 1.0 ) {
			return 13.0/11.0*x*x*x - 453.0/209.0*x*x - 3.0/209.0*x + 1.0;
		} else if( x <= 2.0 ) {
			return -6.0/11.0*x*x*x + 612.0/209.0*x*x* - 1038.0/209.0*x + 540.0/209.0;
		} else if( x <= 3.0 ) {
			return 1.0/11.0*x*x*x - 159.0/209.0*x*x + 434.0/209.0*x - 384.0/209.0;
		} else {
			return 0.0;
		}
	}
};

struct AxisParam {
	std::vector<int> start_;	// �J�n�C���f�b�N�X
	std::vector<int> length_;	// �e�v�f����
	std::vector<double> weight_;

	template<typename TWeightFunc>
	void calculateAxis( int srcstart, int srcend, int srclength, int dstlength, double tap, TWeightFunc& func );
};
// srclenght = srcwidth
// dstlength = dstwidth
// srcstart = srcleft;
// srcend = srcright;
template<typename TWeightFunc>
void AxisParam::calculateAxis( int srcstart, int srcend, int srclength, int dstlength, double tap, TWeightFunc& func ) {
	start_.clear();
	start_.reserve( dstlength );
	length_.clear();
	length_.reserve( dstlength );
	if( srclength <= dstlength ) { // �g��
		double rangex = tap;
		int length = dstlength * (int)rangex * 2 + dstlength;
		weight_.reserve( length );
		for( int x = 0; x < dstlength; x++ ) {
			double cx = (x+0.5)*(double)srclength/(double)dstlength + srcstart;
			int left = (int)std::floor(cx-rangex);
			int right = (int)std::floor(cx+rangex);
			if( left < srcstart ) left = srcstart;
			if( right >= srcend ) right = srcend;
			start_.push_back( left );
			int len = 0;
			for( int sx = left; sx < right; sx++ ) {
				double dist = std::abs(sx + 0.5 - cx);
				double weight = func(dist);
				len++;
				weight_.push_back( weight );
			}
			length_.push_back( len );
		}
	} else { // �k��
		double rangex = tap*(double)srclength/(double)dstlength;
		int length = srclength * (int)rangex * 2 + srclength;
		weight_.reserve( length );
		for( int x = 0; x < dstlength; x++ ) {
			double cx = (x+0.5)*(double)srclength/(double)dstlength + srcstart;
			int left = (int)std::floor(cx-rangex);
			int right = (int)std::floor(cx+rangex);
			if( left < srcstart ) left = srcstart;
			if( right >= srcend ) right = srcend;
			start_.push_back( left );
			// �]������W�ł̈ʒu
			double delta = (double)dstlength/(double)srclength;
			double dx = (left+0.5) * delta;
			int len = 0;
			for( int sx = left; sx < right; sx++ ) {
				double dist = std::abs(dx - (x+0.5));
				double weight = func(dist);
				dx += delta;
				len++;
				weight_.push_back( weight );
			}
			length_.push_back( len );
		}
	}
}

static void TVPWeightCopy( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, AxisParam &paramx, AxisParam &paramy ) {
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();
	const double* weighty = &paramy.weight_[0];
	int wiy = 0;
	for( int y = 0; y < dstheight; y++ ) {
		const double* weightx = &paramx.weight_[0];
		tjs_uint32* dstbits = (tjs_uint32*)dest->GetScanLineForWrite( destrect.top+y );
		int wix = 0;
		int wstarty = wiy;
		for( int x = 0; x < dstwidth; x++ ) {
			wiy = wstarty;
			int top = paramy.start_[y];
			int bottom = top + paramy.length_[y];
			int left = paramx.start_[x];
			int right = left + paramx.length_[x];
			double color_element[4] = {0.0,0.0,0.0,0.0};
			double w_total = 0.0;
			int wstartx = wix;
			for( int sy = top; sy < bottom; sy++ ) {
				wix = wstartx;
				const tjs_uint32* srcbits = (const tjs_uint32*)src->GetScanLine(sy);
				for( int sx = left; sx < right; sx++ ) {
					double weight = (weightx[wix]) * (weighty[wiy]);
					tjs_uint32 color = srcbits[sx];
					color_element[0] += (color&0xff)*weight;
					color_element[1] += ((color>>8)&0xff)*weight;
					color_element[2] += ((color>>16)&0xff)*weight;
					color_element[3] += ((color>>24)&0xff)*weight;
					wix++;
					w_total += weight;
				}
				wiy++;
			}
			if( w_total != 0 ) {
				double mul = 1.0 / w_total;
				color_element[0] *= mul;
				color_element[1] *= mul;
				color_element[2] *= mul;
				color_element[3] *= mul;
			}
			tjs_uint32 color = (tjs_uint32)((color_element[0] > 255) ? 255 : (color_element[0] < 0) ? 0 : color_element[0]);
			color += (tjs_uint32)((color_element[1] > 255) ? 255 : (color_element[1] < 0) ? 0 : color_element[1]) << 8;
			color += (tjs_uint32)((color_element[2] > 255) ? 255 : (color_element[2] < 0) ? 0 : color_element[2]) << 16;
			color += (tjs_uint32)((color_element[3] > 255) ? 255 : (color_element[3] < 0) ? 0 : color_element[3]) << 24;
			dstbits[destrect.left+x] = color;
		}
	}
}

void TVPLanczos3( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	const int srcwidth = srcrect.get_width();
	const int srcheight = srcrect.get_height();
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();

	LanczosWeight<3> weightfunc;
	AxisParam paramx, paramy;
	paramx.calculateAxis( srcrect.left, srcrect.right, srcwidth, dstwidth, 3.0, weightfunc );
	paramy.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, 3.0, weightfunc );
	TVPWeightCopy( dest, destrect, src, paramx, paramy );
}

void TVPLanczos2( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	const int srcwidth = srcrect.get_width();
	const int srcheight = srcrect.get_height();
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();

	LanczosWeight<2> weightfunc;
	AxisParam paramx, paramy;
	paramx.calculateAxis( srcrect.left, srcrect.right, srcwidth, dstwidth, 2.0, weightfunc );
	paramy.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, 2.0, weightfunc );
	TVPWeightCopy( dest, destrect, src, paramx, paramy );
}
/**
 * ���̃��\�b�h�͊��Ғʂ�@�\���Ȃ��̂ŏC���̕K�v������
 */
void TVPSpline36Scale( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	const int srcwidth = srcrect.get_width();
	const int srcheight = srcrect.get_height();
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();

	Spline36Weight weightfunc;
	AxisParam paramx, paramy;
	paramx.calculateAxis( srcrect.left, srcrect.right, srcwidth, dstwidth, 3.0, weightfunc );
	paramy.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, 3.0, weightfunc );
	TVPWeightCopy( dest, destrect, src, paramx, paramy );
}
