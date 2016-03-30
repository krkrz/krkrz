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
#include <AutoOggSeekTable.h>

#ifdef UNICODE
AutoOggSeekTable::AutoOggSeekTable(wstring inFileName)
#else
AutoOggSeekTable::AutoOggSeekTable(string inFileName)
#endif
	:	mFilePos(0)
	,	mLastSeekTime(0)
	,	mPacketCount(0)
	,	mSampleRate(0)
	,	mFileDuration(0)
	,	mNumHeaders(0)
	,	mSerialNoToTrack(LINT_MAX)
	,	isTheora(false)
	,	isFLAC(false)
	,	isOggFLAC_1_0(false)
	,	mFoundStreamInfo(false)
	,	mGranulePosShift(0)
	,	mLastIsSeekable(false)

{

	mFileName = inFileName;
	mOggDemux = new OggDataBuffer();			//Deleted in destructor.
	mOggDemux->registerVirtualCallback(this);
	//debugLog.open("G:\\logs\\seektable.log", ios_base::out);
	//debugLog<<"Constructing seek table for "<<inFileName<<endl;
}

AutoOggSeekTable::~AutoOggSeekTable(void)
{
	//debugLog<<"Closing file (Constructor)..."<<endl;
	//debugLog.close();
	mFile.close();
	delete mOggDemux;
}

bool AutoOggSeekTable::acceptOggPage(OggPage* inOggPage) {			//Correctly deletes page.
	

	//TODO ::: Some of this could be shared from other places.
	if (!mFoundStreamInfo) {
		if (strncmp((const char*)inOggPage->getPacket(0)->packetData(), "\001vorbis", 7) == 0) {
			mSampleRate = iLE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 12);
			mNumHeaders = 3;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			mFoundStreamInfo = true;
		} else if (strncmp((const char*)inOggPage->getPacket(0)->packetData(), "Speex   ", 8) == 0) {
			mSampleRate = iLE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 36);
			mNumHeaders = 2;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			mFoundStreamInfo = true;
		} else if ((strncmp((char*)inOggPage->getPacket(0)->packetData(), "\200theora", 7)) == 0){
			//FIX ::: Dunno what this is... do something better than this later !!
			//mEnabled = false;
			//mPacketCount == 0;
			isTheora = true;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			mGranulePosShift = (((inOggPage->getPacket(0)->packetData()[40]) % 4) << 3) + ((inOggPage->getPacket(0)->packetData()[41]) >> 5);
			mSampleRate = iBE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 22) / iBE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 26);
			mNumHeaders = 3;
			mFoundStreamInfo = true;
			
			//Need denominators
			//mTheoraFormatBlock->frameRateDenominator = FLACMath::charArrToULong(locIdentHeader + 26);
		} else if ((strncmp((char*)inOggPage->getPacket(0)->packetData(),  "fLaC", 4) == 0)) {
			//mPacketCount--;
			mNumHeaders = 1;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			isFLAC = true;
		} else if (isFLAC && (mSerialNoToTrack == inOggPage->header()->StreamSerialNo()) ) {
			//Loop any other packets

			const int FLAC_LAST_HEADERS_FLAG = 128;
			const int FLAC_HEADER_MASK = 127;
			const int FLAC_STREAM_INFO_ID = 0;
			
			//Note ::: Secondary condition in for statement.
            for (unsigned int i = 0; i < inOggPage->numPackets(), !mFoundStreamInfo; i++) {
				mNumHeaders++;
				if ((inOggPage->getPacket(i)->packetData()[0] & FLAC_HEADER_MASK) == FLAC_STREAM_INFO_ID) {
                    //Catch the stream info packet.
                    mSampleRate = iBE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 14) >> 12;
					//mFoundStreamInfo = true;
				}
				if ((inOggPage->getPacket(i)->packetData()[0] & FLAC_LAST_HEADERS_FLAG)) {
					mFoundStreamInfo = true;
				}
			}
		} else if ((strncmp((char*)inOggPage->getPacket(0)->packetData(),  "\177FLAC", 5) == 0)) {
			//debugLog<<"Identified new flac..."<<endl;
			//mPacketCount--;
			//POTENTIAL BUG::: Only looks at low order byte
			mNumHeaders = inOggPage->getPacket(0)->packetData()[8] + 1;
			//debugLog<<"Header says there are this many headers "<<mNumHeaders<<endl;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			if (mNumHeaders == 0) {
				//Variable number of headers... need to pick it up again...
				//debugLog<<"Variable number of headers... 1 so far..."<<endl;
				mNumHeaders = 1;
                isOggFLAC_1_0 = true;
			} else {
				//debugLog<<"Fixed number of headers..."<<endl;
				mFoundStreamInfo = true;
			}
			mSampleRate = iBE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 27) >> 12;
		} else if (isOggFLAC_1_0 && (mSerialNoToTrack == inOggPage->header()->StreamSerialNo()) ) {
			//Loop any other packets

			const int FLAC_LAST_HEADERS_FLAG = 128;
			// const int FLAC_HEADER_MASK = 127;  // Unused
			// const int FLAC_STREAM_INFO_ID = 0;  // Unused
			
			//Note ::: Secondary condition in for statement.
            for (unsigned int i = 0; i < inOggPage->numPackets(), !mFoundStreamInfo; i++) {
				mNumHeaders++;

				//Don't need this, we already got this data... we're just counting headers.
				//if ((inOggPage->getPacket(i)->packetData()[0] & FLAC_HEADER_MASK) == FLAC_STREAM_INFO_ID) {
    //                //Catch the stream info packet.
    //                mSampleRate = iBE_Math::charArrToULong(inOggPage->getPacket(0)->packetData() + 14) >> 12;
				//}
				if ((inOggPage->getPacket(i)->packetData()[0] & FLAC_LAST_HEADERS_FLAG)) {
					mFoundStreamInfo = true;
				}
			}
			
		} else if ((strncmp((char*)inOggPage->getPacket(0)->packetData(), "\001video\000\000\000", 9)) == 0) {
			//FFDSHOW
			LOOG_INT64 locTimePerBlock = iLE_Math::CharArrToInt64(inOggPage->getPacket(0)->packetData() + 17);
			LOOG_INT64 locSamplesPerBlock = iLE_Math::CharArrToInt64(inOggPage->getPacket(0)->packetData() + 25);

			//TODO::: The division is the wrong way round !!
			mSampleRate = (unsigned long) ( (10000000 / locTimePerBlock) * locSamplesPerBlock );
			mFoundStreamInfo = true;
			mSerialNoToTrack = inOggPage->header()->StreamSerialNo();
			mNumHeaders = 1;

		} else {
			mFoundStreamInfo = true;		//Why do this ?
			mEnabled = false;
			mSampleRate = 1;
			
		}
	}

	if (mSerialNoToTrack == inOggPage->header()->StreamSerialNo()) {
		mPacketCount += inOggPage->numPackets();
	}


	if ((mFoundStreamInfo) && (mSerialNoToTrack == inOggPage->header()->StreamSerialNo()) && (inOggPage->header()->GranulePos() != -1)) {
		//if ((mPacketCount > 3) && (mLastIsSeekable == true)) {
		//debugLog<<"Stream headers complete..."<<endl;
		//debugLog<<"Num Headers = "<<mNumHeaders<<endl;
		//debugLog<<"Packet COunt = "<<mPacketCount<<endl;
		if ((mPacketCount > mNumHeaders) && ((inOggPage->header()->HeaderFlags() & 1) != 1)) {
			//debugLog<<"Adding seek point Time = "<<mLastSeekTime<<"  --  File pos = "<<mFilePos<<endl;
			addSeekPoint(mLastSeekTime, mFilePos);
			
		}

		mLastIsSeekable = true;
		
		if (isTheora) {
			unsigned long locMod = (unsigned long)pow((double) 2, (double) mGranulePosShift);
			unsigned long locInterFrameNo = (unsigned long) ( (inOggPage->header()->GranulePos()) % locMod );
			
			//if (locInterFrameNo == 0) {
			//	mLastIsSeekable = true;
			//} else {
			//	mLastIsSeekable = false;
			//}
			mLastSeekTime = ((((inOggPage->header()->GranulePos()) >> mGranulePosShift) + locInterFrameNo) * DS_UNITS) / mSampleRate;
		} else {
			mLastSeekTime = ((inOggPage->header()->GranulePos()) * DS_UNITS) / mSampleRate;
			//stDebug<<"Last Seek Time : "<<mLastSeekTime;
		}
		if (((inOggPage->header()->HeaderFlags() & 1) == 1)) {
			//stDebug <<"    NOT SEEKABLE";
			mLastIsSeekable = false;
		}
		//stDebug<<endl;
		mFileDuration = mLastSeekTime;
		
	}
	mFilePos += inOggPage->pageSize();
	//stDebug<<"File Pos : "<<mFilePos<<endl;

	//Memory leak ::: Need to delete the page.
	delete inOggPage;

	return true;
}
unsigned long AutoOggSeekTable::serialisedSize() {
	// TODO: This really should return a size_t (in fact, all our file-size-related variables should really be
	// a size_t), but let's just use ye olde unsigned long for now ...
	return (unsigned long) mSeekMap.size() * 12;
	
}
bool AutoOggSeekTable::serialiseInto(unsigned char* inBuff, unsigned long inBuffSize) {
	if (inBuffSize >= serialisedSize()) {
		unsigned long locUpto = 0;
		for (tSeekMap::const_iterator i = mSeekMap.begin(); i != mSeekMap.end(); i++) {

			//Time is .first
			iLE_Math::Int64ToCharArr((*i).first, inBuff + locUpto);
			locUpto += 8;

			//Byte offset is .second
			iLE_Math::ULongToCharArr((*i).second, inBuff + locUpto);
			locUpto += 4;
		}
		return true;
	} else {
		return false;
	}
}

#ifdef UNICODE
bool AutoOggSeekTable::serialiseInto(const wstring inSeekTableFilename)
#else
bool AutoOggSeekTable::serialiseInto(const string inSeekTableFilename)
#endif
{
	unsigned long locSerialisedSeekTableSize = serialisedSize();
	unsigned char *locBuffer = new unsigned char[locSerialisedSeekTableSize];

	if (serialiseInto(locBuffer, locSerialisedSeekTableSize)) {
		fstream locOutputFile;

		locOutputFile.open(inSeekTableFilename.c_str(), ios_base::out | ios_base::binary);
		locOutputFile.write((char*)locBuffer, locSerialisedSeekTableSize);
		locOutputFile.close();
	} else {
		delete [] locBuffer;
		return false;
	}

	delete [] locBuffer;
	return true;
}


LOOG_INT64 AutoOggSeekTable::fileDuration()
{
	return mFileDuration;
}

bool AutoOggSeekTable::buildTable()
{
	//HACK::: To ensure we don't try and build a table on the network file.
	//debugLog<<"Anx Build table : "<<mFileName<<endl;
	if (mFileName.find(TEXT("http")) != 0) {
		
		mSeekMap.clear();
		addSeekPoint(0, 0);
		//debugLog<<"Opening file... "<<endl;
		mFile.open(mFileName.c_str(), ios_base::in | ios_base::binary);
		const unsigned long BUFF_SIZE = 4096;
		unsigned char* locBuff = new unsigned char[BUFF_SIZE];		//Deleted this function.
		while (!mFile.eof()) {
			mFile.read((char*)locBuff, BUFF_SIZE);
			mOggDemux->feed((const unsigned char*)locBuff, (unsigned long)mFile.gcount());
		}
		delete[] locBuff;
		//debugLog<<"Closing File..."<<endl;
		mFile.close();
		
	} else {
		//debugLog<<"Not SEEKABLE"<<endl;
		mEnabled = false;
		mSampleRate = 1;
	}

	return true;
}

bool AutoOggSeekTable::buildTableFromBuffer(const unsigned char *inBuffer, const unsigned long inBufferSize)
{
	for (const unsigned char *locBufferPosition = inBuffer; locBufferPosition < inBuffer + inBufferSize; ) {
		LOOG_INT64 locTimePoint = iLE_Math::CharArrToInt64(locBufferPosition);
		locBufferPosition += 8;

		unsigned long locBytePosition = iLE_Math::charArrToULong(locBufferPosition);
		locBufferPosition += 4;

		addSeekPoint(locTimePoint, locBytePosition);
	}

	return true;
}

/** Note that this method does not do any verification that the file is
    up-to-date or valid (i.e. if you are using the serialised seek table
	as a cache, you must check yourself that the cached seek table is not
	out of date).
  */

#ifdef UNICODE
bool AutoOggSeekTable::buildTableFromFile(const wstring inCachedSeekTableFilename)
#else
bool AutoOggSeekTable::buildTableFromFile(const string inCachedSeekTableFilename)
#endif
{
	LOOG_INT64 locTimePoint;
	unsigned long locBytePosition;

	fstream locSeekFile;
	locSeekFile.open(inCachedSeekTableFilename.c_str(), ios_base::in | ios_base::binary);

	// Look ma, we got us zergling-size buffer
	unsigned char* locBuffer = new unsigned char[16];

	while (!locSeekFile.eof()) {
		locSeekFile.read((char*)locBuffer, 8);
		if (locSeekFile.gcount() == 8) {
			locTimePoint = iLE_Math::CharArrToInt64(locBuffer);

			locSeekFile.read((char*)locBuffer, 4);
			if (locSeekFile.gcount() == 4) {
				locBytePosition = iLE_Math::charArrToULong(locBuffer);
			} else {
				delete[] locBuffer;
				return false;
			}

			addSeekPoint(locTimePoint, locBytePosition);
		} else {
			delete [] locBuffer;
			return false;
		}
	}

	delete [] locBuffer;

	return true;
}

