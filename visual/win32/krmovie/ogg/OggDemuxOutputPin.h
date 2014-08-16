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
#pragma once

#include <OggPacket.h>
#include <IOggCallback.h>
#include <OggPacketiser.h>
#include "IOggDecoder.h"
#include "IOggOutputPin.h"

class OggDemuxFilter;

class OggDemuxOutputPin
	:	public CBaseOutputPin
	,	public BasicSeekPassThrough
	,	public IOggCallback
	,	public IOggOutputPin
	,	protected IStampedOggPacketSink
{
public:

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	//OggDemuxOutputPin();
	OggDemuxOutputPin(	TCHAR* inObjectName, 
						OggDemuxFilter* inParentFilter,
						CCritSec* inFilterLock,
						OggPacket* inIdentHeader,
						unsigned long inSerialNo);
	virtual ~OggDemuxOutputPin();

	static const unsigned long NUM_PAGE_BUFFERS = 100;

	unsigned long getSerialNo();
	CComPtr<IOggDecoder> getDecoderInterface();
	bool IsStreamReady();
	void SetIsStreamReady(bool inIsStreamReady);

	//IOggCallback Interface
	virtual bool acceptOggPage(OggPage* inOggPage);


	//CBasePin virtuals
	virtual HRESULT GetMediaType(int inPosition, CMediaType* outMediaType);
	virtual HRESULT CheckMediaType(const CMediaType* inMediaType);
	virtual HRESULT DecideBufferSize(IMemAllocator* inoutAllocator, ALLOCATOR_PROPERTIES* inoutInputRequest);

	//Pin Conenction Methods
	virtual HRESULT BreakConnect();
	virtual HRESULT CompleteConnect(IPin *inReceivePin);

	//Pin streaming methods
	virtual HRESULT DeliverNewSegment(REFERENCE_TIME inStart, REFERENCE_TIME inStop, double inRate);
	virtual HRESULT DeliverEndOfStream();
	virtual HRESULT DeliverEndFlush();
	virtual HRESULT DeliverBeginFlush();

	//IOggOutputPin interface
	virtual bool notifyStreamBaseTime(__int64 inStreamTime);
	virtual __int64 getGlobalBaseTime();
protected:
	//IStampedOggPacketSink
	virtual bool acceptStampedOggPacket(StampedOggPacket* inPacket);
	virtual bool dispatchPacket(StampedOggPacket* inPacket);

    OggDemuxFilter* GetFilter();
private:

	//What is this actually for ?
	HRESULT mFilterHR;

	BYTE* getIdentAsFormatBlock();
	unsigned long getIdentSize();
	unsigned long m_serialNo;

	CCritSec* m_packetiserLock;

	unsigned long m_numBuffers;
	
	OggPacket* m_identHeader;
	CComPtr<IOggDecoder> m_decoderInterface;
	OggPacketiser m_packetiser;

	COutputQueue* m_dataQueue;

	bool m_isStreamReady;
	bool m_acceptingData;
};
