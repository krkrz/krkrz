
#ifndef __TVP_VIDEO_OVERLAY_H__
#define __TVP_VIDEO_OVERLAY_H__

extern void __stdcall GetVideoOverlayObject(
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out);

extern void __stdcall GetVideoLayerObject(
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out);

extern void __stdcall GetMixingVideoOverlayObject(
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out);

extern void __stdcall GetMFVideoOverlayObject(
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out);

#endif // __TVP_VIDEO_OVERLAY_H__
