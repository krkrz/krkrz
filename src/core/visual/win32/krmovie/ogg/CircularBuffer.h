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
#include <IFIFOBuffer.h>


//Empty Buffer
//==============
//
//		<--------------- Buffer Size -------------------->
//      
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//      R
//		W
//
//
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//									R
//									W
//
//When R = W Buffer is empty
//
//		when R = W:	available bytes = 0
//		when R = W: space left = buffer size
////

//Full Buffer
//===========
//
//
//
//		<--------------- Buffer Size -------------------->
//      
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//                               R
//								W
//
//
//		<--------------- Buffer Size -------------------->
//      
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//      R
//								                          W
//
//Buffer is full when R = (W + 1) MOD (bufferSize + 1)
//
//		when	R = (W + 1) MOD (bufferSize + 1):		available bytes = buffer size
//		when	R = (W + 1) MOD (bufferSize + 1):		space left  = 0
//
//
//	
//
//////

//Partial Buffers
//===============
//
//Case 1
//======
//
//
//		<--------------- Buffer Size -------------------->
//      
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//    								           R
//											    W
//
//
//		when W > R:		available bytes								=	W - R
//		when W > R:		space left = buffer size - available bytes	=	buffer size + R - W
//
//
//Case 2
//======
//
//
//		<--------------- Buffer Size -------------------->
//				  1			2		  3			4
//		0123456789 123456789 123456789 123456789 123456789*
//      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//                                                      R
//		W
//
//		when R > W:			available bytes									=	buffer size + 1 - R + W
//		when R > W:			space left = buffer size - available bytes		=	R - W - 1
//		
//
//
//
//

class CircularBuffer:	public IFIFOBuffer
{
public:
	/// Constructor taking in the size in bytes of the internal buffer.
	CircularBuffer(unsigned long inBufferSize);
	virtual ~CircularBuffer(void);

	/// Read bytes from the internal buffer. Returns how many actually read.
	virtual unsigned long read(unsigned char* outData, unsigned long inBytesToRead, bool inAllowShortRead = false);

	/// Write bytes into the internal buffer. Returns how many written.
	virtual unsigned long write(const unsigned char* inData, unsigned long inBytesToWrite);

	/// Returns how many bytes are available in the buffer.
	virtual unsigned long numBytesAvail();

	/// Returns how much space is left in the buffer.
	virtual unsigned long spaceLeft();

	/// Resets the buffer.
	virtual void reset();

protected:
	unsigned long mBufferSize;
	unsigned long mReadPtr;
	unsigned long mWritePtr;

	void bufASSERT(bool inBool) {	if (!inBool) throw 0; };
	unsigned char* mBuffer;

private:
	CircularBuffer& operator=(const CircularBuffer& other);  /* Don't assign me */
	CircularBuffer(const CircularBuffer& other); /* Don't copy me */
};
