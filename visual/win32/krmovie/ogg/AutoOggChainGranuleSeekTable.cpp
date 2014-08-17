#include "oggstdafx.h"
#include "autooggchaingranuleseektable.h"
#include <limits>
#undef max

#ifdef UNICODE
AutoOggChainGranuleSeekTable::AutoOggChainGranuleSeekTable(wstring inFilename)
#else
AutoOggChainGranuleSeekTable::AutoOggChainGranuleSeekTable(string inFilename)
#endif
	:	mFilename(inFilename)
	,	mFilePos(0)
	,	mOggDemux(NULL)
	,	mDuration(0)
	,	mPreviousFilePos(0)
	,	mIsEnabled(false)
{
	mOggDemux = new OggDataBuffer;
	mOggDemux->registerVirtualCallback(this);

}

AutoOggChainGranuleSeekTable::~AutoOggChainGranuleSeekTable(void)
{
	delete mOggDemux;

	for (size_t i = 0; i < mStreamMaps.size(); i++) {
		delete mStreamMaps[i].mSeekTable;
	}
}


bool AutoOggChainGranuleSeekTable::buildTable()
{
	if (mFilename.find(TEXT("http")) != 0) {
		
		//mSeekMap.clear();
		//addSeekPoint(0, 0);

		mFile.open(mFilename.c_str(), ios_base::in | ios_base::binary);
		//TODO::: Error check
		const unsigned long BUFF_SIZE = 4096;
		unsigned char* locBuff = new unsigned char[BUFF_SIZE];		//Deleted this function.
		while (!mFile.eof()) {
			mFile.read((char*)locBuff, BUFF_SIZE);
			mOggDemux->feed((const unsigned char*)locBuff, (unsigned long)mFile.gcount());
		}
		delete[] locBuff;

		mFile.close();
		mIsEnabled = true;
		
	} else {
		mIsEnabled = false;
	}
	return true;
}
OggGranuleSeekTable::tSeekPair AutoOggChainGranuleSeekTable::seekPos(LOOG_INT64 inTime)
{
    LOOG_INT64 retEarliestPos = std::numeric_limits<LOOG_INT64>::max();

	LOOG_INT64 locStreamTime = -1;
	bool locGotAValidPos = false;


	OggGranuleSeekTable::tSeekPair locSeekInfo;
	OggGranuleSeekTable::tSeekPair retBestSeekInfo;
	for (size_t i = 0; i < mStreamMaps.size(); i++) {

		if (mStreamMaps[i].mSeekTable != NULL && 
            mStreamMaps[i].mSeekInterface != NULL) 
        {
			//Get the preliminary seek info
			locSeekInfo = mStreamMaps[i].mSeekTable->getStartPos(inTime);
			//1. Get the granule pos in the preliminary seek
			//2. Ask the seek interface what granule we must seek before to make this a valid seek
			//		ie if preroll or keyframes, this value must be less than the original seek value
			//3. Convert the new granule to time
			//4. Repeat the seek
			locStreamTime = mStreamMaps[i].mSeekInterface->convertGranuleToTime(mStreamMaps[i].mSeekInterface->mustSeekBefore(locSeekInfo.second.second));
			locSeekInfo = mStreamMaps[i].mSeekTable->getStartPos(locStreamTime);

			if (retEarliestPos >= locSeekInfo.second.first) {
				//Update the earliest position
				retEarliestPos = locSeekInfo.second.first;
				retBestSeekInfo = locSeekInfo;
				locGotAValidPos = true;
			}
		}
	}	

	return retBestSeekInfo;//retEarliestPos;

}
LOOG_INT64 AutoOggChainGranuleSeekTable::fileDuration()
{
	return mDuration;
}

bool AutoOggChainGranuleSeekTable::isUnstampedPage(OggPage* inOggPage)
{
	//This handles all the broken files out there which incorrectly have non -1 gran pos
	//	when they should have -1.
	//
	//A page is now considered unstamped (and thus should according to the spec have -1 gran pos)
	//	if it does in fact have -1 gran pos
	//	OR 
	//	if there is only one packet, and that packet is truncated.
	if (inOggPage->header()->GranulePos() == -1) {
		return true;
	} else if ((inOggPage->numPackets() == 1) && (inOggPage->getPacket(0)->isTruncated())) {
		return true;
	} else {
		return false;
	}
}
bool AutoOggChainGranuleSeekTable::acceptOggPage(OggPage* inOggPage)
{
	//Get the granule pos of this page
	LOOG_INT64 locGranule = inOggPage->header()->GranulePos();

	//Get the serial number of this page
	unsigned long locSerialNo = inOggPage->header()->StreamSerialNo();

	sStreamMapping locMapping = getMapping(locSerialNo);

	//There can be upto 2 incomplete packets on any page, one at the end and one at the start
	unsigned long locNumBrokenPacks = (inOggPage->header()->isContinuation() ? 1 : 0);
	if (inOggPage->numPackets() > 0) {
		locNumBrokenPacks += (inOggPage->getPacket(inOggPage->numPackets() - 1)->isTruncated() ? 1 : 0);
	}
	//Exclude pages, with -1 granule pos, or that have no complete packets
	//if (locGranule != -1) { 
	if (!isUnstampedPage(inOggPage)) {
		LOOG_INT64 locRealTime = -1;
		if ((inOggPage->numPackets() > locNumBrokenPacks)) {
			
			if ((locMapping.mSeekInterface != NULL) && (locMapping.mSeekTable != NULL)) {
				//There is valid stream info
				locRealTime = locMapping.mSeekInterface->convertGranuleToTime(locGranule);
				if (locRealTime >= 0) {
					locMapping.mSeekTable->addSeekPoint(locRealTime, mFilePos, locGranule);
					if (locRealTime > mDuration) {
						mDuration = locRealTime;
					}
				}
			}
		} else {
			//If there's a granule pos, but no complete packets, there must at least be the end of a packet
			//	so mark  the seek point with the previous filepos from a page that had a packet start on it
			if ((locMapping.mSeekInterface != NULL) && (locMapping.mSeekTable != NULL)) {
				//There is valid stream info
				locRealTime = locMapping.mSeekInterface->convertGranuleToTime(locGranule);
				if (locRealTime >= 0) {
					locMapping.mSeekTable->addSeekPoint(locRealTime, mPreviousFilePos, locGranule);
					if (locRealTime > mDuration) {
						mDuration = locRealTime;
					}
				}
			}
		}
	}

	//Only remember the previous file position, if a packet started on this page, otherwise, we might
	//	use the start point of the previous page, and that previous page may have not had any packets
	//	on it.
	//
	//Any page that is not a continuation and has more than 1 packet, must have a packet starting on it
	if (!(inOggPage->header()->isContinuation() && (inOggPage->numPackets() <= 1))) {
		mPreviousFilePos = mFilePos;
	}
	mFilePos += inOggPage->pageSize();

	delete inOggPage;
	return true;
}
AutoOggChainGranuleSeekTable::sStreamMapping AutoOggChainGranuleSeekTable::getMapping(unsigned long inSerialNo)
{
	for (size_t i = 0; i < mStreamMaps.size(); i++) {
		if (mStreamMaps[i].mSerialNo == inSerialNo) {
			return mStreamMaps[i];
		}
	}

	sStreamMapping retMapping;
	retMapping.mSeekInterface = NULL;
	retMapping.mSeekTable = NULL;
	retMapping.mSerialNo = 0;

	return retMapping;
}
bool AutoOggChainGranuleSeekTable::addStream(unsigned long inSerialNo, IOggDecoderSeek* inSeekInterface)
{
	sStreamMapping locMapping;
	locMapping.mSerialNo = inSerialNo;
	locMapping.mSeekInterface = inSeekInterface;
	if (inSeekInterface == NULL) {
		locMapping.mSeekTable = NULL;
	} else {
		locMapping.mSeekTable = new OggGranuleSeekTable;
	}
	mStreamMaps.push_back(locMapping);

	return true;

}