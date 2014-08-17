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

#include <string>
using namespace std;
#include <StringHelper.h>

class OggPacket
{
public:
	//Constructors
	OggPacket(void);
	OggPacket(unsigned char* inPackData, unsigned long inPacketSize, bool inIsTruncated, bool inIsContinuation);
	virtual ~OggPacket(void);

	static const unsigned long HEX_DUMP_LINE_LENGTH = 16;

	/// Does a deep copy of this packet and returns a new one for you to keep.
	virtual OggPacket* clone();
					
	/// Returns the size of the contained packet.
	unsigned long packetSize() const;

	/// Returns a pointer to the internal packet buffer.
	unsigned char* packetData();
	
	/// Returns whether this packet is truncated.
	bool isTruncated() const;

	/// Returns whether this packet is continued from another one.
	bool isContinuation() const;

	/// Set the truncated flag on this packet.
	void setIsTruncated(bool inIsTruncated);

	/// Set the continuation flag on this packet.
	void setIsContinuation(bool inIsContinuation);

	/// Set the size of this packet.
	void setPacketSize (unsigned long inPacketSize );

	/// Give a buffer to kept as the internal packet buffer.
	void setPacketData (unsigned char* inPacketData );

	/// Merges this packet to another one you pass it.
	virtual void merge(const OggPacket* inMorePacket);

	/// Turns the packet into a hex dump string.
	string toPackDumpString();
	
protected:
	//Packet member data
	unsigned long mPacketSize;
	unsigned char* mPacketData;

	bool mIsTruncated;
	bool mIsContinuation;

	//TODO::Should these be here ?
	string OggPacket::dumpNCharsToString(unsigned char* inStartPoint, unsigned long inNumChars) ;
	string OggPacket::padField(string inString, unsigned long inPadWidth, unsigned char inPadChar);

private:
	OggPacket& operator=(const OggPacket& other);  /* Don't assign me */
	OggPacket(const OggPacket& other); /* Don't copy me */
	
};
