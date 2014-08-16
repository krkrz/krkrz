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



#include <vorbis/vorbisenc.h>

#include <libOOOgg/dllstuff.h>
#include <libOOOgg/StampedOggPacket.h>

#include "VorbisEncodeSettings.h"

#include <vector>

using namespace std;

class VorbisEncoder
{
public:
    VorbisEncoder(void);
    ~VorbisEncoder(void);

    static const unsigned long NUM_VORBIS_HEADERS = 3;

    vector<StampedOggPacket*> setupCodec(VorbisEncodeSettings inSettings);
    vector<StampedOggPacket*> encodeVorbis(const short* const inSampleBuffer, unsigned long inNumSamplesPerChannel);
    vector<StampedOggPacket*> flush();
protected:
	/// Converts a xiph like ogg packet into a StampedOggPacket.
	StampedOggPacket* oldToNewPacket(ogg_packet* inPacket);

    vector<StampedOggPacket*> extractOutputPackets();

    vorbis_info mVorbisInfo;
    vorbis_comment mVorbisComment;
    vorbis_dsp_state mVorbisDSPState;
    vorbis_block mVorbisBlock;

    VorbisEncodeSettings mSettings;
};
