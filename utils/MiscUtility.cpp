
#include "tjsCommHead.h"
#include "CharacterSet.h"
#ifdef _WIN32
#include <mmsystem.h>
#elif defined(ANDROID)
#include <time.h>
#endif


//---------------------------------------------------------------------------
// TVPGetRoughTickCount
// 32bit値のtickカウントを得る
//---------------------------------------------------------------------------
tjs_uint32 TVPGetRoughTickCount32()
{
#ifdef _WIN32
	return timeGetTime();
#elif defined(ANDROID)
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec*1000LL + now.tv_nsec/1000000LL;
#else
	#error Not implemented yet.
#endif
}
//---------------------------------------------------------------------------
bool TVPEncodeUTF8ToUTF16( tjs_string &output, const std::string &source )
{
	tjs_int len = TVPUtf8ToWideCharString( source.c_str(), NULL );
	if(len == -1) return false;
	std::vector<tjs_char> outbuf( len+1, 0 );
	tjs_int ret = TVPUtf8ToWideCharString( source.c_str(), &(outbuf[0]));
	if( ret ) {
		outbuf[ret] = TJS_W('\0');
		output = tjs_string( &(outbuf[0]) );
		return true;
	}
	return false;
}
