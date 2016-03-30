//===========================================================================
//Copyright (C) 2003, 2004, 2005 Zentaro Kavanagh
//Copyright (C) 2009, 2010 Cristian Adam
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
#include "OggDemuxFilter.h"
#include "OggDemuxOutputPin.h"
#include "OggStreamMapper.h"
#include "ds_guids.h"
#include "oggutil.h"
/*
const GUID FORMAT_Vorbis = 
{ 0x44e04f43, 0x58b3, 0x4de1, {0x9b, 0xaa, 0x89, 0x1, 0xf8, 0x52, 0xda, 0xe4 } };
const GUID FORMAT_Vorbis2 = 
{ 0xB36E107F, 0xA938, 0x4387, {0x93, 0xC7, 0x55, 0xE9, 0x66, 0x75, 0x74, 0x73} };
const GUID CLSID_VorbisDecodeFilter = 
{ 0x5a1d945, 0xa794, 0x44ef, {0xb4, 0x1a, 0x2f, 0x85, 0x1a, 0x11, 0x71, 0x55} };
const GUID MEDIASUBTYPE_Vorbis = 
{ 0x8a0566ac, 0x42b3, 0x4ad9, {0xac, 0xa3, 0x93, 0xb9, 0x6, 0xdd, 0xf9, 0x8a} };
const GUID MEDIASUBTYPE_Vorbis2 =
{ 0x8D2FD10B, 0x5841, 0x4a6b, {0x89, 0x05, 0x58, 0x8F, 0xEC, 0x1A, 0xDE, 0xD9} };
const GUID IID_IDownmixAudio = 
{ 0xBFF86BE7, 0x9E32, 0x40EF, {0xB2, 0x00, 0x7B, 0xCC, 0x78, 0x00, 0xCC, 0x72} };
*/
// This template lets the Object factory create us properly and work with COM infrastructure.
/*
CFactoryTemplate g_Templates[] = 
{
    { 
        OggDemuxFilter::NAME,			// Name
	    &CLSID_OggDemuxFilter,          // CLSID
	    OggDemuxFilter::CreateInstance,	// Method to create an instance of MyComponent
        NULL,							// Initialization function
        &OggDemuxFilter::m_filterReg    // Set-up information (for filters)
    }
};

// Generic way of determining the number of items in the template
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]); 
*/
const wchar_t* OggDemuxFilter::NAME = L"Xiph.Org Ogg Demuxer";

const AMOVIESETUP_PIN OggDemuxFilter::m_pinReg[] = 
{	
    {
        L"Ogg Input",                       //Name (obsoleted)
        FALSE,                              //Renders from this pin ?? Not sure about this.
        FALSE,                              //Is an output pin
        FALSE,                              //Can have zero instances of this pin
        FALSE,                              //Can have more than one instance of this pin
        &CLSID_NULL,                        //Connects to filter (obsoleted)
        NULL,                               //Connects to pin (obsoleted)
        1,                                  //Only support one media type
        &m_inputMediaTypes                  //Pointer to media type (Audio/Vorbis or Audio/Speex)
    } ,
    {
        L"Ogg Packet Out",					//Name (obsoleted)
        FALSE,								//Renders from this pin ?? Not sure about this.
        TRUE,								//Is an output pin
        TRUE,								//Can have zero instances of this pin
        TRUE,								//Can have more than one instance of this pin
        &GUID_NULL,							//Connects to filter (obsoleted)
        NULL,								//Connects to pin (obsoleted)
        1,									//Only support one media type
        &m_outputMediaTypes	                //Pointer to media type (Audio/Vorbis or Audio/Speex)	
    }
};

const AMOVIESETUP_FILTER OggDemuxFilter::m_filterReg = 
{
    &CLSID_OggDemuxFilter,              // Filter CLSID.
    NAME,                               // Filter name.
    MERIT_NORMAL,                       // Merit.
    2,                                  // Number of pin types.
    m_pinReg                            // Pointer to pin information.
};

const AMOVIESETUP_MEDIATYPE OggDemuxFilter::m_outputMediaTypes = 
{
    &MEDIATYPE_OggPacketStream,
    &MEDIASUBTYPE_None
};

const AMOVIESETUP_MEDIATYPE OggDemuxFilter::m_inputMediaTypes =
{ 
    &MEDIATYPE_Stream,
    &MEDIASUBTYPE_NULL 
}; 

namespace {
    const int BUFFER_LENGTH = 4096;
}

#ifdef WINCE
LPAMOVIESETUP_FILTER OggDemuxFilter::GetSetupData()
{	
	return (LPAMOVIESETUP_FILTER)&m_filterReg;	
}
#endif

//COM Creator Function
CUnknown* WINAPI OggDemuxFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
    util::ConfigureLogSettings();

    OggDemuxFilter *pNewObject = new (std::nothrow) OggDemuxFilter(pHr);
    if (pNewObject == NULL) 
    {
        *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
} 

OggDemuxFilter::OggDemuxFilter(HRESULT* outHR) :	
CBaseFilter(NAME, NULL, &m_filterLock, CLSID_OggDemuxFilter),
m_seenAllBOSPages(false),
m_seenPositiveGranulePos(false),
m_pendingPage(NULL),
m_justReset(true),
m_seekTable(NULL),
m_globalBaseTime(0),
m_usingCustomSource(false),
m_inputPin(this, m_pLock, outHR),
m_currentReaderPos(0),
m_requestedSeekPos(0)
{
	LOG(logDEBUG) << L"Creating OggDemuxFilter object";

	m_streamMapper = new OggStreamMapper(this, m_pLock);
}

OggDemuxFilter::~OggDemuxFilter()
{
	LOG(logDEBUG) << L"Destroying OggDemuxFilter";

    delete m_streamMapper;
	delete m_seekTable;
	
	size_t locSize = m_bufferedPages.size();
    for (size_t i = 0; i < locSize; i++)     {
		if( m_bufferedPages[i] ) delete m_bufferedPages[i];
	}
	m_bufferedPages.clear();
}

STDMETHODIMP OggDemuxFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    LOG(logDEBUG4) << L"NonDelegatingQueryInterface: " << riid;

    if (riid == IID_IOggBaseTime) 
    {
        return GetInterface((IOggBaseTime*)this, ppv);
    }
    else if (riid == IID_IOggSeekTable)
    {
        return GetInterface((IOggSeekTable*)this, ppv);
    }

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv); 
}

//IMEdiaStreaming
STDMETHODIMP OggDemuxFilter::Run(REFERENCE_TIME tStart) 
{
	CAutoLock locLock(m_pLock);
    LOG(logDEBUG) << "Run: " << ReferenceTime(tStart);
	
    return CBaseFilter::Run(tStart);
}

STDMETHODIMP OggDemuxFilter::Pause() 
{
	CAutoLock locLock(m_pLock);
    LOG(logDEBUG) << __FUNCTIONW__;

	if (m_State == State_Stopped) 
    {
        LOG(logDEBUG) <<L"Pause -- was stopped";

		if (ThreadExists() == FALSE) 
        {
            LOG(logDEBUG) << L"Pause -- CREATING THREAD";
			Create();
		}

        LOG(logDEBUG) << L"Pause -- RUNNING THREAD";
		CallWorker(THREAD_RUN);
	}

	HRESULT hr = CBaseFilter::Pause();

    LOG(logDEBUG) << __FUNCTIONW__ << L"Base class returned: 0x" << std::hex << hr;
	
	return hr;	
}

STDMETHODIMP OggDemuxFilter::Stop() 
{
	CAutoLock locLock(m_pLock);
    LOG(logDEBUG) << __FUNCTIONW__;

	CallWorker(THREAD_EXIT);
	Close();
	DeliverBeginFlush();
	//mSetIgnorePackets = true;
	DeliverEndFlush();
	
	return CBaseFilter::Stop();
}

void OggDemuxFilter::DeliverBeginFlush() 
{
	CAutoLock locLock(m_pLock);
    LOG(logDEBUG) << __FUNCTIONW__;
	
	for (unsigned long i = 0; i < m_streamMapper->numPins(); i++) 
    {
		m_streamMapper->getPinByIndex(i)->DeliverBeginFlush();
	}

	//Should this be here or endflush or neither ?
	resetStream();
}

void OggDemuxFilter::DeliverEndFlush() 
{
	CAutoLock locLock(m_pLock);
    LOG(logDEBUG) << __FUNCTIONW__;

	for (unsigned long i = 0; i < m_streamMapper->numPins(); i++) 
    {
		//m_streamMapper->getOggStream(i)->flush();
		m_streamMapper->getPinByIndex(i)->DeliverEndFlush();
	}
}

void OggDemuxFilter::DeliverEOS() 
{
	//m_streamMapper->toStartOfData();
    CAutoLock locStreamLock(&m_streamLock);
    LOG(logDEBUG) << __FUNCTIONW__;
	
    for (unsigned long i = 0; i < m_streamMapper->numPins(); i++) 
    {
		//m_streamMapper->getOggStream(i)->flush();
		m_streamMapper->getPinByIndex(i)->DeliverEndOfStream();
	}

    resetStream();
}

void OggDemuxFilter::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) 
{
    CAutoLock locStreamLock(&m_streamLock);
	LOG(logDEBUG) << __FUNCTIONW__;
	
    for (unsigned long i = 0; i < m_streamMapper->numPins(); i++) 
    {
		m_streamMapper->getPinByIndex(i)->DeliverNewSegment(tStart, tStop, dRate);
	}
}

void OggDemuxFilter::resetStream() 
{
	CAutoLock locDemuxLock(&m_demuxLock);
    LOG(logDEBUG) << __FUNCTIONW__;

	m_oggBuffer.clearData();
	
	m_justReset = true;   //TODO::: Look into this !
}

bool OggDemuxFilter::acceptOggPage(OggPage* inOggPage)
{
	if (!m_seenAllBOSPages) 
    {
		if (!inOggPage->header()->isBOS()) 
        {
			m_seenAllBOSPages = true;
			m_bufferedPages.push_back(inOggPage);
			return true;
		} 
        else 
        {
            LOG(logDEBUG) << __FUNCTIONW__ << " Found BOS\r\n" << inOggPage->header()->toString().c_str();
			return m_streamMapper->acceptOggPage(inOggPage);
		}
	} 
    else if (!m_seenPositiveGranulePos) 
    {
		if (inOggPage->header()->GranulePos() > 0) 
        {
			m_seenPositiveGranulePos = true;
		}
		
        m_bufferedPages.push_back(inOggPage);
		return true;
	} 
    else 
    {
		//OGGCHAIN::: Here, need to check for an eos, and reset stream, else do it in strmapper
		return m_streamMapper->acceptOggPage(inOggPage);
	}
}

HRESULT OggDemuxFilter::SetUpPins()
{
	CAutoLock locDemuxLock(&m_demuxLock);
	
	LOG(logDEBUG) << __FUNCTIONW__;
	
	//Register a callback
	m_oggBuffer.registerVirtualCallback(this);

    std::auto_ptr<BYTE> buffer(new BYTE[SETUP_BUFFER_SIZE]);
    LONGLONG position = 0;
    HRESULT hr = S_OK;

    CComPtr<IAsyncReader> reader = m_inputPin.GetReader();

    //Feed the data in until we have seen all BOS pages.
    while (SUCCEEDED(hr) && !m_seenPositiveGranulePos) 
    {
        hr = reader->SyncRead(position, SETUP_BUFFER_SIZE, buffer.get());
        if (hr == S_OK) 
        {
            m_oggBuffer.feed(buffer.get(), SETUP_BUFFER_SIZE);
            position += SETUP_BUFFER_SIZE;
        } 
        else if (!(hr == S_FALSE && m_seenPositiveGranulePos)) 
        {
            //This prevents us dying on small files, if we hit eof but we also saw a +'ve gran pos, this file is ok.
            LOG(logERROR) << __FUNCTIONW__ << " Bailing out";
            hr = VFW_E_CANNOT_RENDER;
        }
    }

    m_oggBuffer.clearData();
	LOG(logDEBUG) << "COMPLETED SETUP";
	
    return S_OK;
}

void OggDemuxFilter::getMatchingBufferedPages(unsigned long inSerialNo, std::vector<OggPage*>& pages )
{
	for (size_t i = 0; i < m_bufferedPages.size(); i++) 
    {
		if (m_bufferedPages[i]->header()->StreamSerialNo() == inSerialNo) 
        {
			pages.push_back(m_bufferedPages[i]->clone());
		}
	}
}

void OggDemuxFilter::removeMatchingBufferedPages(unsigned long inSerialNo)
{
    std::vector<OggPage*> locNewList;
	size_t locSize = m_bufferedPages.size();
	
    for (size_t i = 0; i < locSize; i++) 
    {
		if (m_bufferedPages[i]->header()->StreamSerialNo() != inSerialNo) 
        {
			locNewList.push_back(m_bufferedPages[i]);
		} 
        else 
        {
			delete m_bufferedPages[i];
		}
	}

	m_bufferedPages = locNewList;
}

int OggDemuxFilter::GetPinCount() 
{
	return static_cast<int>(m_streamMapper->numPins() + 1);
}

CBasePin* OggDemuxFilter::GetPin(int inPinNo) 
{
	if (inPinNo < 0) 
    {
		return NULL;
	}
    else if (inPinNo == 0)
    {
        return &m_inputPin;
    }

	return m_streamMapper->getPinByIndex(inPinNo - 1);
}

//CAMThread Stuff
DWORD OggDemuxFilter::ThreadProc() 
{	
	while(true) 
    {
		DWORD locThreadCommand = GetRequest();
	
		switch(locThreadCommand) 
        {
			case THREAD_EXIT:
	
				Reply(S_OK);
                LOG(logDEBUG) << __FUNCTIONW__ << " THREAD IS EXITING";
				return S_OK;

			case THREAD_RUN:
	
				Reply(S_OK);
				DataProcessLoop();
                LOG(logDEBUG) << __FUNCTIONW__ << " Data Process Loop has returned";
				break;

           case THREAD_SEEK:

               m_oggBuffer.clearData();
               SetCurrentReaderPos(GetRequestedSeekPos());
               Reply(S_OK);

               DataProcessLoop();
               LOG(logDEBUG) << __FUNCTIONW__ << " Seek request";
               break;
		}
	}

	return S_OK;
}

void OggDemuxFilter::notifyPinConnected()
{
    LOG(logDEBUG) << __FUNCTIONW__;

    if (!m_streamMapper->allStreamsReady()) 
    {
        return;
    }

    if (m_seekTable) 
    {
        return;
    }

    m_seekTable = new CustomOggChainGranuleSeekTable();
    int outputPinCount = GetPinCount();

    for (int i = 1; i < outputPinCount; i++) 
    {
        OggDemuxOutputPin* pin = static_cast<OggDemuxOutputPin*>(GetPin(i));

        LOG(logDEBUG) << L"Adding decoder interface to seek table, serial no: " << pin->getSerialNo();
        m_seekTable->addStream(pin->getSerialNo(), pin->getDecoderInterface());
    }

#ifndef WINCE
    LOG(logDEBUG) << __FUNCTIONW__ << L" Building seek table...";

    CComPtr<IAsyncReader> reader = m_inputPin.GetReader();
    static_cast<CustomOggChainGranuleSeekTable*>(m_seekTable)->buildTable(reader);

    LOG(logDEBUG) << __FUNCTIONW__ << L" Built.";
#endif
}

HRESULT OggDemuxFilter::DataProcessLoop() 
{
	//Mess with the locking mechanisms at your own risk.
	DWORD threadCommand = 0;
    std::auto_ptr<BYTE> buffer(new  BYTE[BUFFER_LENGTH]);

    unsigned long bytesRead = 0;
	bool isEOF = false;

	OggDataBuffer::eFeedResult feedResult;

	bool continueLooping = true;
	while (continueLooping) 
    {
		if (CheckRequest(&threadCommand) == TRUE) 
        {
		    LOG(logDEBUG) << __FUNCTIONW__ << " ThreadProc command encountered (" << threadCommand << ") Exiting.";
		    return S_OK;
		}

        if (m_inputPin.Read(GetCurrentReaderPos(), BUFFER_LENGTH, buffer.get()) == S_FALSE)
        {
            isEOF = true;
            bytesRead = static_cast<unsigned long>(m_inputPin.Length() % BUFFER_LENGTH);
        }
        else
        {
            bytesRead = BUFFER_LENGTH;
        }
        SetCurrentReaderPos(GetCurrentReaderPos() + bytesRead);

		m_justReset = false;

		try
		{
			CAutoLock locDemuxLock(&m_demuxLock);
			//CAutoLock locStreamLock(m_streamLock);
            //To avoid blocking problems... restart the loop if it was just reset while waiting for lock.
			if (m_justReset) 
            {		
                LOG(logDEBUG) << __FUNCTIONW__ << " Detected JustRest condition";
				continue;
			}
			feedResult = m_oggBuffer.feed(buffer.get(), bytesRead);

            LOG(logDEBUG) << __FUNCTIONW__ << " Feed result = " << feedResult 
                << " BytesRead: " << bytesRead << " CurrentReadPos: " << GetCurrentReaderPos();

            if (!(feedResult == OggDataBuffer::FEED_OK || 
                  feedResult == OggDataBuffer::PROCESS_DISPATCH_FALSE))
			{
				break;
			}
		}
		catch (int)
		{
            LOG(logDEBUG) << __FUNCTIONW__ << " Caught an exception.";

			isEOF = true;
			continueLooping = false;
		}

		if (isEOF) 
        {
			//debugLog << "DataProcessLoop : EOF"<<endl;
            CAutoLock locStreamLock(&m_streamLock);
			LOG(logDEBUG) << __FUNCTIONW__ << " EOF Deliver EOS";
			DeliverEOS();
		}
	}

	LOG(logDEBUG) << __FUNCTIONW__ << " Exiting.";

    return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetCapabilities(DWORD* inCapabilities) 
{
	if (m_seekTable == NULL || !m_seekTable->enabled())  
    {
        *inCapabilities = 0;
        return S_OK;;
    }

	*inCapabilities = mSeekingCap;
    return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetDuration(LONGLONG* pDuration) 
{
	if (m_seekTable == NULL || !m_seekTable->enabled()) 
    {
        return E_NOTIMPL;
    }

	*pDuration = m_seekTable->fileDuration();

    LOG(logDEBUG4) << "IMediaSeeking::GetDuration(" << ReferenceTime(*pDuration) << ") -> 0x" << std::hex << S_OK;

    return S_OK;
}
	 
STDMETHODIMP OggDemuxFilter::CheckCapabilities(DWORD *pCapabilities)
{
    HRESULT result = S_OK;

    DWORD dwActual;
    GetCapabilities(&dwActual);
    if (*pCapabilities & (~dwActual))
    {
        result = S_FALSE;
    }

    LOG(logDEBUG3) << "IMediaSeeking::CheckCapabilities(" << *pCapabilities << ") -> 0x" << std::hex << result;

    return result;
}

STDMETHODIMP OggDemuxFilter::IsFormatSupported(const GUID *pFormat)
{
    HRESULT result = S_FALSE;

    if (*pFormat == TIME_FORMAT_MEDIA_TIME) 
    {
        result = S_OK;
    } 

    LOG(logDEBUG4) << "IMediaSeeking::IsFormatSupported(" << ToString(*pFormat) << ") -> 0x" << std::hex << result;

    return result;
}

STDMETHODIMP OggDemuxFilter::QueryPreferredFormat(GUID *pFormat)
{
	*pFormat = TIME_FORMAT_MEDIA_TIME;

    LOG(logDEBUG3) << "IMediaSeeking::QueryPreferredFormat(" << ToString(*pFormat) << ") -> 0x" << std::hex << S_OK; 

	return S_OK;
}

STDMETHODIMP OggDemuxFilter::SetTimeFormat(const GUID *pFormat)
{
    LOG(logDEBUG3) << "IMediaSeeking::SetTimeFormat(" << ToString(pFormat) << ") -> 0x" << std::hex << E_NOTIMPL; 
	
    return E_NOTIMPL;
}

STDMETHODIMP OggDemuxFilter::GetTimeFormat( GUID *pFormat)
{
	*pFormat = TIME_FORMAT_MEDIA_TIME;

    LOG(logDEBUG3) << "IMediaSeeking::GetTimeFormat(" << ToString(*pFormat) << ") -> 0x" << std::hex << S_OK; 

    return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetStopPosition(LONGLONG *pStop)
{
	if (m_seekTable == NULL || !m_seekTable->enabled())  
    {
        return E_NOTIMPL;
    }

    *pStop = m_seekTable->fileDuration();

    LOG(logDEBUG3) << "IMediaSeeking::GetStopPosition(" << ReferenceTime(*pStop) << ") -> 0x" << std::hex << S_OK;

	return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetCurrentPosition(LONGLONG *pCurrent)
{
	return E_NOTIMPL;
}

STDMETHODIMP OggDemuxFilter::ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat)
{
    LOG(logDEBUG3) << "IMediaSeeking::ConvertTimeFormat(" << ToString(pTarget) 
        << ", " << ToString(pTargetFormat) << ", " << ToString(Source)
        << ", " << ToString(pSourceFormat) << ") -> 0x" << std::hex << E_NOTIMPL;

    return E_NOTIMPL;
}

STDMETHODIMP OggDemuxFilter::SetPositions(LONGLONG *pCurrent,DWORD dwCurrentFlags,LONGLONG *pStop,DWORD dwStopFlags)
{
    CAutoLock locLock(m_pLock);

    LOG(logDEBUG3) << "IMediaSeeking::SetPositions(" << ReferenceTime(*pCurrent) << ", " << dwCurrentFlags
        << ", " << ReferenceTime(*pStop) << ", " << dwStopFlags << ") -> 0x" << std::hex << S_OK;

    if (m_seekTable == NULL || !m_seekTable->enabled())  
    {
        return E_NOTIMPL;
    }
	
	DeliverBeginFlush();
	
	//Find the byte position for this time.
	if (*pCurrent > m_seekTable->fileDuration()) 
    {
		*pCurrent = m_seekTable->fileDuration();
	} 
    else if (*pCurrent < 0) 
    {
		*pCurrent = 0;
	}

	OggGranuleSeekTable::tSeekPair locStartPos = m_seekTable->seekPos(*pCurrent);
		
	//For now, seek to the position directly, later we will discard the preroll
	//Probably don't ever want to do this. We want to record the desired time,
	//	and it will be up to the decoders to drop anything that falls before it.
	
    DeliverEndFlush();
	DeliverNewSegment(*pCurrent, m_seekTable->fileDuration(), 1.0);

	// Second is the file position.
    SetRequestedSeekPos(locStartPos.second.first);
    if (CallWorker(THREAD_SEEK) == E_FAIL)
    {
        // Thread not running, we're changing the current position ourselfs
        SetCurrentReaderPos(GetRequestedSeekPos());
    }

	return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
	return E_NOTIMPL;
}

STDMETHODIMP OggDemuxFilter::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    if (m_seekTable == NULL || !m_seekTable->enabled())  
    {
        return E_NOTIMPL;
    }

    *pEarliest = 0;
    *pLatest = m_seekTable->fileDuration();

    LOG(logDEBUG3) << "IMediaSeeking::GetAvailable(" << ToString(*pEarliest) << ", " << ToString(*pLatest)
                   << ") -> 0x" << std::hex << S_OK;

    return S_OK;
}

STDMETHODIMP OggDemuxFilter::SetRate(double dRate)
{
    HRESULT result = VFW_E_UNSUPPORTED_AUDIO;

    if (dRate == 1.00f)
    {
        result = S_OK;
    }
    else if (dRate <= 0.00f)
    {
        result = E_INVALIDARG;
    }

    LOG(logDEBUG3) << "IMediaSeeking::SetRate(" << std::setprecision(3) << std::showpoint
                   << dRate << ") -> 0x" << std::hex << result;

    return result;
}

STDMETHODIMP OggDemuxFilter::GetRate(double *dRate)
{
    *dRate = 1.0;

    LOG(logDEBUG3) << "IMediaSeeking::GetRate(" << std::setprecision(3) << std::showpoint
                   << *dRate << ") -> 0x" << std::hex << S_OK;

    return S_OK;
}

STDMETHODIMP OggDemuxFilter::GetPreroll(LONGLONG *pllPreroll)
{
    *pllPreroll = 0;

    LOG(logDEBUG3) << "IMediaSeeking::GetPreroll(" << ToString(*pllPreroll) << ") -> 0x" << std::hex << S_OK;

    return S_OK;
}

STDMETHODIMP OggDemuxFilter::IsUsingTimeFormat(const GUID *pFormat) 
{
    HRESULT result = S_FALSE;

    if (*pFormat == TIME_FORMAT_MEDIA_TIME) 
    {
        result = S_OK;
    }

    LOG(logDEBUG4) << "IMediaSeeking::IsUsingTimeFormat(" << ToString(*pFormat) << ") -> 0x" << std::hex << result;

    return result;
}

bool OggDemuxFilter::notifyStreamBaseTime(__int64 inStreamBaseTime)
{
	if (inStreamBaseTime > m_globalBaseTime) 
    {
		m_globalBaseTime = inStreamBaseTime;
	}
	
    return true;
}

__int64 OggDemuxFilter::getGlobalBaseTime()
{
	return m_globalBaseTime;
}

CCritSec* OggDemuxFilter::streamLock()
{
    return &m_streamLock;
}

void OggDemuxFilter::SetCurrentReaderPos(LONGLONG val)
{
    m_currentReaderPos = val;
}

LONGLONG OggDemuxFilter::GetCurrentReaderPos() const
{
    return m_currentReaderPos;
}

LONGLONG OggDemuxFilter::GetRequestedSeekPos() const
{
    return m_requestedSeekPos;
}

void OggDemuxFilter::SetRequestedSeekPos(LONGLONG val)
{
    m_requestedSeekPos = val;
}

void OggDemuxFilter::buildSeekTable()
{
    unsigned threadID = 0;
    _beginthreadex( NULL, 0, &SeekTableThread, this, 0, &threadID);
}

unsigned __stdcall OggDemuxFilter::SeekTableThread(void* arg)
{
    OggDemuxFilter* self = reinterpret_cast<OggDemuxFilter*>(arg);
    self->BuildSeekTable();
    
    return 0;
}

void OggDemuxFilter::BuildSeekTable()
{
    LOG(logDEBUG) << __FUNCTIONW__ << L" Building seek table...";

    CComPtr<IAsyncReader> reader = m_inputPin.GetReader();
    if (reader)
    {
        static_cast<CustomOggChainGranuleSeekTable*>(m_seekTable)->buildTable(reader);
    }

    LOG(logDEBUG) << __FUNCTIONW__ << L" Built.";    
}
