// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CFileList.h"
#include "IrrCompileConfig.h"
#include "irrArray.h"
#include <stdlib.h>

namespace irr
{
namespace io
{

#if (defined(_IRR_POSIX_API_) || defined(_IRR_OSX_PLATFORM_))
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _IRR_WINDOWS_API_
#include <io.h>
#include <direct.h>
#endif


CFileList::CFileList()
{
	#ifdef _DEBUG
	setDebugName("CFileList");
	#endif

	// --------------------------------------------
	// Windows version
	#ifdef _IRR_WINDOWS_API_

	char tmp[_MAX_PATH];
	_getcwd(tmp, _MAX_PATH);
	Path = tmp;

	struct _finddata_t c_file;
	long hFile;
	FileEntry entry;

	if( (hFile = _findfirst( "*", &c_file )) != -1L )
	{
		do
		{
			entry.Name = c_file.name;
			entry.Size = c_file.size;
			entry.isDirectory = (_A_SUBDIR & c_file.attrib) != 0;
			Files.push_back(entry);
		}
		while( _findnext( hFile, &c_file ) == 0 );

		_findclose( hFile );
	}

	//TODO add drives
	//entry.Name = "E:\\";
	//entry.isDirectory = true;
	//Files.push_back(entry);
	#endif

	// --------------------------------------------
	// Linux version
	#if (defined(_IRR_POSIX_API_) || defined(_IRR_OSX_PLATFORM_))

	FileEntry entry;

	// Add default parent - even when at /, this is available
	entry.Name = "..";
	entry.Size = 0;
	entry.isDirectory = true;
	Files.push_back(entry);

	// getting the CWD is rather complex as we do not know the size
	// so try it until the call was successful
	// Note that neither the first nor the second parameter may be 0 according to POSIX
	u32 pathSize=256;
	char *tmpPath = new char[pathSize];
	while ((pathSize < (1<<16)) && !(getcwd(tmpPath,pathSize)))
	{
		delete [] tmpPath;
		pathSize *= 2;
		tmpPath = new char[pathSize];
	}
	if (!tmpPath)
		return;
	Path = tmpPath;
	delete [] tmpPath;
	// We use the POSIX compliant methods instead of scandir
	DIR* dirHandle=opendir(Path.c_str());
	if (!dirHandle)
		return;

	struct dirent *dirEntry;
	while ((dirEntry=readdir(dirHandle)))
	{
		if((strcmp(dirEntry->d_name, ".")==0) ||
		   (strcmp(dirEntry->d_name, "..")==0))
			continue;
		entry.Name = dirEntry->d_name;
		entry.Size = 0;
		entry.isDirectory = false;
		struct stat buf;
		if (stat(dirEntry->d_name, &buf)==0)
		{
			entry.Size = buf.st_size;
			entry.isDirectory = S_ISDIR(buf.st_mode);
		}
		#if !defined(_IRR_SOLARIS_PLATFORM_) && !defined(__CYGWIN__)
		// only available on some systems
		else
		{
			entry.isDirectory = dirEntry->d_type == DT_DIR;
		}
		#endif
		Files.push_back(entry);
	}
	closedir(dirHandle);
	#endif
	// sort the list on all platforms
	Files.sort();
}


u32 CFileList::getFileCount() const
{
	return Files.size();
}


const c8* CFileList::getFileName(u32 index) const
{
	if (index < Files.size())
		return Files[index].Name.c_str();
	else
		return 0;
}


//! Gets the full name of a file in the list, path included, based on an index.
const c8* CFileList::getFullFileName(u32 index)
{
	if (index >= Files.size())
		return 0;

	if (Files[index].FullName.size() < Files[index].Name.size())
	{
		// create full name
		Files[index].FullName = Path;

		if (Path.size() > 3)
			Files[index].FullName.append('/');

		Files[index].FullName.append(Files[index].Name);
	}

	return Files[index].FullName.c_str();
}


bool CFileList::isDirectory(u32 index) const
{
	bool ret;
	if (index >= Files.size())
		ret = false;
	else
		ret = Files[index].isDirectory;

	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return ret;
}

} // end namespace irr
} // end namespace io

