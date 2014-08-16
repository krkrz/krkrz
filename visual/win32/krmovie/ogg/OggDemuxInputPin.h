//===========================================================================
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
//- Neither the name of Cristian Adam nor the names of contributors 
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

class OggDemuxFilter;

// Since we are not using IMemInputPin, it does not make sense to derive from 
// CBaseInputPin. We need random access to the data, so we do not use CPullPin.
// A worker thread on the filter calls a Read method to synchronously fetch data
// from the source.
class OggDemuxInputPin : public CBasePin
{
public:
    OggDemuxInputPin(OggDemuxFilter* pFilter, CCritSec* pLock, HRESULT* phr);

    // base pin overrides
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    HRESULT CompleteConnect(IPin* pPeer);
    HRESULT CheckConnect(IPin* pPin);
    HRESULT BreakConnect();
    
    HRESULT __stdcall BeginFlush();
    HRESULT __stdcall EndFlush();

    HRESULT Read(LONGLONG llOffset, long cBytes, BYTE* pBuffer);
    LONGLONG Length();

    CComQIPtr<IAsyncReader> GetReader();

private:
    OggDemuxFilter* m_filter;
    CMediaType m_mediaType;
};

