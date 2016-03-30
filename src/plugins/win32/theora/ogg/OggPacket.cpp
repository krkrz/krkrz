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

#include "oggstdafx.h"
#include <OggPacket.h>

//LEAK CHECK::: 20041018 - OK.
OggPacket::OggPacket(void)
	:	mPacketSize(0)
	,	mPacketData(NULL)
	,	mIsContinuation(false)
	,	mIsTruncated(false)
		
{

}

//Accepts responsibility for inPackData pointer - deletes it in destructor.
OggPacket::OggPacket(unsigned char* inPackData, unsigned long inPacketSize, bool inIsTruncated, bool inIsContinuation)
	:	mPacketSize(inPacketSize)
	,	mPacketData(inPackData)
	,	mIsContinuation(inIsContinuation)
	,	mIsTruncated(inIsTruncated)
		
{
}

//This method creates a pointer which the caller is responsible for.
OggPacket* OggPacket::clone() {
	//Make a new buffer for packet data
	unsigned char* locBuff = new unsigned char[mPacketSize];			//Given to constructor of OggPacket

	//Copy the packet data into the new buffer
	memcpy((void*)locBuff, (const void*)mPacketData, mPacketSize);

	//Create the new packet - accepts locBuff poitner
	OggPacket* retPack = new OggPacket(locBuff, mPacketSize, mIsTruncated, mIsContinuation);	//Gives this pointer to the caller.
	return retPack;
}


OggPacket::~OggPacket(void)
{
	//Now deletes the packetData
	delete[] mPacketData;

	//Should be careful about allowing this to be copied...
}

string OggPacket::toPackDumpString() {
	string retStr = "";


	///NOTE::: ShOuld be reworked.
	//Needs dataSize and data pointer

	//Put the stream in hex mode with a fill character of 0
	
	//cout << setfill('0');

	//Loop through every character of data
	for (unsigned long i = 0; i < mPacketSize; i++) {
		//If it is the end of the previous hex dump line or first line)
		if ( (i % HEX_DUMP_LINE_LENGTH == 0) ) {
			//And this is not the first line
			if ( i != 0 ) {
				//Put the characters on the end
				retStr.append(dumpNCharsToString( &mPacketData[i - HEX_DUMP_LINE_LENGTH],  HEX_DUMP_LINE_LENGTH));
			}

			//At the start of the line write out the base address in an 8 hex-digit field 
			//NOTE::: Just decimal for now.
			//cout << setw(8) << i << ": ";
			retStr.append(padField(StringHelper::numToString(i), 8, '0') + ": ");
		}

		//Write out the value of the character in a 2 hex-digit field
		//cout << setw(2) << (int)mPageData[i] << " ";
		retStr.append(StringHelper::charToHexString(mPacketData[i]) + " ");
	}

	//Find out how many leftover charcters didn't get written out.
	unsigned long locLeftovers = (mPacketSize % HEX_DUMP_LINE_LENGTH);

	locLeftovers = (locLeftovers > 0)	? (locLeftovers)	
										: (HEX_DUMP_LINE_LENGTH);


	//If there was any data in this dump
	if ( mPacketSize > 0 ) {
		//Dump the last part out
		retStr.append(dumpNCharsToString( &mPacketData[mPacketSize - locLeftovers], locLeftovers ));
	}

	retStr+= "==============================================================================\n" ;
	//Put the stream back to decimal mode
	//dec(cout);

	return retStr;
}

string OggPacket::padField(string inString, unsigned long inPadWidth, unsigned char inPadChar) {
	//NOTE::: Need check for string being  bigger than pad space
	string retStr = "";
	retStr.append(inPadWidth - inString.length(), inPadChar);
	retStr.append(inString);

	return retStr;
}
string OggPacket::dumpNCharsToString(unsigned char* inStartPoint, unsigned long inNumChars) {
	//NOTE::: Also needs reworking
	//const unsigned char BELL = 7;  // unused
	//Set the fill character back to space ' '
	//cout << setfill(' ');


	//Put some space after the hex section
	unsigned long locPadding = 3 * (HEX_DUMP_LINE_LENGTH - inNumChars) + 4;
	//cout << setw(locPadding) << "    ";
	
	string retStr = padField("    ", locPadding, ' ');   
	
	//Loop through the characters
	for (unsigned long i = 0; i < inNumChars; i++) {

		//If they are *not* going to mess up the layout (i.e. the thing is printable)
		if ( (inStartPoint[i] >= 32) && (inStartPoint[i] <= 126) ) {
			//Write them out
			retStr += (char)inStartPoint[i];						
		} else {
			//Otherwise just write a placeholder char
			retStr += ((char) '.');
		}
	}
	retStr += "\n";
	
	return retStr;
	
}



unsigned long OggPacket::packetSize() const {
	return mPacketSize;
}

/** Note that you should reset the checksum on the Ogg page via
    OggPage::computeAndSetCRCChecksum() if you change the packet data, otherwise
	you'll end up with an invalid page.  (Arguably this should automatically
	be done for you, but that's the way it is for now.) */
unsigned char* OggPacket::packetData() {
	return mPacketData;
}
bool OggPacket::isTruncated() const {
	return mIsTruncated;
}
bool OggPacket::isContinuation() const {
	return mIsContinuation;
}

//bool OggPacket::isComplete() const {
//	return mIsComplete;
//}

//void OggPacket::setIsComplete(bool inIsComplete) {
//	mIsComplete = inIsComplete;
//}
void OggPacket::setPacketSize(unsigned long inPacketSize) {
	mPacketSize = inPacketSize;
}

/** This function accepts responsibility for the pointer it is passed, and it
    deletes it in the destructor.  Note that you should reset the checksum on
	the Ogg page via OggPage::computeAndSetCRCChecksum(), otherwise you'll end up
	with an invalid page.  (Arguably this should automatically be done for you,
	but that's the way it is for now.) */
void OggPacket::setPacketData(unsigned char* inPacketData) {
	mPacketData = inPacketData;
}

//Only views the incoming pointer.
void OggPacket::merge(const OggPacket* inMorePacket) {
	//Make a new buffer the size of both data segs together
	unsigned char* locBuff = new unsigned char[mPacketSize + inMorePacket->mPacketSize];		//This is put into the member vvariable, where it will be deleted in destructor.
	//Copy this packets data to the start
	memcpy((void*)locBuff, (const void*)mPacketData, mPacketSize);
	//Copy the next packets data after it
	memcpy((void*)(locBuff + mPacketSize), (const void*)inMorePacket->mPacketData, inMorePacket->mPacketSize);
	//Delete our original packet data
	delete[] mPacketData;
	//Now make our data be the combined data
	mPacketData = locBuff;
	//Make the size the sum of both packets
	mPacketSize += inMorePacket->mPacketSize;

	//If the next part of the packet isn't complete then this packet is not complete.
	//mIsComplete = inMorePacket->mIsComplete;
	//The new packet is truncated only if the incoming packet is
	mIsTruncated = inMorePacket->mIsTruncated;    //->isTruncated();

	//This is not a continuation... a continuation is a packet that does not start at the start of the real packet.
	mIsContinuation = false;

}