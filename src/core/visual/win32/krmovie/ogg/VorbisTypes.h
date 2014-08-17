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

#ifndef VORBISTYPES_H
#define VORBISTYPES_H

struct VORBISFORMAT 
{
    unsigned long vorbisVersion;
    unsigned long samplesPerSec;
    unsigned long minBitsPerSec;
    unsigned long avgBitsPerSec;
    unsigned long maxBitsPerSec;
    unsigned char numChannels;
};

struct VORBISFORMAT2
{
	unsigned long channels;
	unsigned long samplesPerSec;
	unsigned long bitsPerSample;
	unsigned long headerSize[3];  // 0: Identification, 1: Comment, 2: CodecSetup
};

// {A538F05F-DC08-4bf9-994F-18A86CCA6CC4}
DEFINE_GUID(CLSID_PropsVorbisEncoder, 
0xa538f05f, 0xdc08, 0x4bf9, 0x99, 0x4f, 0x18, 0xa8, 0x6c, 0xca, 0x6c, 0xc4);

// {5C94FE86-B93B-467f-BFC3-BD6C91416F9B}
DEFINE_GUID(CLSID_VorbisEncodeFilter, 
0x5c94fe86, 0xb93b, 0x467f, 0xbf, 0xc3, 0xbd, 0x6c, 0x91, 0x41, 0x6f, 0x9b);

// {8A0566AC-42B3-4ad9-ACA3-93B906DDF98A}
DEFINE_GUID(MEDIASUBTYPE_Vorbis, 
0x8a0566ac, 0x42b3, 0x4ad9, 0xac, 0xa3, 0x93, 0xb9, 0x6, 0xdd, 0xf9, 0x8a);

// {8D2FD10B-5841-4a6b-8905-588FEC1ADED9}
DEFINE_GUID(MEDIASUBTYPE_Vorbis2,
0x8D2FD10B, 0x5841, 0x4a6b, 0x89, 0x05, 0x58, 0x8F, 0xEC, 0x1A, 0xDE, 0xD9);

// {44E04F43-58B3-4de1-9BAA-8901F852DAE4}
DEFINE_GUID(FORMAT_Vorbis, 
0x44e04f43, 0x58b3, 0x4de1, 0x9b, 0xaa, 0x89, 0x1, 0xf8, 0x52, 0xda, 0xe4);

// {B36E107F-A938-4387-93C7-55E966757473}    
DEFINE_GUID(FORMAT_Vorbis2,
0xB36E107F, 0xA938, 0x4387, 0x93, 0xC7, 0x55, 0xE9, 0x66, 0x75, 0x74, 0x73);

// {A4C6A887-7BD3-4b33-9A57-A3EB10924D3A}
DEFINE_GUID(IID_IVorbisEncodeSettings, 
0xa4c6a887, 0x7bd3, 0x4b33, 0x9a, 0x57, 0xa3, 0xeb, 0x10, 0x92, 0x4d, 0x3a);

// {05A1D945-A794-44ef-B41A-2F851A117155}
DEFINE_GUID(CLSID_VorbisDecodeFilter, 
0x5a1d945, 0xa794, 0x44ef, 0xb4, 0x1a, 0x2f, 0x85, 0x1a, 0x11, 0x71, 0x55);

// {BFF86BE7-9E32-40EF-B200-7BCC7800CC72}
DEFINE_GUID(IID_IDownmixAudio, 
0xBFF86BE7, 0x9E32, 0x40EF, 0xB2, 0x00, 0x7B, 0xCC, 0x78, 0x00, 0xCC, 0x72);

#endif // VORBISTYPES_H