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
#include <iLE_Math.h>
#include <StringHelper.h>
//#include "OggSegmentTable.h"
//#include "OggInt64.h"
//#include "OggMath.h"


using namespace std;

#define OGG_CAPTURE_PATTERN "Oggs"
class OggPageHeader			//Do not derive - or needs virtual destructor.
{
public:
	OggPageHeader(void);
	~OggPageHeader(void);
	OggPageHeader* clone();
	//0-3			CapPattern						"Oggs"
	//4				Struct Ver
	//5				Head Flags
	//6-13			Granule Pos
	//14-17			Stream Serial No
	//18-21			Page Seq No
	//22-25			CheckSum
	//26			Num Segments
	//27...			SegmentTable

	static const int MAX_NUM_SEGMENTS = 255;
	static const int MAX_SEGMENT_SIZE = 255;
	static const int SEGMENT_WIDTH = 1;

	static const unsigned char OGG_CAPTURE_PATTERN_SIZE = 4;
	static const unsigned char OGG_BASE_HEADER_SIZE = 27;
	static const LOOG_INT64 UNKNOWN_GRANULE_POS = -1;
	enum ePageState {
		INVALID,
		BLANK,
		BASE_HEAD_SET,
		FULL_HEAD_SET,
		COMPLETE
	};

	enum eHeadFlags {
		NO_FLAGS = 0,
		CONTINUATION = 1,
		BOS = 2,
		EOS = 4
	};

	
	enum eFieldOffsets {
		CAPTURE_PATTURN = 0,
		STRUCTURE_VERSION = 4,
		HEADER_FLAGS = 5,
		GRANULE_POS = 6,
		SERIAL_NO = 14,
		SEQUENCE_NO = 18,
		OGG_CHECKSUM = 22,
		NUM_SEGMENTS = 26,
		SEGMENT_TABLE = 27
	};
	

	bool rawData(unsigned char* outData, unsigned long inBuffSize);
	
	//State variables
	ePageState pageState();
	void setPageState(ePageState inPageState);

	//Access header flags
	bool isContinuation();
	bool isBOS();
	bool isEOS();	

	//Size variables	
	unsigned long pageSize();
	unsigned long headerSize();
	unsigned long dataSize();
	unsigned long calculateDataSize();
	
	//Bulk Mutators
	bool setBaseHeader(const unsigned char* inBaseHeader);
	
	//Accesors
	unsigned char StructureVersion();
	unsigned char HeaderFlags();
	LOOG_INT64 GranulePos();
	unsigned long StreamSerialNo();
	unsigned long PageSequenceNo();
	unsigned long CRCChecksum();
	unsigned char NumPageSegments();
	unsigned char* SegmentTable();

	//Mutators
	void setStructureVersion(unsigned char inVal);
	void setHeaderFlags(unsigned char inVal);
	void setGranulePos(LOOG_INT64 inPtr);
	void setGranulePos(const unsigned char* inPtr);
	void setStreamSerialNo(unsigned long inVal);
	void setStreamSerialNo(const unsigned char* inPtr);
	void setPageSequenceNo(unsigned long inVal);
	void setPageSequenceNo(const unsigned char* inPtr);
	void setCRCChecksum(unsigned long inVal);
	void setCRCChecksum(const unsigned char* inPtr);
	void setNumPageSegments(unsigned char inVal);
	void setSegmentTable(unsigned char* inPtr);
	void setSegmentTable(const unsigned char* inPtr, unsigned char inNumSegs);

	void setHeaderSize(unsigned long inVal);
	void setDataSize(unsigned long inVal);


	//unsigned long headerCheckSum();
	string toString();
protected:
	//Size fields
	unsigned long mPageSize;
	unsigned long mHeaderSize;
	unsigned long mDataSize;

	//Header fields
	unsigned char mStructureVersion;
	unsigned char mHeaderFlags;
	LOOG_INT64 mGranulePos;
	unsigned long mStreamSerialNo;
	unsigned long mPageSequenceNo;
	unsigned long mCRCChecksum;
	unsigned char mNumPageSegments;

	//Segment table
	unsigned char* mSegmentTable;
	
	//Page state
	ePageState mPageState;
private:
	OggPageHeader& operator=(const OggPageHeader& other);  /* Don't assign me */
	OggPageHeader(const OggPageHeader& other); /* Don't copy me */
};
