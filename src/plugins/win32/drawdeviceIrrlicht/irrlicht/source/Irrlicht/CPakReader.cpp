// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// Code contributed by skreamz

#include "CPakReader.h"
#include "os.h"

#include "IrrCompileConfig.h"

namespace irr
{
namespace io
{


CPakReader::CPakReader(IReadFile* file, bool ignoreCase, bool ignorePaths)
: File(file), IgnoreCase(ignoreCase), IgnorePaths(ignorePaths)
{
	#ifdef _DEBUG
	setDebugName("CPakReader");
	#endif

	if (File)
	{
		File->grab();

		// scan local headers
		scanLocalHeader();

		// prepare file index for binary search
		FileList.sort();
	}
}

CPakReader::~CPakReader()
{
	if (File)
		File->drop();
}



//! splits filename from zip file into useful filenames and paths
void CPakReader::extractFilename(SPakFileEntry* entry)
{
	s32 lorfn = 56; // length of real file name

	if (!lorfn)
		return;

	if (IgnoreCase)
		entry->pakFileName.make_lower();

	const c8* p = entry->pakFileName.c_str() + lorfn;
	
	// suche ein slash oder den anfang.

	while (*p!='/' && p!=entry->pakFileName.c_str())
	{
		--p;
		--lorfn;
	}

	bool thereIsAPath = p != entry->pakFileName.c_str();

	if (thereIsAPath)
	{
		// there is a path
		++p;
		++lorfn;
	}

	entry->simpleFileName = p;
	entry->path = "";

	// pfad auch kopieren
	if (thereIsAPath)
	{
		lorfn = (s32)(p - entry->pakFileName.c_str());
		entry->path.append(entry->pakFileName, lorfn);
	}

	if (!IgnorePaths)
		entry->simpleFileName = entry->pakFileName; // thanks to Pr3t3nd3r for this fix
}



//! scans for a local header, returns false if there is no more local file header.
bool CPakReader::scanLocalHeader()
{
	c8 tmp[1024];

	SPakFileEntry entry;
	entry.pos = 0;

	memset(&header, 0, sizeof(SPAKFileHeader));
	File->read(&header, sizeof(SPAKFileHeader));


	if (header.tag[0] != 'P' && header.tag[1] != 'A')
		return false; // local file headers end here.

	File->seek(header.offset);

	const int count = header.length / ((sizeof(u32) * 2) + 56);

	for(int i = 0; i < count; i++)
	{
		// read filename
		entry.pakFileName.reserve(56+2);
		File->read(tmp, 56);
		tmp[56] = 0x0;
		entry.pakFileName = tmp;

		#ifdef _DEBUG
		os::Printer::log(entry.pakFileName.c_str());
		#endif

		extractFilename(&entry);

		File->read(&entry.pos, sizeof(u32));
		File->read(&entry.length, sizeof(u32));
		FileList.push_back(entry);
	}

	return true;
}



//! opens a file by file name
IReadFile* CPakReader::openFile(const c8* filename)
{
	s32 index = findFile(filename);

	if (index != -1)
		return openFile(index);

	return 0;
}



//! opens a file by index
IReadFile* CPakReader::openFile(s32 index)
{
	File->seek(FileList[index].pos);
	return createLimitReadFile(FileList[index].simpleFileName.c_str(), File, FileList[index].length);
}



//! returns count of files in archive
s32 CPakReader::getFileCount()
{
	return FileList.size();
}



//! returns data of file
const SPakFileEntry* CPakReader::getFileInfo(s32 index) const
{
	return &FileList[index];
}



//! deletes the path from a filename
void CPakReader::deletePathFromFilename(core::stringc& filename)
{
	// delete path from filename
	const c8* p = filename.c_str() + filename.size();

	// search for path separator or beginning

	while (*p!='/' && *p!='\\' && p!=filename.c_str())
		--p;

	if (p != filename.c_str())
	{
		++p;
		filename = p;
	}
}



//! returns fileindex
s32 CPakReader::findFile(const c8* simpleFilename)
{
	SPakFileEntry entry;
	entry.simpleFileName = simpleFilename;

	if (IgnoreCase)
		entry.simpleFileName.make_lower();

	if (IgnorePaths)
		deletePathFromFilename(entry.simpleFileName);

	s32 res = FileList.binary_search(entry);

	#ifdef _DEBUG
	if (res == -1)
	{
		for (u32 i=0; i<FileList.size(); ++i)
			if (FileList[i].simpleFileName == entry.simpleFileName)
			{
				os::Printer::log("File in archive but not found.", entry.simpleFileName.c_str(), ELL_ERROR);
				break;
			}
	}
	#endif

	return res;
}



} // end namespace io
} // end namespace irr

