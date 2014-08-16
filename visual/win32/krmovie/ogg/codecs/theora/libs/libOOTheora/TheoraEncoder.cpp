//===========================================================================
//Copyright (C) 2003-2006 Zentaro Kavanagh
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

#include "stdafx.h"
#include "theoraencoder.h"

TheoraEncoder::TheoraEncoder(void)
{
	memset(&mTheoraInfo, 0, sizeof(mTheoraInfo));
	memset(&mTheoraComment, 0, sizeof(mTheoraComment));
	
	memset(&mTheoraState, 0, sizeof(mTheoraState));
}

TheoraEncoder::~TheoraEncoder(void)
{
	theora_info_clear(&mTheoraInfo);
	theora_comment_clear(&mTheoraComment);
	
	theora_clear(&mTheoraState);
}

/** Returns three header packets which you must delete when done. Give it a theora_info.
 */
StampedOggPacket** TheoraEncoder::initCodec(theora_info inTheoraInfo)
{
	mTheoraInfo = inTheoraInfo;
	theora_encode_init(&mTheoraState,&mTheoraInfo);

	StampedOggPacket** locHeaders = new StampedOggPacket*[3];

	ogg_packet locOldPacket;
	theora_encode_header(&mTheoraState, &locOldPacket);

	locHeaders[0] = oldToNewPacket(&locOldPacket);
	
	theora_comment_init(&mTheoraComment);
	theora_encode_comment(&mTheoraComment, &locOldPacket);

	locHeaders[1] = oldToNewPacket(&locOldPacket);

	theora_encode_tables(&mTheoraState, &locOldPacket);
	
	locHeaders[2] = oldToNewPacket(&locOldPacket);

	//This should really have some error checking ! And trash packets if faild.
	return locHeaders;
}


/** Converts our StampedOggPacket into a packet that the theora library will accept.
	You still own the old packet. But you must delete the returned packet.
 */
StampedOggPacket* TheoraEncoder::oldToNewPacket(ogg_packet* inOldPacket)
{
	const unsigned char NOT_USED = 0;

	//Need to clone the packet data
	unsigned char* locBuff = new unsigned char[inOldPacket->bytes];
	memcpy((void*)locBuff, (const void*)inOldPacket->packet, inOldPacket->bytes);
																					//Not truncated or continued... it's a full packet.
	StampedOggPacket* locOggPacket = new StampedOggPacket(locBuff, inOldPacket->bytes, false, false, NOT_USED, inOldPacket->granulepos, StampedOggPacket::OGG_END_ONLY);
	return locOggPacket;

}

/** Returns a packet you must delete, otherwise returns NULL if it fails. Pass it a yuv frame buffer which you own.
 */
StampedOggPacket* TheoraEncoder::encodeTheora(yuv_buffer* inYUVBuffer) 
{
	const int NOT_LAST_FRAME = 0;
	//const int IS_LAST_FRAME = 1;
	int retVal = 0;


	ogg_packet locOldOggPacket;
	retVal = theora_encode_YUVin(&mTheoraState, inYUVBuffer);
	
	if (retVal != 0) {
		//FAILED
		return NULL;
	}

	//We don't delete the buffer we get back in the old ogg packet, it's owned by libtheora
	retVal = theora_encode_packetout(&mTheoraState, NOT_LAST_FRAME, &locOldOggPacket);
	
	if (retVal != 1) {
		//Weird return convention.
		return NULL;
	}

	return oldToNewPacket(&locOldOggPacket);
	
}
