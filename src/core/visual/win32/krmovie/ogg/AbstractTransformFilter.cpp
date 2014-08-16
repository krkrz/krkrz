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

//
// AbstracttransformFilter.cpp :	Abstract Filter Class for transform filters.
//


#include "oggstdafx.h"
#include "AbstractTransformFilter.h"

//Constructors
AbstractTransformFilter::AbstractTransformFilter(const wchar_t* inFilterName, REFCLSID inFilterGUID)
	:	CBaseFilter(inFilterName, NULL, m_pLock, inFilterGUID)

	,	mInputPin(NULL)
	,	mOutputPin(NULL)
	
{
	//Create the filter lock.
	m_pLock = new CCritSec;		//Deleted in destructor... check what is happening in the base class.
}

AbstractTransformFilter::~AbstractTransformFilter(void)
{
	DestroyPins();
	delete m_pLock;		//Deleting filter lock
}

void AbstractTransformFilter::DestroyPins() 
{
	delete mOutputPin;
	delete mInputPin;
}

//If you want to handle an interface, do it here.
STDMETHODIMP AbstractTransformFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
{

	//May just get rid of this !!
	return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

CBasePin* AbstractTransformFilter::GetPin(int inPinNo) 
{
	//Pin Constants
	const int INPUT_PIN = 0;
	const int OUTPUT_PIN = 1;
	
	//Return the pin.
	switch (inPinNo) {
		case INPUT_PIN:		
			return mInputPin;
		case OUTPUT_PIN:
			return mOutputPin;
		default:
			return NULL;
	};
}

int AbstractTransformFilter::GetPinCount(void) 
{
	const long NUM_PINS = 2;
	return NUM_PINS;
}	

