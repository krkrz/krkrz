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

#include <libilliCore.h>
#include <libOOOggSeek.h>

#include <fstream>
#include <map>

using namespace std;

class OggSeekTable
{
public:
	OggSeekTable(void);
	virtual ~OggSeekTable(void);

	typedef pair<LOOG_INT64, unsigned long> tSeekPair;
	typedef map<LOOG_INT64, unsigned long> tSeekMap;

	/// Returns a copy of the seek table.
	tSeekMap getSeekMap();

	/// Add a seek point (which consists of a time in DirectShow units, and a byte offset corresponding to that time) to the seek table.
	bool addSeekPoint(LOOG_INT64 inTime, unsigned long inStartPos);

	/// Given a requested seek time in DirectShow units, returns the closest time and byte to the seek time.
	tSeekPair getStartPos(LOOG_INT64 inTime);

	/// Returns whether this table is enabled or disabled.
	bool enabled();
    
protected:
	tSeekMap mSeekMap;
	tSeekMap::value_type mSeekValue;
	LOOG_INT64 mRealStartPos;

	//fstream stDebug;
	bool mEnabled;

private:
	OggSeekTable(const OggSeekTable&);  // Don't copy me
    OggSeekTable &operator=(const OggSeekTable&);  // Don't assign me
};
