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
#include <iBE_Math.h>

iBE_Math::iBE_Math(void)
{
}

iBE_Math::~iBE_Math(void)
{
}

unsigned long iBE_Math::charArrToULong(unsigned char* inCharArray)
{
	//Turns the next four bytes from the pointer in a long MSB (most sig. byte first/leftmost)
	unsigned long locVal = 0;
	for (int i = 0; i < 4; i++) {
		locVal <<= 8;
		locVal += inCharArray[i];
	}
	return locVal;
}
void iBE_Math::ULongToCharArr(unsigned long inLong, unsigned char* outCharArray)
{
	//Writes a long MSB (Most sig. byte first/leftmost) out to the char arr
	outCharArray[0] = (unsigned char) (inLong >> 24);
	outCharArray[1] = (unsigned char) ((inLong << 8) >> 24);
	outCharArray[2] = (unsigned char) ((inLong << 16) >> 24);
	outCharArray[3] = (unsigned char) ((inLong << 24) >> 24);

}

unsigned short iBE_Math::charArrToUShort(unsigned char* inCharArray) {
	unsigned short retShort = inCharArray[0];
	retShort = (retShort << 8) + inCharArray[1];
	return retShort;

}