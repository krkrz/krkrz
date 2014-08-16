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

#include "StdAfx.h"
#include "VorbisEncoder.h"

VorbisEncoder::VorbisEncoder(void)
{
}

VorbisEncoder::~VorbisEncoder(void)
{
}

vector<StampedOggPacket*> VorbisEncoder::setupCodec(VorbisEncodeSettings inSettings)
{
    mSettings = inSettings;
    vector<StampedOggPacket*> retPackets;

    int ret = 0;
    vorbis_info_init(&mVorbisInfo);
    vorbis_comment_init(&mVorbisComment);
    //TODO::: Add version/encoder info here?
    //TODO::: Allow max and min in unmanaged mode
    //TODO::: Allow advanced bitrate management

    if (mSettings.mIsQualitySet) {
        ret = vorbis_encode_setup_vbr(&mVorbisInfo, mSettings.mNumChannels, mSettings.mSampleRate, (float)inSettings.mQuality / 100.0f);
        if (ret != 0) {
            vorbis_info_clear(&mVorbisInfo);
            return retPackets;
        }
    } else {
        ret = vorbis_encode_setup_managed(&mVorbisInfo, mSettings.mNumChannels, mSettings.mSampleRate, mSettings.mMaxBitrate * 1000, mSettings.mBitrate * 1000, mSettings.mMinBitrate * 1000);
        if (ret != 0) {
            vorbis_info_clear(&mVorbisInfo);
            return retPackets;
        }
    }

    if (!mSettings.mIsManaged) {
        vorbis_encode_ctl(&mVorbisInfo, OV_ECTL_RATEMANAGE2_SET, NULL);
    }

    //Advanced options?

    vorbis_encode_setup_init(&mVorbisInfo);
    vorbis_analysis_init(&mVorbisDSPState, &mVorbisInfo);
    vorbis_block_init(&mVorbisDSPState, &mVorbisBlock);

    ogg_packet locIdentHeader;
    ogg_packet locCommentHeader;
    ogg_packet locCodebookHeader;

    vorbis_analysis_headerout(&mVorbisDSPState, &mVorbisComment, &locIdentHeader, &locCommentHeader, &locCodebookHeader);
    
    retPackets.push_back(oldToNewPacket(&locIdentHeader));
    retPackets.push_back(oldToNewPacket(&locCommentHeader));
    retPackets.push_back(oldToNewPacket(&locCodebookHeader));

    return retPackets;
}

StampedOggPacket* VorbisEncoder::oldToNewPacket(ogg_packet* inOldPacket)
{
    //This is duplicated from the theora encoder... do something about that.
	const unsigned char NOT_USED = 0;

	//Need to clone the packet data
	unsigned char* locBuff = new unsigned char[inOldPacket->bytes];
	memcpy((void*)locBuff, (const void*)inOldPacket->packet, inOldPacket->bytes);
																					//Not truncated or continued... it's a full packet.
	StampedOggPacket* locOggPacket = new StampedOggPacket(locBuff, inOldPacket->bytes, false, false, NOT_USED, inOldPacket->granulepos, StampedOggPacket::OGG_END_ONLY);
	return locOggPacket;

}

vector<StampedOggPacket*> VorbisEncoder::encodeVorbis(const short* const inSampleBuffer, unsigned long inNumSamplesPerChannel)
{
    if (inNumSamplesPerChannel == 0) {
        return vector<StampedOggPacket*>();
    }

    float** locBuffer = vorbis_analysis_buffer(&mVorbisDSPState, inNumSamplesPerChannel);

    float* locOneOutputChannelBuffer = NULL;
    const short* locReadChannelBuffer = NULL;

	const short* channel_order;
	const short channel_order_mono[] = {0};
	const short channel_order_stereo[] = {0, 1};
	const short channel_order_three[] = {0, 2, 1};
	const short channel_order_four[] = {0, 1, 2, 3};
	const short channel_order_five[] = {0, 2, 1, 3, 4};
	const short channel_order_5_1[] = {0, 2, 1, 5, 3, 4};
	const short channel_order_6_1[] = {0, 2, 1, 6, 5, 3, 4};
	const short channel_order_7_1[] = {0, 2, 1, 7, 5, 6, 3, 4};

	switch (mSettings.mNumChannels) {
        case 1: // mono ch. 
		    channel_order = channel_order_mono;
            break;
        case 2: // stereo ch. 
		    channel_order = channel_order_stereo;
            break;
        case 3: // 3 ch. 
			channel_order = channel_order_three;
            break;
        case 4: // 4 ch. 
			channel_order = channel_order_four;
            break;
        case 5: // 5 ch. 
			channel_order = channel_order_five;
            break;
        case 6: // 5.1 ch. 
			channel_order = channel_order_5_1;
            break;
        case 7: // 6.1 ch. 
			channel_order = channel_order_6_1;
            break;
        case 8: // 7.1 ch.
			channel_order = channel_order_7_1;
            break;
		default:
			channel_order = channel_order_mono;  //?
			break;
	}

    for (unsigned long chan = 0; chan < mSettings.mNumChannels; chan++) {
        //locOneOutputChannelBuffer = locBuffer[chan];
        locOneOutputChannelBuffer = locBuffer[channel_order[chan]];
        locReadChannelBuffer = inSampleBuffer + chan;

        for (unsigned long sam = 0; sam < inNumSamplesPerChannel; sam++) {
            locOneOutputChannelBuffer[sam] = ((float)(*locReadChannelBuffer)) / 32768.0f;
            locReadChannelBuffer += mSettings.mNumChannels;
        }
    }

    vorbis_analysis_wrote(&mVorbisDSPState, inNumSamplesPerChannel);

    return extractOutputPackets();
}

vector<StampedOggPacket*> VorbisEncoder::flush()
{
    vorbis_analysis_wrote(&mVorbisDSPState, 0);
    return extractOutputPackets();
}

vector<StampedOggPacket*> VorbisEncoder::extractOutputPackets()
{
    vector<StampedOggPacket*> retPackets;
    ogg_packet locWorkingPacket;

    while (vorbis_analysis_blockout(&mVorbisDSPState, &mVorbisBlock) == 1) {
        vorbis_analysis(&mVorbisBlock, NULL);
        vorbis_bitrate_addblock(&mVorbisBlock);

        while (vorbis_bitrate_flushpacket(&mVorbisDSPState, &locWorkingPacket)) {
            retPackets.push_back(oldToNewPacket(&locWorkingPacket));
        }
    }

    return retPackets;
}
