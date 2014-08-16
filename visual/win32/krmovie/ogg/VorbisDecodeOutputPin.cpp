//===========================================================================
//Copyright (C) 2003-2006 Zentaro Kavanagh
//Copyright (2) 2009-2010 Cristian Adam
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
#undef INITGUID
#include "Vorbisdecodeoutputpin.h"
#include "VorbisTypes.h"

VorbisDecodeOutputPin::VorbisDecodeOutputPin(VorbisDecodeFilter* inParentFilter, CCritSec* inFilterLock, 
    const MediaTypesList& inAcceptableMediaTypes) :	
AbstractTransformOutputPin(inParentFilter, inFilterLock, NAME("VorbisDecodeOutputPin"),	
    L"PCM Out", 65535, 20, inAcceptableMediaTypes)
{
}

VorbisDecodeOutputPin::~VorbisDecodeOutputPin()
{	
}

HRESULT VorbisDecodeOutputPin::DecideBufferSize(IMemAllocator* inAllocator, ALLOCATOR_PROPERTIES *inReqAllocProps)
{
    VORBISFORMAT* formatBlock = static_cast<VorbisDecodeFilter*>(m_pFilter)->getVorbisFormatBlock();
    if (formatBlock)
    {
        mDesiredBufferSize = formatBlock->numChannels * formatBlock->samplesPerSec * 2;
        mDesiredBufferCount = NUM_BUFFERS;
    }

    HRESULT hr = AbstractTransformOutputPin::DecideBufferSize(inAllocator, inReqAllocProps);

    LOG(logINFO) << "Desired buffer size: " << mDesiredBufferSize << ", buffers: " << mDesiredBufferCount
        << " Actual buffer size: " << mActualBufferSize << ", buffers: " << mActualBufferCount
        << " Result: 0x" << std::hex << hr;
    
    return hr;
}

STDMETHODIMP VorbisDecodeOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IMediaSeeking) 
    {
        return GetInterface((IMediaSeeking*)this, ppv);
	}

	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv); 
}

HRESULT VorbisDecodeOutputPin::CreateAndFillFormatBuffer(CMediaType* mediaType, int inPosition)
{
    HRESULT result = E_FAIL;

#ifdef WINCE
    if (inPosition == 0)
    {
        result = FillMediaType(*mediaType, false);
    }
#else
    if (inPosition == 0) 
    {
        result = FillMediaType(*mediaType, true);
    } 
    else if (inPosition == 1) 
    {
        result = FillMediaType(*mediaType, false);
    }
#endif

    return result;
}

HRESULT VorbisDecodeOutputPin::FillMediaType(CMediaType& mediaType, bool useWaveFormatEx)
{
    if (useWaveFormatEx)
    {
#ifndef WINCE
        WAVEFORMATEXTENSIBLE* formatEx = (WAVEFORMATEXTENSIBLE*)mediaType.AllocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));

        formatEx->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

        formatEx->Format.nChannels = 0;
        formatEx->Format.nSamplesPerSec = 0;
        if (GetFilter()->mVorbisFormatInfo)
        {
            formatEx->Format.nChannels = (WORD)GetFilter()->mVorbisFormatInfo->numChannels;
            formatEx->Format.nSamplesPerSec =  GetFilter()->mVorbisFormatInfo->samplesPerSec;
        }

        formatEx->Format.wBitsPerSample = 16;

        formatEx->Samples.wValidBitsPerSample = 16;

        switch (formatEx->Format.nChannels)
        {
        case 1:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_MONO;
            break;
        case 2:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
            break;
        case 3:
            formatEx->dwChannelMask = SPEAKER_FRONT_LEFT
                                    | SPEAKER_FRONT_RIGHT
                                    | SPEAKER_FRONT_CENTER;
            break;
        case 4:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_QUAD;
            break;
        case 5:
            formatEx->dwChannelMask = SPEAKER_FRONT_LEFT
                                    | SPEAKER_FRONT_RIGHT
                                    | SPEAKER_FRONT_CENTER
                                    | SPEAKER_BACK_LEFT
                                    | SPEAKER_BACK_RIGHT;
            break;
        case 6:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
            break;
        case 7:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_5POINT1_SURROUND 
                                    | SPEAKER_BACK_CENTER;
            break;
        case 8:
            formatEx->dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND;
            break;
        default:
            formatEx->dwChannelMask = 0;
            break;
        }

        formatEx->Format.nBlockAlign = (WORD)(formatEx->Format.nChannels * (formatEx->Format.wBitsPerSample >> 3));
        formatEx->Format.nAvgBytesPerSec = (formatEx->Format.nChannels * (formatEx->Format.wBitsPerSample >> 3)) * formatEx->Format.nSamplesPerSec;
        formatEx->Format.cbSize = 22; //sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        formatEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

        return S_OK;
#endif
    }
    else
    {
        WAVEFORMATEX* waveFormat = (WAVEFORMATEX*)mediaType.AllocFormatBuffer(sizeof(WAVEFORMATEX));

        waveFormat->wFormatTag = WAVE_FORMAT_PCM;
        waveFormat->nChannels = 0;
        waveFormat->nSamplesPerSec = 0;
        if (GetFilter()->mVorbisFormatInfo)
        {
            waveFormat->nChannels = GetFilter()->mVorbisFormatInfo->numChannels;
            waveFormat->nSamplesPerSec =  GetFilter()->mVorbisFormatInfo->samplesPerSec;
        }
        waveFormat->wBitsPerSample = 16;
        waveFormat->nBlockAlign = waveFormat->nChannels * (waveFormat->wBitsPerSample >> 3);
        waveFormat->nAvgBytesPerSec = (waveFormat->nChannels * (waveFormat->wBitsPerSample >> 3)) * waveFormat->nSamplesPerSec;
        waveFormat->cbSize = 0;

        return S_OK;
    }

    return S_FALSE;
}

VorbisDecodeFilter* VorbisDecodeOutputPin::GetFilter()
{
    return static_cast<VorbisDecodeFilter*>(m_pFilter);
}