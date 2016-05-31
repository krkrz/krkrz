#include <windows.h>
#include "tp_stub.h"
#include <tchar.h>
#include <string.h>
#include <vector>

#include "OggFilterFactory.h"
#ifndef WINCE
#pragma comment (lib, "winmm")
#else
#pragma comment (lib, "mmtimer")
#endif

#pragma comment (lib, "strmiids")
#pragma comment (lib, "quartz")

#pragma comment (lib, "libogg_static")
#pragma comment (lib, "libtheora_static")
#pragma comment (lib, "libvorbis_static")

#ifdef _DEBUG
#pragma comment (lib, "strmbasd")
#else
#pragma comment (lib, "strmbase")
#endif


const GUID MEDIASUBTYPE_Ogg = 
{ 0xdd142c1e, 0xc1e, 0x4381, { 0xa2, 0x4e, 0xb, 0x2d, 0x80, 0xb6, 0x9, 0x8a } };


void* tTVPCreateOggFilter( void* formatdata );
void* tTVPCreateTheoraFilter( void* formatdata );
void* tTVPCreateVorbisFilter( void* formatdata );

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	TVPRegisterDSVideoCodec( TJS_W(".ogg"), (void*)&MEDIASUBTYPE_Ogg, tTVPCreateOggFilter, tTVPCreateTheoraFilter, tTVPCreateVorbisFilter, NULL );
	TVPRegisterDSVideoCodec( TJS_W(".ogv"), (void*)&MEDIASUBTYPE_Ogg, tTVPCreateOggFilter, tTVPCreateTheoraFilter, tTVPCreateVorbisFilter, NULL );

	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	return S_OK;
}
extern "C" HRESULT _stdcall V2Unlink() {
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
	
	TVPUnregisterDSVideoCodec( TJS_W(".ogg"), (void*)&MEDIASUBTYPE_Ogg, tTVPCreateOggFilter, tTVPCreateTheoraFilter, tTVPCreateVorbisFilter, NULL );
	TVPUnregisterDSVideoCodec( TJS_W(".ogv"), (void*)&MEDIASUBTYPE_Ogg, tTVPCreateOggFilter, tTVPCreateTheoraFilter, tTVPCreateVorbisFilter, NULL );

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();
	return S_OK;
}

void* tTVPCreateOggFilter( void* formatdata ) {
	return (void*)CreateOggSplitter();
}
void* tTVPCreateTheoraFilter( void* formatdata ) {
	return (void*)CreateTheoraDecoder();
}
void* tTVPCreateVorbisFilter( void* formatdata ) {
	return (void*)CreateVorbisDecoder();
}
