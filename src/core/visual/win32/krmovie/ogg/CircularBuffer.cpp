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
#include <CircularBuffer.h>
#undef min

//Leak checked : 20041017 - OK
CircularBuffer::CircularBuffer(unsigned long inBufferSize)
	:	mBufferSize(inBufferSize)
	,	mBuffer(NULL)
	,	mReadPtr(0)
	,	mWritePtr(0)

{
	mBuffer = new unsigned char[inBufferSize + 1];			//Deleted in destructor.
}

CircularBuffer::~CircularBuffer()
{
	delete[] mBuffer;
}

unsigned long CircularBuffer::read(unsigned char* outData, unsigned long inBytesToRead, bool inAllowShortRead) 
{
	unsigned long locBytesToRead =	inBytesToRead;
	
	if (inAllowShortRead) 
    {
        locBytesToRead = std::min(inBytesToRead, numBytesAvail());
	}
    else if (inBytesToRead >  numBytesAvail()) 
    {
        return 0;
    }

	if (locBytesToRead == 0) 
    {
		return 0;
	}
	
	bufASSERT(locBytesToRead <= mBufferSize);
	bufASSERT(locBytesToRead <= inBytesToRead);
	bufASSERT(locBytesToRead <= numBytesAvail());
	

	//When mReadPtr = 0, there are mBufferSize + 1 bytes from the end, but we are only allowed to
	// write mBufferSize. Below where locEndDistabnce is used as a parameter to memcpy
	// but that branch is only taken where locEndDistance is less than the number of bytes to
	// read. Because we have already established above that the number of bytes to read is less
	// than the size of the buffer, this should not be a problem.
	unsigned long locEndDistance = (mBufferSize + 1 - mReadPtr);
	
	//bufASSERT(locEndDistance <= mBufferSize);
	if (locEndDistance >= locBytesToRead) 
    {
		//Within the buffer
		bufASSERT(mReadPtr <= mBufferSize);
		
		memcpy((void*)outData, (const void*)(mBuffer + mReadPtr), locBytesToRead);
	} 
    else 
    {
		bufASSERT(locEndDistance <= mBufferSize);

		//Copy from the end of the raw buffer as much as we can into outdtata
		memcpy((void*)outData, (const void*)(mBuffer + mReadPtr), locEndDistance);

		//Copy from the start of the raw buffer whatever is left
		memcpy((void*)(outData + locEndDistance), (const void*)(mBuffer), locBytesToRead - locEndDistance);
	}
	mReadPtr = (mReadPtr + locBytesToRead) % (mBufferSize + 1);

	return locBytesToRead;
}

unsigned long CircularBuffer::write(const unsigned char* inData, unsigned long inBytesToWrite) 
{
	if (inBytesToWrite >  spaceLeft()) 
    {
		return 0;
	}

	unsigned long locBytesToWrite =	inBytesToWrite;
		
		//		(inBytesToWrite >  spaceLeft())		?	spaceLeft()
		//										:	inBytesToWrite;

	bufASSERT(locBytesToWrite <= spaceLeft());
	bufASSERT(locBytesToWrite <= inBytesToWrite);
	bufASSERT(locBytesToWrite <= mBufferSize);
	bufASSERT(mWritePtr <= mBufferSize);

	unsigned long locEndDistance = (mBufferSize + 1 - mWritePtr);

	//bufASSERT(locEndDistance <= mBufferSize + 1);
	//Where we will be, in relation to the end of the raw buffer if we wrote the buffer out from here.
	//Negative values indicate bytes past the end ofthe buffer.
	//signed long locEndOffset = locEndDistance - locBytesToWrite;


	if (locEndDistance >= locBytesToWrite) 
    {
		//Within the buffer
		memcpy((void*)(mBuffer + mWritePtr), ((const void*)inData), locBytesToWrite);
	} 
    else 
    {
		bufASSERT(locEndDistance <= mBufferSize);

		//Copy from the end of the raw buffer as much as we can into outdtata
		memcpy((void*)(mBuffer + mWritePtr), (const void*)inData, locEndDistance);

		//Copy from the start of the raw buffer whatever is left
		memcpy((void*)(mBuffer), (const void*)(inData + locEndDistance), locBytesToWrite - locEndDistance);
		
		//Advance the write pointer wrapping voer the end.
		
	}
	mWritePtr = (mWritePtr + locBytesToWrite) % (mBufferSize + 1);

	return locBytesToWrite;
}

unsigned long CircularBuffer::spaceLeft() 
{
	bufASSERT(mReadPtr <= mBufferSize);
	bufASSERT(mWritePtr <= mBufferSize);

	//The write pointer is always treated as being equal to or in front of the read pointer.
	//return mBufferSize - numBytesAvail() - 1;
	if (mReadPtr > mWritePtr) 
    {
		//Read pointer is to the right of the Write pointer
		// Since the write pointer is always in front, this means all the data from the read ptr
		// to the end of the buffer, plus everything from the start up to the write pointer is
		// available
		//
		////

		bufASSERT(mReadPtr > mWritePtr);
		return  (mReadPtr - mWritePtr - 1);
	} 
    else 
    {
		bufASSERT(mReadPtr <= mWritePtr);
		return mBufferSize + mReadPtr - mWritePtr ;
	}
}
unsigned long CircularBuffer::numBytesAvail() 
{
	bufASSERT(mReadPtr <= mBufferSize);
	bufASSERT(mWritePtr <= mBufferSize);

	if (mReadPtr > mWritePtr) 
    {
		//Read pointer is to the right of the Write pointer
		// Since the write pointer is always in front, this means all the data from the read ptr
		// to the end of the buffer, plus everything from the start up to the write pointer is
		// available
		//
		////

		bufASSERT(mReadPtr > mWritePtr);

		//Here
		return  (mBufferSize + 1 + mWritePtr - mReadPtr);
	} 
    else 
    {
		//if (mReadPtr <= mWritePtr)
		return mWritePtr - mReadPtr;
	}
}

void CircularBuffer::reset() 
{
	mWritePtr = 0;
	mReadPtr = 0;
}
