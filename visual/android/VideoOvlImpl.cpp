//---------------------------------------------------------------------------
/*
	Kirikiri Z
	See details of license at "LICENSE"
*/
//---------------------------------------------------------------------------
// Video Overlay support implementation
//---------------------------------------------------------------------------


#include "tjsCommHead.h"

#include <algorithm>
#include "VideoOvlImpl.h"
#include "TVPVideoOverlay.h"



//---------------------------------------------------------------------------
// tTJSNI_VideoOverlay
//---------------------------------------------------------------------------
tTJSNI_VideoOverlay::tTJSNI_VideoOverlay(){}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_VideoOverlay::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_VideoOverlay::Invalidate()
{
	inherited::Invalidate();
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Open(const ttstr &_name) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Close() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Shutdown() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Disconnect() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Play() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Stop() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Pause() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Rewind() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Prepare() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSegmentLoop( int comeFrame, int goFrame ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPeriodEvent( int eventFrame ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetRectangleToVideoOverlay() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPosition(tjs_int left, tjs_int top) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSize(tjs_int width, tjs_int height) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetBounds(const tTVPRect & rect) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLeft(tjs_int l) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetTop(tjs_int t) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetWidth(tjs_int w) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetHeight(tjs_int h) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetVisible(bool b) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::ResetOverlayParams() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::DetachVideoOverlay() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetRectOffset(tjs_int ofsx, tjs_int ofsy) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetTimePosition( tjs_uint64 p ) {}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_VideoOverlay::GetTimePosition()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetFrame()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetStopFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetDefaultStopFrame() {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetStopFrame()
{
	return 0;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetFPS()
{
	return 0.0;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetNumberOfFrame()
{
	return 0;
}
//---------------------------------------------------------------------------
tjs_int64 tTJSNI_VideoOverlay::GetTotalTime()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLoop( bool b ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLayer1( tTJSNI_BaseLayer *l ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLayer2( tTJSNI_BaseLayer *l ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMode( tTVPVideoOverlayMode m ) {}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetPlayRate()
{
	return 0.0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPlayRate(tjs_real r) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetAudioBalance()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetAudioBalance(tjs_int b) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetAudioVolume()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetAudioVolume(tjs_int b) {}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetNumberOfAudioStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SelectAudioStream(tjs_uint n) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetEnabledAudioStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::DisableAudioStream() {}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetNumberOfVideoStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SelectVideoStream(tjs_uint n) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetEnabledVideoStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingLayer( tTJSNI_BaseLayer *l )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::ResetMixingBitmap()
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingMovieAlpha( tjs_real a )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetMixingMovieAlpha()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingMovieBGColor( tjs_uint col )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetMixingMovieBGColor()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrast()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetContrast( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightness()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetBrightness( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetHue( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturation()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSaturation( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetOriginalWidth()
{
	return 0;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetOriginalHeight()
{
	return 0;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNC_VideoOverlay::CreateNativeInstance : returns proper instance object
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_VideoOverlay::CreateNativeInstance()
{
	return new tTJSNI_VideoOverlay();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCreateNativeClass_VideoOverlay
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_VideoOverlay()
{
	return new tTJSNC_VideoOverlay();
}
//---------------------------------------------------------------------------

