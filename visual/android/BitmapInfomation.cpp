
#include "tjsCommHead.h"
#include "BitmapInfomation.h"
#include "MsgIntf.h"

BitmapInfomation::BitmapInfomation( tjs_uint width, tjs_uint height, int bpp )
: width_(width), height_(height), bpp_(bpp) {

	tjs_int PitchBytes;
	tjs_uint bitmap_width = width;

	// note that the allocated bitmap size can be bigger than the
	// original size because the horizontal pitch of the bitmap
	// is aligned to a paragraph (16bytes)
	if( bpp == 8 ) {
		bitmap_width = (((bitmap_width-1) / 16)+1) *16; // align to a paragraph
		PitchBytes = (((bitmap_width-1) >> 2)+1) <<2;
	} else {
		bitmap_width = (((bitmap_width-1) / 4)+1) *4; // align to a paragraph
		PitchBytes = bitmap_width * 4;
	}
	bitmap_width_ = bitmap_width;
	image_size_ = PitchBytes * height;
}

BitmapInfomation::~BitmapInfomation() {
}

