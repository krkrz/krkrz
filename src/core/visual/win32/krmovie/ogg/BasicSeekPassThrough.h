//===========================================================================
//Copyright (C) 2003, 2004 Zentaro Kavanagh
//
//Copyright (C) 2003, 2004 Commonwealth Scientific and Industrial Research
//Organisation (CSIRO) Australia
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//notice, this list of conditions and the following disclaimer in the
//documentation and/or other materials provided with the distribution.
//
//- Neither the name of CSIRO Australia nor the names of its
//contributors may be used to endorse or promote products derived from
//this software without specific prior written permission.
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

class BasicSeekPassThrough
	//Base classes.
	:	public IMediaSeeking
{
public:
	//Constructors
	BasicSeekPassThrough(void);
	BasicSeekPassThrough(IMediaSeeking* inDelegate);
	virtual ~BasicSeekPassThrough(void);

	//Delegate Control Methods.
	bool SetDelegate(IMediaSeeking* inDelegate);
	bool ReleaseDelegate();
	
	//IMediaSeeking Interface
	virtual STDMETHODIMP GetCapabilities(DWORD *pCapabilities);
	virtual STDMETHODIMP CheckCapabilities(DWORD *pCapabilities);
	virtual STDMETHODIMP IsFormatSupported(const GUID *pFormat);
	virtual STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
	virtual STDMETHODIMP SetTimeFormat(const GUID *pFormat);
	virtual STDMETHODIMP GetTimeFormat( GUID *pFormat);
	virtual STDMETHODIMP GetDuration(LONGLONG *pDuration);
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

protected:
	//Member data
	IMediaSeeking* mSeekDelegate;
	HRESULT mHR;
	DWORD mSeekingCap;

	//Debug only.
	//fstream seekDebug;
};
