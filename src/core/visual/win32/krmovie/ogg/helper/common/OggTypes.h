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

#ifndef OGGTYPES_H
#define OGGTYPES_H

// {60891713-C24F-4767-B6C9-6CA05B3338FC}
DEFINE_GUID(MEDIATYPE_OggPacketStream, 
0x60891713, 0xc24f, 0x4767, 0xb6, 0xc9, 0x6c, 0xa0, 0x5b, 0x33, 0x38, 0xfc);

// {95388704-162C-42a9-8149-C3577C12AAF9}
DEFINE_GUID(FORMAT_OggIdentHeader, 
0x95388704, 0x162c, 0x42a9, 0x81, 0x49, 0xc3, 0x57, 0x7c, 0x12, 0xaa, 0xf9);

// {43F0F818-10B0-4c86-B9F1-F6B6E2D33462}
DEFINE_GUID(IID_IOggDecoder, 
0x43f0f818, 0x10b0, 0x4c86, 0xb9, 0xf1, 0xf6, 0xb6, 0xe2, 0xd3, 0x34, 0x62);

// {83D7F506-53ED-4f15-B6D8-7D8E9E72A918}
DEFINE_GUID(IID_IOggOutputPin, 
0x83d7f506, 0x53ed, 0x4f15, 0xb6, 0xd8, 0x7d, 0x8e, 0x9e, 0x72, 0xa9, 0x18);

// {90D6513C-A665-4b16-ACA7-B3D1D4EFE58D}
DEFINE_GUID(IID_IOggMuxProgress, 
0x90d6513c, 0xa665, 0x4b16, 0xac, 0xa7, 0xb3, 0xd1, 0xd4, 0xef, 0xe5, 0x8d);

// {30EB3AD8-B2DD-4f9a-9C25-845999B03476}
DEFINE_GUID(IID_IOggSeekTable, 
0x30eb3ad8, 0xb2dd, 0x4f9a, 0x9c, 0x25, 0x84, 0x59, 0x99, 0xb0, 0x34, 0x76);

// {30393ca2-c404-4744-a21e-90975700ea8f}
DEFINE_GUID(CLSID_PropsOggMux,
0x30393ca2, 0xc404, 0x4744, 0xa2, 0x1e, 0x90, 0x97, 0x57, 0x00, 0xea, 0x8f);

// {3a2cf997-0aeb-4d3f-9846-b5db2ca4c80b}
DEFINE_GUID(IID_IOggMuxSettings, 
0x3a2cf997, 0x0aeb, 0x4d3f, 0x98, 0x46, 0xb5, 0xdb, 0x2c, 0xa4, 0xc8, 0x0b);

// Stream subtype for Ogg byte streams. The filter can handle
// MEDIASUBTYPE_NULL, but specifying MEDIASUBTYPE_Ogg can save
// the filter graph the trouble of trying out all the available
// filters.

// {DD142C1E-0C1E-4381-A24E-0B2D80B6098A}
DEFINE_GUID(MEDIASUBTYPE_Ogg, 
0xdd142c1e, 0xc1e, 0x4381, 0xa2, 0x4e, 0xb, 0x2d, 0x80, 0xb6, 0x9, 0x8a);

// {C9361F5A-3282-4944-9899-6D99CDC5370B}
DEFINE_GUID(CLSID_OggDemuxFilter, 
0xc9361f5a, 0x3282, 0x4944, 0x98, 0x99, 0x6d, 0x99, 0xcd, 0xc5, 0x37, 0xb);

// {EB5AED9C-8CD0-4c4b-B5E8-F5D10AD1314D}
DEFINE_GUID(IID_IOggBaseTime, 
0xeb5aed9c, 0x8cd0, 0x4c4b, 0xb5, 0xe8, 0xf5, 0xd1, 0xa, 0xd1, 0x31, 0x4d);

// {1F3EFFE4-0E70-47c7-9C48-05EB99E20011}
DEFINE_GUID(CLSID_OggMuxFilter, 
0x1f3effe4, 0xe70, 0x47c7, 0x9c, 0x48, 0x5, 0xeb, 0x99, 0xe2, 0x0, 0x11);


#endif // OGGTYPES_H