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
#include "oggstdafx.h"
#include "OggDemuxInputPin.h"
#include "OggDemuxFilter.h"
#include "ds_guids.h"

OggDemuxInputPin::OggDemuxInputPin(OggDemuxFilter* pFilter, CCritSec* pLock, HRESULT* phr)
: m_filter(pFilter), 
CBasePin(NAME("OggDemuxInputPin"), pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{
    m_mediaType.SetType(&MEDIATYPE_Stream);
    m_mediaType.SetSubtype(&MEDIASUBTYPE_Ogg);
}

// base pin overrides
HRESULT OggDemuxInputPin::CheckMediaType(const CMediaType* inMediaType)
{
    CheckPointer(inMediaType, E_POINTER);

    HRESULT hr = S_FALSE;
    
    if (inMediaType->majortype == MEDIATYPE_Stream ||
        inMediaType->majortype == GUID_NULL)
    {
        hr = S_OK;
    }

    LOG(logDEBUG) << __FUNCTIONW__ << " Media type " << (hr == S_OK ? "OK" : "*not* OK");
    LOG(logDEBUG) << __FUNCTIONW__ << " Majortype: " << ToString(inMediaType->majortype); 
    LOG(logDEBUG) << __FUNCTIONW__ << " Subtype: " << ToString(inMediaType->subtype); 

    return hr;
}

HRESULT OggDemuxInputPin::GetMediaType(int iPosition, CMediaType* outMediaType)
{
	CheckPointer(outMediaType, E_POINTER);
    if (iPosition != 0)
    {
        return VFW_S_NO_MORE_ITEMS;
    }
    *outMediaType = m_mediaType;

    LOG(logDEBUG) << __FUNCTIONW__ << " Majortype: " << ToString(outMediaType->majortype); 
    LOG(logDEBUG) << __FUNCTIONW__ << " Subtype: " << ToString(outMediaType->subtype); 

    return S_OK;
}

HRESULT OggDemuxInputPin::BeginFlush()
{
    m_filter->DeliverBeginFlush();
    return S_OK;
}

HRESULT OggDemuxInputPin::EndFlush()
{
    m_filter->DeliverEndFlush();
    return S_OK;
}

HRESULT OggDemuxInputPin::CompleteConnect(IPin* pPeer)
{
    HRESULT hr = CBasePin::CompleteConnect(pPeer);

    if (SUCCEEDED(hr))
    {
        hr = m_filter->SetUpPins();
    }

    LOG(logDEBUG) << __FUNCTIONW__ << " result: 0x" << std::hex << hr;

    return hr;
}

HRESULT OggDemuxInputPin::CheckConnect(IPin* pPin)
{
    CheckPointer(pPin, E_POINTER);

    // Base implementation verifies pin direction.
    HRESULT hr = CBasePin::CheckConnect(pPin);

    // Verify "OggS" magic bytes
    CComQIPtr<IAsyncReader> reader = pPin;
    if (!reader)
    {
        LOG(logERROR) << __FUNCTIONW__ << " No IAsyncReader interface found";
        hr = VFW_E_NO_TRANSPORT;
    }

    BYTE magic[4];
    if (SUCCEEDED(hr))
    {
        hr = reader->SyncRead(0, 4, magic);
    }

    if (SUCCEEDED(hr))
    {
        if (magic[0] != 'O' ||
            magic[1] != 'g' ||
            magic[2] != 'g' ||
            magic[3] != 'S')
        {
            LOG(logERROR) << __FUNCTIONW__ << " Magic is different than 'OggS': " << 
                magic[0] << ", " << magic[1] << ", " << magic[2] << ", " << magic[3];

            hr = VFW_E_UNSUPPORTED_STREAM;
        }
    }
    else
    {
        LOG(logERROR) << __FUNCTIONW__ << " SyncRead failed. Error: 0x" << hex << hr;
    }

    return hr;
}

HRESULT OggDemuxInputPin::BreakConnect()
{
    return CBasePin::BreakConnect();
}

HRESULT OggDemuxInputPin::Read(LONGLONG llOffset, long cBytes, BYTE* pBuffer)
{
    HRESULT hr = E_NOINTERFACE;
    
    CComQIPtr<IAsyncReader> reader = GetConnected();
    if (reader)
    {
        hr = reader->SyncRead(llOffset, cBytes, pBuffer);
    }
    return hr;
}

LONGLONG OggDemuxInputPin::Length()
{
    LONGLONG llTotal = 0;
    LONGLONG llAvail;
    
    CComQIPtr<IAsyncReader> reader = GetConnected();
    if (reader)
    {
        HRESULT hr = reader->Length(&llTotal, &llAvail);
    }

    return llTotal;
}

CComQIPtr<IAsyncReader> OggDemuxInputPin::GetReader()
{
    return GetConnected();
}
