
#include "OggFilterFactory.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <vector>
#include <string>
#include <algorithm>
#include <Dshow.h>
#include <Qnetwork.h>
#include <streams.h>
#include <pullpin.h>
#include <dvdmedia.h>

#include "oggLog.h"

#include "OggDemuxFilter.h"
#include "TheoraDecodeFilter.h"
#include "VorbisDecodeFilter.h"
/*
C:\ToolDev\kirikiri_vc\krkrz\src\core\visual\win32\krmovie\ogg\src\lib\factory

..\codecs\theora\filters\dsfTheoraDecoder
..\codecs\vorbis\filters\dsfVorbisDecoder
..\codecs\theora\libs\libOOTheora;
..\codecs\theora\libs\libtheora\include;
"..\codecs\theora\libs\libtheora-exp\include";
..\codecs\vorbis\libs\libOOVorbis;
..\codecs\vorbis\\libs\libvorbis\include;

..\core\directshow\dsfOggDemux2

..\core\ogg;
..\core\ogg\libogg\include;
..\core\directshow\libDirectshowAbstracts;
..\core\directshow\dsfOggDemux2;

..\helper;
..\helper\iDSHelper;

..\codecs\theora\filters\dsfTheoraDecoder;..\codecs\vorbis\filters\dsfVorbisDecoder;..\codecs\theora\libs\libOOTheora;..\codecs\theora\libs\libtheora\include;"..\codecs\theora\libs\libtheora-exp\include";..\codecs\vorbis\libs\libOOVorbis;..\codecs\vorbis\\libs\libvorbis\include;..\core\directshow\dsfOggDemux2;..\core\ogg;..\core\ogg\libogg\include;..\core\directshow\libDirectshowAbstracts;..\core\directshow\dsfOggDemux2;..\helper;..\helper\iDSHelper;

BOOL APIENTRY DllMain( HANDLE, 
                       DWORD ul_reason_for_call, 
                       LPVOID 
					 )
{
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

*/
extern "C" {
IBaseFilter* __stdcall CreateVorbisDecoder() {
	return new VorbisDecodeFilter();
}
IBaseFilter* __stdcall CreateTheoraDecoder() {
	return new TheoraDecodeFilter();
}
IBaseFilter* __stdcall CreateOggSplitter() {
	HRESULT hr;
	return new OggDemuxFilter(&hr);
}
};
