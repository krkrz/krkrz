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


#include <IOggCallback.h>
#include <INotifyComplete.h>
#include <OggMuxStream.h>

#include <vector>

/** @class OggPageInterleaver

    @brief Use OggPageInterleaver to interleave two or more Ogg logical
	       bitstreams into one physical bitstream.

    To feed data into OggPageInterleaver, you will need to create one
	OggMuxStream for every logical bitstream that you wish to interleave.  Use the
	newStream() method to do this (don't create your own OggMuxStreams, otherwise
	OggPageInterleaver won't know anything about them, which is Bad).  Feed your
	pages to each OggMuxStream using its OggMuxStream::acceptOggPage() method.
	(If you have your data in packets instead of pages, you'll need to use
	OggPaginator to put them into pages first: see its documentation for details).
	Note that you'll probably need to set the conversion parameters in
	OggMuxStream using OggMuxStream::setConversionParams() to get proper Ogg
	pages.  Again, see its documentation for details.
  */


using namespace std;


class OggPageInterleaver : public INotifyArrival
{
public:
	/// Construct a new OggPageInterleaver, which sends its output and completeness notification to inDataWriter and inNotifier, respectively
	OggPageInterleaver(IOggCallback* inDataWriter, INotifyComplete* inNotifier);
	virtual ~OggPageInterleaver(void);

	/// Create a new OggMuxStream.  You need one OggMuxStream per logical bitstream you wish to interleave.
	virtual OggMuxStream* newStream();

	virtual void processData();

	/// Returns the mux progress in 100 nanoseconds
	virtual LOOG_INT64 progressTime();

	/// Returns the number of bytes written.
	virtual LOOG_INT64 bytesWritten();

	//INotifyArrival Implementation
	virtual void notifyArrival();

protected:
	/// Writes the lowest stream out
	virtual void writeLowest();

	/// Returns if there is enough data to do some interleaving
	virtual bool isProcessable();

	/// Returns true if all the streams are at the end.
	virtual bool isAllEOS();

	/// Returns true if all the streams are empty.
	virtual bool isAllEmpty();

	vector<OggMuxStream*> mInputStreams;
	IOggCallback* mFileWriter;		//TODO::: Shuoldn't be called filewriter.
	INotifyComplete* mNotifier;

	LOOG_INT64 mBytesWritten;
	LOOG_INT64 mProgressTime;

private:
	OggPageInterleaver& operator=(const OggPageInterleaver& other);  /* Don't assign me */
	OggPageInterleaver(const OggPageInterleaver& other); /* Don't copy me */
};
