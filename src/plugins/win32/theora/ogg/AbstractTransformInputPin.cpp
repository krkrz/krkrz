//===========================================================================
//Copyright (C) 2003, 2004 Zentaro Kavanagh
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
#include "AbstractTransformInputPin.h"


AbstractTransformInputPin::AbstractTransformInputPin (AbstractTransformFilter* inParentFilter, CCritSec* inFilterLock, AbstractTransformOutputPin* inOutputPin, TCHAR* inObjectName, LPCWSTR inPinDisplayName, vector<CMediaType*> inAcceptableMediaTypes)
	:	CBaseInputPin (inObjectName, inParentFilter, inFilterLock, &mHR, inPinDisplayName)

	,	mOutputPin (inOutputPin)
	,	mParentFilter (inParentFilter)
	
	,	mAcceptableMediaTypes(inAcceptableMediaTypes)
	,	m_dsTimeStart(0)
	,	m_dsTimeEnd(0)

{
	mStreamLock = new CCritSec;			//Deleted in destructor.
}

STDMETHODIMP AbstractTransformInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	//TODO::: This really shouldn't be exposed on an input pin
	if (riid == IID_IMediaSeeking) {
		*ppv = (IMediaSeeking*)this;
		((IUnknown*)*ppv)->AddRef();
		return NOERROR;
	}

	return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv); 
}

HRESULT AbstractTransformInputPin::BreakConnect() 
{
	CAutoLock locLock(m_pLock);
	//Release the seeking delegate
	ReleaseDelegate();
	return CBaseInputPin::BreakConnect();
}
HRESULT AbstractTransformInputPin::CompleteConnect (IPin *inReceivePin) 
{
	CAutoLock locLock(m_pLock);
	
	IMediaSeeking* locSeeker = NULL;
	inReceivePin->QueryInterface(IID_IMediaSeeking, (void**)&locSeeker);
	SetDelegate(locSeeker);
	return CBaseInputPin::CompleteConnect(inReceivePin);
}
AbstractTransformInputPin::~AbstractTransformInputPin(void)
{

	delete mStreamLock;
	for (size_t i = 0; i < mAcceptableMediaTypes.size(); i++) {
		delete mAcceptableMediaTypes[i];
	}

}


bool AbstractTransformInputPin::SetSampleParams(IMediaSample* outMediaSample, unsigned long inDataSize, REFERENCE_TIME* inStartTime, REFERENCE_TIME* inEndTime) 
{
	outMediaSample->SetTime(inStartTime, inEndTime);
	outMediaSample->SetMediaTime(NULL, NULL);
	outMediaSample->SetActualDataLength(inDataSize);
	outMediaSample->SetPreroll(FALSE);
	outMediaSample->SetDiscontinuity(FALSE);
	outMediaSample->SetSyncPoint(TRUE);
	return true;
}


STDMETHODIMP AbstractTransformInputPin::Receive(IMediaSample* inSample) 
{
	CAutoLock locLock(mStreamLock);

	HRESULT locHR = CheckStreaming();

	if (locHR == S_OK) {
		BYTE* locBuff = NULL;
		locHR = inSample->GetPointer(&locBuff);

		if (locHR != S_OK) {
			//TODO::: Do a debug dump or something here with specific error info.
			return locHR;
		} 
		else 
		{
			/* Read DirectShow timestamps */
			REFERENCE_TIME pTimeStart, pTimeEnd;
			if (inSample->GetTime(&pTimeStart, &pTimeEnd) == NOERROR) 
			{
				m_dsTimeStart = pTimeStart/10000;
				m_dsTimeEnd = pTimeEnd/10000;
			}

            //http://windowssdk.msdn.microsoft.com/en-us/library/ms787541.aspx
            //Consider using receive to validate conditions ^^^
			HRESULT locResult = TransformData(locBuff, inSample->GetActualDataLength());
			if (locResult == S_OK) {
				return S_OK;
			} else {
				return S_FALSE;
			}
		}
	} else {
		//Not streaming - Bail out.
		return S_FALSE;
	}
}

HRESULT AbstractTransformInputPin::CheckMediaType(const CMediaType *inMediaType) 
{
	//TO DO::: Neaten this up.
	for (size_t i = 0; i < mAcceptableMediaTypes.size(); i++) {
		if	(		(inMediaType->majortype == mAcceptableMediaTypes[i]->majortype) 
				&&	(inMediaType->subtype == mAcceptableMediaTypes[i]->subtype) 
				&&	(inMediaType->formattype == mAcceptableMediaTypes[i]->formattype)
			)
		{
			return S_OK;
		} 
	}
	//If it matched none... return false.
	return S_FALSE;
}

STDMETHODIMP AbstractTransformInputPin::EndOfStream(void) {
	CAutoLock locLock(mStreamLock);
	
	return mParentFilter->mOutputPin->DeliverEndOfStream();
}

STDMETHODIMP AbstractTransformInputPin::BeginFlush() {
	CAutoLock locLock(m_pLock);

	CBaseInputPin::BeginFlush();
	return mParentFilter->mOutputPin->DeliverBeginFlush();
}
STDMETHODIMP AbstractTransformInputPin::EndFlush() {
	CAutoLock locLock(m_pLock);

	mParentFilter->mOutputPin->DeliverEndFlush();
	return CBaseInputPin::EndFlush();
}

STDMETHODIMP AbstractTransformInputPin::NewSegment(REFERENCE_TIME inStartTime, REFERENCE_TIME inStopTime, double inRate) {
	CAutoLock locLock(mStreamLock);

	//This is called on BasePin and not BaseInputPin because the implementation is not overriden in BaseOutputPin.
	CBasePin::NewSegment(inStartTime, inStopTime, inRate);
	return mParentFilter->mOutputPin->DeliverNewSegment(inStartTime, inStopTime, inRate);
}

HRESULT AbstractTransformInputPin::GetMediaType(int inPosition, CMediaType *outMediaType) 
{
	//TODO::: Check for NULL Pointer.
	if (inPosition < 0) {
		return E_INVALIDARG;
	} else 	if (((size_t)inPosition) < mAcceptableMediaTypes.size()) {
		outMediaType->SetType(&(mAcceptableMediaTypes[inPosition]->majortype));
		outMediaType->SetSubtype(&(mAcceptableMediaTypes[inPosition]->subtype));
		//Don't set the format data here... its up to the connecting output pin
		// to do this, and we will verify it in CheckMediaType
		return S_OK;
	} else {
		return VFW_S_NO_MORE_ITEMS;
	}

}