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
#pragma once
#include "BasicSeekPassThrough.h"
#include "IOggBaseTime.h"
#include "IOggSeekTable.h"
#include <OggDataBuffer.h>

#include <AutoOggChainGranuleSeekTable.h>
#include "CustomOggChainGranuleSeekTable.h"
#include "OggDemuxInputPin.h"

class OggStreamMapper;
class OggDemuxFilter:	
    public CBaseFilter,	
    public CAMThread,	
    public IOggCallback,	
    public IOggBaseTime,
    public IOggSeekTable,
    public BasicSeekPassThrough
{
public:
    OggDemuxFilter(HRESULT* outHR);
	virtual ~OggDemuxFilter();

    enum eThreadCommands 
    {
		THREAD_EXIT = 0,
		THREAD_PAUSE = 1,
		THREAD_RUN = 2,
        THREAD_SEEK = 3
	};

	//Com Stuff
	DECLARE_IUNKNOWN

    static const wchar_t* NAME;
    static const AMOVIESETUP_MEDIATYPE m_outputMediaTypes;
    static const AMOVIESETUP_MEDIATYPE m_inputMediaTypes;
    static const AMOVIESETUP_PIN m_pinReg[];
    static const AMOVIESETUP_FILTER m_filterReg;

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);

	//Streaming MEthods
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();

#ifdef WINCE
	virtual LPAMOVIESETUP_FILTER GetSetupData();
#endif

#if defined (_DEBUG) && defined (WINCE)
    ULONG __stdcall NonDelegatingRelease()
    {
        if (m_cRef == 1) 
        {
            ASSERT(m_pGraph == NULL);
        }
        return CUnknown::NonDelegatingRelease();
    }
#endif

	//IOggCallback Interface
	virtual bool acceptOggPage(OggPage* inOggPage);

	//PURE VIRTUALS From CBaseFilter
	virtual int GetPinCount();
	virtual CBasePin* GetPin(int inPinNo);

	//PURE VIRTUALS from CAMThread
	virtual DWORD ThreadProc();

	//IMediaSeeking
	virtual STDMETHODIMP GetDuration(LONGLONG* outDuration);
	virtual STDMETHODIMP GetCapabilities(DWORD* inCapabilities);
	 
	virtual STDMETHODIMP CheckCapabilities(DWORD *pCapabilities);
	virtual STDMETHODIMP IsFormatSupported(const GUID *pFormat);
	virtual STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
	virtual STDMETHODIMP SetTimeFormat(const GUID *pFormat);
	virtual STDMETHODIMP GetTimeFormat( GUID *pFormat);
	
	virtual STDMETHODIMP GetStopPosition(LONGLONG *pStop);
	virtual STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
	virtual STDMETHODIMP ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat);
	virtual STDMETHODIMP SetPositions(LONGLONG *pCurrent,DWORD dwCurrentFlags,LONGLONG *pStop,DWORD dwStopFlags);
	virtual STDMETHODIMP GetPositions(LONGLONG *pCurrent, LONGLONG *pStop);
	virtual STDMETHODIMP GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest);
	virtual STDMETHODIMP SetRate(double dRate);
	virtual STDMETHODIMP GetRate(double *dRate);
	virtual STDMETHODIMP GetPreroll(LONGLONG *pllPreroll);
	virtual STDMETHODIMP IsUsingTimeFormat(const GUID *pFormat);

    void getMatchingBufferedPages(unsigned long inSerialNo, std::vector<OggPage*>& pages );
	void removeMatchingBufferedPages(unsigned long inSerialNo);

	CCritSec* streamLock();

	virtual void notifyPinConnected();
	virtual bool notifyStreamBaseTime(__int64 inStreamBaseTime);

	//IOggBaseTime Interface
	virtual __int64 getGlobalBaseTime();

    LONGLONG GetCurrentReaderPos() const;
    void SetCurrentReaderPos(LONGLONG val);

    LONGLONG GetRequestedSeekPos() const;
    void SetRequestedSeekPos(LONGLONG val);

    //IOggSeekTable Interface
    void buildSeekTable();

protected:
    friend class OggDemuxInputPin;
    friend class OggDemuxOutputPin;

	static const unsigned long SETUP_BUFFER_SIZE = 24;
	virtual HRESULT SetUpPins();

	void resetStream();
	HRESULT DataProcessLoop();

	void DeliverEOS();
	void DeliverBeginFlush();
	void DeliverEndFlush();
	void DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    static unsigned __stdcall SeekTableThread(void* arg);
    void BuildSeekTable();

	CCritSec m_filterLock;
	CCritSec m_demuxLock;
	CCritSec m_streamLock;
    CCritSec m_positionLock;

	wstring m_fileName;

	bool m_seenAllBOSPages;
	bool m_seenPositiveGranulePos;
	OggPage* m_pendingPage;
    std::vector<OggPage*> m_bufferedPages;

	OggDataBuffer m_oggBuffer;
	OggStreamMapper* m_streamMapper;

    OggDemuxInputPin m_inputPin;

	AutoOggChainGranuleSeekTable* m_seekTable;

	bool m_usingCustomSource;
	bool m_justReset;
	LONGLONG m_globalBaseTime;

    LONGLONG m_currentReaderPos; 
    LONGLONG m_requestedSeekPos;
};
