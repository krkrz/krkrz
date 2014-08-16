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
#include "VorbisEncodeSettings.h"

VorbisEncodeSettings::VorbisEncodeSettings(void)
{
    setToDefaults();
}

VorbisEncodeSettings::~VorbisEncodeSettings(void)
{
}

void VorbisEncodeSettings::setToDefaults()
{
    mIsManaged = false;
    mBitrate = 0;
    mMinBitrate = 0;
    mMaxBitrate = 0;
    mQuality = 30;
    mIsQualitySet = true;

    //TODO::: Do we want to do this?
    mNumChannels = 0;
    mSampleRate = 0;

    //Advanced
    //double mBitrateAverageDamping;
    //unsigned int mBitrateAverage;
    //double mBitReservoirBias;
    //unsigned int mBitReservoirBits;
    //unsigned int mBitrateHardMin;
    //unsigned int mBitrateHardMax;
    //double mImpulseNoiseTune;
    //double mLowpassFrequency;
}

bool VorbisEncodeSettings::setQuality(int inQuality)
{
    if ((inQuality >= MIN_QUALITY) && (inQuality <= MAX_QUALITY)) {
        mQuality = inQuality;
        mIsManaged = false;
        mIsQualitySet = true;
        return true;
    }
    return false;

}

bool VorbisEncodeSettings::setBitrateQualityMode(int inBitrate)
{
    //TODO::: Can set hard min max too apparently
    if (inBitrate > 0) {
        mMinBitrate = -1;
        mMaxBitrate = -1;
        mQuality = 0;
        mBitrate = inBitrate;
        
        mIsManaged = false;
        mIsQualitySet = true;
    }
    return false;

}

bool VorbisEncodeSettings::setManaged(int inBitrate, int inMinBitrate, int inMaxBitrate)
{
    //TODO::: What other things to check?
    if (inMinBitrate < inMaxBitrate) {
        mBitrate = inBitrate;
        mMinBitrate = inMinBitrate;
        mMaxBitrate = inMaxBitrate;
        mIsManaged = true;
        mIsQualitySet = false;
        return true;
    }
    return false;
}

bool VorbisEncodeSettings::setAudioParameters(unsigned int inNumChannels, unsigned int inSampleRate)
{
    //TODO::: What else to check?
    if ((inNumChannels == 0) || (inSampleRate == 0)) {
        return false;
    }
    mNumChannels = inNumChannels;
    mSampleRate = inSampleRate;
    return true;
}