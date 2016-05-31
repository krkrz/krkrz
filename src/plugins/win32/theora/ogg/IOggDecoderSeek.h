
#pragma once

DECLARE_INTERFACE_(IOggDecoderSeek, IUnknown)
{
public:
	IOggDecoderSeek(void)		{}
	virtual ~IOggDecoderSeek(void)		{}

	virtual LOOG_INT64 __stdcall convertGranuleToTime(LOOG_INT64 inGranule) = 0;
	virtual LOOG_INT64 __stdcall mustSeekBefore(LOOG_INT64 inGranule) = 0;
};
