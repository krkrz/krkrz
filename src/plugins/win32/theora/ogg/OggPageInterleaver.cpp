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
#include <OggPageInterleaver.h>

OggPageInterleaver::OggPageInterleaver(IOggCallback* inFileWriter, INotifyComplete* inNotifier)
	:	mFileWriter(inFileWriter)
	,	mNotifier(inNotifier)
	,	mProgressTime(0)
	,	mBytesWritten(0)
{
#ifdef OGGCODECS_LOGGING
	debugLog.open("G:\\logs\\interleaver.log", ios_base::out);
#endif
}

OggPageInterleaver::~OggPageInterleaver(void)
{
#ifdef OGGCODECS_LOGGING
	debugLog.close();
#endif


	//Need to delete stream objects
}

OggMuxStream* OggPageInterleaver::newStream() {
	OggMuxStream* retStream = new OggMuxStream(this);
	mInputStreams.push_back(retStream);
	return retStream;
}

void OggPageInterleaver::notifyArrival() {
#ifdef OGGCODECS_LOGGING
	debugLog<<endl;
	debugLog<<"notifyArrival : "<<endl;
#endif
	processData();
}
void OggPageInterleaver::processData() {
	/*
		IF ALL EOS THEN
			FINSH UP
			SIGNAL END OF STREAMS
		ELSE
			WHILE IS PROCESSABLE
				lowestStream = NULL
				FOR EACH stream in mInputStreams
					IF NOT stream IS EMPTY THEN
						IF lowestStream = NULL THEN
							lowestStream = stream
						ELSE IF stream.frontTime < lowestStream.frontTime THEN
							lowestStream = stream
						END IF
					END IF
				NEXT stream
				IF lowestStream = NULL THEN
					CURSE LOUDLY
				ELSE
					WRITE lowestStream.frontPage
				END IF
			WEND
		END IF
	*/
	//
	//Temp
#ifdef OGGCODECS_LOGGING
	debugLog<<endl;
	debugLog<<"ProcessData : "<<endl;
#endif
	
	if (isAllEOS()) {
		//debugLog<<"ProcessData : All Streams EOS."<<endl;

		//Finish up
		while (!isAllEmpty()) {
			//debugLog<<"ProcessData : All Streams EOS : Flushing."<<endl;
			writeLowest();
		}
		//debugLog<<"ProcessData : All Streams EOS : Notify complete."<<endl;
		mNotifier->NotifyComplete();
	} else {
		//debugLog<<"ProcessData : All Streams *NOT* EOS."<<endl;
		while (isProcessable()) {
			//debugLog<<"ProcessData : Writing lowest"<<endl;
			writeLowest();
		}
		//debugLog<<"ProcessData : No more processable data"<<endl;
		if (isAllEOS() && isAllEmpty()) {
			//debugLog<<"ProcessData : All EOS and all Empty... Notifying complete..."<<endl;
			mNotifier->NotifyComplete();
		}
	}
	//debugLog<<"==============="<<endl;

}



void OggPageInterleaver::writeLowest() {
		OggMuxStream* locLowestStream = NULL;
		for (size_t i = 0; i < mInputStreams.size(); i++) {
			if (!mInputStreams[i]->isEmpty() && mInputStreams[i]->isActive()) {
				if (locLowestStream == NULL) {
					if (mInputStreams[i]->peekFront() == NULL) {
						int x = 0;
						x= x/x;
					}
					locLowestStream = mInputStreams[i];
					//debugLog<<"writeLowest : Defaulting stream "<<i<<" @ Gran = "<<locLowestStream->frontTime()<<" & Time = "<<locLowestStream->scaledFrontTime()<<endl;
					//debugLog<<"writeLowest : Defaulting stream "<<i<<endl;
				} else {
					LOOG_INT64 locCurrLowTime = locLowestStream->scaledFrontTime();
					LOOG_INT64 locTestLowTime = mInputStreams[i]->scaledFrontTime();

					//debuging
					//LOOG_INT64 locCurrLowTimeUNS = locLowestStream->frontTime();
					//LOOG_INT64 locTestLowTimeUNS = mInputStreams[i]->frontTime();
					//debugging end

					//debugLog<<"writeLowest : Scaled : Curr = "<<locCurrLowTime<<" -- Test["<<(unsigned long)i<<"] = "<<locTestLowTime<<endl;
					//debugLog<<"writeLowest : UNSCAL : Curr = "<<locCurrLowTimeUNS<<" -- Test["<<(unsigned long)i<<"] = "<<locTestLowTimeUNS<<endl;

					
					//ASSERT (all header packets have granule pos 0)
					//

					//In english this means... any bos pages go first... then any no gran pos pages (-1 gran pos).. 
					// then any remaining streams with headers then whoevers got the lowest time.
					if (
						//Precedence goes to BOS pages.
						(	(mInputStreams[i]->peekFront() != NULL) && 
							(mInputStreams[i]->peekFront()->header()->isBOS()) ) ||
						
						(	(mInputStreams[i]->peekFront() != NULL) && 
							((mInputStreams[i]->peekFront()->header()->GranulePos()) == -1) ) ||
							
						//If the tested page hasn't sent all it's headers and the current one either
						// a) Has already sent all it's pages OR
						// b) Has sent more than the test page
						(	(mInputStreams[i]->peekFront() != NULL) && 
							(!mInputStreams[i]->sentAllHeaders()) &&
							//(!locLowestStream->peekFront()->header()->isBOS()) ) ||
							((locLowestStream->sentAllHeaders()) ||
							(mInputStreams[i]->packetsSent() < locLowestStream->packetsSent())) ) ||
						
						(
							(locTestLowTime < locCurrLowTime))
					   ) 
					{
						
						//DeBUGGIN BLOCK
						if (	(mInputStreams[i]->peekFront() != NULL) && 
								(mInputStreams[i]->peekFront()->header()->isBOS()) ) {
							//debugLog<<"WriteLowest : Selecting because BOS"<<endl;
						}
						if		(	(mInputStreams[i]->peekFront() != NULL) && 
									((mInputStreams[i]->peekFront()->header()->GranulePos()) == -1) ) {
							//debugLog<<"WriteLowest : Selecting because gran pos = -1"<<endl;
						}

						if	((mInputStreams[i]->peekFront() != NULL) && 
							(!mInputStreams[i]->sentAllHeaders()) &&
							(mInputStreams[i]->packetsSent() < locLowestStream->packetsSent()) ) {

									//debugLog<<"WriteLowest : Selecting because hasn't sent all headers"<<endl;
						}

						if (locTestLowTime < locCurrLowTime) {
						
							//debugLog<<"WriteLowest : Selecting because test time "<<locTestLowTime<<" less than "<<locCurrLowTime<<endl;
						}
						//END BEBUGGING BLOCK
						locLowestStream = mInputStreams[i];
						//debugLog<<"writeLowest : Selecting stream "<<(unsigned long)i<<" @ Gran = "<<locLowestStream->frontTime()<<" & Time = "<<locLowestStream->scaledFrontTime()<<endl;
					}
				}
			}
		}
		if (locLowestStream == NULL) {
			throw 0;
		} else {
			//debugLog<<"writeLowest : Writing..."<<endl;
			if (locLowestStream->scaledFrontTime() != -1) {
				mProgressTime = locLowestStream->scaledFrontTime();
			}
		
			//debugLog<<"writeLowest : Progress Time = "<<mProgressTime<<endl;

			OggPage* locPageToWrite = locLowestStream->popFront();
			mBytesWritten += locPageToWrite->pageSize();
			//TODO::: Handle case where the popped page is a null pointer. Can it even happen ?? There should never be a null pointer.
			mFileWriter->acceptOggPage(locPageToWrite);		//Gives away page
		}

}

LOOG_INT64 OggPageInterleaver::progressTime() 
{
	return mProgressTime;
}

LOOG_INT64 OggPageInterleaver::bytesWritten()
{
	return mBytesWritten;
}
bool OggPageInterleaver::isProcessable() {
	bool retVal = true;
	//ASSERT(mInputStreams.size() >= 1)
	for (size_t i = 0; i < mInputStreams.size(); i++) {
		retVal = retVal && (mInputStreams[i]->isProcessable());
	}
	//if (retVal) {
	//	debugLog<<"isPRocessable : TRUE"<<endl;
	//} else {
	//	debugLog<<"isPRocessable : FALSE"<<endl;
	//}
	return retVal;
}
bool OggPageInterleaver::isAllEOS() {
	bool retVal = true;
	//ASSERT(mInputStreams.size() >= 1)
	for (size_t i = 0; i < mInputStreams.size(); i++) {
		//if (mInputStreams[i]->isEOS()) {
		//	debugLog<<"isAllEOS : *****                  Stream "<<(unsigned long)i<<" is EOS"<<endl;
		//} else {
		//	debugLog<<"isAllEOS : *****                  Stream "<<(unsigned long)i<<" not EOS"<<endl;
		//}
		retVal = retVal && (mInputStreams[i]->isEOS() || !mInputStreams[i]->isActive());
	}
	//if (retVal) {
	//	debugLog<<"isAllEOS : TRUE"<<endl;
	//} else {
	//	debugLog<<"isAllEOS : FALSE"<<endl;
	//}
	return retVal;
}

bool OggPageInterleaver::isAllEmpty() {
	bool retVal = true;
	//ASSERT(mInputStreams.size() >= 1)
	for (size_t i = 0; i < mInputStreams.size(); i++) {
		retVal = retVal && (mInputStreams[i]->isEmpty());
	}
	return retVal;
}
