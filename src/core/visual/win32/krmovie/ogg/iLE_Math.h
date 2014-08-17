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
#include <illicoreconfig.h>

class iLE_Math
{
public:
	iLE_Math(void);
	~iLE_Math(void);

	/// Reads 4 bytes (Little Endian) and returns an unsigned long
	static unsigned long charArrToULong(const unsigned char* inCharArray);

	/// Takes an unsigned long and writes 4 bytes (Little Endian) into the buffer you pass.
	static void ULongToCharArr(unsigned long inLong, unsigned char* outCharArray);

	/// Takes an unsigned short and writes 2 bytes (Little Endian) into the buffer you pass.
	static void UShortToCharArr(unsigned short inShort, unsigned char* outCharArray);
	
	/// Reads 8 bytes (Little Endian) and returns an int 64.
	static LOOG_INT64 CharArrToInt64(const unsigned char* inCharArray);

	/// Takes an int64 and writes 8 bytes (Little Endian) into the buffer you pass.
	static void Int64ToCharArr(LOOG_INT64 inInt64, unsigned char* outCharArray);

	/// Reads 2 bytes (Little Endian) and returns an unsigned short
	static unsigned short charArrToUShort(const unsigned char* inCharArray);	
};
