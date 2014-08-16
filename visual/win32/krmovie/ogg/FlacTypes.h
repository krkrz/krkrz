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

#ifndef FLACTYPES_H
#define FLACTYPES_H

struct FLACFORMAT 
{
    unsigned short numChannels;
    unsigned long numBitsPerSample;
    unsigned long samplesPerSec;
};

// {EE66A998-4E5C-4e23-A0F3-97C40D87EC48}
DEFINE_GUID(CLSID_PropsFLACEncoder,
0xee66a998, 0x4e5c, 0x4e23, 0xa0, 0xf3, 0x97, 0xc4, 0xd, 0x87, 0xec, 0x48);

// {DF9C0DC3-1924-4bfe-8DC1-1084453A0F8F}
DEFINE_GUID(IID_IFLACEncodeSettings,
0xdf9c0dc3, 0x1924, 0x4bfe, 0x8d, 0xc1, 0x10, 0x84, 0x45, 0x3a, 0xf, 0x8f);

// {3913F0AB-E7ED-41c4-979B-1D1FDD983C07}
DEFINE_GUID(MEDIASUBTYPE_FLAC, 
0x3913f0ab, 0xe7ed, 0x41c4, 0x97, 0x9b, 0x1d, 0x1f, 0xdd, 0x98, 0x3c, 0x7);

// {2C409DB0-95BF-47ba-B0F5-587256F1EDCF}
DEFINE_GUID(MEDIASUBTYPE_OggFLAC_1_0, 
0x2c409db0, 0x95bf, 0x47ba, 0xb0, 0xf5, 0x58, 0x72, 0x56, 0xf1, 0xed, 0xcf);

// {1CDC48AC-4C24-4b8b-982B-7007A29D83C4}
DEFINE_GUID(FORMAT_FLAC, 
0x1cdc48ac, 0x4c24, 0x4b8b, 0x98, 0x2b, 0x70, 0x7, 0xa2, 0x9d, 0x83, 0xc4);

// {77E3A6A3-2A24-43fa-B929-00747E4B560B}
DEFINE_GUID(CLSID_FLACEncodeFilter, 
0x77e3a6a3, 0x2a24, 0x43fa, 0xb9, 0x29, 0x0, 0x74, 0x7e, 0x4b, 0x56, 0xb);

// {3376086C-D6F9-4ce4-8B89-33CD570106B5}
DEFINE_GUID(CLSID_FLACDecodeFilter, 
0x3376086c, 0xd6f9, 0x4ce4, 0x8b, 0x89, 0x33, 0xcd, 0x57, 0x1, 0x6, 0xb5);

#endif // FLACTYPES_H