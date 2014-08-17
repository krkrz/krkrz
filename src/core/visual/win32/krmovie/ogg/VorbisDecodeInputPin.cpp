//===========================================================================
//Copyright (C) 2003-2006 Zentaro Kavanagh
//Copyright (C) 2009 Cristian Adam
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
#include "VorbisDecodeInputPin.h"
#include <assert.h>
#include "OggTypes.h"
#include "VorbisTypes.h"

VorbisDecodeInputPin::VorbisDecodeInputPin(AbstractTransformFilter* inFilter,    CCritSec* inFilterLock,    
                                           AbstractTransformOutputPin* inOutputPin, 
                                           vector<CMediaType*> inAcceptableMediaTypes) :    
AbstractTransformInputPin (inFilter, inFilterLock,    inOutputPin, NAME("VorbisDecodeInputPin"),    
                           L"Vorbis In", inAcceptableMediaTypes),    
mBegun(false),    
mNumChannels(0),    
mFrameSize(0),    
mSampleRate(0),
// mUptoFrame(0),
mSetupState(VSS_SEEN_NOTHING),    
mDecodedBuffer(NULL),    
mDecodedByteCount(0),    
mRateNumerator(RATE_DENOMINATOR),    
mOggOutputPinInterface(NULL),    
mSentStreamOffset(false),
m_isVorbisFormat2(false),
m_isDownmix(false)
{
    LOG(logDEBUG) << "Pin constructor";
    ConstructCodec();
    LOG(logDEBUG) << "Pin constructor - post construct codec";

    mDecodedBuffer = new unsigned char[DECODED_BUFFER_SIZE];
}

VorbisDecodeInputPin::~VorbisDecodeInputPin(void)
{
    DestroyCodec();
    delete[] mDecodedBuffer;
}

//Is this needed ??
STDMETHODIMP VorbisDecodeInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IMediaSeeking) 
    {
        return GetInterface((IMediaSeeking*)this, ppv);
    } 
    else if (riid == IID_IOggDecoder) 
    {
        return GetInterface((IOggDecoder*)this, ppv);
    }
    else if (riid == IID_IDownmixAudio)
    {
        return GetInterface((IDownmixAudio*)this, ppv);
    }

    return AbstractTransformInputPin::NonDelegatingQueryInterface(riid, ppv); 
}

bool VorbisDecodeInputPin::ConstructCodec() 
{
    return true;
    //Vorbis decoder should be good to go
}

void VorbisDecodeInputPin::DestroyCodec() 
{

}

STDMETHODIMP VorbisDecodeInputPin::NewSegment(REFERENCE_TIME inStartTime, REFERENCE_TIME inStopTime, double inRate) 
{
    CAutoLock locLock(mStreamLock);
    //LOG(logDEBUG) << "New segment " << inStartTime<< " - " << inStopTime;
    //mUptoFrame = 0;
    mRateNumerator = (__int64)(RATE_DENOMINATOR * inRate);
    if (mRateNumerator > RATE_DENOMINATOR) 
    {
        mRateNumerator = RATE_DENOMINATOR;
    }
    return AbstractTransformInputPin::NewSegment(inStartTime, inStopTime, inRate);
}

STDMETHODIMP VorbisDecodeInputPin::EndFlush()
{
    CAutoLock locLock(m_pLock);
    
    HRESULT locHR = AbstractTransformInputPin::EndFlush();
    mDecodedByteCount = 0;

    return locHR;
}

STDMETHODIMP VorbisDecodeInputPin::Receive(IMediaSample* inSample) 
{
    CAutoLock locLock(mStreamLock);

    HRESULT hr = CheckStreaming();

    if (hr != S_OK) 
    {
        //Not streaming - Bail out.
        return S_FALSE;
    }

    BYTE* buffer = NULL;
    hr = inSample->GetPointer(&buffer);

    if (hr != S_OK) 
    {
        LOG(logERROR) << __FUNCTIONW__ << " inSample->GetPointer failed, error code: 0x" << std::hex << hr;
        return hr;
    } 

    REFERENCE_TIME tStart = -1;
    REFERENCE_TIME tStop = -1;
    __int64 sampleDuration = 0;
    inSample->GetTime(&tStart, &tStop);

    if (m_isVorbisFormat2) // Support VORBISFORMAT2
    {
        hr = TransformVorbis2(buffer, inSample->GetActualDataLength());
        if (hr != S_OK) 
            return S_FALSE;

        IMediaSample* sample;
        HRESULT hr = mOutputPin->GetDeliveryBuffer(&sample, NULL, NULL, NULL);
        if (hr != S_OK) 
            return hr;

        BYTE* buffer = NULL;
        hr = sample->GetPointer(&buffer);
        if (hr != S_OK) 
            return hr;

        reorderChannels(buffer, mDecodedBuffer, mDecodedByteCount);
        //memcpy((void*)buffer, (const void*)mDecodedBuffer, mDecodedByteCount);

        sample->SetTime(&tStart, &tStop);
        sample->SetSyncPoint(TRUE);
        hr = sample->SetActualDataLength(mDecodedByteCount);
        assert(hr == S_OK);

        hr = ((VorbisDecodeOutputPin*)(mOutputPin))->mDataQueue->Receive(sample);
        if (hr != S_OK) 
            return hr;
    }
    else
    {
        hr = TransformData(buffer, inSample->GetActualDataLength());
        if (hr != S_OK) 
        {
            return S_FALSE;
        }
        if (tStop > 0) 
        {
            //Can dump it all downstream now    
            IMediaSample* sample;
            unsigned long bytesCopied = 0;
            unsigned long bytesToCopy = 0;

            REFERENCE_TIME globalOffset = 0;
            if (mFrameSize != 0 && mSampleRate != 0)
            {
                tStart = convertGranuleToTime(tStop) - (((mDecodedByteCount / mFrameSize) * UNITS) / mSampleRate);
            }

            //Handle stream offsetting
            if (!mSentStreamOffset && (mOggOutputPinInterface != NULL)) 
            {
                mOggOutputPinInterface->notifyStreamBaseTime(tStart);
                mSentStreamOffset = true;    
            }

            if (mOggOutputPinInterface != NULL) 
            {
                globalOffset = mOggOutputPinInterface->getGlobalBaseTime();
            }

            do 
            {
                HRESULT hr = mOutputPin->GetDeliveryBuffer(&sample, NULL, NULL, NULL);
                if (hr != S_OK) 
                {
                    return hr;
                }

                BYTE* locBuffer = NULL;
                hr = sample->GetPointer(&locBuffer);

                if (hr != S_OK) 
                {
                    return hr;
                }

                LOG(logDEBUG4) << __FUNCTIONW__ << " Sample Size: " << sample->GetSize();
                bytesToCopy = sample->GetSize();

                if (mDecodedByteCount - bytesCopied < (unsigned long)sample->GetSize()) 
                {
                    bytesToCopy = mDecodedByteCount - bytesCopied;
                }

                LOG(logDEBUG4) << __FUNCTIONW__ << " Bytes to copy: " << bytesToCopy;

                sampleDuration = (((bytesToCopy/mFrameSize) * UNITS) / mSampleRate);
                tStop = tStart + sampleDuration;

                //Adjust the time stamps for rate and seeking
                REFERENCE_TIME adjustedStart = (tStart * RATE_DENOMINATOR) / mRateNumerator;
                REFERENCE_TIME adjustedStop = (tStop * RATE_DENOMINATOR) / mRateNumerator;
                adjustedStart -= (m_tStart + globalOffset);
                adjustedStop -= (m_tStart + globalOffset);

                __int64 seekStripOffset = 0;
                if (adjustedStop < 0) 
                {
                    sample->Release();
                } 
                else 
                {
                    if (adjustedStart < 0) 
                    {
                        seekStripOffset = (-adjustedStart) * mSampleRate;
                        seekStripOffset *= mFrameSize;
                        seekStripOffset /= UNITS;
                        seekStripOffset += (mFrameSize - (seekStripOffset % mFrameSize));
                        __int64 strippedDuration = (((seekStripOffset/mFrameSize) * UNITS) / mSampleRate);
                        adjustedStart += strippedDuration;
                    }                    

                    LOG(logDEBUG4) << __FUNCTIONW__ << " Seek strip offset: " << seekStripOffset;

                    if (bytesToCopy - seekStripOffset < 0)
                    {
                        sample->Release();
                    }
                    else
                    {
                        //memcpy((void*)locBuffer, (const void*)&mDecodedBuffer[bytesCopied + seekStripOffset], bytesToCopy - seekStripOffset);
                        reorderChannels(locBuffer, &mDecodedBuffer[bytesCopied + seekStripOffset], (unsigned long)(bytesToCopy - seekStripOffset));

                        sample->SetTime(&adjustedStart, &adjustedStop);
                        sample->SetMediaTime(&tStart, &tStop);
                        sample->SetSyncPoint(TRUE);
                        sample->SetActualDataLength((long)(bytesToCopy - seekStripOffset));
                        hr = ((VorbisDecodeOutputPin*)(mOutputPin))->mDataQueue->Receive(sample);
                        if (hr != S_OK) 
                        {
                            return hr;
                        }
                        tStart += sampleDuration;
                    }
                }
                bytesCopied += bytesToCopy;


            } while(bytesCopied < mDecodedByteCount);

            LOG(logDEBUG4) << __FUNCTIONW__ << " Decoded byte count: " << mDecodedByteCount;

            mDecodedByteCount = 0;

        }
    }
    return S_OK;
}



void VorbisDecodeInputPin::reorderChannels(unsigned char* inDestBuffer, const unsigned char* inSourceBuffer, unsigned long inNumBytes)
{
    const short* channel_order;
    const short channel_order_mono[] = {0};
    const short channel_order_stereo[] = {0, 1};
    const short channel_order_three[] = {0, 2, 1};
    const short channel_order_four[] = {0, 1, 2, 3};
    const short channel_order_five[] = {0, 2, 1, 3, 4};
    const short channel_order_5_1[] = {0, 2, 1, 5, 3, 4};
    const short channel_order_6_1[] = {0, 2, 1, 6, 5, 3, 4};
    const short channel_order_7_1[] = {0, 2, 1, 7, 5, 6, 3, 4};

    switch (mNumChannels) {
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
            memcpy((void*)inDestBuffer, (const void*)inSourceBuffer, inNumBytes);
            return;
    }

    const unsigned long locSampleCount = inNumBytes / (mNumChannels * sizeof(short));

    short* locDest = (short*)inDestBuffer;
    const short* locSource = (short*)inSourceBuffer;

    //Test code
    //bool setDownmix = true;
    //setDownmixAudio(setDownmix);

    if (getDownmixAudio()) 
    {
        for (unsigned long i = 0; i < locSampleCount; ++i) 
        {
            for (unsigned long chan = 0; chan < (unsigned long)mNumChannels; ++chan) 
            {
                switch (mNumChannels) 
                {
                case 3: // 3 ch.
                    locDest[0] = locSource[0] + static_cast<short>(0.7 * locSource[1]); 
                    locDest[1] = locSource[2] + static_cast<short>(0.7 * locSource[1]); 
                    break;
                case 4: // 4 ch.
                    locDest[0] = locSource[0] + locSource[2];
                    locDest[1] = locSource[1] + locSource[3];
                    break;
                case 5: // 5 ch.
                    locDest[0] = locSource[0] + static_cast<short>(0.7 * locSource[1]) + static_cast<short>(0.7 * locSource[3]);
                    locDest[1] = locSource[2] + static_cast<short>(0.7 * locSource[1]) + static_cast<short>(0.7 * locSource[4]);
                    break;
                case 6: // 5.1 ch. 
                    locDest[0] = locSource[0] + locSource[5] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[3]); 
                    locDest[1] = locSource[2] + locSource[5] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[4]);
                    break;
                case 7: // 6.1 ch. 
                    locDest[0] = locSource[0] + locSource[7] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[3]) + static_cast<short>(0.7 * locSource[5]);
                    locDest[1] = locSource[2] + locSource[7] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[4]) + static_cast<short>(0.7 * locSource[6]);
                    break;
                case 8: // 7.1 ch. 
                    locDest[0] = locSource[0] + locSource[7] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[3]) + static_cast<short>(0.7 * locSource[5]);
                    locDest[1] = locSource[2] + locSource[7] + static_cast<short>(0.7 * locSource[1])\
                        + static_cast<short>(0.7 * locSource[4]) + static_cast<short>(0.7 * locSource[6]);
                    break;
                }
                locDest += mNumChannels;
                locSource += mNumChannels;
            }
        }
    }
    else 
    {
        for (unsigned long i = 0; i < locSampleCount; ++i) 
        {
            for (unsigned long chan = 0; chan < (unsigned long)mNumChannels; ++chan) 
            {
                *locDest++ = locSource[channel_order[chan]];
            }
            locSource += mNumChannels;
        }
    }

    return;
}

HRESULT VorbisDecodeInputPin::TransformData(BYTE* inBuf, long inNumBytes) 
{
    //TODO::: Return types !!!

    VorbisDecoder::eVorbisResult locResult;
    unsigned long locNumSamples = 0;
    locResult = mVorbisDecoder.DecodePacket(inBuf, inNumBytes,(short*) (mDecodedBuffer + mDecodedByteCount), locNumSamples);

    if (locResult == VorbisDecoder::VORBIS_DATA_OK) 
    {
        mDecodedByteCount += locNumSamples * mFrameSize;
        return S_OK;
    }

    //For now, just silently ignore busted packets.
    return S_OK;
}

HRESULT VorbisDecodeInputPin::TransformVorbis2(const BYTE* const buffer, const long length_of_buffer) 
{
    memset(mDecodedBuffer, 0, DECODED_BUFFER_SIZE);
    unsigned long num_of_samples = 0;
    const VorbisDecoder::eVorbisResult hr = mVorbisDecoder.DecodePacket(buffer, length_of_buffer, reinterpret_cast<short*>(mDecodedBuffer), num_of_samples);

    if (hr == VorbisDecoder::VORBIS_DATA_OK) 
    {
        mDecodedByteCount = num_of_samples * mFrameSize;
        return S_OK;
    }

    return S_OK;
}

VorbisDecodeFilter* VorbisDecodeInputPin::GetFilter()
{
    return static_cast<VorbisDecodeFilter*>(mParentFilter);
}

HRESULT VorbisDecodeInputPin::SetMediaType(const CMediaType* inMediaType) 
{
    LOG(logDEBUG) << __FUNCTIONW__;

    //FIX:::Error checking
    if (CheckMediaType(inMediaType) == S_OK) 
    {
        if (inMediaType->majortype == MEDIATYPE_OggPacketStream &&
            inMediaType->formattype == FORMAT_OggIdentHeader &&
            inMediaType->cbFormat == VORBIS_IDENT_HEADER_SIZE) 
        {
            LOG(logINFO) << __FUNCTIONW__ << " MEDIATYPE_OggPacketStream, FORMAT_OggIdentHeader";
            GetFilter()->setVorbisFormat(inMediaType->pbFormat);
        }
        else if (inMediaType->majortype == MEDIATYPE_Audio &&
                 inMediaType->subtype == MEDIASUBTYPE_Vorbis &&
                 inMediaType->formattype == FORMAT_Vorbis)
        {
            LOG(logINFO) << __FUNCTIONW__ << " MEDIATYPE_Audio, MEDIASUBTYPE_Vorbis";
            GetFilter()->setVorbisFormat(reinterpret_cast<VORBISFORMAT*>(inMediaType->pbFormat));
        }
        else if (inMediaType->majortype == MEDIATYPE_Audio &&
            inMediaType->subtype == MEDIASUBTYPE_Vorbis2 &&
            inMediaType->formattype == FORMAT_Vorbis2)
        {
            LOG(logINFO) << __FUNCTIONW__ << " MEDIATYPE_Audio, MEDIASUBTYPE_Vorbis2";

            m_isVorbisFormat2 = true;
            GetFilter()->setVorbisFormat(reinterpret_cast<VORBISFORMAT2*>(inMediaType->pbFormat));
            mVorbisDecoder.Init(reinterpret_cast<VORBISFORMAT2*>(inMediaType->pbFormat));
        }

    } 
    else 
    {
        LOG(logERROR) << __FUNCTIONW__ << " MediaType not OK, Exiting";
        throw 0;
    }
    
    return CBaseInputPin::SetMediaType(inMediaType);
}
HRESULT VorbisDecodeInputPin::CheckMediaType(const CMediaType *inMediaType)
{
    if (inMediaType->majortype == MEDIATYPE_OggPacketStream &&
        inMediaType->formattype == FORMAT_OggIdentHeader &&
        inMediaType->cbFormat == VORBIS_IDENT_HEADER_SIZE) 
    {
        if (strncmp((char*)inMediaType->pbFormat, "\001vorbis", 7) == 0) 
        {
            LOG(logDEBUG) << "Check media type OK (MEDIATYPE_OggPacketStream)";
            return S_OK;
        }
    }
    else if (inMediaType->majortype == MEDIATYPE_Audio &&
             inMediaType->subtype == MEDIASUBTYPE_Vorbis &&
             inMediaType->formattype == FORMAT_Vorbis)
    {
        LOG(logDEBUG) << "Check media type OK (MEDIASUBTYPE_Vorbis)";
        return S_OK;
    }
    else if (inMediaType->majortype == MEDIATYPE_Audio &&
             inMediaType->subtype == MEDIASUBTYPE_Vorbis2 &&
             inMediaType->formattype == FORMAT_Vorbis2)
    {
        LOG(logDEBUG) << "Check media type OK (MEDIASUBTYPE_Vorbis2)";
        return S_OK;
    }

    LOG(logDEBUG) << __FUNCTIONW__ << " Input type not OK.";
    if (inMediaType->cbFormat > 7)
    {
        char format[8] = {};
        //strncpy(format, reinterpret_cast<const char*>(inMediaType->pbFormat), 7);
		strncpy_s(format, 8, reinterpret_cast<const char*>(inMediaType->pbFormat), 7);
        LOG(logDEBUG) << __FUNCTIONW__ << " cbFormat start: " << format;
    }

    return S_FALSE;
}

HRESULT VorbisDecodeInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *outRequestedProps)
{
    outRequestedProps->cbBuffer = VORBIS_BUFFER_SIZE;
    outRequestedProps->cBuffers = VORBIS_NUM_BUFFERS;
    outRequestedProps->cbAlign = 1;
    outRequestedProps->cbPrefix = 0;

    return S_OK;
}

LOOG_INT64 VorbisDecodeInputPin::convertGranuleToTime(LOOG_INT64 inGranule)
{
    if (mBegun) 
    {    
        return (inGranule * UNITS) / mSampleRate;
    } 
    else 
    {
        return -1;
    }
}

LOOG_INT64 VorbisDecodeInputPin::mustSeekBefore(LOOG_INT64 inGranule)
{
    //TODO::: Get adjustment from block size info... for now, it doesn't matter if no preroll
    //return (inGranule <= 4096) ? 0 : (inGranule - 4096);
    return inGranule;
}
IOggDecoder::eAcceptHeaderResult VorbisDecodeInputPin::showHeaderPacket(OggPacket* inCodecHeaderPacket)
{
    LOG(logDEBUG) << __FUNCTIONW__ << " SetupState: " << mSetupState;
    unsigned long locDummy;
    switch (mSetupState) 
    {
        case VSS_SEEN_NOTHING:
            if (strncmp((char*)inCodecHeaderPacket->packetData(), "\001vorbis", 7) == 0) 
            {
                //TODO::: Possibly verify version
                if (mVorbisDecoder.DecodePacket(        inCodecHeaderPacket->packetData()
                                                    ,    inCodecHeaderPacket->packetSize()
                                                    ,    NULL
                                                    ,    locDummy) == VorbisDecoder::VORBIS_HEADER_OK) {
                    mSetupState = VSS_SEEN_BOS;
                    LOG(logDEBUG) << "Saw first header";
                    return IOggDecoder::AHR_MORE_HEADERS_TO_COME;
                }
            }
            return IOggDecoder::AHR_INVALID_HEADER;
            
            
        case VSS_SEEN_BOS:
            if (strncmp((char*)inCodecHeaderPacket->packetData(), "\003vorbis", 7) == 0) 
            {
                if (mVorbisDecoder.DecodePacket(        inCodecHeaderPacket->packetData()
                                                    ,    inCodecHeaderPacket->packetSize()
                                                    ,    NULL
                                                    ,    locDummy) == VorbisDecoder::VORBIS_COMMENT_OK) {

                    mSetupState = VSS_SEEN_COMMENT;
                    LOG(logDEBUG) << "Saw second header";
                    return IOggDecoder::AHR_MORE_HEADERS_TO_COME;
                }    
            }
            return IOggDecoder::AHR_INVALID_HEADER;
            
            
        case VSS_SEEN_COMMENT:
            if (strncmp((char*)inCodecHeaderPacket->packetData(), "\005vorbis", 7) == 0) 
            {
                if (mVorbisDecoder.DecodePacket(        inCodecHeaderPacket->packetData()
                                                    ,    inCodecHeaderPacket->packetSize()
                                                    ,    NULL
                                                    ,    locDummy) == VorbisDecoder::VORBIS_CODEBOOK_OK) {

        
                    //Is mBegun useful ?
                    mBegun = true;
            
                    mNumChannels = mVorbisDecoder.numChannels();
                    mFrameSize = mNumChannels * SIZE_16_BITS;
                    mSampleRate = mVorbisDecoder.sampleRate(); 

        
                    mSetupState = VSS_ALL_HEADERS_SEEN;
                    LOG(logDEBUG) << "Saw third header";
                    return IOggDecoder::AHR_ALL_HEADERS_RECEIVED;
                }
                
            }
            return IOggDecoder::AHR_INVALID_HEADER;
            
        case VSS_ALL_HEADERS_SEEN:
        case VSS_ERROR:
        default:
            return IOggDecoder::AHR_UNEXPECTED;
    }
}

string VorbisDecodeInputPin::getCodecShortName()
{
    return "vorbis";
}

string VorbisDecodeInputPin::getCodecIdentString()
{
    //TODO::: Get full ident string
    return "vorbis";
}

HRESULT VorbisDecodeInputPin::CompleteConnect(IPin *inReceivePin)
{
    IOggOutputPin* locOggOutput = NULL;
    mSentStreamOffset = false;
    HRESULT locHR = inReceivePin->QueryInterface(IID_IOggOutputPin, (void**)&locOggOutput);
    if (locHR == S_OK) 
    {
        mOggOutputPinInterface = locOggOutput;    
    } 
    else 
    {
        mOggOutputPinInterface = NULL;
    }
    LOG(logDEBUG) << __FUNCTIONW__ << " QueryInterface(IOggOutputPin) " << std::boolalpha 
        << (mOggOutputPinInterface != NULL ? "succeeded" : "failed");

    if (GetFilter()->getVorbisFormatBlock())
    {
        mNumChannels = GetFilter()->getVorbisFormatBlock()->numChannels;
        mFrameSize = mNumChannels * SIZE_16_BITS;
        mSampleRate = GetFilter()->getVorbisFormatBlock()->samplesPerSec;
    }

    return AbstractTransformInputPin::CompleteConnect(inReceivePin);
}

void VorbisDecodeInputPin::setDownmixAudio(const bool setDownmix)
{
     m_isDownmix = setDownmix;
     return;
}

bool VorbisDecodeInputPin::getDownmixAudio()
{
    return m_isDownmix;
}
