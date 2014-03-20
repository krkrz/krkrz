
#ifndef __LANCZOS_H__
#define __LANCZOS_H__


extern void TVPLanczos2( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect );
extern void TVPLanczos3( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect );
//extern void TVPSpline36Scale( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect );

#endif

