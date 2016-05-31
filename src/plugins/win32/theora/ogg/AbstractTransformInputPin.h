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

#pragma once

//Local Includes
#include "BasicSeekPassThrough.h"

//STL Includes
#include <vector>
//
using namespace std;

//Forward Declarations
class AbstractTransformOutputPin;
class AbstractTransformFilter;

class AbstractTransformInputPin 
	//Base Classes
	:	public CBaseInputPin
			//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/cbaseinputpinclass.asp
	,	public BasicSeekPassThrough
{
public:
	//Friend Classes
	friend class AbstractTransformOutputPin;

	//COM Setup
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
		
	//Constructors	
	AbstractTransformInputPin(AbstractTransformFilter* inParentFilter, CCritSec* inFilterLock, AbstractTransformOutputPin* inOutputPin, TCHAR* inObjectName, LPCWSTR inPinDisplayName, vector<CMediaType*> inAcceptableMediaTypes);
	virtual ~AbstractTransformInputPin(void);

	//Pin Conenction Methods
	virtual HRESULT BreakConnect();
	virtual HRESULT CompleteConnect(IPin *inReceivePin);

	//Media Type control methods
	virtual HRESULT CheckMediaType(const CMediaType *inMediaType);
	virtual HRESULT GetMediaType(int inPosition, CMediaType *outMediaType);
	virtual HRESULT SetMediaType(const CMediaType* inMediaType) = 0;

	//TODO::: Should be virtual ??
	virtual STDMETHODIMP Receive(IMediaSample *pSample);

	//Stream Messaging methods
	virtual STDMETHODIMP EndOfStream(void);
	virtual STDMETHODIMP BeginFlush();
	virtual STDMETHODIMP EndFlush();
	virtual STDMETHODIMP NewSegment(REFERENCE_TIME inStartTime, REFERENCE_TIME inStopTime, double inRate);

protected:
	//Constants and Enumerations
	static const short SIZE_16_BITS = 2;
	static const signed short SINT_MAX = 32767;
	static const signed short SINT_MIN = -32768;

	//Pure Virtuals for codec specific methods
	virtual HRESULT TransformData(unsigned char* inBuf, long inNumBytes) = 0;
	virtual bool ConstructCodec() = 0;
	virtual void DestroyCodec() = 0;

	virtual bool SetSampleParams(IMediaSample* outMediaSample, unsigned long inDataSize, REFERENCE_TIME* inStartTime, REFERENCE_TIME* inEndTime);

	HRESULT mHR;

	//TODO::: Couldn't this be gotten from the parent filter if we make them friends ??
	AbstractTransformOutputPin* mOutputPin;
	AbstractTransformFilter* mParentFilter;
	vector<CMediaType*> mAcceptableMediaTypes;

	//DirectShow timestamps in milliseconds
	__int64 m_dsTimeStart;
	__int64 m_dsTimeEnd;

	//fstream debugLog;
	CCritSec* mStreamLock;
};
