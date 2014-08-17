#pragma once

#include <libOOOgg.h>
#include "OggGranuleSeekTable.h"
#include "IOggDecoderSeek.h"


class AutoOggChainGranuleSeekTable : public IOggCallback
{
public:
#ifdef UNICODE
	AutoOggChainGranuleSeekTable(wstring inFilename);
#else
	AutoOggChainGranuleSeekTable(string inFilename);
#endif
	virtual ~AutoOggChainGranuleSeekTable(void);

	/// Builds the actual seek table: only works if we have random access to the file.
	virtual bool buildTable();

	virtual void disableTable()		{	mIsEnabled = false;	}

	//IOggCallback interface
	virtual bool acceptOggPage(OggPage* inOggPage);

	/// The duration of the file, in DirectShow time units.
	LOOG_INT64 fileDuration();

	bool enabled()					{			return mIsEnabled;		}

	bool addStream(unsigned long inSerialNo, IOggDecoderSeek* inSeekInterface);

	OggGranuleSeekTable::tSeekPair seekPos(LOOG_INT64 inTime);
protected:


	struct sStreamMapping {
		unsigned long mSerialNo;
		IOggDecoderSeek* mSeekInterface;
		OggGranuleSeekTable* mSeekTable;
	};

	vector<sStreamMapping> mStreamMaps;

	bool isUnstampedPage(OggPage* inOggPage);

	bool mIsEnabled;

	LOOG_INT64 mDuration;
	sStreamMapping getMapping(unsigned long inSerialNo);


	fstream mFile;
#ifdef UNICODE
	wstring mFilename;
#else
	string mFilename;
#endif
	LOOG_INT64 mFilePos;

	LOOG_INT64 mPreviousFilePos;
	OggDataBuffer* mOggDemux;
};
