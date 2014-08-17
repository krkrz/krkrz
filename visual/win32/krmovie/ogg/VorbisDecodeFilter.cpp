//===========================================================================
//Copyright (C) 2003-2006 Zentaro Kavanagh
//Copyright (C) 2009-2010 Cristian Adam
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

//Include Files
#include "oggstdafx.h"
#include "VorbisDecodeFilter.h"
#include <initguid.h>
#include "oggutil.h"
#include "OggTypes.h"
#include "VorbisTypes.h"

//COM Factory Template
/*
CFactoryTemplate g_Templates[] = 
{
    { 
		VorbisDecodeFilter::NAME,			    // Name
	    &CLSID_VorbisDecodeFilter,              // CLSID
	    VorbisDecodeFilter::CreateInstance,	    // Method to create an instance of MyComponent
        NULL,								    // Initialization function
        &VorbisDecodeFilter::m_filterReg	    // Set-up information (for filters)
    }
};

// Generic way of determining the number of items in the template
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]); 
*/

const wchar_t* VorbisDecodeFilter::NAME = L"Xiph.Org Vorbis Decoder";

const REGPINTYPES VorbisDecodeFilter::m_outputMediaTypes = 
{
    &MEDIATYPE_Audio,
    &MEDIASUBTYPE_PCM
};

const REGPINTYPES VorbisDecodeFilter::m_inputMediaTypes[] = 
{
	{ &MEDIATYPE_OggPacketStream, &MEDIASUBTYPE_None },
	{ &MEDIATYPE_Audio, &MEDIASUBTYPE_Vorbis},
	{ &MEDIATYPE_Audio, &MEDIASUBTYPE_Vorbis2}
};

const AMOVIESETUP_PIN VorbisDecodeFilter::m_pinReg[] = 
{
    {
        L"Vorbis Input",					//Name (obsoleted)
        FALSE,								//Renders from this pin ?? Not sure about this.
        FALSE,								//Not an output pin
        FALSE,								//Cannot have zero instances of this pin
        FALSE,								//Cannot have more than one instance of this pin
        &GUID_NULL,							//Connects to filter (obsoleted)
        NULL,								//Connects to pin (obsoleted)
		sizeof(m_inputMediaTypes) / sizeof(REGPINTYPES),
        m_inputMediaTypes				    //Pointer to media type (Audio/Vorbis or Audio/Speex)
    } ,

    {
        L"PCM Output",						//Name (obsoleted)
        FALSE,								//Renders from this pin ?? Not sure about this.
        TRUE,								//Is an output pin
        FALSE,								//Cannot have zero instances of this pin
        FALSE,								//Cannot have more than one instance of this pin
        &GUID_NULL,							//Connects to filter (obsoleted)
        NULL,								//Connects to pin (obsoleted)
        1,									//Only support one media type
        &m_outputMediaTypes     			//Pointer to media type (Audio/PCM)
    }
};
const AMOVIESETUP_FILTER VorbisDecodeFilter::m_filterReg = 
{
    &CLSID_VorbisDecodeFilter,
    NAME,
    MERIT_NORMAL,
    2,
    m_pinReg
};


#ifdef WINCE
LPAMOVIESETUP_FILTER VorbisDecodeFilter::GetSetupData()
{	
	return (LPAMOVIESETUP_FILTER)&m_filterReg;	
}
#endif

VorbisDecodeFilter::VorbisDecodeFilter() :
AbstractTransformFilter(NAME("Vorbis Decoder"), CLSID_VorbisDecodeFilter),
mVorbisFormatInfo(NULL)
{
    LOG(logINFO) << L"VorbisDecodeFilter object created!" << std::endl;

	ConstructPins();
}

STDMETHODIMP VorbisDecodeFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	//if (riid == IID_IWMPTranscodePolicy) {
	//	*ppv = (IWMPTranscodePolicy*)this;
	//	((IUnknown*)*ppv)->AddRef();
	//	return NOERROR;
	//}

	return AbstractTransformFilter::NonDelegatingQueryInterface(riid, ppv); 
}

//HRESULT VorbisDecodeFilter::allowTranscode(VARIANT_BOOL* outAllowTranscode)
//{
//    *outAllowTranscode = VARIANT_TRUE;
//    return S_OK;
//}

bool VorbisDecodeFilter::ConstructPins() 
{
	// Vector to hold our set of media types we want to accept.
    MediaTypesList acceptableTypes;

	// Setup the media types for the output pin, media types are deleted in pin destructor
	CMediaType* mediaType = new CMediaType(&MEDIATYPE_Audio);
	mediaType->subtype = MEDIASUBTYPE_PCM;
	mediaType->formattype = FORMAT_WaveFormatEx;
	acceptableTypes.push_back(mediaType);

	//Second one the same type... they are actually different one is the extensible 
    // format. See CreateAndFill
	mediaType = new CMediaType(&MEDIATYPE_Audio);
	mediaType->subtype = MEDIASUBTYPE_PCM;
	mediaType->formattype = FORMAT_WaveFormatEx;
	acceptableTypes.push_back(mediaType);

	// Output pin must be done first because it's passed to the input pin. 
    // Deleted in base class destructor
	mOutputPin = new VorbisDecodeOutputPin(this, m_pLock, acceptableTypes);			

	// Clear out the vector, now we've already passed it to the output pin.
	acceptableTypes.clear();

	//Setup the media Types for the input pin.
    // OggPacketStream received from Ogg Demuxer filter
	mediaType = new CMediaType(&MEDIATYPE_OggPacketStream);
	mediaType->subtype = MEDIASUBTYPE_None;
	mediaType->formattype = FORMAT_OggIdentHeader;
    acceptableTypes.push_back(mediaType);

    // Vorbis stream received from Vorbis Encoder
    mediaType = new CMediaType(&MEDIATYPE_Audio);
    mediaType->subtype = MEDIASUBTYPE_Vorbis;
    mediaType->formattype = FORMAT_Vorbis;
	acceptableTypes.push_back(mediaType);

	mediaType = new CMediaType(&MEDIATYPE_Audio);
	mediaType->subtype = MEDIASUBTYPE_Vorbis2;
	mediaType->formattype = FORMAT_Vorbis2;
	acceptableTypes.push_back(mediaType);

	
    //Deleted in base class filter destructor.
	mInputPin = new VorbisDecodeInputPin(this, m_pLock, mOutputPin, acceptableTypes);	
	return true;
}

VorbisDecodeFilter::~VorbisDecodeFilter(void)
{
    LOG(logINFO) << L"VorbisDecodeFilter destroyed!" << std::endl;
	delete mVorbisFormatInfo;
}

CUnknown* WINAPI VorbisDecodeFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
    util::ConfigureLogSettings();

    VorbisDecodeFilter *pNewObject = new (std::nothrow) VorbisDecodeFilter();
    if (pNewObject == NULL) 
    {
        *pHr = E_OUTOFMEMORY;
    }
	return pNewObject;
} 

VORBISFORMAT* VorbisDecodeFilter::getVorbisFormatBlock() 
{
	return mVorbisFormatInfo;
}

void VorbisDecodeFilter::setVorbisFormat(BYTE* inFormatBlock) 
{
    if (!inFormatBlock)
    {
        return;
    }

	delete mVorbisFormatInfo;
	mVorbisFormatInfo = new VORBISFORMAT;

	mVorbisFormatInfo->vorbisVersion = iLE_Math::charArrToULong(inFormatBlock + 7);
	mVorbisFormatInfo->numChannels = inFormatBlock[11];
	mVorbisFormatInfo->samplesPerSec = iLE_Math::charArrToULong(inFormatBlock + 12);
	mVorbisFormatInfo->maxBitsPerSec = iLE_Math::charArrToULong(inFormatBlock + 16);
	mVorbisFormatInfo->avgBitsPerSec = iLE_Math::charArrToULong(inFormatBlock + 20);
	mVorbisFormatInfo->minBitsPerSec = iLE_Math::charArrToULong(inFormatBlock + 24);

    PrintVorbisFormatInfo();
}

void VorbisDecodeFilter::setVorbisFormat(VORBISFORMAT* vorbisFormat)
{
    if (!vorbisFormat)
    {
        return;
    }

    delete mVorbisFormatInfo;
    mVorbisFormatInfo = new VORBISFORMAT;

    *mVorbisFormatInfo = *vorbisFormat;
    PrintVorbisFormatInfo();
}

void VorbisDecodeFilter::setVorbisFormat(VORBISFORMAT2* vorbisFormat2)
{
    if (!vorbisFormat2)
    {
        return;
    }

    delete mVorbisFormatInfo;
    mVorbisFormatInfo = new VORBISFORMAT;

    mVorbisFormatInfo->numChannels = (unsigned char)vorbisFormat2->channels;
    mVorbisFormatInfo->samplesPerSec = vorbisFormat2->samplesPerSec;
    mVorbisFormatInfo->avgBitsPerSec = vorbisFormat2->bitsPerSample;
    mVorbisFormatInfo->maxBitsPerSec = 0;
    mVorbisFormatInfo->avgBitsPerSec = 0;
    mVorbisFormatInfo->minBitsPerSec = 0;

    PrintVorbisFormatInfo();
}


void VorbisDecodeFilter::PrintVorbisFormatInfo()
{
    LOG(logINFO) << "Vorbis Version: " << mVorbisFormatInfo->vorbisVersion;
    LOG(logINFO) << "Channels: " << mVorbisFormatInfo->numChannels;
    LOG(logINFO) << "SamplesPerSec: " << mVorbisFormatInfo->samplesPerSec;
    LOG(logINFO) << "MaxBitsPerSec: " << mVorbisFormatInfo->maxBitsPerSec;
    LOG(logINFO) << "AvgBitsPerSec: " << mVorbisFormatInfo->avgBitsPerSec;
    LOG(logINFO) << "MinBitsPerSec: " << mVorbisFormatInfo->minBitsPerSec;
}
