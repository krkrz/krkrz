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

#include "oggstdafx.h"
#include <guiddef.h>
#include "VorbisDecoder.h"
#include "VorbisTypes.h"

#include <assert.h>

#ifdef USING_TREMOR
    typedef ogg_int32_t pcm_t;
#else
    typedef float pcm_t;
#endif

VorbisDecoder::VorbisDecoder():	
mPacketCount(0),	
mNumChannels(0),	
mSampleRate(0)
{
	vorbis_info_init(&mVorbisInfo);
	vorbis_comment_init(&mVorbisComment);
}

VorbisDecoder::~VorbisDecoder()
{
	vorbis_block_clear(&mVorbisBlock);
	vorbis_dsp_clear(&mVorbisState);
	vorbis_info_clear(&mVorbisInfo);
	vorbis_comment_clear(&mVorbisComment);
}

VorbisDecoder::eVorbisResult VorbisDecoder::decodeHeader()
{
	int ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &mWorkPacket);
	if (ret < 0) 
    {
		return VORBIS_HEADER_BAD;
	}

	return VORBIS_HEADER_OK;
}

VorbisDecoder::eVorbisResult VorbisDecoder::decodeComment()
{
	int ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &mWorkPacket);
	if (ret < 0) 
    {
		return VORBIS_COMMENT_BAD;
	}

	return VORBIS_COMMENT_OK;
}

VorbisDecoder::eVorbisResult VorbisDecoder::decodeCodebook()
{
	int ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &mWorkPacket);
	if (ret < 0) 
    {
		return VORBIS_CODEBOOK_BAD;
	}

	ret = vorbis_synthesis_init(&mVorbisState, &mVorbisInfo);

	//TODO::: What return codes?

	ret = vorbis_block_init(&mVorbisState, &mVorbisBlock);

	mNumChannels = mVorbisInfo.channels;
	mSampleRate = mVorbisInfo.rate;

	return VORBIS_CODEBOOK_OK;
}

VorbisDecoder::eVorbisResult VorbisDecoder::PrepareEncoder()
{
    if (mPacketCount == 0) 
    {
        mPacketCount++;
        mWorkPacket.b_o_s = 1;
        return decodeHeader();
    } 
    else if (mPacketCount == 1) 
    {
        //Comment
        mPacketCount++;
        return decodeComment();
    } 
    else if (mPacketCount == 2) 
    {
        //Codebooks
        mPacketCount++;
        return decodeCodebook();
    } 

    return VORBIS_DATA_OK;
}

void VorbisDecoder::Init(const VORBISFORMAT2* vf2)
{
    assert(vf2);

    // Save the 3 vorbis header packets

    int vorbisId = vf2->headerSize[0];
    unsigned char* pVorbisId = (unsigned char*)vf2 + sizeof(VORBISFORMAT2);

    int vorbisComment = vf2->headerSize[1];
    unsigned char* pVorbisComment = (unsigned char*)vf2 + sizeof(VORBISFORMAT2) + vf2->headerSize[0];

    int vorbisCodebook = vf2->headerSize[2];
    unsigned char* pVorbisCodebook = (unsigned char*)vf2 + sizeof(VORBISFORMAT2) + vf2->headerSize[0] + vf2->headerSize[1];

    vorbis_info_init(&mVorbisInfo);
    vorbis_comment_init(&mVorbisComment);

    ogg_packet ogg_pkt;
    memset(&ogg_pkt, 0, sizeof(ogg_packet));

    ogg_pkt.packet = pVorbisId;
    ogg_pkt.bytes = vorbisId;
    ogg_pkt.b_o_s = 1;
    ogg_pkt.packetno = mPacketCount++;

    // Build the "Comment Header" packet
    ogg_packet ogg_comment;
    memset(&ogg_comment, 0, sizeof(ogg_packet));

    ogg_comment.packet = pVorbisComment;
    ogg_comment.bytes = vorbisComment;
    ogg_comment.b_o_s = 0;
    ogg_comment.packetno = mPacketCount++;

    // Build the "Setup Header" packet
    ogg_packet ogg_codecsetup;
    memset(&ogg_codecsetup, 0, sizeof(ogg_packet));
    ogg_codecsetup.packet = pVorbisCodebook;
    ogg_codecsetup.bytes = vorbisCodebook;
    ogg_codecsetup.b_o_s = 0;
    ogg_codecsetup.packetno = mPacketCount++;

    int ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &ogg_pkt);
    assert( ret >= 0);
    ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &ogg_comment);
    assert( ret >= 0);
    ret = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &ogg_codecsetup);
    assert( ret >= 0);

    vorbis_synthesis_init(&mVorbisState, &mVorbisInfo);
    vorbis_block_init(&mVorbisState, &mVorbisBlock);

    mNumChannels = mVorbisInfo.channels;
}

VorbisDecoder::eVorbisResult VorbisDecoder::DecodePacket( const unsigned char* const packet,
											const unsigned long packet_size,
											short* pcm_samples,
											unsigned long& num_of_pcm_samples)
{
	mWorkPacket.b_o_s = 0;
	mWorkPacket.bytes = packet_size;
	mWorkPacket.e_o_s = 0;
	mWorkPacket.granulepos = 0;
	mWorkPacket.packet = const_cast<unsigned char*>(packet);
	mWorkPacket.packetno = mPacketCount;

	mNumChannels = mVorbisInfo.channels;
	mSampleRate = mVorbisInfo.rate;

    if (mPacketCount <= 2)
    {
        return PrepareEncoder();
    }
    ++mPacketCount;

	if (vorbis_synthesis(&mVorbisBlock, &mWorkPacket) == 0)
	{
		const int ret = vorbis_synthesis_blockin(&mVorbisState, &mVorbisBlock);
		assert(ret == 0);
	}

    pcm_t** pcm;
	int samples = 0;
	
	while ((samples = vorbis_synthesis_pcmout(&mVorbisState, &pcm)) > 0) 
	{
		// TODO:: handling buffer overflow if the size is over 1 mega
		// samples * mNumChannels * sizeof(short) > 1 mega

		for (int ch = 0; ch < mNumChannels; ++ch)
		{
			short* temp_buffer = pcm_samples + ch;
			const pcm_t* const one_channel = pcm[ch];

			for (int i = 0; i < samples; ++i)
			{
				int temp = static_cast<int>(one_channel[i] * 32767.0f);
				*temp_buffer = clip16(temp);
				
				temp_buffer += mNumChannels;
			}
		}
		num_of_pcm_samples = samples;
		vorbis_synthesis_read(&mVorbisState, num_of_pcm_samples);
	}
	return VORBIS_DATA_OK;
}
