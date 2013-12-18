// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CWriteFile.h"

// XXX support for kirikiri
#include "../../../../tp_stub.h"

namespace irr
{
namespace io
{


CWriteFile::CWriteFile(const c8* fileName, bool append)
: FileSize(0)
{
	#ifdef _DEBUG
	setDebugName("CWriteFile");
	#endif

	Filename = fileName;
	openFile(append);
}



CWriteFile::~CWriteFile()
{
	if (File) {
		File->Release();
		File = NULL;
	}
}



//! returns if file is open
inline bool CWriteFile::isOpen() const
{
	return File != 0;
}



//! returns how much was read
s32 CWriteFile::write(const void* buffer, u32 sizeToWrite)
{
	if (!isOpen())
		return 0;

	DWORD len;
	if (File->Write(buffer,sizeToWrite,&len) == S_OK) {
		return len;
	}
	return 0;
}



//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool CWriteFile::seek(long finalPos, bool relativeMovement)
{
	if (!isOpen())
		return false;

	DWORD dwOrigin = relativeMovement ? STREAM_SEEK_CUR : STREAM_SEEK_SET;
	LARGE_INTEGER move;
	move.QuadPart = finalPos;
	ULARGE_INTEGER newposition;
	return File->Seek(move, dwOrigin, &newposition) == S_OK;
}



//! returns where in the file we are.
long CWriteFile::getPos() const
{
	if (File) {
		LARGE_INTEGER move = {0};
		ULARGE_INTEGER newposition;
		if (File->Seek(move, STREAM_SEEK_CUR, &newposition) == S_OK) {
			return (long)newposition.QuadPart;
		}
	}
	return -1;
}



//! opens the file
void CWriteFile::openFile(bool append)
{
	if (Filename.size() == 0)
	{
		File = 0;
		return; 
	}

	File = TVPCreateIStream(ttstr(Filename.c_str()), append ? TJS_BS_APPEND : TJS_BS_WRITE);

	if (File)
	{
		// get FileSize
		STATSTG stat;
		File->Stat(&stat, STATFLAG_NONAME);
		FileSize = (long)stat.cbSize.QuadPart;
	}
}



//! returns name of file
const c8* CWriteFile::getFileName() const
{
	return Filename.c_str();
}



IWriteFile* createWriteFile(const c8* fileName, bool append)
{
	CWriteFile* file = new CWriteFile(fileName, append);
	if (file->isOpen())
		return file;

	file->drop();
	return 0;
}


} // end namespace io
} // end namespace irr

