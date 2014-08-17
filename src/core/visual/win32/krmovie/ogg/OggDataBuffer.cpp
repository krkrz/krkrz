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
#include <OggDataBuffer.h>
#include "ogglog.h"
#undef min

//LEAK CHECKED - 2004/10/17		-	OK.
//LEAK FOUND - 2004/11/29  -  acceptOggPage
OggDataBuffer::OggDataBuffer(void)
	:	mBuffer(NULL)
	,	mPrevGranPos(0)
	,	pendingPage(NULL)
	,	mState(AWAITING_BASE_HEADER)
	,	mNumBytesNeeded(OggPageHeader::OGG_BASE_HEADER_SIZE)
{
	mBuffer = new CircularBuffer(MAX_OGG_PAGE_SIZE);			//Deleted in destructor
}

//Debug only
//OggDataBuffer::OggDataBuffer(bool x)
//	:	mBuffer(NULL)
//	,	mPrevGranPos(0)
//{
//	mBuffer = new CircularBuffer(MAX_OGG_PAGE_SIZE);			//Deleted in destructor
//
//	//debugLog.open("G:\\logs\\OggDataBufferSeek.log", ios_base::out);
//	pendingPage = NULL;
//	mState = AWAITING_BASE_HEADER;
//	mNumBytesNeeded = OggPageHeader::OGG_BASE_HEADER_SIZE;
//}
//

OggDataBuffer::~OggDataBuffer(void)
{
	delete mBuffer;
	mBuffer = 0;
	delete pendingPage;
	pendingPage = 0;
}


/** The inUserData parameter is a pointer of any type.  This pointer is passed
    to the callback when it is called, enabling the user to save any custom
    information in its callback.  Note that your callback function is
    responsible for deleting the OggPage passed to it (and NULL the OggPage
    pointer after deletion): see IOggCallback::acceptOggPage() for more
    information.
  */

bool OggDataBuffer::registerStaticCallback(fPageCallback inPageCallback, void* inUserData)
{
	//Holds the static callback and nulls the virtual one.
	mStaticCallback = inPageCallback;
	mStaticCallbackUserData = (void *) inUserData;
	mVirtualCallback = NULL;
	
	return true;
}

//bool OggDataBuffer::registerSerialNo(SerialNoRego* inSerialRego) {
//	if (inSerialRego != NULL) {
//		mSerialNoCallList.push_back(inSerialRego);
//		return true;
//	} else {
//		return false;
//	}
//}


/** Note that your callback function is responsible for deleting the OggPage
    passed to it (and NULL the OggPage pointer after deletion): see the
    IOggCallback class for more information.
  */

bool OggDataBuffer::registerVirtualCallback(IOggCallback* inPageCallback) 
{
	//Holds the virtual callback and nulls the static one.
	mVirtualCallback = inPageCallback;
	mStaticCallback = NULL;

	return true;	
}


unsigned long OggDataBuffer::numBytesAvail() 
{
	//Returns how many bytes are available in the buffer
	unsigned long locBytesAvail = mBuffer->numBytesAvail();	

	// LOG(logDEBUG1) << "Bytes avail = " << locBytesAvail;
	return locBytesAvail;
}

OggDataBuffer::eState OggDataBuffer::state() 
{
	//returns the state of the stream
	return mState;
}

//This function accepts the responsibility for the incoming page.
OggDataBuffer::eDispatchResult OggDataBuffer::dispatch(OggPage* inOggPage) 
{
	//TODO::: Who owns this pointer inOggPage ?
    // LOG(logDEBUG1) << "Dispatching page to: " << mVirtualCallback;

	//Fire off the oggpage to whoever is registered to get it
	if (mVirtualCallback != NULL) 
    {
		if (mVirtualCallback->acceptOggPage(inOggPage) == true) 
        {		
            //Page given away, never used again.
			return DISPATCH_OK;
		} 
        else 
        {
			return DISPATCH_FALSE;
		}
	} 
    else if (mStaticCallback != NULL) 
    {
		if (mStaticCallback(inOggPage, mStaticCallbackUserData) == true) 
        {
			return DISPATCH_OK;	
		} 
        else 
        {
			return DISPATCH_FALSE;
		}
	}

	//Delete the page... if the called functions wanted a copy they should have taken one for themsselves.
	//Not any more acceptOggPage takes responsibility for the memory you pass into it. See IOggCallback.h
	//delete inOggPage;
	pendingPage = NULL;
	return DISPATCH_NO_CALLBACK;
}

OggDataBuffer::eFeedResult OggDataBuffer::feed(const unsigned char* inData, unsigned long inNumBytes)
{
	if (inNumBytes != 0) 
    {
		if (inData != NULL) 
        {
			//Buffer is not null and there is at least 1 byte of data.
			// LOG(logDEBUG1) << "Fed " << inNumBytes << " bytes...";
			unsigned long locNumWritten = 0;

            eProcessResult locResult = PROCESS_OK;
            while (locNumWritten < inNumBytes && locResult == PROCESS_OK) 
            {
                unsigned long locBytesToWrite = std::min(inNumBytes - locNumWritten, mBuffer->spaceLeft());
                unsigned long locBytesWritten = mBuffer->write(&inData[locNumWritten], locBytesToWrite);
                if (locBytesWritten == 0)
                {
                    return FEED_BUFFER_WRITE_ERROR;
                }
                locNumWritten += locBytesWritten;
                locResult = processBuffer();
            }
            return (eFeedResult)locResult;
		} 
        else 
        {
			//Numbytes not equal to zero but inData pointer is NULL

			// LOG(logDEBUG1) << "Feed : Fed NULL Pointer";
			return FEED_NULL_POINTER;
		}
	} 
    else 
    {
		//numbytes was zero... we do nothing and it's not an error.
		
		// LOG(logDEBUG1) << "Feed : Fed *zero* bytes... Not an error, do nothing, return ok.";
		return FEED_OK;
	}	
}
OggDataBuffer::eProcessResult OggDataBuffer::processBaseHeader() 
{
		// LOG(logDEBUG1) << "Processing base header..." <<endl;
		
		//Delete the previous page
		delete pendingPage;
		
		//Make a fresh ogg page
		pendingPage = new OggPage;			//Either deleted in destructor, or given away by virtue of dispatch method.

		//Make a local buffer for the header
		unsigned char* locBuff = new unsigned char[OggPageHeader::OGG_BASE_HEADER_SIZE];		//deleted before this function returns
		// 配列なので、auto_ptr使わない方がいいが、組み込み方なので気にしないことにする
		auto_ptr<unsigned char> lb(locBuff);
		
		// LOG(logDEBUG1) << "ProcessBaseHeader : Reading from stream...";
		
		//STREAM ACCESS::: Read
		unsigned long locNumRead = mBuffer->read(locBuff, OggPageHeader::OGG_BASE_HEADER_SIZE);
		
		if (locNumRead < OggPageHeader::OGG_BASE_HEADER_SIZE) 
        {
			//TODO::: Handle this case... we read less than we expected.
			//The buffer handles it for us, it won't let us read less, and will return 0
			//	This is fine for valid files, but still needs to be reviewed.

			LOG(logDEBUG1) << "ProcessBaseHeader : ###### Read was short.";
			LOG(logDEBUG1) << "ProcessBaseHeader : ** "<< mBuffer->numBytesAvail() <<" avail, " << mBuffer->spaceLeft() <<" space left.";
			locNumRead = locNumRead;
		}

		bool locRetVal = pendingPage->header()->setBaseHeader((unsigned char*)locBuff);		//Views pointer only.
		if (locRetVal == false) 
        {
			//delete[] locBuff;
			return PROCESS_FAILED_TO_SET_HEADER;
		}
	
		//Set the number of bytes we want for next time
		mNumBytesNeeded = pendingPage->header()->NumPageSegments();

		// LOG(logDEBUG1) << "Setting state to AWAITING_SEG_TABLE";

        //Change the state.
		mState = AWAITING_SEG_TABLE;

		//delete[] locBuff;
		// LOG(logDEBUG1) << "Bytes needed for seg table = " << mNumBytesNeeded<<endl;	
		return PROCESS_OK;
}
OggDataBuffer::eProcessResult OggDataBuffer::processSegTable() 
{
	///
	//	Gets the number segments from from the page header, reads in that much data,
	//	 
	//	
	//
	//Assumes a valid pending page, with numPagesegments set in the header already.
	//creates a chunk of memory size numpagesegments and stores it,.

	// LOG(logDEBUG1) << "Processing Segment Table...";

	//TODAY::: What happens when numpage segments is zero.

	//Save a local copy of the number of page segments - Get this from the already set header.
	unsigned char locNumSegs = pendingPage->header()->NumPageSegments();

	// LOG(logDEBUG1) << "Num segments = " <<(int)locNumSegs;

	//Make a local buffer the size of the segment table. 0 - 255
	unsigned char* locBuff = new unsigned char[locNumSegs];				//Given to setSegmentTable. Not deleted here.
	
	// LOG(logDEBUG1) << "ProcessSegTable : Reading from buffer...";

	//Read the segment table from the buffer to locBuff
	unsigned long locNumRead = mBuffer->read(locBuff, (unsigned long)locNumSegs);
	
	if (locNumRead < locNumSegs) 
    {
		//TODO::: Handle this case
		LOG(logDEBUG1) << "ProcessSegTable : ##### Short read";
		LOG(logDEBUG1) << "ProcessSegTable : ** "<< mBuffer->numBytesAvail() << " avail, " << mBuffer->spaceLeft() << " space left.";		
	}


	//Make a new segment table from the bufferd data.
	pendingPage->header()->setSegmentTable(locBuff);			//This function accepts responsibility for the pointer.
	locBuff = NULL;
	
	//Set the number of bytes we want for next time - which is the size of the page data.
	mNumBytesNeeded = pendingPage->header()->calculateDataSize();

	// LOG(logDEBUG1) << "Num bytes needed for data = " << mNumBytesNeeded;
	// LOG(logDEBUG1) << "Transition to AWAITING_DATA";
	
	mState = AWAITING_DATA;
	return PROCESS_OK;

}

OggDataBuffer::eProcessResult OggDataBuffer::processDataSegment() 
{
	unsigned long locPageDataSize = pendingPage->header()->dataSize();  //unused	
	// LOG(logDEBUG1) << "ProcessDataSegment : Page data size = " << locPageDataSize;

	unsigned char* locBuff = NULL;
	//unsigned long locPacketOffset = 0;

	//TODO::: Should this be const ?
	//THis is a raw pointer into the segment table, don't delete it.
	unsigned char* locSegTable = pendingPage->header()->SegmentTable();			//View only don't delete.
	unsigned int locNumSegs = pendingPage->header()->NumPageSegments();
	
	// LOG(logDEBUG1) << "ProcessDataSegment : Num segs = " << locNumSegs;

	unsigned long locCurrPackSize = 0;
	bool locIsFirstPacket = true;
	LOOG_INT64 locPrevGranPos = 0;

	for (unsigned long i = 0; i < locNumSegs; i++) 
    {
		//Packet sums the lacing values of the segment table.
		locCurrPackSize += locSegTable[i];

		//If its the last segment  in the page or if the lacing value is not 255(ie packet boundary.

		/* TRUTH TABLE:
			last lacing value							lacing value is *not* 255
			=================							=========================
			true										true						}	If its the last one or a packet boundary(255 lacing value) we add it.
			true										false						}
			false										true						}
			false										false						If it is a 255 (packet continues) and it's not the last one do nothibng
			it is the last lacing value on the page


			Lacing values for a Packet never end with 255... if multiple of 255 have a next 0 lacing value.
		*/
		
		if ( (locSegTable[i] != 255) || (locNumSegs - 1 == i) ) 
        {
			//If its the last lacing value or the the lacing value is not 255 (ie packet boundry)
			
			//This pointer is given to the packet... it deletes it.
			locBuff = new unsigned char[locCurrPackSize];				//Given away to constructor of StampedOggPacket.

			//STREAM ACCESS:::
			//Read data from the stream into the local buffer.
			
			unsigned long locNumRead = mBuffer->read(locBuff, locCurrPackSize);
			
			if (locNumRead < locCurrPackSize) 
            {
				//TODO::: Handle this case.

				LOG(logDEBUG1) << "ProcessDataSegment : ###### Short read";
				LOG(logDEBUG1) << "ProcessDataSegment : ** " << mBuffer->numBytesAvail() << " avail, " << mBuffer->spaceLeft() << " space left.";
				locNumRead = locNumRead;
			}

			// LOG(logDEBUG1) << "Adding packet - size = "<<locCurrPackSize;
			
			//A packet ends when a lacing value is not 255. So the check for != 255 means the isComplete property of the packet is not set unless the
			// lacing value is not equal to 255.
			//ERROR CHECK:::
			bool locIsContinuation = false;
			
			if (locIsFirstPacket) 
            {
				locIsFirstPacket = false;

				//Remember what the granule pos was and get it from the new page.
				locPrevGranPos = mPrevGranPos;
				mPrevGranPos = pendingPage->header()->GranulePos();
				
				//First packet, check if the continuation flag is set.
				if ((pendingPage->header()->HeaderFlags() & OggPageHeader::CONTINUATION) == OggPageHeader::CONTINUATION) {
					//Continuation flag is set.
					locIsContinuation = true;
				}
			}
			//locBuff is given to the constructor of Stamped Ogg Packet... it deletes it.
			//The new StampedPacket is given to the page... it deletes it
			pendingPage->addPacket( new StampedOggPacket(locBuff, locCurrPackSize, (locSegTable[i] == 255), locIsContinuation, locPrevGranPos, pendingPage->header()->GranulePos(), StampedOggPacket::OGG_BOTH ) );
			locBuff = NULL;				//We've given this away.
			//Reset the packet size counter.
			locCurrPackSize = 0;
		}
	}

	
	//Update the state for how many bytes are now needed
	mNumBytesNeeded = OggPageHeader::OGG_BASE_HEADER_SIZE;
	
	// LOG(logDEBUG1) << "ProcessDataSegment : num bytes needed = "<<mNumBytesNeeded;

	//Dispatch the finished pagbve
	eDispatchResult locRet = dispatch(pendingPage);			//The dispatch function takes responsibility for this page.
	pendingPage = NULL;   //We give away the pointer
	
	if (locRet == DISPATCH_OK) 
    {
        // LOG(logDEBUG1) << "ProcessDataSegment : Transition to AWAITING_BASE_HEADER";
		mState = AWAITING_BASE_HEADER;
		return PROCESS_OK;
	} 
    else if (locRet == DISPATCH_FALSE) 
    {
		mState = AWAITING_BASE_HEADER;
		return PROCESS_DISPATCH_FALSE;	
	} 
    else 
    {
		// LOG(logDEBUG1) << "ProcessDataSegment : Dispatch failed.";
		return PROCESS_DISPATCH_FAILED;
	}		
}

void OggDataBuffer::clearData() 
{
	mBuffer->reset();
	mPrevGranPos = 0;
	// LOG(logDEBUG1) << "ClearData : Transition back to AWAITING_BASE_HEADER";
	
	// This might leak, but fixes crash with invalid data
	if( pendingPage ) delete pendingPage;
	pendingPage = 0;

	mNumBytesNeeded = OggPageHeader::OGG_BASE_HEADER_SIZE;
	mState = AWAITING_BASE_HEADER;

	// LOG(logDEBUG1) << "ClearData : Num bytes needed = " << mNumBytesNeeded;
}

OggDataBuffer::eProcessResult OggDataBuffer::processBuffer() 
{		
	eProcessResult locResult = PROCESS_OK;

	while (numBytesAvail() >= mNumBytesNeeded) 
    {
		// LOG(logDEBUG1) << "ProcessBuffer : Bytes Needed = " << mNumBytesNeeded << " --- " << "Bytes avail = "<< numBytesAvail();
		switch (mState) 
        {

			//QUERY:::	Should it be a bug when the if state inside the switch falls through,... potential for infinite loop.
			case AWAITING_BASE_HEADER:
				// LOG(logDEBUG1) << "ProcessBuffer : State = AWAITING_BASE_HEADER";
				
				//If theres enough data to form the base header
				if (numBytesAvail() >= OggPageHeader::OGG_BASE_HEADER_SIZE) 
                {
					// LOG(logDEBUG1) << "ProcessBuffer : Enough to process...";
					
					locResult = processBaseHeader();
                    
					if (locResult != PROCESS_OK) 
                    {
						mState = LOST_PAGE_SYNC;
						//Base header process failed
						return locResult;
					}
				}
				break;
			
			case AWAITING_SEG_TABLE:
				// LOG(logDEBUG1) << "ProcessBuffer : State = AWAITING_SEG_TABLE";
				
				//If there is enough data to get the segt table
				if (numBytesAvail() >= pendingPage->header()->NumPageSegments()) 
                {
					// LOG(logDEBUG1) << "ProcessBuffer : Enough to process...";
					
					locResult = processSegTable();
               
					if (locResult != PROCESS_OK) 
                    {
						mState = LOST_PAGE_SYNC;
						//segment table process failed
						return locResult;
					}
				}
				break;

			case AWAITING_DATA:
				// LOG(logDEBUG1) << "ProcessBuffer : State = AWAITING_DATA";
				//If all the data segment is available
				if (numBytesAvail() >= pendingPage->header()->dataSize()) 
                {
					// LOG(logDEBUG1) << "ProcessBuffer : Enough to process...";

					//FIX::: Need error check.
					locResult = processDataSegment();
					
					if (locResult == PROCESS_DISPATCH_FAILED) 
                    {
						mState = LOST_PAGE_SYNC;
						//segment table process failed
						return locResult;
					}
				}	
				break;
			case LOST_PAGE_SYNC:
				//TODO::: Insert resync code here.

				LOG(logDEBUG1) << "ProcessBuffer : State = LOST_PAGE_SYNC";
				return PROCESS_LOST_SYNC;
			default:
				//TODO::: What are we supposed to do with this. Anything need cleaning up ?
				
				LOG(logDEBUG1) << __FUNCTIONW__ << " Ogg Buffer Error";
				return PROCESS_UNKNOWN_INTERNAL_ERROR;
				break;
		}
	}

	//There wasn't enough data to progress if we are here.
	return locResult;
}

