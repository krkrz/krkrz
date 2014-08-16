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
#include <StringHelper.h>

StringHelper::StringHelper(void)
{
}

StringHelper::~StringHelper(void)
{
}


wstring StringHelper::toWStr(string inString) {
	wstring retVal;

	//LPCWSTR retPtr = new wchar_t[retVal.length() + 1];
	for (std::string::const_iterator i = inString.begin(); i != inString.end(); i++) {
		retVal.append(1, *i);
	}
	
	return retVal;
}

string StringHelper::toUTF8Str(wstring inString) {
	string retVal;

	unsigned char a;
	//LPCWSTR retPtr = new wchar_t[retVal.length() + 1];
	for (std::wstring::const_iterator i = inString.begin(); i != inString.end(); i++) 
	{
		// 0xxxxxxx
		if (*i < 0x80)
		{
			retVal.append(1, (unsigned char) *i);
		}
		// 110xxxxx 10xxxxxx
		else if (*i < 0x800)
		{
			a = (unsigned char)(0xC0 | (unsigned int)*i >> 6);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i & 0x3F);
 			retVal.append(1, a);
		}
		// 1110xxxx 10xxxxxx 10xxxxxx
		else if (*i < 0x10000)
		{
			a = (unsigned char)(0xE0 | (unsigned int)*i >> 12);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i >> 6 & 0x3F);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i & 0x3F);
 			retVal.append(1, a);
		}
		// 1111xxxx 10xxxxxx 10xxxxxx 10xxxxxx
		else if (*i < 0x200000)
		{
			a = (unsigned char)(0xF0 | (unsigned int)*i >> 18);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i >> 12 & 0x3F);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i >> 6 & 0x3F);
 			retVal.append(1, a);
			a = (unsigned char)(0x80 | (unsigned int)*i & 0x3F);
 			retVal.append(1, a);
		}
	}

	return retVal;
}


wstring StringHelper::fromUTF8Str(string inString) {
	wstring retVal;

    wchar_t a;

	//LPCWSTR retPtr = new wchar_t[retVal.length() + 1];
	for (unsigned int i=0; i < inString.length(); )
	{
		// 0xxxxxxx
		if ((inString[i] & 0x80) == 0)
		{
			a = inString[i];
			i++;
		}
		// 1111xxxx 10xxxxxx 10xxxxxx 10xxxxxx
		else if ((inString[i] & 0xF0) == 0xF0)
		{
			a = ((inString[i] & 0x0F) << 18) | 
				((inString[i+1] & 0x3F) << 12) | 
				((inString[i+2] & 0x3F) << 6) | 
				(inString[i+3] & 0x3F);
			i += 3;
		}
		// 1110xxxx 10xxxxxx 10xxxxxx
		else if ((inString[i] & 0xE0) == 0xE0)
		{
			a = ((inString[i] & 0x0F) << 12) | 
				((inString[i+1] & 0x3F) << 6) | 
				(inString[i+2] & 0x3F);
			i += 3;
		}
		// 110xxxxx 10xxxxxx
		else if ((inString[i] & 0xC0) == 0xC0)
		{
			a = ((inString[i] & 0x1F) << 6) | (inString[i+1] & 0x3F);
			i += 2;
		}
		else
		{
			// something has gone wrong. Get out!
			break;
		}

		retVal.append(1, a);
	}

	return retVal;
}

string StringHelper::toNarrowStr(wstring inString) {
	string retVal;

	//TODO::: This conversion may result in loss of data. Warning stays.

	//LPCWSTR retPtr = new wchar_t[retVal.length() + 1];
	for (std::wstring::const_iterator i = inString.begin(); i != inString.end(); i++) {
		retVal.append(1, (char) *i);
	}
	

	return retVal;
}

string StringHelper::numToString(LOOG_UINT64 inNum) {
	char locDigit = 0;
	string retStr = "";
	string temp = "";

	if (inNum == 0) return "0";

	while (inNum > 0) {
		locDigit = ((char)(inNum % 10)) + '0';
		inNum /= 10;
		temp = locDigit;
		temp.append(retStr);
		retStr = temp;
		//retStr.append(1, locDigit);
	}
	return retStr;
}

//Returns a value between 0 and 9 999 999 to represent a fraction / 10 000 000
LOOG_UINT64 StringHelper::stringToFractNum(string inString) {
	int locDigit = 0;
	LOOG_UINT64 retVal = 0;

	LOOG_UINT64 locMult = 1000000;

	size_t locStrLen = inString.length();

	for (unsigned long i = 0; i < locStrLen; i++) {
		locDigit = inString[i] - '0';
		//If it's not in the range 0-9 we bail out
		if ( !((locDigit >= 0) && (locDigit <=9)) ) {
			//FIX::: throw exception
			throw 0;
		}
		//retVal *= 10;
		retVal += (locDigit * locMult);
		locMult /= 10;

	}
	return retVal;

}

LOOG_UINT64 StringHelper::stringToNum(string inString) {
	int locDigit = 0;
	LOOG_UINT64 retVal = 0;
	size_t locStrLen = inString.length();

	for (unsigned long i = 0; i < locStrLen; i++) {
		locDigit = inString[i] - '0';
		//If it's not in the range 0-9 we bail out
		if ( !((locDigit >= 0) && (locDigit <=9)) ) {
			//FIX::: throw exception
			throw 0;
		}
		retVal *= 10;
		retVal += locDigit;

	}
	return retVal;

}

LOOG_UINT64 StringHelper::stringToNum(wstring inString) {
	int locDigit = 0;
	LOOG_UINT64 retVal = 0;
	size_t locStrLen = inString.length();

	for (unsigned long i = 0; i < locStrLen; i++) {
		locDigit = inString[i] - '0';
		//If it's not in the range 0-9 we bail out
		if ( !((locDigit >= 0) && (locDigit <=9)) ) {
			//FIX::: throw exception
			throw 0;
		}
		retVal *= 10;
		retVal += locDigit;

	}
	return retVal;

}


unsigned char StringHelper::digitToHex(unsigned char inDigit) {
	
	unsigned char locDigit = (inDigit > 9)		?	(inDigit  - 10) + A_BASE
												:	(inDigit) + ZERO_BASE;
	return locDigit;

}

string StringHelper::charToHexString(unsigned char inChar) {
	
	string retStr ="";
	retStr +=digitToHex(inChar / 16);

	retStr+= digitToHex(inChar % 16);
	return retStr;
}

