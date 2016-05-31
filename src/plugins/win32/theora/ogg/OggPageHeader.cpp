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
#include <OggPageHeader.h>


OggPageHeader::OggPageHeader(void)
	:	mPageSize(0)
	,	mHeaderSize(0)
	,	mDataSize(0)
	,	mStructureVersion(0)
	,	mHeaderFlags(0)
	,	mStreamSerialNo(0)
	,	mPageSequenceNo(0)
	,	mCRCChecksum(0)
	,	mPageState(BLANK)
	,	mGranulePos(0)
	,	mSegmentTable(NULL)
	,	mNumPageSegments(0)
		
{

}

OggPageHeader::~OggPageHeader(void)
{
	delete[] mSegmentTable;
}

//Gives a pointer to the caller.
OggPageHeader* OggPageHeader::clone() {

	OggPageHeader* retClone = new OggPageHeader();
	retClone->mCRCChecksum = mCRCChecksum;
	retClone->mDataSize = mDataSize;
	retClone->mGranulePos = mGranulePos;
	retClone->mHeaderFlags = mHeaderFlags;
	retClone->mHeaderSize = mHeaderSize;
	retClone->mNumPageSegments = mNumPageSegments;
	retClone->mPageSequenceNo = mPageSequenceNo;
	retClone->mPageSize = mPageSize;
	retClone->mPageState = mPageState;
	retClone->mStreamSerialNo = mStreamSerialNo;
	retClone->mStructureVersion = mStructureVersion;

	//Copy the segment table.
	unsigned char* locBuff = new unsigned char[mNumPageSegments];			//Stored in the returned classes member variable, deleted in it's destructor
	memcpy((void*)locBuff, (const void*)mSegmentTable, mNumPageSegments);
	retClone->mSegmentTable = locBuff;
	locBuff = NULL;		//Given away.
	//

	return retClone;
}


unsigned long OggPageHeader::calculateDataSize() {
	//Sums the bytes in the segment table to calculate the size of data.
	//FIX::: ??? No checks on pointers.

	unsigned long retDataSize = 0;
	for (int i = 0; i < mNumPageSegments; i++) {
		retDataSize += mSegmentTable[i];
	}

	setDataSize(retDataSize);
	return retDataSize;
}
//Views pointer data only
bool OggPageHeader::setBaseHeader(const unsigned char* inBaseHeader) {
	//This now does not delete the buffer
	
	//unsigned long locOffset = 0;  // unused

	//Check if the page has the correct capture pattern
	if (strncmp((const char*)inBaseHeader, "OggS", OGG_CAPTURE_PATTERN_SIZE) == 0) {
		mStructureVersion = inBaseHeader[STRUCTURE_VERSION];
		mHeaderFlags = inBaseHeader[HEADER_FLAGS];
		mGranulePos = iLE_Math::CharArrToInt64(inBaseHeader + GRANULE_POS);
		mStreamSerialNo = iLE_Math::charArrToULong(inBaseHeader + SERIAL_NO);
		mPageSequenceNo = iLE_Math::charArrToULong(inBaseHeader + SEQUENCE_NO);
		mCRCChecksum = iLE_Math::charArrToULong(inBaseHeader + OGG_CHECKSUM);
		mNumPageSegments = inBaseHeader[NUM_SEGMENTS];
		mHeaderSize = OGG_BASE_HEADER_SIZE + mNumPageSegments;
		
		mPageSize = mHeaderSize + mDataSize;
		mPageState = BASE_HEAD_SET;
		return true;
	} else {
		return false;
	}

		////Assign the structure version
		//setStructureVersion(inBaseHeader[locOffset]);
		//locOffset++;

		////Assign the header flags
		//setHeaderFlags(inBaseHeader[locOffset]);
		//locOffset++;

		////Assign the granule pos	
		//setGranulePos((const unsigned char*)(inBaseHeader + locOffset));
		//locOffset += 8;

		////Assign Serial No
		//setStreamSerialNo((const unsigned char*)(inBaseHeader + locOffset));
		//locOffset += 4;

		////Assign Page Seq No	
		//setPageSequenceNo(inBaseHeader + locOffset);
		//locOffset += 4;

		////Assign CheckSum
		//setCRCChecksum(inBaseHeader + locOffset);
		//locOffset += 4;

		////Assign Num Page Segments
		//setNumPageSegments(inBaseHeader[locOffset]);
		//locOffset++;

		////Set the size of the header
		//setHeaderSize(OGG_BASE_HEADER_SIZE + mNumPageSegments);

		//mPageState = BASE_HEAD_SET;

				
	

}


void OggPageHeader::setPageState(ePageState inPageState) 
{
	mPageState = inPageState;
}
unsigned char OggPageHeader::StructureVersion() 
{
	return mStructureVersion;
}

unsigned char OggPageHeader::HeaderFlags() 
{
	return mHeaderFlags;
}

LOOG_INT64 OggPageHeader::GranulePos()
{
	return mGranulePos;
}

unsigned long OggPageHeader::StreamSerialNo()
{
	return mStreamSerialNo;
}
unsigned long OggPageHeader::PageSequenceNo()
{
	return mPageSequenceNo;
}
unsigned long OggPageHeader::CRCChecksum()
{
	return mCRCChecksum;
}

unsigned char OggPageHeader::NumPageSegments()
{
	return mNumPageSegments;
}
unsigned char* OggPageHeader::SegmentTable()
{
	return mSegmentTable;
}



void OggPageHeader::setStructureVersion(unsigned char inVal) 
{
	mStructureVersion = inVal;
}

void OggPageHeader::setHeaderFlags(unsigned char inVal) 
{
	mHeaderFlags = inVal;
}

void OggPageHeader::setGranulePos(LOOG_INT64 inGranulePos)
{
	mGranulePos = inGranulePos;
}
void OggPageHeader::setGranulePos(const unsigned char* inPtr)
{
	mGranulePos = iLE_Math::CharArrToInt64(inPtr);
}

void OggPageHeader::setStreamSerialNo(unsigned long inVal)
{
	mStreamSerialNo = inVal;
}
void OggPageHeader::setStreamSerialNo(const unsigned char* inPtr)
{
	mStreamSerialNo = iLE_Math::charArrToULong(inPtr);
}
void OggPageHeader::setPageSequenceNo(unsigned long inVal)
{
	mPageSequenceNo = inVal;
}
void OggPageHeader::setPageSequenceNo(const unsigned char* inPtr)
{
	mPageSequenceNo = iLE_Math::charArrToULong(inPtr);;
}

void OggPageHeader::setCRCChecksum(unsigned long inVal)
{
	mCRCChecksum = inVal;
}
void OggPageHeader::setCRCChecksum(const unsigned char* inPtr)
{
	mCRCChecksum = iLE_Math::charArrToULong(inPtr);;
}

void OggPageHeader::setNumPageSegments(unsigned char inVal)
{
	if (mSegmentTable == NULL) {
		mNumPageSegments = inVal;
	}
}
///
//	Copies the data from the pointer it is given.
//	
void OggPageHeader::setSegmentTable(const unsigned char* inPtr, unsigned char inNumSegs) 
{
	unsigned char* locSegTable = new unsigned char[inNumSegs];		//This is stored in the member variable and deleted in the destructor.
	memcpy((void*)locSegTable, (const void*)inPtr, inNumSegs);
	delete[] mSegmentTable;
	mSegmentTable = locSegTable;
	mNumPageSegments = inNumSegs;
	setHeaderSize(OggPageHeader::OGG_BASE_HEADER_SIZE + inNumSegs);
}
void OggPageHeader::setSegmentTable(unsigned char* inPtr) {
	delete[] mSegmentTable;
	mSegmentTable = inPtr;

}

OggPageHeader::ePageState OggPageHeader::pageState() {
	return mPageState;
}
unsigned long OggPageHeader::pageSize()
{
	return mPageSize;
}
unsigned long OggPageHeader::headerSize()
{
	return mHeaderSize;
}
unsigned long OggPageHeader::dataSize()
{
	return mDataSize;
}
bool OggPageHeader::rawData(unsigned char* outData, unsigned long inBuffSize) {
	
	//0-3			CapPattern						"OggS"
	//4				Struct Ver
	//5				Head Flags
	//6-13			Granule Pos
	//14-17			Stream Serial No
	//18-21			Page Seq No
	//22-25			CheckSum
	//26			Num Segments
	//27...			SegmentTable
	//

	if (mHeaderSize > inBuffSize) {
		return false;
	}
	outData[0] = 'O';
	outData[1] = 'g';
	outData[2] = 'g';
	outData[3] = 'S';
	outData[4] = mStructureVersion;
	outData[5] = mHeaderFlags;
	iLE_Math::Int64ToCharArr(mGranulePos, &outData[6]);
	iLE_Math::ULongToCharArr(mStreamSerialNo, &outData[14]);
	iLE_Math::ULongToCharArr(mPageSequenceNo, &outData[18]);
	iLE_Math::ULongToCharArr(mCRCChecksum, &outData[22]);
	outData[26] = mNumPageSegments;

	//TODO::: Validate the length of all this.
	memcpy((void*)(outData + 27), (const void*)mSegmentTable, mNumPageSegments);

	return true;
}

//unsigned long OggPageHeader::headerCheckSum() 
//{
//	
//
//}


bool OggPageHeader::isBOS() {
	return ((mHeaderFlags & BOS) != 0 );
}
bool OggPageHeader::isEOS() {
	return ((mHeaderFlags & EOS) != 0 );
}

bool OggPageHeader::isContinuation() {
	return ((mHeaderFlags & CONTINUATION) != 0 );
}


string OggPageHeader::toString() {

	string retStr =	"Ver No      : " + StringHelper::numToString((unsigned int)mStructureVersion) + "\n";
	
	retStr +=		"Head Flags  :";
	if ((mHeaderFlags & CONTINUATION) != 0) {
		retStr += " continuation";
	}
	if ((mHeaderFlags & BOS) != 0) {
		retStr += " bos";
	}
	if ((mHeaderFlags & EOS) != 0) {
		retStr += " eos";
	}
	retStr += "\n";

	retStr +=		"Granule Pos : " + StringHelper::numToString(mGranulePos) + "\n";
	retStr +=		"Serial No   : " + StringHelper::numToString(mStreamSerialNo) + "\n";
	retStr +=		"Seq No      : " + StringHelper::numToString(mPageSequenceNo) + "\n";
	retStr +=		"Checksum    : " + StringHelper::numToString(mCRCChecksum) + "\n";
	retStr +=		"Num Segs    : " + StringHelper::numToString((unsigned int)mNumPageSegments) + "\n";
	retStr +=		"------------------------\n";
	retStr +=		"Head Size   : " + StringHelper::numToString(mHeaderSize) + "\n";
	retStr +=		"Data Size   : " + StringHelper::numToString(mDataSize) + "\n";
	retStr +=		"Page Size   : " + StringHelper::numToString(mPageSize) +"\n";
	
	return retStr;
}


//This could be unsigned short.
void OggPageHeader::setHeaderSize(unsigned long inVal)
{
	mHeaderSize = inVal;
	mPageSize = mHeaderSize + mDataSize;
}
void OggPageHeader::setDataSize(unsigned long inVal)
{
	mDataSize = inVal;
	mPageSize = mHeaderSize + mDataSize;
}

