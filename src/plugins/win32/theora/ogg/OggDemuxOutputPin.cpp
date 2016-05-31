//===========================================================================
//Copyright (C) 2003, 2004, 2005 Zentaro Kavanagh
//Copyright (C) 2010 Cristian Adam
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
#include "ds_guids.h"
#include "OggDemuxFilter.h"
#include "OggDemuxOutputPin.h"
#include "ogglog.h"

OggDemuxOutputPin::	OggDemuxOutputPin(TCHAR* inObjectName, OggDemuxFilter* inParentFilter, CCritSec* inFilterLock,	
                                      OggPacket* inIdentHeader,	unsigned long inSerialNo):	
CBaseOutputPin(NAME("Ogg Demux Output Pin"), inParentFilter, inFilterLock, &mFilterHR, L"Ogg Stream"),	
m_identHeader(inIdentHeader),	
m_serialNo(inSerialNo),	
m_isStreamReady(false),	
m_acceptingData(false),	
m_numBuffers(0),
m_dataQueue(NULL),
mFilterHR(S_OK)
{
	m_packetiserLock = new CCritSec;
	
	//(BYTE*)inBOSPage->createRawPageData();
	m_packetiser.setPacketSink(this);

	//Subvert COM and do this directly... this way, the source filter won't expose the interface to the
	// graph but we can still delegate to it.
	IMediaSeeking* locSeeker = (IMediaSeeking*)inParentFilter;
	SetDelegate(locSeeker);
}

STDMETHODIMP OggDemuxOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IMediaSeeking) 
    {
        return GetInterface((IMediaSeeking*)this, ppv);
	} 
    else if (riid == IID_IOggOutputPin) 
    {
		*ppv = (IOggOutputPin*)this;
		//((IUnknown*)*ppv)->AddRef();
		return NOERROR;		
	}

	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv); 
}

OggDemuxOutputPin::~OggDemuxOutputPin()
{
	//Since we didn't addref the filter when we set the seek delegate onto it, we have to avoid
	//	it getting released, so set it to NULL, to avoid the destructor releasing it.
	SetDelegate(NULL);
	//delete[] mBOSAsFormatBlock;
	//delete mBOSPage;
	delete m_identHeader;
	delete m_dataQueue;

	delete m_packetiserLock;
}

bool OggDemuxOutputPin::acceptOggPage(OggPage* inOggPage)
{
	CAutoLock locPackLock(m_packetiserLock);
	if (m_isStreamReady) 
    {
		m_acceptingData = true;
		return m_packetiser.acceptOggPage(inOggPage);
	} 
    else 
    {
		delete inOggPage;
	}
	
    return false;
}

BYTE* OggDemuxOutputPin::getIdentAsFormatBlock()
{
	return (BYTE*)m_identHeader->packetData();
}

unsigned long OggDemuxOutputPin::getIdentSize()
{
	return m_identHeader->packetSize();
}

unsigned long OggDemuxOutputPin::getSerialNo()
{
	return m_serialNo;//mBOSPage->header()->StreamSerialNo();
}

CComPtr<IOggDecoder> OggDemuxOutputPin::getDecoderInterface()
{
    if (!m_decoderInterface) 
    {
        if (IsConnected()) 
        {   
            IPin* locPin = GetConnected();
            if (locPin != NULL) 
            {
                locPin->QueryInterface(IID_IOggDecoder, (void**)&m_decoderInterface);
            }
        }
    }
    return m_decoderInterface;
}

HRESULT OggDemuxOutputPin::GetMediaType(int inPosition, CMediaType* outMediaType) 
{
	//Put it in from the info we got in the constructor.
	if (inPosition == 0) 
    {
		AM_MEDIA_TYPE locAMMediaType;
		locAMMediaType.majortype = MEDIATYPE_OggPacketStream;

		locAMMediaType.subtype = MEDIASUBTYPE_None;
		locAMMediaType.formattype = FORMAT_OggIdentHeader;
		locAMMediaType.cbFormat = getIdentSize();
		locAMMediaType.pbFormat = getIdentAsFormatBlock();
		locAMMediaType.pUnk = NULL;
	
		CMediaType locMediaType(locAMMediaType);		
		*outMediaType = locMediaType;
		return S_OK;
    }
        
    return VFW_S_NO_MORE_ITEMS;
}

HRESULT OggDemuxOutputPin::CheckMediaType(const CMediaType* inMediaType) 
{
	if (inMediaType->majortype == MEDIATYPE_OggPacketStream &&
		inMediaType->subtype == MEDIASUBTYPE_None &&
		inMediaType->formattype == FORMAT_OggIdentHeader) 
    {
		//&&	(inMediaType->cbFormat == mBOSPage->pageSize()) {
		return S_OK;
    }
    
    return E_FAIL;
}

HRESULT OggDemuxOutputPin::DecideBufferSize(IMemAllocator* inoutAllocator, ALLOCATOR_PROPERTIES* inoutInputRequest) 
{
	HRESULT hr = S_OK;

	ALLOCATOR_PROPERTIES locReqAlloc = *inoutInputRequest;
	ALLOCATOR_PROPERTIES locActualAlloc;

	hr = inoutAllocator->SetProperties(&locReqAlloc, &locActualAlloc);

	if (hr != S_OK) 
    {
		return hr;
	}

	m_numBuffers = locActualAlloc.cBuffers;
	hr = inoutAllocator->Commit();

    LOG(logINFO) << __FUNCTIONW__ << " BufferSize: " << locActualAlloc.cbBuffer << ", Buffers: " << locActualAlloc.cBuffers;

	return hr;
}

//Pin Conenction Methods
HRESULT OggDemuxOutputPin::BreakConnect()
{
	delete m_dataQueue;
	m_dataQueue = NULL;
	return CBaseOutputPin::BreakConnect();
}

HRESULT OggDemuxOutputPin::CompleteConnect(IPin *inReceivePin)
{
    CComPtr<IOggDecoder> decoder;
    inReceivePin->QueryInterface(IID_IOggDecoder, (void**)&decoder);
	if (decoder != NULL) 
    {
		m_decoderInterface = decoder;

		OggPacket* packetClone = m_identHeader->clone();
		IOggDecoder::eAcceptHeaderResult result = m_decoderInterface->showHeaderPacket(packetClone);
		delete packetClone;
		if (result == IOggDecoder::AHR_ALL_HEADERS_RECEIVED) 
        {
			m_isStreamReady = true;
		} 
        else 
        {
			OggPacketiser packetiser;
			packetiser.setPacketSink(this);
            std::vector<OggPage*> locList;
			GetFilter()->getMatchingBufferedPages(m_serialNo,locList);
			for (size_t i = 0; i < locList.size(); i++)  {
				packetiser.acceptOggPage(locList[i]);
			}
			locList.clear();
			GetFilter()->removeMatchingBufferedPages(m_serialNo);	
		}

		if (m_isStreamReady) 
        {
			HRESULT hr = CBaseOutputPin::CompleteConnect(inReceivePin);
			if (hr == S_OK) 
            {
				GetFilter()->notifyPinConnected();
				m_dataQueue = new COutputQueue (inReceivePin, &mFilterHR, FALSE, TRUE,1,TRUE, m_numBuffers);
				
                return S_OK;
			}  
            else 
            {
				return hr;
			}
		}	
	}
	return E_FAIL;
}

bool OggDemuxOutputPin::dispatchPacket(StampedOggPacket* inPacket)
{
	auto_ptr<StampedOggPacket> ip(inPacket);
	CAutoLock locStreamLock(GetFilter()->streamLock());

	//Set up the sample info
	IMediaSample* locSample = NULL;
	REFERENCE_TIME locStart = inPacket->startTime();
	REFERENCE_TIME locStop = inPacket->endTime();
	
	//Get a delivery buffer
	HRESULT	locHR = GetDeliveryBuffer(&locSample, &locStart, &locStop, NULL);
	
	//Error checks
	if (locHR != S_OK) 
    {
		//Stopping, flushing or error

		//delete inPacket;
		return false;
	}

	//Set time stamps. These are granule pos, and may be -1
	locSample->SetTime(&locStart, &locStop);
	
	locSample->SetMediaTime(&locStart, &locStop);
	locSample->SetSyncPoint(TRUE);
	

	// Create a pointer for the samples buffer
	BYTE* locBuffer = NULL;
	locSample->GetPointer(&locBuffer);

	if (static_cast<unsigned long>(locSample->GetSize()) >= inPacket->packetSize()) 
    {
		memcpy(locBuffer, inPacket->packetData(), inPacket->packetSize());
		locSample->SetActualDataLength(inPacket->packetSize());

		locHR = m_dataQueue->Receive(locSample);

		if (locHR != S_OK) 
        {
            LOG(logERROR) << __FUNCTIONW__ << " Failure... Queue rejected sample, error: 0x" << std::hex << locHR;
			//Stopping ??

			//delete inPacket;
			return false;
		} 
        else 
        {
			//delete inPacket;
			return true;
		}
	} 
    else 
    {
		LOG(logERROR) << __FUNCTIONW__ << " Buffer to small. " << locSample->GetSize() << " vs " << inPacket->packetSize();
		throw 0;
	}	
}

bool OggDemuxOutputPin::acceptStampedOggPacket(StampedOggPacket* inPacket)
{
	if (m_acceptingData) 
    {
		return dispatchPacket(inPacket);
	} 
    else 
    {
		//This handles callbacks with header packets
		IOggDecoder::eAcceptHeaderResult locResult;
		if ((m_decoderInterface != NULL) && (!m_isStreamReady)) 
        {
			locResult = m_decoderInterface->showHeaderPacket(inPacket);
			if (locResult == IOggDecoder::AHR_ALL_HEADERS_RECEIVED) 
            {
				m_isStreamReady = true;
			}
		}
		delete inPacket;
		return true;
	}
}

//Pin streaming methods
HRESULT OggDemuxOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	NewSegment(tStart, tStop, dRate);

	if (m_dataQueue != NULL) 
    {
		m_dataQueue->NewSegment(tStart, tStop, dRate);
	}

	return S_OK;
}

HRESULT OggDemuxOutputPin::DeliverEndOfStream()
{
	if (m_dataQueue != NULL) 
    {
		m_dataQueue->EOS();
	}
    return S_OK;
}

HRESULT OggDemuxOutputPin::DeliverEndFlush()
{
	CAutoLock locPackLock(m_packetiserLock);
	
	if (m_dataQueue != NULL) 
    {
		m_dataQueue->EndFlush();
	}

	m_packetiser.reset();
    return S_OK;
}

HRESULT OggDemuxOutputPin::DeliverBeginFlush()
{
	if (m_dataQueue != NULL) 
    {
		m_dataQueue->BeginFlush();
	}
	
    return S_OK;
}

bool OggDemuxOutputPin::notifyStreamBaseTime(__int64 inStreamTime)
{
	return GetFilter()->notifyStreamBaseTime(inStreamTime);
}

__int64 OggDemuxOutputPin::getGlobalBaseTime()
{
	return GetFilter()->getGlobalBaseTime();
}

bool OggDemuxOutputPin::IsStreamReady()
{
    return m_isStreamReady;
}

void OggDemuxOutputPin::SetIsStreamReady(bool inIsStreamReady)
{
    m_isStreamReady = inIsStreamReady;
}

OggDemuxFilter* OggDemuxOutputPin::GetFilter()
{
    return static_cast<OggDemuxFilter*>(m_pFilter);
}
