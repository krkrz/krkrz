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

#ifndef THEORATYPES_H
#define THEORATYPES_H

struct THEORAFORMAT 
{
    unsigned long theoraVersion;
    unsigned long outerFrameWidth;
    unsigned long outerFrameHeight;
    unsigned long pictureWidth;
    unsigned long pictureHeight;
    unsigned long frameRateNumerator;
    unsigned long frameRateDenominator;
    unsigned long aspectNumerator;
    unsigned long aspectDenominator;
    unsigned long maxKeyframeInterval;
    unsigned long targetBitrate;
    unsigned char targetQuality;
    unsigned char xOffset;
    unsigned char yOffset;
    unsigned char colourSpace;
    unsigned char pixelFormat;
};

// {121EA765-6D3F-4519-9686-A0BA6E5281A2}
DEFINE_GUID(CLSID_PropsTheoraEncoder, 
0x121ea765, 0x6d3f, 0x4519, 0x96, 0x86, 0xa0, 0xba, 0x6e, 0x52, 0x81, 0xa2);

// {4F063B3A-B397-4c22-AFF4-2F8DB96D292A}
DEFINE_GUID(IID_ITheoraEncodeSettings, 
0x4f063b3a, 0xb397, 0x4c22, 0xaf, 0xf4, 0x2f, 0x8d, 0xb9, 0x6d, 0x29, 0x2a);

// {5C769985-C3E1-4f95-BEE7-1101C465F5FC}
DEFINE_GUID(CLSID_TheoraEncodeFilter, 
0x5c769985, 0xc3e1, 0x4f95, 0xbe, 0xe7, 0x11, 0x1, 0xc4, 0x65, 0xf5, 0xfc);

// TheoraDecodeFilter {05187161-5C36-4324-A734-22BF37509F2D}
DEFINE_GUID(CLSID_TheoraDecodeFilter,
0x5187161, 0x5c36, 0x4324, 0xa7, 0x34, 0x22, 0xbf, 0x37, 0x50, 0x9f, 0x2d);

// {D124B2B1-8968-4ae8-B288-FE16EA34B0CE}
DEFINE_GUID(MEDIASUBTYPE_Theora, 
0xd124b2b1, 0x8968, 0x4ae8, 0xb2, 0x88, 0xfe, 0x16, 0xea, 0x34, 0xb0, 0xce);

// {A99F116C-DFFA-412c-95DE-725F99874826}
DEFINE_GUID(FORMAT_Theora, 
0xa99f116c, 0xdffa, 0x412c, 0x95, 0xde, 0x72, 0x5f, 0x99, 0x87, 0x48, 0x26);

#endif // THEORATYPES_H
