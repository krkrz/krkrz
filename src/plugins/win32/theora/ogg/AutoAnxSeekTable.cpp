#include "oggstdafx.h"
#include <AutoAnxSeekTable.h>

#undef DEBUG

#ifdef UNICODE
AutoAnxSeekTable::AutoAnxSeekTable(wstring inFileName)
#else
AutoAnxSeekTable::AutoAnxSeekTable(string inFileName)
#endif
	:	AutoOggSeekTable(inFileName)
	,	mAnxPackets(0)
	,	mSeenAnything(false)
	,	mAnnodexMajorVersion(0)
	,	mAnnodexSerialNo(0)
	,	mReadyForOgg(false)
	,	mSkippedCMML(false)
{
#ifdef DEBUG
	mDebugFile.open("G:\\Logs\\AutoAnxSeekTable.log", ios_base::out);
	mDebugFile << "AutoAnxSeekTable 1" << endl;
#endif
}

AutoAnxSeekTable::~AutoAnxSeekTable(void)
{
	mFile.close();

#ifdef DEBUG
	mDebugFile.close();
#endif
}

//IOggCallback interface
bool AutoAnxSeekTable::acceptOggPage(OggPage* inOggPage) {
	//FIX::: This is serious mess !

	unsigned char* locPacketData;
	if (		inOggPage != NULL
			&&	inOggPage->getPacket(0) != NULL
			&&	inOggPage->getPacket(0)->packetData() != NULL) {
		locPacketData = inOggPage->getPacket(0)->packetData();
	} else {
		// Empty page: skip it
		mFilePos += inOggPage->pageSize();
		delete inOggPage;
		return true;
	}

	if (mSeenAnything == false) {

		// Scan for Annodex v2 header
		if (strncmp((const char*)locPacketData, "Annodex", 7) == 0) {
			mAnnodexSerialNo = inOggPage->header()->StreamSerialNo();
			mSeenAnything = true;
			mAnnodexMajorVersion = 2;
			mFilePos += inOggPage->pageSize();
#ifdef DEBUG
			mDebugFile << "Found Annodex: serialno is " << mAnnodexSerialNo << endl;
#endif
			delete inOggPage;
			return true;
			//Need to grab other info here.
		} else if (strncmp((const char*)locPacketData, "fishead", 7) == 0) {
			mAnnodexSerialNo = inOggPage->header()->StreamSerialNo();
			mSeenAnything = true;
			mAnnodexMajorVersion = 3;
			mFilePos += inOggPage->pageSize();
			mReadyForOgg = true;
#ifdef DEBUG
			mDebugFile << "Found fishead: serialno is " << mAnnodexSerialNo << endl;
#endif
			delete inOggPage;
			return true;
		} else {
			mFilePos += inOggPage->pageSize();
			delete inOggPage;
			return false;
		}
	}

	// If we got up to this point, either the Annodex header (v2) or fishead (v2)
	// has been seen.  What we do now depends on whether we're dealing with a v2
	// or v3 file ...

	if (mAnnodexMajorVersion == 2) {

		if (	(mAnnodexSerialNo == inOggPage->header()->StreamSerialNo())
			&& ((inOggPage->header()->HeaderFlags() & 4) != 0)) {
			//This is the EOS of the annodex section... everything that follows is ogg like
#ifdef DEBUG
			mDebugFile << "Found Annodex EOS" << endl;
#endif
			mReadyForOgg = true;
			mFilePos += inOggPage->pageSize();
			delete inOggPage;
			return true;
		}

		if (mReadyForOgg) {
			// FIXME: Should we be skipping all CMML packets up to the <head> packet?

			if (mSkippedCMML == false) {
				mSkippedCMML = true;
				mFilePos += inOggPage->pageSize();
			} else {
				return AutoOggSeekTable::acceptOggPage(inOggPage);		//Gives away page.
			}
		} else {
			mFilePos += inOggPage->pageSize();
		}

	} else if (mAnnodexMajorVersion == 3) {

		if (mAnnodexSerialNo == inOggPage->header()->StreamSerialNo()) {
			// We found a page with the Annodex serialno, but no EOS flag set on it.
			// This is impossible for an Annodex v2 file (since the Annodex track
			// consists of only the "Annodex" header packet and an EOS packet), but
			// can be true for an Annodex v3 track.  Skip all pages in this track,
			// otherwise we'll confuse AutoOggSeekTable.
#ifdef DEBUG
			mDebugFile << "Found page belonging to Annodex v3 skeleton; skipping" << endl;
#endif
			mFilePos += inOggPage->pageSize();
		} else if (		locPacketData != NULL
					&&	memcmp((const char*)locPacketData, "CMML\0\0\0\0", 8) == 0) {
			// Found the CMML track: remember it since we need to skip the header packets
			mCMMLSerialNo = inOggPage->header()->StreamSerialNo();
			// We should really get this value from the fisbone packets, but what the hell!
			mCMMLPacketsToSkip = 2;
			mFilePos += inOggPage->pageSize();
		} else if (mCMMLPacketsToSkip > 0 && mCMMLSerialNo == inOggPage->header()->StreamSerialNo()) {
			// Found a CMML header page: ignore it
			mCMMLPacketsToSkip--;
			mFilePos += inOggPage->pageSize();
		} else {
			return AutoOggSeekTable::acceptOggPage(inOggPage);
		}

	} else {
		// Encountered non-v2 and non-v3 file: we should never get here!
#ifdef DEBUG
		mDebugFile << "Annodex major version number " << mAnnodexMajorVersion << "?" << endl;
#endif
		delete inOggPage;
		return false;
	}

	delete inOggPage;
	return true;
}
