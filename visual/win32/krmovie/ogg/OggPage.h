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

//STL Includes
#include <vector>
// using namespace std;

//Class Includes
#include "StampedOggPacket.h"
#include "OggPageHeader.h"
#include "IOggPackSource.h"

//OggPage represents an Ogg Encapsulation Format page
class OggPage : public IOggPackSource
{
public:
	//Constants
	static const unsigned long HEX_DUMP_LINE_LENGTH = 16;
	
	//Constructors
	OggPage(void);
	virtual ~OggPage(void);

	/// Doa deep copy of the page and return one you can keep.
	OggPage* clone();
	
	//// Returns the total byte size of the current page.
	unsigned long pageSize();

	/// Returns the size of the header including segment table.
	unsigned long headerSize();

	/// Returns the size of the data part of the page.
	unsigned long dataSize();

	//IOggPackSource Implementation
	virtual OggPacket* getPacket(unsigned long inPacketNo);
	virtual unsigned long numPackets();

	/// Get the numbers stamped packet from the page.
	StampedOggPacket* getStampedPacket(unsigned long inPacketNo);

	//TODO::: This really shouldn't be ehere.
	bool addPacket(StampedOggPacket* inPacket);
	
	/// Returns a pointer to the internal header.
	OggPageHeader* header();
	
	/// Creates a buffer of size pageSize and returns you a pointer to keep.
	unsigned char* createRawPageData();

	/// (Re-)compute the page's checksum and set it
	void computeAndSetCRCChecksum();

protected:
	//Member data of packets and header.
	vector<StampedOggPacket*> mPacketList;
	OggPageHeader* mHeader;

private:
	OggPage& operator=(const OggPage& other);  /* Don't assign me */
	OggPage(const OggPage& other); /* Don't copy me */
};
