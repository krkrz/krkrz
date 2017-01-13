/******************************************************************************/
/**
 * 拡大縮小を実装する
 * ----------------------------------------------------------------------------
 * 	Copyright (C) T.Imoto <http://www.kaede-software.com>
 * ----------------------------------------------------------------------------
 * @author		T.Imoto
 * @date		2014/04/02
 * @note
 *****************************************************************************/


#ifndef __RESAMPLE_IMAGE_H__
#define __RESAMPLE_IMAGE_H__

extern void TVPResampleImage( const tTVPRect &cliprect, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect,
	tTVPBBStretchType type, tjs_real typeopt, tTVPBBBltMethod method, tjs_int opa, bool hda );

#endif // __RESAMPLE_IMAGE_H__

