#pragma once
//
//#ifndef LOOG_INT64
//# ifdef WIN32
//#  define LOOG_INT64 signed __int64
//# else  /* assume POSIX */
//#  define LOOG_INT64 int64_t
//# endif
//#endif
//

#include <libOOOgg.h>
#include <IOggDecoderSeek.h>
#include <string>
using namespace std;

DECLARE_INTERFACE_(IOggDecoder, IOggDecoderSeek)
{
public:
	enum eAcceptHeaderResult 
    {
		AHR_ALL_HEADERS_RECEIVED,
		AHR_MORE_HEADERS_TO_COME,
		AHR_INVALID_HEADER,
		AHR_UNEXPECTED,
		AHR_NULL_POINTER,

	};

	virtual LOOG_INT64 __stdcall convertGranuleToTime(LOOG_INT64 inGranule) = 0;
	virtual LOOG_INT64 __stdcall mustSeekBefore(LOOG_INT64 inGranule) = 0;
	virtual eAcceptHeaderResult __stdcall showHeaderPacket(OggPacket* inCodecHeaderPacket) = 0;
	virtual string __stdcall getCodecShortName() = 0;
	virtual string __stdcall getCodecIdentString() = 0;
	
};