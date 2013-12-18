// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_WRITE_FILE_H_INCLUDED__
#define __C_WRITE_FILE_H_INCLUDED__

#include <windows.h>
#include <stdio.h>
#include "IWriteFile.h"
#include "irrString.h"

namespace irr
{

namespace io
{

	/*!
		Class for writing a real file to disk.
	*/
	class CWriteFile : public IWriteFile
	{
	public:

		CWriteFile(const wchar_t* fileName, bool append);
		CWriteFile(const c8* fileName, bool append);

		virtual ~CWriteFile();

		//! Reads an amount of bytes from the file.
		virtual s32 write(const void* buffer, u32 sizeToWrite);

		//! Changes position in file, returns true if successful.
		virtual bool seek(long finalPos, bool relativeMovement = false);

		//! Returns the current position in the file.
		virtual long getPos() const;

		//! Returns name of file.
		virtual const c8* getFileName() const;

		//! returns if file is open
		bool isOpen() const;

	private:

		//! opens the file
		void openFile(bool append);

		core::stringc Filename;
		IStream* File;
		long FileSize;
	};

} // end namespace io
} // end namespace irr

#endif

