//===========================================================================
//Copyright (C) 2003, 2004 Zentaro Kavanagh
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//- Neither the name of Zentaro Kavanagh nor the names of contributors 
//  may be used to endorse or promote products derived from this software 
//  without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================

#pragma once
#include "oggstdafx.h"
#include <OggPacket.h>

class StampedOggPacket : public OggPacket
{
public:
	//Public Constants and Enums
	enum eStampType {
		NONE = 0,
		OGG_END_ONLY = 1,
		OGG_BOTH = 2,
		DIRECTSHOW = 3
	};

	//Constructors
	StampedOggPacket(void);
	StampedOggPacket(unsigned char* inPackData, unsigned long inPacketSize, bool inIsTruncated, bool inIsContinuation, LOOG_INT64 inStartTime, LOOG_INT64 inEndTime, unsigned short inStampType);
	virtual ~StampedOggPacket(void);

	/// Does a deep copy of the packet a returns you a new one you can keep.
	virtual OggPacket* clone();

	/// Does a deep copy of the packet a returns you a new one you can keep.
	virtual StampedOggPacket* cloneStamped();

	//TODO::: should not be global.
	unsigned short mStampType;

	/// Returns the start time of the packet. (Check stamp type)
	LOOG_INT64 startTime();

	/// Returns the end time of the packet (Check stamp type)
	LOOG_INT64 endTime();

	/// Sets the start time of the packet (You need to set stamp type appropriately)
	void setStartTime(LOOG_INT64 inStartTime);

	/// Sets the end time of the packet (You need to set stamp type appropriately)
	void setEndTime(LOOG_INT64 inEndTime);

	/// Sets the time stamp in one hit.
	void setTimeStamp(LOOG_INT64 inStartTime, LOOG_INT64 inEndTime, StampedOggPacket::eStampType inStampType);

	/// Merges two packets together.
	virtual void merge(const StampedOggPacket* inMorePacket);

protected:
	LOOG_INT64 mStartTime;
	LOOG_INT64 mEndTime;

private:
	StampedOggPacket& operator=(const StampedOggPacket& other);  /* Don't assign me */
	StampedOggPacket(const StampedOggPacket& other); /* Don't copy me */
};
