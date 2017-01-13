
#include "tjsCommHead.h"
#include "BitmapInfomation.h"
#include "MsgIntf.h"

BitmapInfomation::BitmapInfomation( tjs_uint width, tjs_uint height, int bpp ) {
	BitmapInfoSize = sizeof(BITMAPINFOHEADER) + ((bpp==8)?sizeof(RGBQUAD)*256 : 0);
	BitmapInfo = (BITMAPINFO*)GlobalAlloc(GPTR, BitmapInfoSize);
	if(!BitmapInfo) TVPThrowExceptionMessage(TVPCannotAllocateBitmapBits,
		TJS_W("allocating BITMAPINFOHEADER"), ttstr((tjs_int)BitmapInfoSize));

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
	BitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo->bmiHeader.biWidth = bitmap_width;
	BitmapInfo->bmiHeader.biHeight = height;
	BitmapInfo->bmiHeader.biPlanes = 1;
	BitmapInfo->bmiHeader.biBitCount = bpp;
	BitmapInfo->bmiHeader.biCompression = BI_RGB;
	BitmapInfo->bmiHeader.biSizeImage = PitchBytes * height;
	BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biClrUsed = 0;
	BitmapInfo->bmiHeader.biClrImportant = 0;

	// create grayscale palette
	if(bpp == 8) {
		RGBQUAD *pal = (RGBQUAD*)((tjs_uint8*)BitmapInfo + sizeof(BITMAPINFOHEADER));
		for( tjs_int i=0; i<256; i++ ) {
			pal[i].rgbBlue = pal[i].rgbGreen = pal[i].rgbRed = (BYTE)i;
			pal[i].rgbReserved = 0;
		}
	}
}

BitmapInfomation::~BitmapInfomation() {
	::GlobalFree((HGLOBAL)BitmapInfo);
	BitmapInfo = NULL;
}

