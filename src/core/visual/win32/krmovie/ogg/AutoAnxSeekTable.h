#pragma once

#include <libOOOggSeek.h>

#include <AutoOggSeekTable.h>

#include <string>

using namespace std;

class AutoAnxSeekTable : public AutoOggSeekTable
{
public:
#ifdef UNICODE
	AutoAnxSeekTable(wstring inFileName);
#else
	AutoAnxSeekTable(string inFileName);
#endif

	virtual ~AutoAnxSeekTable(void);

	//virtual bool buildTable();

	//IOggCallback interface
	virtual bool acceptOggPage(OggPage* inOggPage);

protected:
	unsigned long mAnxPackets;
	bool mSeenAnything;
	unsigned long mAnnodexSerialNo;
	bool mReadyForOgg;
	bool mSkippedCMML;

	unsigned short mAnnodexMajorVersion;

	// V3-related member variables
	unsigned long mCMMLSerialNo;
	unsigned long mCMMLPacketsToSkip;

	fstream mDebugFile;
};
