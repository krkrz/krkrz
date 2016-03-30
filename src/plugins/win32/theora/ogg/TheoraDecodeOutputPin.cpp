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

#include "oggstdafx.h"
#include "TheoraDecodeOutputpin.h"
#include "TheoraDecodeFilter.h"

TheoraDecodeOutputPin::TheoraDecodeOutputPin(TheoraDecodeFilter* inParentFilter, HRESULT* outHR) :	
CTransformOutputPin(NAME("Theora Output Pin"), inParentFilter, outHR, L"Theora Out")
{
}

TheoraDecodeOutputPin::~TheoraDecodeOutputPin() 
{
}

STDMETHODIMP TheoraDecodeOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
{
	//LOG(logDEBUG) << "Querying interface";
	if (riid == IID_IMediaSeeking) 
    {
        return GetInterface((IMediaSeeking*)this, ppv);
	} 
    else if (riid == IID_IMediaPosition) 
    {
		//LOG(logDEBUG) << "Asking for OLD SEEKER";
	}
	
    //LOG(logDEBUG) << "Trying base output pin";
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv); 
}

HRESULT TheoraDecodeOutputPin::BreakConnect() 
{
	CAutoLock locLock(m_pLock);
	//Need a lock ??
	ReleaseDelegate();
	LOG(logDEBUG) << __FUNCTIONW__;
	
    return CTransformOutputPin::BreakConnect();
}

HRESULT TheoraDecodeOutputPin::CompleteConnect (IPin *inReceivePin) 
{
	CAutoLock locLock(m_pLock);
	LOG(logDEBUG) << __FUNCTIONW__;

	IMediaSeeking* locSeeker = NULL;

	m_pFilter->GetPin(0)->QueryInterface(IID_IMediaSeeking, (void**)&locSeeker);

	if (locSeeker == NULL) 
    {
		LOG(logDEBUG) << __FUNCTIONW__ << "Seeker was NULL";
	}

	SetDelegate(locSeeker);
	
    return CTransformOutputPin::CompleteConnect(inReceivePin);
}

STDMETHODIMP TheoraDecodeOutputPin::Notify(IBaseFilter* inMessageSource, Quality inQuality) 
{
	return E_NOTIMPL;
}

TheoraDecodeFilter* TheoraDecodeOutputPin::GetFilter()
{
    return static_cast<TheoraDecodeFilter*>(m_pTransformFilter);
}
