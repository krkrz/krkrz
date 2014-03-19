
#define _USE_MATH_DEFINES

#include "tjsCommHead.h"
#include "LayerBitmapIntf.h"
#include "LayerBitmapImpl.h"

#include <float.h>
#include <math.h>
#include <cmath>
#include <vector>
#include "Lanczos.h"

double lanczos3Weight( double phase, double tap ) {
	double ret;
	if( std::abs(phase) < DBL_EPSILON ){
		return 1.0;
	}
	if( std::abs(phase) >= tap ) {
		return 0.0;
	}
	ret = std::sin(M_PI*phase)*std::sin(M_PI*phase/tap)/(M_PI*M_PI*phase*phase/tap);
	return ret;
}

void TVPLanczos( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, double tap) {
	static const int num_of_color = 4;
	const int srcwidth = srcrect.get_width();
	const int srcheight = srcrect.get_height();
	const int destwidth = destrect.get_width();
	const int destheight = destrect.get_height();

	for( int y = 0; y < destheight; y++ ) {
		double cy = (y+0.5)*(double)srcheight/(double)destheight + srcrect.top;
		if( srcheight <= destheight ) { // ècägëÂ
			int top = (int)std::floor(cy-tap);
			int bottom = (int)std::floor(cy+tap);
			if( srcwidth <= destwidth ) { // â°ägëÂ
				double rangex = tap;
				for( int x = 0; x < destwidth; x++ ) {
					double color_element[num_of_color] = {0.0,0.0,0.0,0.0};
					double w_total = 0.0;
					double cx = (x+0.5)*(double)srcwidth/(double)destwidth + srcrect.left;
					int left = (int)std::floor(cx-rangex);
					int right = (int)std::floor(cx+rangex);
					for( int sy = top; sy <= bottom; sy++ ) {
						if( sy < srcrect.top || sy >= srcrect.bottom ) continue;

						double distY = std::abs(sy + 0.5 - cy);
						double weighty = lanczos3Weight(distY,tap);
						if( weighty == 0.0 ) continue;
						for( int sx = left; sx <= right; sx++ ) {
							if( sx < srcrect.left || sx >= srcrect.right ) continue;

							double distX = std::abs(sx + 0.5 - cx);
							double weightx = lanczos3Weight(distX,tap);
							if( weightx == 0.0 ) continue;
							double weight = weightx * weighty;

							int color_process = (int)src->GetPoint( sx, sy );
							for( int k = 0; k < num_of_color; k++ ) {
								color_element[k] += ((color_process >> k*8) & 0xff)*weight;
							}
							w_total += weight;
						}
					}
					tjs_uint32 color = 0;
					for( int i = 0; i < num_of_color; i++ ) {
						if( w_total != 0 ) color_element[i] /= w_total;
						color_element[i] = (color_element[i] > 255) ? 255
								: (color_element[i] < 0) ? 0 : color_element[i];
						color += (tjs_uint32)color_element[i] << i*8;
					}
					dest->SetPoint( destrect.left+x, destrect.top+y, color );
				}
			} else { // â°èkè¨
				double rangex = tap*(double)srcwidth/(double)destwidth;
				for( int x = 0; x < destwidth; x++ ) {
					double color_element[num_of_color] = {0.0,0.0,0.0,0.0};
					double w_total = 0.0;
					double cx = (x+0.5)*(double)srcwidth/(double)destwidth + srcrect.left;
					int left = (int)std::floor(cx-rangex);
					int right = (int)std::floor(cx+rangex);
					for( int sy = top; sy <= bottom; sy++ ) {
						if( sy < srcrect.top || sy >= srcrect.bottom ) continue;

						double distY = std::abs(sy + 0.5 - cy);
						double weighty = lanczos3Weight(distY,tap);
						if( weighty == 0.0 ) continue;
						for( int sx = left; sx <= right; sx++ ) {
							if( sx < srcrect.left || sx >= srcrect.right ) continue;

							double dx = (sx-srcrect.left+0.5)*(double)destwidth/(double)srcwidth;
							double distX = std::abs(dx - (x+0.5));
							double weightx = lanczos3Weight(distX,tap);
							if( weightx == 0.0 ) continue;
							double weight = weightx * weighty;

							int color_process = (int)src->GetPoint( sx, sy );
							for( int k = 0; k < num_of_color; k++ ) {
								color_element[k] += ((color_process >> k*8) & 0xff)*weight;
							}
							w_total += weight;
						}
					}
					tjs_uint32 color = 0;
					for( int i = 0; i < num_of_color; i++ ) {
						if( w_total != 0 ) color_element[i] /= w_total;
						color_element[i] = (color_element[i] > 255) ? 255
								: (color_element[i] < 0) ? 0 : color_element[i];
						color += (tjs_uint32)color_element[i] << i*8;
					}
					dest->SetPoint( destrect.left+x, destrect.top+y, color );
				}
			}
		} else { // ècèkè¨
			double rangey = tap*(double)srcheight/(double)destheight;
			double cy = (y+0.5)*(double)srcheight/(double)destheight + srcrect.top;
			int top = (int)std::floor(cy-rangey);
			int bottom = (int)std::floor(cy+rangey);
			if( srcwidth <= destwidth ) { // â°ägëÂ
				double rangex = tap*(double)srcwidth/(double)destwidth;
				for( int x = 0; x < destwidth; x++ ) {
					double color_element[num_of_color] = {0.0,0.0,0.0,0.0};
					double w_total = 0.0;
					double cx = (x+0.5)*(double)srcwidth/(double)destwidth + srcrect.left;
					int left = (int)std::floor(cx-rangex);
					int right = (int)std::floor(cx+rangex);
					for( int sy = top; sy <= bottom; sy++ ) {
						if( sy < srcrect.top || sy >= srcrect.bottom ) continue;

						double dy = (sy-srcrect.top+0.5)*(double)destheight/(double)srcheight;
						double distY = std::abs(dy - (y+0.5));
						double weighty = lanczos3Weight(distY,tap);
						if( weighty == 0.0 ) continue;
						for( int sx = left; sx <= right; sx++ ) {
							if( sx < srcrect.left || sx >= srcrect.right ) continue;
							
							double distX = std::abs(sx + 0.5 - cx);
							double weightx = lanczos3Weight(distX,tap);
							if( weightx == 0.0 ) continue;
							double weight = weightx * weighty;

							int color_process = (int)src->GetPoint( sx, sy );
							for( int k = 0; k < num_of_color; k++ ) {
								color_element[k] += ((color_process >> k*8) & 0xff)*weight;
							}
							w_total += weight;
						}
					}
					tjs_uint32 color = 0;
					for( int i = 0; i < num_of_color; i++ ) {
						if( w_total != 0 ) color_element[i] /= w_total;
						color_element[i] = (color_element[i] > 255) ? 255
								: (color_element[i] < 0) ? 0 : color_element[i];
						color += (tjs_uint32)color_element[i] << i*8;
					}
					dest->SetPoint( destrect.left+x, destrect.top+y, color );
				}
			} else { // â°èkè¨
				double rangex = tap*(double)srcwidth/(double)destwidth;
				for( int x = 0; x < destwidth; x++ ) {
					double color_element[num_of_color] = {0.0,0.0,0.0,0.0};
					double w_total = 0.0;
					double cx = (x+0.5)*(double)srcwidth/(double)destwidth + srcrect.left;
					int left = (int)std::floor(cx-rangex);
					int right = (int)std::floor(cx+rangex);
					for( int sy = top; sy <= bottom; sy++ ) {
						if( sy < srcrect.top || sy >= srcrect.bottom ) continue;

						double dy = (sy-srcrect.top+0.5)*(double)destheight/(double)srcheight;
						double distY = std::abs(dy - (y+0.5));
						double weighty = lanczos3Weight(distY,tap);
						if( weighty == 0.0 ) continue;
						for( int sx = left; sx <= right; sx++ ) {
							if( sx < srcrect.left || sx >= srcrect.right ) continue;

							double dx = (sx-srcrect.left+0.5)*(double)destwidth/(double)srcwidth;
							double distX = std::abs(dx - (x+0.5));
							double weightx = lanczos3Weight(distX,tap);
							if( weightx == 0.0 ) continue;
							double weight = weightx * weighty;

							int color_process = (int)src->GetPoint( sx, sy );
							for( int k = 0; k < num_of_color; k++ ) {
								color_element[k] += ((color_process >> k*8) & 0xff)*weight;
							}
							w_total += weight;
						}
					}
					tjs_uint32 color = 0;
					for( int i = 0; i < num_of_color; i++ ) {
						if( w_total != 0 ) color_element[i] /= w_total;
						color_element[i] = (color_element[i] > 255) ? 255
								: (color_element[i] < 0) ? 0 : color_element[i];
						color += (tjs_uint32)color_element[i] << i*8;
					}
					dest->SetPoint( destrect.left+x, destrect.top+y, color );
				}
			}
		}
	}
}

