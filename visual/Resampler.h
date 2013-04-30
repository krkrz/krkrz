//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Image resampler
//---------------------------------------------------------------------------
#ifndef ResamplerH
#define ResamplerH


//---------------------------------------------------------------------------
class tTVPBaseBitmap;
extern void TVPResampleImage(tTVPBaseBitmap *dst,
	const tTVPRect &destrect,
	const tTVPBaseBitmap *src,
	const tTVPRect &srcrect,
	int mode);
//---------------------------------------------------------------------------

#endif