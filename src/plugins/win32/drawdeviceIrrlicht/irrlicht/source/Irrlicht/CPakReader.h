// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PAK_READER_H_INCLUDED__
#define __C_PAK_READER_H_INCLUDED__

#include "IReferenceCounted.h"
#include "IReadFile.h"
#include "irrArray.h"
#include "irrString.h"

namespace irr
{
namespace io
{
	struct SPAKFileHeader
	{
		c8 tag[4];
		u32 offset;
		u32 length;
	};


	struct SPakFileEntry
	{
		core::stringc pakFileName;
		core::stringc simpleFileName;
		core::stringc path;
		u32 pos;
		u32 length;

		bool operator < (const SPakFileEntry& other) const
		{
			return simpleFileName < other.simpleFileName;
		}


		bool operator == (const SPakFileEntry& other) const
		{
			return simpleFileName == other.simpleFileName;
		}
	};


	class CPakReader : public IReferenceCounted
	{
	public:

		CPakReader(IReadFile* file, bool ignoreCase, bool ignorePaths);
		virtual ~CPakReader();

		//! opens a file by file name
		virtual IReadFile* openFile(const c8* filename);

		//! opens a file by index
		IReadFile* openFile(s32 index);

		//! returns count of files in archive
		s32 getFileCount();

		//! returns data of file
		const SPakFileEntry* getFileInfo(s32 index) const;

		//! returns fileindex
		s32 findFile(const c8* filename);

	private:
		
		//! scans for a local header, returns false if there is no more local file header.
		bool scanLocalHeader();

		//! splits filename from zip file into useful filenames and paths
		void extractFilename(SPakFileEntry* entry);

		//! deletes the path from a filename
		void deletePathFromFilename(core::stringc& filename);

		IReadFile* File;

		SPAKFileHeader header;

		core::array<SPakFileEntry> FileList;

		bool IgnoreCase;
		bool IgnorePaths;
	};

} // end namespace io
} // end namespace irr

#endif

