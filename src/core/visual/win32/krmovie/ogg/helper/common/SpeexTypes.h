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

#ifndef SPEEXTYPES_H
#define SPEEXTYPES_H

struct SPEEXFORMAT 
{
    unsigned long speexVersion;
    unsigned long samplesPerSec;
    unsigned long minBitsPerSec;
    unsigned long avgBitsPerSec;
    unsigned long maxBitsPerSec;
    unsigned long numChannels;
};

// {ED79AEC0-68AD-4be6-B06E-B4D3C8101624}
DEFINE_GUID(CLSID_PropsSpeexEncoder, 
0xed79aec0, 0x68ad, 0x4be6, 0xb0, 0x6e, 0xb4, 0xd3, 0xc8, 0x10, 0x16, 0x24);

// {479038D2-57FF-41ee-B397-FB98199BF1E8}
DEFINE_GUID(IID_ISpeexEncodeSettings, 
0x479038d2, 0x57ff, 0x41ee, 0xb3, 0x97, 0xfb, 0x98, 0x19, 0x9b, 0xf1, 0xe8);

// {25A9729D-12F6-420e-BD53-1D631DC217DF}
DEFINE_GUID(MEDIASUBTYPE_Speex, 
0x25a9729d, 0x12f6, 0x420e, 0xbd, 0x53, 0x1d, 0x63, 0x1d, 0xc2, 0x17, 0xdf);

// {78701A27-EFB5-4157-9553-38A7854E3E81}
DEFINE_GUID(FORMAT_Speex, 
0x78701a27, 0xefb5, 0x4157, 0x95, 0x53, 0x38, 0xa7, 0x85, 0x4e, 0x3e, 0x81);

// {7036C2FE-A209-464c-97AB-95B9260EDBF7}
DEFINE_GUID(CLSID_SpeexEncodeFilter, 
0x7036c2fe, 0xa209, 0x464c, 0x97, 0xab, 0x95, 0xb9, 0x26, 0xe, 0xdb, 0xf7);

// {7605E26C-DE38-4b82-ADD8-FE2568CC0B25}
DEFINE_GUID(CLSID_SpeexDecodeFilter, 
0x7605e26c, 0xde38, 0x4b82, 0xad, 0xd8, 0xfe, 0x25, 0x68, 0xcc, 0xb, 0x25);

#endif // SPEEXTYPES_H