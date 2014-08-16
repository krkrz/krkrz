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
#include "AbstractTransformOutputPin.h"


AbstractTransformOutputPin::AbstractTransformOutputPin(
    AbstractTransformFilter* inParentFilter, CCritSec* inFilterLock, TCHAR* inObjectName, 
    LPCWSTR inPinDisplayName, int inBuffSize, int inNumBuffs, const MediaTypesList& inAcceptableMediaTypes) :	

CBaseOutputPin(inObjectName, inParentFilter, inFilterLock, &mHR, inPinDisplayName),	

mParentFilter(inParentFilter),	
mDataQueue(NULL),	
mDesiredBufferSize(inBuffSize),	
mDesiredBufferCount(inNumBuffs),	
mActualBufferSize(0),	
mActualBufferCount(0),	
mAcceptableMediaTypes(inAcceptableMediaTypes)
{
}

AbstractTransformOutputPin::~AbstractTransformOutputPin(void)
{	
	ReleaseDelegate();

    delete mDataQueue;
	mDataQueue = NULL;

	for (size_t i = 0; i < mAcceptableMediaTypes.size(); i++) {
		delete mAcceptableMediaTypes[i];
	}
}

STDMETHODIMP AbstractTransformOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
{
	if (riid == IID_IMediaSeeking) {
		*ppv = (IMediaSeeking*)this;
		((IUnknown*)*ppv)->AddRef();
		return NOERROR;
	}

	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv); 
}

HRESULT AbstractTransformOutputPin::DecideBufferSize(IMemAllocator* inAllocator, ALLOCATOR_PROPERTIES* inPropertyRequest) 
{
	HRESULT locHR = S_OK;

	ALLOCATOR_PROPERTIES locReqAlloc;
	ALLOCATOR_PROPERTIES locActualAlloc;
	
	if (inPropertyRequest->cbAlign <= 0) {
		locReqAlloc.cbAlign = 1;
	} else {
		locReqAlloc.cbAlign = inPropertyRequest->cbAlign;
	}

	if (inPropertyRequest->cbBuffer < mDesiredBufferSize) {
		locReqAlloc.cbBuffer = mDesiredBufferSize;
	} else {
		locReqAlloc.cbBuffer = inPropertyRequest->cbBuffer;
	}

	if (inPropertyRequest->cbPrefix < 0) {
			locReqAlloc.cbPrefix = 0;
	} else {
		locReqAlloc.cbPrefix = inPropertyRequest->cbPrefix;
	}
	
	if (inPropertyRequest->cBuffers < mDesiredBufferCount) {
		locReqAlloc.cBuffers = mDesiredBufferCount;
	} else {
		locReqAlloc.cBuffers = inPropertyRequest->cBuffers;
	}
	
	locHR = inAllocator->SetProperties(&locReqAlloc, &locActualAlloc);

	if (locHR != S_OK) {
		//TODO::: Handle a fail state here.
		return locHR;
	} else {
		//TODO::: Need to save this pointer to decommit in destructor ???
		locHR = inAllocator->Commit();

		if (locHR == S_OK) {
			mActualBufferCount = locActualAlloc.cBuffers;
			mActualBufferSize = locActualAlloc.cbBuffer;
		} else {
			//TODO::: Handle commit failure.
		}
	
		return locHR;
	}
}
HRESULT AbstractTransformOutputPin::CheckMediaType(const CMediaType *inMediaType) 
{
	for (size_t i = 0;  i < mAcceptableMediaTypes.size(); i++) {
		if	(		(inMediaType->majortype == mAcceptableMediaTypes[i]->majortype) 
				&& 	(inMediaType->subtype == mAcceptableMediaTypes[i]->subtype) 
				&&	(inMediaType->formattype == mAcceptableMediaTypes[i]->formattype)
			) 
		{
			return S_OK;
		
		}
	}

	//If it was none of them return false.
	return S_FALSE;
}

void AbstractTransformOutputPin::FillMediaType(CMediaType* outMediaType, int inPosition) 
{
	//We are gauranteed that inPosition is within the range of the vector (See GetMediaType)
	// If you override the mediaformat functions you must ensure this yourself before calling.

	outMediaType->SetType(&(mAcceptableMediaTypes[inPosition]->majortype));
	outMediaType->SetSubtype(&(mAcceptableMediaTypes[inPosition]->subtype));
	outMediaType->SetFormatType(&(mAcceptableMediaTypes[inPosition]->formattype));

	//If you want something different you need to override this yourself.
	// Sample size of 0 means variable size... so that is almost always acceptable.
	outMediaType->SetTemporalCompression(FALSE);
	outMediaType->SetSampleSize(0);
}

HRESULT AbstractTransformOutputPin::GetMediaType(int inPosition, CMediaType *outMediaType) 
{
	if (inPosition < 0) {
		return E_INVALIDARG;
	}

	if (((size_t)inPosition) < mAcceptableMediaTypes.size()) {
		FillMediaType(outMediaType, inPosition);
		CreateAndFillFormatBuffer(outMediaType, inPosition);
		return S_OK;
	} else {
		return VFW_S_NO_MORE_ITEMS;
	}
}

HRESULT AbstractTransformOutputPin::DeliverNewSegment(REFERENCE_TIME inStartTime, REFERENCE_TIME inStopTime, double inRate) 
{
    if (!mDataQueue)
    {
        return S_FALSE;
    }

	mDataQueue->NewSegment(inStartTime, inStopTime, inRate);
	return S_OK;
}
HRESULT AbstractTransformOutputPin::DeliverEndOfStream(void) 
{
    if (!mDataQueue)
    {
        return S_FALSE;
    }

    //Lock ?????
	mDataQueue->EOS();
    return S_OK;
}

HRESULT AbstractTransformOutputPin::DeliverEndFlush(void) 
{
	CAutoLock locLock(m_pLock);

    if (!mDataQueue)
    {
        return S_FALSE;
    }

	mDataQueue->EndFlush();
    return S_OK;
}

HRESULT AbstractTransformOutputPin::DeliverBeginFlush(void) 
{
	CAutoLock locLock(m_pLock);

    if (!mDataQueue)
    {
        return S_FALSE;
    }

	mDataQueue->BeginFlush();
    return S_OK;
}

HRESULT AbstractTransformOutputPin::CompleteConnect(IPin *inReceivePin) 
{
	CAutoLock locLock(m_pLock);
	HRESULT locHR = S_OK;

	//Here when another pin connects to us, we internally connect the seek delegate
	// from this output pin onto the input pin... and we release it on breakconnect.
	//
	IMediaSeeking* locSeeker = NULL;
	mParentFilter->mInputPin->NonDelegatingQueryInterface(IID_IMediaSeeking, (void**)&locSeeker);
	SetDelegate(locSeeker);
	
	mDataQueue = new COutputQueue (inReceivePin, &locHR, FALSE, FALSE, 1, TRUE, mActualBufferCount);			//Deleted in destructor

	if (FAILED(locHR)) {
		//Handle data Q failure
		
	}
	
	return CBaseOutputPin::CompleteConnect(inReceivePin);
}

HRESULT AbstractTransformOutputPin::BreakConnect(void) 
{
	CAutoLock locLock(m_pLock);

	delete mDataQueue;
	mDataQueue = NULL;

	HRESULT locHR = CBaseOutputPin::BreakConnect();
	ReleaseDelegate();

	return locHR;
}