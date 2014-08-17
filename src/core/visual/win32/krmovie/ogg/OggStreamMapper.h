#pragma once

#include <IOggCallback.h>
#include <vector>

class OggDemuxFilter;
class OggDemuxOutputPin;

class OggStreamMapper:	public IOggCallback
{
public:

	enum eStreamState {
		STRMAP_READY,
		STRMAP_PARSING_BOS_PAGES,
		STRMAP_PARSING_HEADERS,
		STRMAP_DATA,
		STRMAP_FINISHED,
		STRMAP_ERROR

	};
	OggStreamMapper(OggDemuxFilter* inParentFilter, CCritSec* inParentFilterLock);
	virtual ~OggStreamMapper(void);

	//IOggCallback Interface
	virtual bool acceptOggPage(OggPage* inOggPage);

	eStreamState streamState();

	bool allStreamsReady();

	size_t numPins()				{		return mPins.size();		}
	OggDemuxOutputPin* getPinByIndex(unsigned long inIndex);

protected:
	eStreamState mStreamState;
    std::vector<OggDemuxOutputPin*> mPins;
	OggDemuxFilter* mParentFilter;
	CCritSec* mParentFilterLock;

	OggPacket* mFishHeadPacket;
	unsigned long mSkeletonSerialNo;

	bool addNewPin(OggPage* inOggPage);
	OggDemuxOutputPin* getMatchingPin(unsigned long inSerialNo);

	bool handleFishHead(OggPage* inOggPage);
	bool isFishHead(OggPage* inOggPage);
};
