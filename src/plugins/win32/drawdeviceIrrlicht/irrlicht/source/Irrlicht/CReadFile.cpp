// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CReadFile.h"

// XXX support for kirikiri
#include "../../../../tp_stub.h"

namespace irr
{
namespace io
{


CReadFile::CReadFile(const c8* fileName)
: File(0), FileSize(0)
{
	#ifdef _DEBUG
	setDebugName("CReadFile");
	#endif

	Filename = fileName;
	openFile();
}



CReadFile::~CReadFile()
{
	if (File) {
		File->Release();
		File = NULL;
	}
}



//! returns how much was read
s32 CReadFile::read(void* buffer, u32 sizeToRead)
{
	if (!isOpen())
		return 0;

	ULONG len;
	if (File->Read(buffer,sizeToRead,&len) == S_OK) {
		return len;
	} 
	return 0;
}



//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool CReadFile::seek(long finalPos, bool relativeMovement)
{
	if (!isOpen())
		return false;

	DWORD dwOrigin = relativeMovement ? STREAM_SEEK_CUR : STREAM_SEEK_SET;
	LARGE_INTEGER move;
	move.QuadPart = finalPos;
	ULARGE_INTEGER newposition;
	return File->Seek(move, dwOrigin, &newposition) == S_OK;
}



//! returns size of file
long CReadFile::getSize() const
{
	return FileSize;
}



//! returns where in the file we are.
long CReadFile::getPos() const
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
void CReadFile::openFile()
{
	if (Filename.size() == 0) // bugfix posted by rt
	{
		File = 0;
		return;
	}

	File = TVPCreateIStream(ttstr(Filename.c_str()), TJS_BS_READ);

	if (File)
	{
		// get FileSize
		STATSTG stat;
		File->Stat(&stat, STATFLAG_NONAME);
		FileSize = (long)stat.cbSize.QuadPart;
	}
}


//! returns name of file
const c8* CReadFile::getFileName() const
{
	return Filename.c_str();
}



IReadFile* createReadFile(const c8* fileName)
{
	CReadFile* file = new CReadFile(fileName);
	if (file->isOpen())
		return file;

	file->drop();
	return 0;
}


} // end namespace io
} // end namespace irr

