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
#include "Application.h"
#include "ActivityEvents.h"
#include "StorageIntf.h"

//---------------------------------------------------------------------------
// tTJSNI_VideoOverlay
//---------------------------------------------------------------------------
tTJSNI_VideoOverlay::tTJSNI_VideoOverlay() : VideoOverlay(nullptr) {}
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
void tTJSNI_VideoOverlay::Open(const ttstr &name) {
	ttstr path = name;
	ttstr newpath = TVPGetPlacedPath(path);
	if( newpath.IsEmpty() ) {
		path = TVPNormalizeStorageName(path);
	} else {
		path = newpath;
	}
	if( path[0] == TJS_W('a') &&
		path[1] == TJS_W('s') &&
		path[2] == TJS_W('s') &&
		path[3] == TJS_W('e') &&
		path[4] == TJS_W('t') &&
		path[5] == TJS_W(':') ) {
        TVPGetLocalName(path);
		path = ttstr("file:///android_asset") + path;
	} else {
		TVPGetLocalName(path);
	}
	TragetVideoFileName = path;
	VideoOverlay = Application;
	VideoOverlay->OpenMovie( TragetVideoFileName.c_str() );
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Close() {
	VideoOverlay = nullptr;
	SetStatus(tTVPVideoOverlayStatus::Unload);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Shutdown() {
	VideoOverlay = nullptr;
	SetStatus(tTVPVideoOverlayStatus::Unload);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Disconnect() {
	Shutdown();
	VideoOverlay = nullptr;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Play() {
	if( VideoOverlay ) {
		VideoOverlay->PlayMovie();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Stop() {
	if( VideoOverlay ) {
		VideoOverlay->StopMovie();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Pause() {
	if( VideoOverlay ) {
		VideoOverlay->PauseMovie();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Rewind() {
	if( VideoOverlay ) {
		VideoOverlay->SetMovieCurrentPosition( 0 );
	}
}
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
bool tTJSNI_VideoOverlay::GetVisible() const {
	if( VideoOverlay ) {
		return VideoOverlay->GetMovieVisible();
	}
	return false; 
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::ResetOverlayParams() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::DetachVideoOverlay() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetRectOffset(tjs_int ofsx, tjs_int ofsy) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetTimePosition( tjs_uint64 p ) {
	if( VideoOverlay ) {
		VideoOverlay->SetMovieCurrentPosition( p );
	}
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_VideoOverlay::GetTimePosition() {
	if( VideoOverlay ) {
		return VideoOverlay->GetMovieCurrentPosition();
	}
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetFrame() {
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetStopFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetDefaultStopFrame() {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetStopFrame() {
	return 0;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetFPS() {
	return 0.0;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetNumberOfFrame() {
	return 0;
}
//---------------------------------------------------------------------------
tjs_int64 tTJSNI_VideoOverlay::GetTotalTime() {
	if( VideoOverlay) {
		return VideoOverlay->GetMovieDuration();
	}
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
tjs_int tTJSNI_VideoOverlay::GetAudioVolume() {
	if( VideoOverlay ) {
		float volume = VideoOverlay->GetMovieVolume();
		return (tjs_int)(volume * 100000);
	}
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetAudioVolume(tjs_int b) {
	if( VideoOverlay ) {
		if( b < 0 ) b = 0;
		if( b > 100000 ) b = 100000;
		float volume = (float)b / 100000.0f;
		VideoOverlay->SetMovieVolume ( volume );
	}
}
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
void tTJSNI_VideoOverlay::HandleEvent( tjs_uint ev ) {
    switch( ev ) {
    case AM_MOVIE_ENDED:
        SetStatusAsync( tTVPVideoOverlayStatus::Stop );
        break;
    case AM_MOVIE_PLAYER_ERROR:
    	SetStatusAsync( tTVPVideoOverlayStatus::PlayerError );
        break;
    case AM_MOVIE_LOAD_ERROR:
    	SetStatusAsync( tTVPVideoOverlayStatus::LoadError );
        break;
    case AM_MOVIE_BUFFERING:
    	SetStatusAsync( tTVPVideoOverlayStatus::Buffering );
        break;
    case AM_MOVIE_IDLE:
    	SetStatusAsync( tTVPVideoOverlayStatus::Idle );
        break;
    case AM_MOVIE_READY:
        SetStatusAsync( tTVPVideoOverlayStatus::Ready );
        break;
    case AM_MOVIE_PLAY:
        SetStatusAsync( tTVPVideoOverlayStatus::Play );
    	break;
    }
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

