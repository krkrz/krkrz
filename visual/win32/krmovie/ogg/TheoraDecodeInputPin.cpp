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
#include "OggTypes.h"
#include "theoradecodeinputpin.h"


TheoraDecodeInputPin::TheoraDecodeInputPin(CTransformFilter* inParentFilter, HRESULT* outHR) 
	:	CTransformInputPin(NAME("Theora Input Pin"), inParentFilter, outHR, L"Theora In")
	,	m_setupState(VSS_SEEN_NOTHING)
	,	m_oggOutputPinInterface(NULL)
	,	m_sentStreamOffset(false)
{
}

TheoraDecodeInputPin::~TheoraDecodeInputPin() 
{
}

STDMETHODIMP TheoraDecodeInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
{
	if (riid == IID_IMediaSeeking) 
    {
        return GetInterface((IMediaSeeking*) this, ppv);
	} 
    else if (riid == IID_IOggDecoder) 
    {
        return GetInterface((IOggDecoder*)this, ppv);
	}

	return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv); 
}

HRESULT TheoraDecodeInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *outRequestedProps)
{
	TheoraDecodeFilter* locParent = (TheoraDecodeFilter*)m_pFilter;
	unsigned long locBuffSize = (locParent->m_theoraFormatInfo->outerFrameHeight * locParent->m_theoraFormatInfo->outerFrameWidth * 3) >> 2;

    LOG(logDEBUG) << __FUNCTIONW__ << " Buffer Size: " << locBuffSize;

#ifdef WINCE
	if (locBuffSize < 4096) 
    {
		locBuffSize = 4096;
	}
#else
	if (locBuffSize < 65536) 
    {
		locBuffSize = 65536;
	}
#endif

	outRequestedProps->cbBuffer =  locBuffSize;
	outRequestedProps->cBuffers = THEORA_NUM_BUFFERS;
	outRequestedProps->cbAlign = 1;
	outRequestedProps->cbPrefix = 0;

	return S_OK;
}

HRESULT TheoraDecodeInputPin::BreakConnect() 
{
	CAutoLock locLock(m_pLock);
	LOG(logDEBUG) << "BreakConnect";

    //Need a lock ??
	ReleaseDelegate();
	return CTransformInputPin::BreakConnect();
}

HRESULT TheoraDecodeInputPin::CompleteConnect (IPin *inReceivePin) 
{
	CAutoLock locLock(m_pLock);

	//Offsets
	IOggOutputPin* locOggOutput = NULL;
	m_sentStreamOffset = false;
	HRESULT locHR = inReceivePin->QueryInterface(IID_IOggOutputPin, (void**)&locOggOutput);
	if (locHR == S_OK) 
    {
		m_oggOutputPinInterface = locOggOutput;	
	} 
    else 
    {
		m_oggOutputPinInterface = NULL;
	}

	IMediaSeeking* locSeeker = NULL;
	inReceivePin->QueryInterface(IID_IMediaSeeking, (void**)&locSeeker);

	if (locSeeker == NULL) 
    {
		//LOG(logDEBUG) << "Seeker is null";
	}
	SetDelegate(locSeeker);
	locHR = CTransformInputPin::CompleteConnect(inReceivePin);
    LOG(logDEBUG) << __FUNCTIONW__ <<  " returned: " << locHR;

	return locHR;
}

LOOG_INT64 TheoraDecodeInputPin::convertGranuleToTime(LOOG_INT64 inGranule)
{
	//if (mBegun) {	
	//	return (inGranule * UNITS) / mSampleRate;
	//} else {
	//	return -1;
	//}
	TheoraDecodeFilter* locParent = (TheoraDecodeFilter*)m_pFilter;

	LOOG_INT64 locMod = ((LOOG_INT64)1) << locParent->GetTheoraFormatBlock()->maxKeyframeInterval; //(unsigned long)pow((double) 2, (double) mGranulePosShift);
	LOOG_INT64 locInterFrameNo = (LOOG_INT64) ( inGranule % locMod );
			
	//LOOG_INT64 retTime ((((inGranule >> locParent->getTheoraFormatBlock()->maxKeyframeInterval) + locInterFrameNo) * UNITS) * locParent->getTheoraFormatBlock()->frameRateDenominator) / locParent->getTheoraFormatBlock()->frameRateNumerator;

	LOOG_INT64 retTime = inGranule >> locParent->GetTheoraFormatBlock()->maxKeyframeInterval;
	retTime += locInterFrameNo + 1;
	retTime *= UNITS;
	retTime *= locParent->GetTheoraFormatBlock()->frameRateDenominator;
	retTime /= locParent->GetTheoraFormatBlock()->frameRateNumerator;

	return retTime;
}

LOOG_INT64 TheoraDecodeInputPin::mustSeekBefore(LOOG_INT64 inGranule)
{
	TheoraDecodeFilter* locParent = (TheoraDecodeFilter*)m_pFilter;
	LOOG_INT64 locShift = locParent->GetTheoraFormatBlock()->maxKeyframeInterval;

	return (inGranule >> locShift) << locShift;
}

IOggDecoder::eAcceptHeaderResult TheoraDecodeInputPin::showHeaderPacket(OggPacket* inCodecHeaderPacket)
{
	LOG(logDEBUG) << "Show header packet...";

	unsigned char* locPacketData = new unsigned char[inCodecHeaderPacket->packetSize()];
	memcpy((void*)locPacketData, (const void**)inCodecHeaderPacket->packetData(), inCodecHeaderPacket->packetSize());
	StampedOggPacket* locStamped = new StampedOggPacket(locPacketData, inCodecHeaderPacket->packetSize(), false, false, 0,0, StampedOggPacket::NONE);

	TheoraDecodeFilter* locParent = (TheoraDecodeFilter*)m_pFilter;

	IOggDecoder::eAcceptHeaderResult retResult = IOggDecoder::AHR_INVALID_HEADER;
	switch (m_setupState) 
    {
		case VSS_SEEN_NOTHING:
			if (strncmp((char*)inCodecHeaderPacket->packetData(), "\200theora", 7) == 0) 
            {
				//TODO::: Possibly verify version
				if (locParent->m_theoraDecoder->decodeTheora(locStamped) == NULL) 
                {
					m_setupState = VSS_SEEN_BOS;
					retResult = IOggDecoder::AHR_MORE_HEADERS_TO_COME;

					LOG(logDEBUG) << "Seen ident header 1";
				}
			}
			//return IOggDecoder::AHR_INVALID_HEADER;
			break;
			
		case VSS_SEEN_BOS:
			if (strncmp((char*)inCodecHeaderPacket->packetData(), "\201theora", 7) == 0) 
            {
				if (locParent->m_theoraDecoder->decodeTheora(locStamped) == NULL) 
                {
					m_setupState = VSS_SEEN_COMMENT;
					retResult = IOggDecoder::AHR_MORE_HEADERS_TO_COME;

					LOG(logDEBUG) << "Seen comment header 2";
                }
			}
			//return IOggDecoder::AHR_INVALID_HEADER;
			break;
			
		case VSS_SEEN_COMMENT:
			if (strncmp((char*)inCodecHeaderPacket->packetData(), "\202theora", 7) == 0) 
            {
				if (locParent->m_theoraDecoder->decodeTheora(locStamped) == NULL) 
                {
					//fish_sound_command (mFishSound, FISH_SOUND_GET_INFO, &(mFishInfo), sizeof (FishSoundInfo)); 
					//Is mBegun useful ?
					//mBegun = true;
			
					//mNumChannels = mFishInfo.channels;
					//mFrameSize = mNumChannels * SIZE_16_BITS;
					//mSampleRate = mFishInfo.samplerate;

		
					m_setupState = VSS_ALL_HEADERS_SEEN;
					retResult = IOggDecoder::AHR_ALL_HEADERS_RECEIVED;

					LOG(logDEBUG) << "Seen code book header 3";
				}
				
			}
			//return IOggDecoder::AHR_INVALID_HEADER;
			break;
			
		case VSS_ALL_HEADERS_SEEN:
		case VSS_ERROR:
		default:
			LOG(logDEBUG) << "Discarding header packet... bad state";
			
            delete locStamped;
			retResult = IOggDecoder::AHR_UNEXPECTED;
			break;
	}
	
	LOG(logDEBUG) << "Unexpected header packet...";
	
    return retResult;
}

std::string TheoraDecodeInputPin::getCodecShortName()
{
	return "theora";
}

std::string TheoraDecodeInputPin::getCodecIdentString()
{
	//TODO:::
	return "theora";
}

IOggOutputPin* TheoraDecodeInputPin::GetOutputPinInterface()
{
    return m_oggOutputPinInterface;
}

bool TheoraDecodeInputPin::GetSentStreamOffset()
{
    return m_sentStreamOffset;
}

void TheoraDecodeInputPin::SetSentStreamOffset(bool inSentStreamOffset)
{
    m_sentStreamOffset = inSentStreamOffset;
}
