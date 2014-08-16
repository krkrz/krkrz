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

class VorbisEncodeSettings
{
public:
    VorbisEncodeSettings(void);
    ~VorbisEncodeSettings(void);

    static const int MIN_QUALITY = -20;
    static const int MAX_QUALITY = 100;

    void setToDefaults();

    bool setQuality(int inQuality);
    bool setBitrateQualityMode(int inBitrate);
    bool setManaged(int inBitrate, int inMinBitrate, int inMaxBitrate);
    bool setAudioParameters(unsigned int inNumChannels, unsigned int inSampleRate);

    unsigned int mNumChannels;
    unsigned int mSampleRate;

    bool mIsManaged;
    int mBitrate;
    int mMinBitrate;
    int mMaxBitrate;
    int mQuality;
    bool mIsQualitySet;

    //More inof at
    //http://svn.xiph.org/trunk/vorbis-tools/oggenc/encode.c
    //http://svn.xiph.org/trunk/vorbis-tools/oggenc/encode.h
    //http://wiki.hydrogenaudio.org/index.php?title=Recommended_Ogg_Vorbis
    //http://www.hydrogenaudio.org/forums/index.php?showtopic=15049
    //http://www.hydrogenaudio.org/forums/index.php?showtopic=15049&st=160&p=357461&#entry357461

    //Advanced
    //double mBitrateAverageDamping;
    //unsigned int mBitrateAverage;
    //double mBitReservoirBias;
    //unsigned int mBitReservoirBits;
    //unsigned int mBitrateHardMin;
    //unsigned int mBitrateHardMax;
    //double mImpulseNoiseTune;
    //double mLowpassFrequency;

};
