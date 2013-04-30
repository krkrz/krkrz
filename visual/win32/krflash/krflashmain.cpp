//---------------------------------------------------------------------------
// krflashmain.cpp ( part of KRFLASH.DLL )
// (c)2001-2009, W.Dee <dee@kikyou.info> and contributors
//---------------------------------------------------------------------------
/*
	We separated this module because VCL's ActiveX support is too large (in size)
	to link with the main module.

	This requires Macromedia Flash Player plug-in for Internet Explorer.
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#include <windows.h>
#pragma hdrstop


#include "FlashContainerFormUnit.h"
#include "krflashmain.h"
#include <evcode.h>
#include "tp_stub.h"

//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTVPFlashOverlay
//---------------------------------------------------------------------------
tTVPFlashOverlay::tTVPFlashOverlay(const wchar_t *flashfile, HWND callbackwindow)
{
	RefCount = 1;
	CallbackWindow = callbackwindow;
	OwnerWindow = NULL;
	FileName = new wchar_t[wcslen(flashfile) + 1];
	wcscpy(FileName, flashfile);
	Visible = false;
	Shutdown = false;
	CompleteSent = false;
	Rect.left = 0; Rect.top = 0; Rect.right = 0; Rect.bottom = 0;
	Form = NULL;
}
//---------------------------------------------------------------------------
tTVPFlashOverlay::~tTVPFlashOverlay()
{
	Shutdown = true;
	if(FileName) delete [] FileName;
	if(Form) delete Form;
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::AddRef()
{
	RefCount ++;
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::Release()
{
	if(RefCount == 1)
	{
		delete this;
	}
	else
	{
		RefCount--;
	}
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::SetMessageDrainWindow(HWND wnd)
{
	// nothing to do, because flash support does not drain messages.
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::SetWindow(HWND wnd)
{
	if(Shutdown) return;
	OwnerWindow = wnd;
	try
	{
		if(Form)
			Form->SetFormParent(wnd);
		else
			ResetForm();
	}
	catch(Exception &e)
	{
		TVPThrowExceptionMessage(ttstr(("Cannot create Flash ActiveX control : " + e.Message).c_str()).c_str());
	}
	catch(...)
	{
		TVPThrowExceptionMessage(L"Cannot create window instance (Flash player ActiveX is not installed ?)");
	}
	return;
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::SetRect(RECT *rect)
{
	if(Shutdown) return;
	Rect = *rect;
	if(Form) Form->SetBounds(Rect.left, Rect.top, Rect.right - Rect.left,
				Rect.bottom - Rect.top);
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::SetVisible(bool b)
{
 	if(Shutdown) return;
	Visible = true;
	if(Form)
		Form->SetFlashVisible(b);
	else
		TVPThrowExceptionMessage(L"Owner window not specified");
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::Play()
{
	if(Shutdown) return;
	if(Form)
		Form->Play();
	else
		TVPThrowExceptionMessage(L"Owner window not specified");
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::Stop()
{
	if(Shutdown) return;
	if(Form)
		Form->Stop();
	else
		TVPThrowExceptionMessage(L"Owner window not specified");
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::Pause()
{
	if(Shutdown) return;
	if(Form)
		Form->Pause();
	else
		TVPThrowExceptionMessage(L"Owner window not specified");
}
//---------------------------------------------------------------------------
void __stdcall  tTVPFlashOverlay::SetPosition(unsigned __int64 tick)
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetPosition: Currently not supported");
}
//---------------------------------------------------------------------------
void __stdcall  tTVPFlashOverlay::GetPosition(unsigned __int64 *tick)
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetPosition: Currently not supported");
}
//---------------------------------------------------------------------------
void __stdcall  tTVPFlashOverlay::GetStatus(tTVPVideoStatus *status)
{
	if(Shutdown) return;
	if(Form)
		*status = Form->GetStatus();
	else
		TVPThrowExceptionMessage(L"Owner window not specified");
}
//---------------------------------------------------------------------------
void __stdcall tTVPFlashOverlay::GetEvent(long *evcode, long *param1,
			long *param2, bool *got)
{
	if(Shutdown) return;
	*got = false; // not implemented
}
//---------------------------------------------------------------------------
void tTVPFlashOverlay::SendCommand(wchar_t *command, wchar_t *arg)
{
	::SendMessage(CallbackWindow, WM_CALLBACKCMD, (WPARAM)command, (LPARAM)arg);
}
//---------------------------------------------------------------------------
void tTVPFlashOverlay::ResetForm()
{
	if(Form) delete Form;
	Form = new TFlashContainerForm(Application, this, OwnerWindow, Rect);
	Form->SetMovie(FileName);
	Form->Visible = Visible;
}
//---------------------------------------------------------------------------
// Start:	Add:	T.Imoto
void __stdcall tTVPFlashOverlay::FreeEventParams(long evcode, long param1, long param2)
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"FreeEventParams: Currently not supported");
}
void __stdcall tTVPFlashOverlay::Rewind()
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"Rewind: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetFrame( int f )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetFrame( int *f )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetFPS( double *f )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetFPS: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetNumberOfFrame( int *f )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetNumberOfFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetTotalTime( __int64 *t )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetTotalTime: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetVideoSize( long *width, long *height )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetVideoSize: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetFrontBuffer( BYTE **buff )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetFrontBuffer: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetVideoBuffer( BYTE *buff1, BYTE *buff2, long size )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetVideoBuffer: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetStopFrame( int frame )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetStopFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetStopFrame( int *frame )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetStopFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetDefaultStopFrame()
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetDefaultStopFrame: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetPlayRate( double rate )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetPlayRate: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetPlayRate( double *rate )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetPlayRate: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetAudioBalance( long balance )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetAudioBalance: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetAudioBalance( long *balance )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetAudioBalance: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetAudioVolume( long volume )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetAudioVolume: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetAudioVolume( long *volume )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetAudioVolume: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetNumberOfAudioStream( unsigned long *streamCount )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetNumberOfAudioStream: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SelectAudioStream( unsigned long num )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SelectAudioStream: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetEnableAudioStreamNum( long *num )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetEnableAudioStreamNum: Currently not supported");
}
void __stdcall tTVPFlashOverlay::DisableAudioStream( void )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"DisableAudioStream: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetNumberOfVideoStream( unsigned long *streamCount )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetNumberOfVideoStream: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SelectVideoStream( unsigned long num )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SelectVideoStream: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetEnableVideoStreamNum( long *num )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetEnableVideoStreamNum: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetMixingBitmap( HDC hdc, RECT *dest, float alpha )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetMixingBitmap: Currently not supported");
}
void __stdcall tTVPFlashOverlay::ResetMixingBitmap()
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"ResetMixingBitmap: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetMixingMovieAlpha( float a )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetMixingMovieAlpha: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetMixingMovieAlpha( float *a )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetMixingMovieAlpha: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetMixingMovieBGColor( unsigned long col )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetMixingMovieBGColor: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetMixingMovieBGColor( unsigned long *col )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetMixingMovieBGColor: Currently not supported");
}
void __stdcall tTVPFlashOverlay::PresentVideoImage()
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"PresentVideoImage: Currently not supported");
}


void __stdcall tTVPFlashOverlay::GetContrastRangeMin( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetContrastRangeMin: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetContrastRangeMax( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetContrastRangeMax: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetContrastDefaultValue( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetContrastDefaultValue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetContrastStepSize( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetContrastStepSize: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetContrast( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetContrast: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetContrast( float v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetContrast: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetBrightnessRangeMin( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetBrightnessRangeMin: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetBrightnessRangeMax( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetBrightnessRangeMax: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetBrightnessDefaultValue( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetBrightnessDefaultValue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetBrightnessStepSize( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetBrightnessStepSize: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetBrightness( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetBrightness: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetBrightness( float v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetBrightness: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetHueRangeMin( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetHueRangeMin: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetHueRangeMax( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetHueRangeMax: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetHueDefaultValue( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetHueDefaultValue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetHueStepSize( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetHueStepSize: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetHue( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetHue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetHue( float v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetHue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetSaturationRangeMin( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetSaturationRangeMin: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetSaturationRangeMax( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetSaturationRangeMax: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetSaturationDefaultValue( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetSaturationDefaultValue: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetSaturationStepSize( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetSaturationStepSize: Currently not supported");
}
void __stdcall tTVPFlashOverlay::GetSaturation( float *v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"GetSaturation: Currently not supported");
}
void __stdcall tTVPFlashOverlay::SetSaturation( float v )
{
	if(Shutdown) return;
	TVPThrowExceptionMessage(L"SetSaturation: Currently not supported");
}
// End:	Add:	T.Imoto



//---------------------------------------------------------------------------
// GetVideoOverlayObject
//---------------------------------------------------------------------------
extern "C" void _export __stdcall GetVideoOverlayObject(
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out)
{
	*out = new tTVPFlashOverlay(streamname, callbackwin);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// GetAPIVersion
//---------------------------------------------------------------------------
extern "C" void _export __stdcall GetAPIVersion(DWORD *ver)
{
	*ver = TVP_KRMOVIE_VER;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// V2Link : Initialize TVP plugin interface
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);

	return S_OK;
}
//---------------------------------------------------------------------------
// V2Unlink : Uninitialize TVP plugin interface
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

