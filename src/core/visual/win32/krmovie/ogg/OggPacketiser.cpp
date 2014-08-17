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
#include <OggPacketiser.h>
#include <memory>

OggPacketiser::OggPacketiser(void) 
	:	mPacketSink(NULL)
	,	mPendingPacket(NULL)
	,	mPacketiserState(PKRSTATE_OK)
	,	mLooseMode(true)						//FIX::: This affects the validator.
	,	mNumIgnorePackets(0)
	//,	mPrevGranPos(0)
	//,	mCurrentGranPos(0)
{
	//debugLog.open("g:\\logs\\packetise.log", ios_base::out);

}
OggPacketiser::OggPacketiser(IStampedOggPacketSink* inPacketSink)
	:	mPacketSink(inPacketSink)
	,	mPendingPacket(NULL)
	,	mPacketiserState(PKRSTATE_OK)
	,	mLooseMode(true)						//FIX::: This affects the validator.
	,	mNumIgnorePackets(0)
	//,	mPrevGranPos(0)
	//,	mCurrentGranPos(0)
{
	//debugLog.open("g:\\logs\\packetise.log", ios_base::out);
}

OggPacketiser::~OggPacketiser(void)
{
	//Don't delete the packet sink
	//debugLog.close();
	if( mPendingPacket ) {
		delete mPendingPacket;
		mPendingPacket = NULL;
	}
}

IStampedOggPacketSink* OggPacketiser::packetSink() {
	return mPacketSink;
}
void OggPacketiser::setPacketSink(IStampedOggPacketSink* inPacketSink) {
	mPacketSink = inPacketSink;
}
bool OggPacketiser::reset() {
	//debugLog<<"Reset : "<<endl;
	delete mPendingPacket;
	mPendingPacket = NULL;
	mNumIgnorePackets = 0;
	mPacketiserState = PKRSTATE_OK;
	//mPrevGranPos = 0;
	//mCurrentGranPos = 0;
	return true;
}
bool OggPacketiser::acceptOggPage(OggPage* inOggPage) {				//AOP::: Needs closer look
	auto_ptr<OggPage> op(inOggPage);
	//All callers to acceptOggPage give away their pointer
	// to this function. All functions implementing this interface
	// are responsible for deleting this page. All callers
	// should NULL their pointer immediately after calling
	// to avoid reusing them.
	// 

	//debugLog<<"acceptOggPage : Gran = "<<inOggPage->header()->GranulePos()<<"Num packs = "<<inOggPage->numPackets()<<endl;

	//If the page isn't a -1 page and it's got a different granpos save it.
	//if ( (inOggPage->header()->GranulePos() != -1) && (inOggPage->header()->GranulePos() != mCurrentGranPos)) {
	//	mPrevGranPos = mCurrentGranPos;

	//	//If the previous is higher than the
	//	if (mPrevGranPos > mCurrentGranPos) {
	//		mPrevGranPos = -1;
	//	}
	//	mCurrentGranPos = inOggPage->header()->GranulePos();
	//}

	//If the page header says its a continuation page...
	if ((inOggPage->header()->HeaderFlags() & 1) == 1) {
		//debugLog<<"acceptOggPage : Page says cont..."<<endl;
		
		///... and there is at least 1 packet...
		if (inOggPage->numPackets() > 0) {
			//debugLog<<"acceptOggPage : ...and there is at least 1 packet..."<<endl;
		
			//... and we were expecting a continuation...
			if (mPacketiserState == PKRSTATE_AWAITING_CONTINUATION) {
				//debugLog<<"acceptOggPage : ... and we were waiting for a cont..."<<endl;

				//... and the first packet is marked as a continuation...
				if (inOggPage->getStampedPacket(0)->isContinuation()) {
					//debugLog<<"acceptOggPage : ... and the first packet is a cont..."<<endl;

					//... merge this packet into our pending page.
					//ASSERT when mPacketiserState = PKRSTATE_AWAITING_CONTINUATION, mPending page != NULL
					mPendingPacket->merge(inOggPage->getStampedPacket(0));
					
					//If even after merging this packet is still truncated...
					if (mPendingPacket->isTruncated()) {
						//debugLog<<"acceptOggPage : ... but the pending packet is still truncated..."<<endl;
						//Packet still not full. special case full page.
						//===
						// The only way the the pending packet can be truncated is if
						//  the first packet in the page is truncated, and the first
						//  packet in a page can only be truncated if it's also the 
						//  only packet on the page.
						//Considering it is incomplete ending a page the granule pos
						// will be -1.
						//This is a special type of page :
						// 1 incomlpete packet on the page
						// Continuation flag set
						// No complete packets end on this page
						// Granule pos is -1

						//debugLog<<"acceptOggPage : Go to cont state."<<endl;
						//We are still waiting for another continuation...
						mPacketiserState = PKRSTATE_AWAITING_CONTINUATION;		//This should be redundant, we should already be in this state.
						//First packet on page is now merged into pending packet.
					} else {
						//debugLog<<"acceptOggPage : ... now we can deliver it..."<<endl;
						//... the pending packet is now complete.
						
						//TODO::: Static alternative here ?
						
						//Deliver the packet to the packet sink...
						if (dispatchStampedOggPacket(mPendingPacket) == false) {
							//debugLog<<"acceptOggPage : DELIVERY FAILED !"<<endl;
							mPacketiserState = PKRSTATE_OK;
							mPendingPacket = NULL;
							//delete inOggPage;
							//inOggPage = 0;
							return false;
						}
						//debugLog<<"acceptOggPage : ... delivery sucessful..."<<endl;
						//debugLog<<"acceptOggPage : Back to OK State..."<<endl;
						//Go back to OK state
						mPacketiserState = PKRSTATE_OK;
						mPendingPacket = NULL;
						//First packet on page is merged and delivered.
					}
					//debugLog<<"acceptOggPage : Send all the other packets besides first and last..."<<endl;
					//Send every packet except the first and last to the packet sink.
					processPage(inOggPage, false, false);
				} else {
					//debugLog<<"acceptOggPage : INTERNAL ERROR - Header says cont but packet doesn't."<<endl;
					//Header flag says continuation but first packet is not continued.
					mPacketiserState = PKRSTATE_INVALID_STREAM;
					
					//delete inOggPage;
					//inOggPage = 0;
					throw 0;
				}
			} else {
				//debugLog<<"acceptOggPage : UNEXPECTED CONT !"<<endl;
				if (mLooseMode == true) {
					//debugLog<<"acceptOggPage : ... but we are ignoring it !"<<endl;
					//Just ignore when we get continuation pages, just drop the broken bit of packet.
					if( mPendingPacket ) { delete mPendingPacket; mPendingPacket = NULL; }
					mPendingPacket = NULL;   //MEMCHECK::: Did i just leak memory ?
					mPacketiserState = PKRSTATE_OK;

					//TODO::: Should really return false here if this returns false.
					if( processPage(inOggPage, false, false) == false) {
						//TODO::: State change ???
						//delete inOggPage;
						//inOggPage = 0;
						return false;
					}
				} else {
					//debugLog<<"acceptOggPage : FAILURE !!!!"<<endl;
					//Unexpected continuation
					mPacketiserState = PKRSTATE_INVALID_STREAM;
					throw 0;
				}
			}
		} else {
			//debugLog<<"acceptOggPage : UNKNOWN CASE"<<endl;
			//Is this something ?
			//UNKNOWN CASE::: Header continuation flag set, but no packets on page.
			mPacketiserState = PKRSTATE_INVALID_STREAM;
			//delete inOggPage;
			//inOggPage = 0;
			throw 0;
		}
	} else {
		//debugLog<<"acceptOggPage : We have a normal page... dumping all but the last..."<<endl;
		//Normal page, no continuations... just dump the packets, except the last one
		if (inOggPage->numPackets() == 1) {

			//I think the bug is here... by sending a trunc packet and not updating state.

			//debugLog<<"acceptOggPage : Only one packet on this normal page..."<<endl;

			if (inOggPage->getPacket(0)->isTruncated()) {
				//debugLog<<"acceptOggPage : ...and it's truncated... so we save it."<<endl;
				//ASSERT : mPending packet is NULL, because this is not a continuation page.
				if( mPendingPacket ) { delete mPendingPacket; mPendingPacket = NULL; }
				mPendingPacket = (StampedOggPacket*)inOggPage->getStampedPacket(0)->clone();
				//debugLog<<"acceptOggPage : Moving to CONT state."<<endl;
				mPacketiserState = PKRSTATE_AWAITING_CONTINUATION;

			} else {
				//debugLog<<"acceptOggPage : Only one packet on this normal page..."<<endl;
				if (processPage(inOggPage, true, true) == false ) {			//If there was only one pack process it.
					//debugLog<<"acceptOggPage : FAIL STATE DELIVERY"<<endl;
					//TODO::: State change
					//delete inOggPage;
					//inOggPage = 0;
					return false;
				}

				//We should never go into the if below now as the packet is taken care of.
			}
		} else {
			//debugLog<<"acceptOggPage : More than one packet so dumping all but last..."<<endl;
			if (processPage(inOggPage, true, false) == false ) {			//If there was only one packet, no packets would be written
				//debugLog<<"acceptOggPage : FAIL STATE DELIVERY"<<endl;
				//TODO::: State change
				//delete inOggPage;
				//inOggPage = 0;
				return false;			
			}
		}
		
		//The first packet is delivered.
	}

	//debugLog<<"acceptOggPage : First pack should be delivered..."<<endl;
	//ASSERT: By this point something has been done with the first packet.

	// It was either merged with pending page and possibly delivered
	// or it was delivered by process page.
	//Code following assumes the first packet is dealt with already.
	
	//Now we deal with the last packet...
	//ASSERT : The last packet has only been sent if there was 1 or less packets.

	//If there is at least two packet on the page... ie at least one more packet we haven't processed.
	if (inOggPage->numPackets() > 1) {
		//debugLog<<"acceptOggPage : There is at least one packet on the page we haven't processed"<<endl;
		//... and we are in the OK state
		if (mPacketiserState == PKRSTATE_OK) {
			//debugLog<<"acceptOggPage : ... and we are in the OK state..."<<endl;
			//If the last packet is truncated.
			if (inOggPage->getPacket(inOggPage->numPackets() - 1)->isTruncated()) {
				//debugLog<<"acceptOggPage : ... but the last packet is trunced... so we save it and wait for cont..."<<endl;
				//The last packet is truncated. Save it and await continuation.
				
				//debugLog<<"acceptOggPage : Moving to CONT state..."<<endl;
				mPacketiserState = PKRSTATE_AWAITING_CONTINUATION;
			
				//ASSERT when mPacketiserState = OK, mPendingPacket = NULL
				if( mPendingPacket ) { delete mPendingPacket; mPendingPacket = NULL; }
				mPendingPacket = (StampedOggPacket*)(inOggPage->getStampedPacket(inOggPage->numPackets() - 1)->clone());
				//This packet is not delivered, it waits for a continuation.
			} else {
				//We are in the OK state, with no pending packets, and the last packet is not truncated.
				//debugLog<<"acceptOggPage : The last page is not trunc so we send it..."<<endl;
				//Deliver to the packet sink.
				if ( dispatchStampedOggPacket( (StampedOggPacket*)(inOggPage->getStampedPacket(inOggPage->numPackets() - 1)->clone()) ) == false ) {
					//debugLog<<"acceptOggPage : Delivery failed..."<<endl;
					//TODO::: State change ?
					//delete inOggPage;
					//inOggPage = 0;
					return false;
				}
				//The last packet is complete. So send it.
			}
		} else if (mPacketiserState == PKRSTATE_AWAITING_CONTINUATION) {
			//FIX::: This case should never occur.
			//debugLog<<"acceptOggPage : NEVER BE HERE 1"<<endl;
			
			//Packetiser state is not ok... what to do abo8ut it.
			
			//See special page case above.
			//This can only happen when we went through the special case above, and kept
			// the state in  the continuation state. But by definition it is impossible
			// for a subsequent packet on this page to be a continuation packet
			// as continuation packets can only be the first packet on the page.
			//This is more likely to be due to inconsistency of state code than invalidaity
			// of file.
			mPacketiserState = PKRSTATE_INVALID_STREAM;
			//delete inOggPage;
			//inOggPage = 0;
			throw 0;
		} else {
			//debugLog<<"acceptOggPage : NEVER BE HERE 2"<<endl;
			//Shouldn't be here
			mPacketiserState = PKRSTATE_INVALID_STREAM;
			//delete inOggPage;
			//inOggPage = 0;
			throw 0;
		}
	} else {
		//debugLog<<"acceptOggPage : 1 packet on page only, and we've taken care of it."<<endl;
		//Zero packets on page.
	}
	//debugLog<<"acceptOggPage : All ok... returning..."<<endl<<endl;
	//delete inOggPage;
	//inOggPage = 0;
	return true;
}

bool OggPacketiser::processPage(OggPage* inOggPage, bool inIncludeFirst, bool inIncludeLast) {
	//Returns false only if one of the acceptStampedOggPacket calls return false... means we should stop sending stuff and return.
	bool locIsOK = true;
	//debugLog<<"processPage : "<<endl;
	//Adjusts the loop parameters so that only packets excluding those specified are written.
	for (	int i = ((inIncludeFirst) ? 0 : 1); 
			i < ((int)inOggPage->numPackets()) - ((inIncludeLast) ? 0 : 1);
			i++) 
	{
				//debugLog<<"processPage : Packet "<< i <<endl;		
				locIsOK = (locIsOK && dispatchStampedOggPacket((StampedOggPacket*)inOggPage->getStampedPacket(i)->clone()));	//Gives away new packet.
				if (!locIsOK) {
					//debugLog<<"processPage : FAIL STATE"<<endl;
					//TODO::: State change ???
					return false;
				}
	}
	//debugLog<<"processPage : returning..."<<endl;
	return true;

}

bool OggPacketiser::dispatchStampedOggPacket(StampedOggPacket* inPacket) {	//Accepts packet... and gives it away or deletes it.
	if (mNumIgnorePackets > 0) {
		//Ignore this packet.
		mNumIgnorePackets--;

		//MEMCHECK::: Should probably delete this packet here.]
		delete inPacket;
		return true;
	} else {
		//Modify the header packet to include the gran pos of previous page.
		//if (mPrevGranPos != -1) {
		//	inPacket->setTimeStamp(mPrevGranPos, mCurrentGranPos, StampedOggPacket::OGG_BOTH);
		//}
		//Dispatch it.
		return mPacketSink->acceptStampedOggPacket(inPacket);
	}
}

void OggPacketiser::setNumIgnorePackets(unsigned long inNumIgnorePackets) {
	mNumIgnorePackets = inNumIgnorePackets;
}
unsigned long OggPacketiser::numIgnorePackets() {
	return mNumIgnorePackets;
}
