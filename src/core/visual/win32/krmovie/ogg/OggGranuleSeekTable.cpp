//===========================================================================
//Copyright (C) 2003, 2004, 2005 Zentaro Kavanagh
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
#include <OggGranuleSeekTable.h>

OggGranuleSeekTable::OggGranuleSeekTable(void)
	:	mEnabled(true)
	,	mRealStartPos(0)
{
	mSeekMap.clear();
	mSeekMap.empty();
	//stDebug.open("G:\\logs\\sktable.log", ios_base::out);
}

OggGranuleSeekTable::~OggGranuleSeekTable(void)
{
	//stDebug.close();
}

bool OggGranuleSeekTable::enabled() {
	return mEnabled;
}
bool OggGranuleSeekTable::addSeekPoint(LOOG_INT64 inTime, LOOG_INT64 mStartPos, LOOG_INT64 inGranulePos)
{
	//stDebug<< "Add Point :  Time = "<<inTime<<"   --   Byte Pos : "<<mStartPos<<endl;
	tGranulePair locPair;
	locPair.first = mStartPos;
	locPair.second = inGranulePos;
	
	mSeekMap.insert(tSeekMap::value_type(inTime, locPair));

	return true;
}


/** Returns a tSeekPair whose first element is the
    actual closest possible time that can be seeked to (which will always be either before or at
    the requested seek position).  The second element is the number of bytes into the stream where
    the first page of the actual seek time occurs.
  */
OggGranuleSeekTable::tSeekPair OggGranuleSeekTable::getStartPos(LOOG_INT64 inTime)
{
	// Finds the upper bound of the requested time in mSeekMap, which will always be in the range
	// (0, maxItems], and return the element _before_ the upper bound
    //return *(--(mSeekMap.upper_bound(inTime)));
    if (mSeekMap.empty())
    {
        tSeekPair empty;
        tGranulePair zeroGranule;

        zeroGranule.first = 0;
        zeroGranule.second = 0;
        empty.first = 0;
        empty.second = zeroGranule;

        return empty;
    }

	tSeekMap::iterator locIt = mSeekMap.lower_bound(inTime);
	if (locIt == mSeekMap.begin()) 
    {
		return *(locIt);
	} 
    else 
    {
		return *(--locIt);
	}
}

/** Note that this method returns a copy of the seek table, not the actual seek table used by
    the class.  So, feel free to corrupt your copy to your heart's leisure.
  */
OggGranuleSeekTable::tSeekMap OggGranuleSeekTable::getSeekMap()
{
	tSeekMap locSeekMap = mSeekMap;

	return locSeekMap;
}
