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

#undef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))

#include "IStampedOggPacketSink.h"
#include "OggPaginatorSettings.h"
#include "IOggCallback.h"
#include "StampedOggPacket.h"

class OggPaginator : public IStampedOggPacketSink
{
public:
	//TODO::: Have a constructor that lets you set the numheaders.
	OggPaginator(void);
	virtual ~OggPaginator(void);

	/// Set the pagination options.
	bool setParameters(OggPaginatorSettings* inSettings);

	/// Get the pagination options
	OggPaginatorSettings* parameters();
	
	/// Feed your packets in here.
	virtual bool acceptStampedOggPacket(class StampedOggPacket* inOggPacket);

	/// Set the callback where your finished pages will go.
	bool setPageCallback(IOggCallback* inPageCallback);

	/// Finish stream flushed left over data into a page and EOS marks it.
	bool finishStream();

	/// Sets the number of headers. This is important to make sure they aren't on the same page as data packets.
	void setNumHeaders(unsigned long inNumHeaders);

	/// Returns the number of headers set for this paginator.
	unsigned long numHeaders();

protected:
	/// Internal delivery to the callback.
	bool deliverCurrentPage();

	/// Calculates and sets the checksum on the page.
	bool setChecksum();

	/// Creates a new oggpage to start filling.
	bool createFreshPage();

	/// Adds the packet to the page
	bool addPacketToPage(class StampedOggPacket* inOggPacket);

	/// Adds as much packet as the settings dicate.
	unsigned long addAsMuchPacketAsPossible(class StampedOggPacket* inOggPacket, unsigned long inStartAt, long inRemaining);

	/// Add a part of a packet to a page.
	bool addPartOfPacketToPage(class StampedOggPacket* inOggPacket, unsigned long inStartFrom, unsigned long inLength);
	

	unsigned long mPacketCount;
	unsigned long mNumHeaders;

	unsigned long mCurrentPageSize;
	unsigned char mSegmentTable[255];
	unsigned char mSegmentTableSize;
	unsigned long mSequenceNo;
	bool mPendingPageHasData;

	IOggCallback* mPageCallback;
	OggPaginatorSettings* mSettings;
	OggPage* mPendingPage;

	unsigned char* mHeaderBuff;

	LOOG_INT64 mLastGranulePos;

private:
	OggPaginator& operator=(const OggPaginator& other);  /* Don't assign me */
	OggPaginator(const OggPaginator& other); /* Don't copy me */
};
