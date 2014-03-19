
#ifndef __LANCZOS_H__
#define __LANCZOS_H__


extern void TVPLanczos( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, double tap);

inline void TVPLanczos2( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	TVPLanczos( dest, destrect, src, srcrect, 2.0 );
}
inline void TVPLanczos3( tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	TVPLanczos( dest, destrect, src, srcrect, 3.0 );
}

#endif

