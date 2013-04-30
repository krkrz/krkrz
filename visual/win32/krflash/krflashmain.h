//---------------------------------------------------------------------------
// krflashmain.h ( part of KRFLASH.DLL )
// (c)2001-2009, W.Dee <dee@kikyou.info> and contributors
//---------------------------------------------------------------------------
#ifndef krflashmainh
#define krflashmainh

#include "..\krmovie.h"

//---------------------------------------------------------------------------
// tTVPFlashOverlay
//---------------------------------------------------------------------------
class TFlashContainerForm;
class tTVPFlashOverlay : public iTVPVideoOverlay
{
	ULONG RefCount;
	HWND OwnerWindow;
	HWND CallbackWindow;
	wchar_t *FileName;
	bool Visible;
	bool Shutdown;
	RECT Rect;
	TFlashContainerForm *Form;

	bool CompleteSent;

public:
	tTVPFlashOverlay(const wchar_t *flashfile, HWND callbackwindow);
	~tTVPFlashOverlay();

	void __stdcall AddRef();
	void __stdcall Release();

	void __stdcall SetWindow(HWND window);
	void __stdcall SetMessageDrainWindow(HWND window);
	void __stdcall SetRect(RECT *rect);
	void __stdcall SetVisible(bool b);
	void __stdcall Play();
	void __stdcall Stop();
	void __stdcall Pause();
	void __stdcall SetPosition(unsigned __int64 tick);
	void __stdcall GetPosition(unsigned __int64 *tick);
	void __stdcall GetStatus(tTVPVideoStatus *status);
	void __stdcall GetEvent(long *evcode, long *param1,
			long *param2, bool *got);
// Start:	Add:	T.Imoto
	void __stdcall FreeEventParams(long evcode, long param1, long param2);

	void __stdcall Rewind();
	void __stdcall SetFrame( int f );
	void __stdcall GetFrame( int *f );
	void __stdcall GetFPS( double *f );
	void __stdcall GetNumberOfFrame( int *f );
	void __stdcall GetTotalTime( __int64 *t );

	void __stdcall GetVideoSize( long *width, long *height );
	void __stdcall GetFrontBuffer( BYTE **buff );
	void __stdcall SetVideoBuffer( BYTE *buff1, BYTE *buff2, long size );

	void __stdcall SetStopFrame( int frame );
	void __stdcall GetStopFrame( int *frame );
	void __stdcall SetDefaultStopFrame();

	void __stdcall SetPlayRate( double rate );
	void __stdcall GetPlayRate( double *rate );

	void __stdcall SetAudioBalance( long balance );
	void __stdcall GetAudioBalance( long *balance );
	void __stdcall SetAudioVolume( long volume );
	void __stdcall GetAudioVolume( long *volume );

	void __stdcall GetNumberOfAudioStream( unsigned long *streamCount );
	void __stdcall SelectAudioStream( unsigned long num );
	void __stdcall GetEnableAudioStreamNum( long *num );
	void __stdcall DisableAudioStream( void );

	void __stdcall GetNumberOfVideoStream( unsigned long *streamCount );
	void __stdcall SelectVideoStream( unsigned long num );
	void __stdcall GetEnableVideoStreamNum( long *num );

	void __stdcall SetMixingBitmap( HDC hdc, RECT *dest, float alpha );
	void __stdcall ResetMixingBitmap();

	void __stdcall SetMixingMovieAlpha( float a );
	void __stdcall GetMixingMovieAlpha( float *a );
	void __stdcall SetMixingMovieBGColor( unsigned long col );
	void __stdcall GetMixingMovieBGColor( unsigned long *col );
	void __stdcall PresentVideoImage();

	void __stdcall GetContrastRangeMin( float *v );
	void __stdcall GetContrastRangeMax( float *v );
	void __stdcall GetContrastDefaultValue( float *v );
	void __stdcall GetContrastStepSize( float *v );
	void __stdcall GetContrast( float *v );
	void __stdcall SetContrast( float v );

	void __stdcall GetBrightnessRangeMin( float *v );
	void __stdcall GetBrightnessRangeMax( float *v );
	void __stdcall GetBrightnessDefaultValue( float *v );
	void __stdcall GetBrightnessStepSize( float *v );
	void __stdcall GetBrightness( float *v );
	void __stdcall SetBrightness( float v );

	void __stdcall GetHueRangeMin( float *v );
	void __stdcall GetHueRangeMax( float *v );
	void __stdcall GetHueDefaultValue( float *v );
	void __stdcall GetHueStepSize( float *v );
	void __stdcall GetHue( float *v );
	void __stdcall SetHue( float v );

	void __stdcall GetSaturationRangeMin( float *v );
	void __stdcall GetSaturationRangeMax( float *v );
	void __stdcall GetSaturationDefaultValue( float *v );
	void __stdcall GetSaturationStepSize( float *v );
	void __stdcall GetSaturation( float *v );
	void __stdcall SetSaturation( float v );
// End:	Add:	T.Imoto


	void SendCommand(wchar_t *command, wchar_t *arg);
private:
	void ResetForm();
};
//---------------------------------------------------------------------------
#endif
