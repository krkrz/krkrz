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

#pragma once
#include <StampedOggPacket.h>

extern "C" {
#include <theora/theora.h>
#include <theora/codec.h>
#include <theora/theoradec.h>
}

//Need this for yuv_buffer for now.
//#include "theora_cdecl.h"

class TheoraDecoder
{
public:
	TheoraDecoder();
	~TheoraDecoder();

	/// Initialise the internal theora decoder.
	bool initCodec();

	/// Decode a theora packet returning a yuv_buffer struct. Now owns your packet.
	yuv_buffer* decodeTheora(StampedOggPacket* inPacket);

	/// Returns true if the packet is a keyframe.
	bool isKeyFrame(StampedOggPacket* inPacket);

    th_pixel_fmt GetPixelFormat() const;

protected:

	/// Moves the pointers around to make it look like a xiph ogg packet.
	ogg_packet* simulateOldOggPacket(StampedOggPacket* inPacket);

	/// Decodes a header packet and adjusts the internal state based on it.
	bool decodeHeader(StampedOggPacket* inHeaderPacket);

	//theora_info mTheoraInfo;

    th_info mTheoraInfo;
    th_ycbcr_buffer mYCbCrBuffer;
	th_comment mTheoraComment;
	th_setup_info* mTheoraSetup;
	th_dec_ctx* mTheoraState;
	yuv_buffer mYUVBuffer;

	StampedOggPacket* mPartialPacket; //TEMP !!
	unsigned long mHeadersSeen;
	bool mFirstPacket;
	bool mFirstHeader;
	unsigned long mPacketCount;
	
};
