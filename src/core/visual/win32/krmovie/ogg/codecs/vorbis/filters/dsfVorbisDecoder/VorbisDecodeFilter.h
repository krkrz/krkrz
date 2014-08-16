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


#pragma once
//Include Files
#include "AbstractTransformFilter.h"

//#include "wmpservices.h"

//Forward Declarations
struct VORBISFORMAT;
struct VORBISFORMAT2;
class VorbisDecodeInputPin;
class VorbisDecodeOutputPin;

//Class Interface
class VorbisDecodeFilter:	public AbstractTransformFilter
                            //,   public IWMPTranscodePolicy
{
public:
	//Friends
	friend class VorbisDecodeInputPin;
	friend class VorbisDecodeOutputPin;

	//Constructors and Destructors
	VorbisDecodeFilter();
	virtual ~VorbisDecodeFilter();

    static const wchar_t* NAME;
    static const AMOVIESETUP_MEDIATYPE m_inputMediaTypes[3];
    static const AMOVIESETUP_MEDIATYPE m_outputMediaTypes;
    static const AMOVIESETUP_PIN m_pinReg[];
    static const AMOVIESETUP_FILTER m_filterReg;

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    //IWMPTranscodePolicy interface -- it's documented... but it doesn't really exist.
    //HRESULT allowTranscode(VARIANT_BOOL* outAllowTranscode);


	///COM CreateInstance Function
	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);

	virtual VORBISFORMAT* getVorbisFormatBlock();
	virtual void setVorbisFormat(BYTE* inFormatBlock);
    virtual void setVorbisFormat(VORBISFORMAT* vorbisFormat);
    virtual void setVorbisFormat(VORBISFORMAT2* vorbisFormat2);

#ifdef WINCE
	virtual LPAMOVIESETUP_FILTER GetSetupData();
#endif

protected:
	//VIRTUAL FUNCTIONS - AbstractTransformFilter
	virtual bool ConstructPins();

    void PrintVorbisFormatInfo();

	//Format Block
	VORBISFORMAT* mVorbisFormatInfo;
};
