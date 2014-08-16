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

#include <string>

using namespace std;

class StringHelper
{
public:
	StringHelper(void);
	~StringHelper(void);
	static const unsigned char A_BASE = 65;			//A in ASCII
	static const unsigned char ZERO_BASE = 48;

	/// Takes an unsigned char in the range 0-15 and returns a char between '0'-'9' or 'A'-'F'
	static unsigned char digitToHex(unsigned char inChar);

	/// Takes an unsigned char 0-255 and returns a hex string of length 2.
	static string charToHexString(unsigned char inChar);
	
	/// Takes an unsigned int64 and returns a string representation.
	static string numToString(LOOG_UINT64 inNum);

	/// Takes a string and returns an unsigned int64
	static LOOG_UINT64 stringToNum(string inString);

	/// Takes a wide string and returns an unsigned int64
	static LOOG_UINT64 stringToNum(wstring inString);

	/// Takes a string and Returns a value between 0 and 9 999 999 to represent a fraction / 10 000 000.
	static LOOG_UINT64 stringToFractNum(string inString);

	/// Converts a narrow string to a wide (2 byte) string
	static wstring toWStr(string inString);

	/// Converts a wide (2 byte) string to a narrow string.
	static string toNarrowStr(wstring inString);

	/// DLB. 10/9/2005. Converts a wide (2 byte) string to a UTF-8 narrow string.
	static string toUTF8Str(wstring inString);

	/// DLB. 10/9/2005. Converts a UTF-8 narrow string to a wide (2 byte) string.
	static wstring fromUTF8Str(string inString);

};
