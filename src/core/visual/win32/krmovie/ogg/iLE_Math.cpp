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

#include "oggstdafx.h"
#include <iLE_Math.h>

iLE_Math::iLE_Math(void)
{
}

iLE_Math::~iLE_Math(void)
{
}

unsigned long iLE_Math::charArrToULong(const unsigned char* inCharArray)
{
	//Turns the next four bytes from the pointer in a long LSB (least sig. byte first/leftmost)
	unsigned long locVal = 0;
	for (int i = 3; i >= 0; i--) {
		locVal <<= 8;
		locVal += inCharArray[i];
	}
	return locVal;
}
void iLE_Math::ULongToCharArr(unsigned long inLong, unsigned char* outCharArray)
{
	//Writes a long LSB (least sig. byte first/leftmost) out to the char arr
	
	outCharArray[3] = (unsigned char) (inLong >> 24);
	outCharArray[2] = (unsigned char) ((inLong << 8) >> 24);
	outCharArray[1] = (unsigned char) ((inLong << 16) >> 24);
	outCharArray[0] = (unsigned char) ((inLong << 24) >> 24);

}

void iLE_Math::UShortToCharArr(unsigned short inShort, unsigned char* outCharArray) {
	outCharArray[0] = (unsigned char)(inShort % 256);
	outCharArray[1] = (unsigned char)(inShort / 256);
}
unsigned short iLE_Math::charArrToUShort(const unsigned char* inCharArray) {
	return (inCharArray[1] << 8) + inCharArray[0];
}
LOOG_INT64 iLE_Math::CharArrToInt64(const unsigned char* inCharArray) {
	LOOG_INT64 locData = 0;

	for (int i = 7; i >= 0; i--) {
		locData <<= 8;
		locData += inCharArray[i];
	}
	return locData;
}

void iLE_Math::Int64ToCharArr(LOOG_INT64 inInt64, unsigned char* outCharArray) {
	for (unsigned char i = 0; i < 8; i++) {
		//This way sux !
		outCharArray[i] = (unsigned char)((inInt64 << ((7 - i) * 8)) >> 56);
	}
}