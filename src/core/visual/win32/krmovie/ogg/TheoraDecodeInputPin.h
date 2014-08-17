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
#include "IOggDecoder.h"
#include "IOggOutputPin.h"
#include "BasicSeekPassThrough.h"

#include "TheoraDecodeFilter.h"

class TheoraDecodeInputPin : public CTransformInputPin,	
    public BasicSeekPassThrough, public IOggDecoder
{
public:

	TheoraDecodeInputPin(CTransformFilter* inParentFilter, HRESULT* outHR);
	virtual ~TheoraDecodeInputPin();

	//COM Guff
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	//Overrides
	virtual HRESULT BreakConnect();
	virtual HRESULT CompleteConnect (IPin *inReceivePin);

	//XTODO::: Add a new segment override to get an integer rate change
	//XTODO::: Possibly add an endflush override to clear buffered data
	virtual STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *outRequestedProps);

	//IOggDecoder Interface
	virtual LOOG_INT64 __stdcall convertGranuleToTime(LOOG_INT64 inGranule);
	virtual LOOG_INT64 __stdcall mustSeekBefore(LOOG_INT64 inGranule);
	virtual IOggDecoder::eAcceptHeaderResult __stdcall showHeaderPacket(OggPacket* inCodecHeaderPacket);
	virtual string __stdcall getCodecShortName();
	virtual string __stdcall getCodecIdentString();

	virtual IOggOutputPin* GetOutputPinInterface();
	virtual bool GetSentStreamOffset();
	virtual void SetSentStreamOffset(bool inSentStreamOffset);

protected:

#ifdef WINCE
	static const unsigned long THEORA_NUM_BUFFERS = 15;
#else
	static const unsigned long THEORA_NUM_BUFFERS = 30;
#endif

	enum eTheoraSetupState 
    {
		VSS_SEEN_NOTHING,
		VSS_SEEN_BOS,
		VSS_SEEN_COMMENT,
		VSS_ALL_HEADERS_SEEN,
		VSS_ERROR
	};

	eTheoraSetupState m_setupState;

	IOggOutputPin* m_oggOutputPinInterface;
	bool m_sentStreamOffset;
};
