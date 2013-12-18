// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CZipReader.h"
#include "CFileList.h"
#include "CReadFile.h"
#include "os.h"

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_ZLIB_
    #ifndef _IRR_USE_NON_SYSTEM_ZLIB_
    #include <zlib.h> // use system lib
    #else // _IRR_USE_NON_SYSTEM_ZLIB_
    #include "zlib/zlib.h" 
    #endif // _IRR_USE_NON_SYSTEM_ZLIB_
#endif // _IRR_COMPILE_WITH_ZLIB_

namespace irr
{
namespace io
{


CZipReader::CZipReader(IReadFile* file, bool ignoreCase, bool ignorePaths)
: File(file), IgnoreCase(ignoreCase), IgnorePaths(ignorePaths)
{
	#ifdef _DEBUG
	setDebugName("CZipReader");
	#endif

	if (File)
	{
		File->grab();

		// scan local headers
		while (scanLocalHeader());

		// prepare file index for binary search
		FileList.sort();
	}
}

CZipReader::~CZipReader()
{
	if (File)
		File->drop();
}



//! splits filename from zip file into useful filenames and paths
void CZipReader::extractFilename(SZipFileEntry* entry)
{
	s32 lorfn = entry->header.FilenameLength; // length of real file name

	if (!lorfn)
		return;

	if (IgnoreCase)
		entry->zipFileName.make_lower();

	const c8* p = entry->zipFileName.c_str() + lorfn;
	
	// suche ein slash oder den anfang.

	while (*p!='/' && p!=entry->zipFileName.c_str())
	{
		--p;
		--lorfn;
	}

	bool thereIsAPath = p != entry->zipFileName.c_str();

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
		lorfn = (s32)(p - entry->zipFileName.c_str());
		
		entry->path = entry->zipFileName.subString ( 0, lorfn );

		//entry->path.append(entry->zipFileName, lorfn);
		//entry->path.append ( "" );
	}

	if (!IgnorePaths)
		entry->simpleFileName = entry->zipFileName; // thanks to Pr3t3nd3r for this fix
}



//! scans for a local header, returns false if there is no more local file header.
bool CZipReader::scanLocalHeader()
{
	c8 tmp[1024];

	SZipFileEntry entry;
	entry.fileDataPosition = 0;
	memset(&entry.header, 0, sizeof(SZIPFileHeader));

	File->read(&entry.header, sizeof(SZIPFileHeader));

#ifdef __BIG_ENDIAN__
		entry.header.Sig = os::Byteswap::byteswap(entry.header.Sig);
		entry.header.VersionToExtract = os::Byteswap::byteswap(entry.header.VersionToExtract);
		entry.header.GeneralBitFlag = os::Byteswap::byteswap(entry.header.GeneralBitFlag);
		entry.header.CompressionMethod = os::Byteswap::byteswap(entry.header.CompressionMethod);
		entry.header.LastModFileTime = os::Byteswap::byteswap(entry.header.LastModFileTime);
		entry.header.LastModFileDate = os::Byteswap::byteswap(entry.header.LastModFileDate);
		entry.header.DataDescriptor.CRC32 = os::Byteswap::byteswap(entry.header.DataDescriptor.CRC32);
		entry.header.DataDescriptor.CompressedSize = os::Byteswap::byteswap(entry.header.DataDescriptor.CompressedSize);
		entry.header.DataDescriptor.UncompressedSize = os::Byteswap::byteswap(entry.header.DataDescriptor.UncompressedSize);
		entry.header.FilenameLength = os::Byteswap::byteswap(entry.header.FilenameLength);
		entry.header.ExtraFieldLength = os::Byteswap::byteswap(entry.header.ExtraFieldLength);
#endif

	if (entry.header.Sig != 0x04034b50)
		return false; // local file headers end here.

	// read filename
	entry.zipFileName.reserve(entry.header.FilenameLength+2);
	File->read(tmp, entry.header.FilenameLength);
	tmp[entry.header.FilenameLength] = 0x0;
	entry.zipFileName = tmp;

	extractFilename(&entry);

	// move forward length of extra field.

	if (entry.header.ExtraFieldLength)
		File->seek(entry.header.ExtraFieldLength, true);

	// if bit 3 was set, read DataDescriptor, following after the compressed data
	if (entry.header.GeneralBitFlag & ZIP_INFO_IN_DATA_DESCRITOR)
	{
		// read data descriptor
		File->read(&entry.header.DataDescriptor, sizeof(entry.header.DataDescriptor));
#ifdef __BIG_ENDIAN__
		entry.header.DataDescriptor.CRC32 = os::Byteswap::byteswap(entry.header.DataDescriptor.CRC32);
		entry.header.DataDescriptor.CompressedSize = os::Byteswap::byteswap(entry.header.DataDescriptor.CompressedSize);
		entry.header.DataDescriptor.UncompressedSize = os::Byteswap::byteswap(entry.header.DataDescriptor.UncompressedSize);
#endif
	}

	// store position in file
	entry.fileDataPosition = File->getPos();

	// move forward length of data
	File->seek(entry.header.DataDescriptor.CompressedSize, true);

	#ifdef _DEBUG
	//os::Debuginfo::print("added file from archive", entry.simpleFileName.c_str());
	#endif

	FileList.push_back(entry);

	return true;
}



//! opens a file by file name
IReadFile* CZipReader::openFile(const c8* filename)
{
	s32 index = findFile(filename);

	if (index != -1)
		return openFile(index);

	return 0;
}



//! opens a file by index
IReadFile* CZipReader::openFile(s32 index)
{
	//0 - The file is stored (no compression)
	//1 - The file is Shrunk
	//2 - The file is Reduced with compression factor 1
	//3 - The file is Reduced with compression factor 2
	//4 - The file is Reduced with compression factor 3
	//5 - The file is Reduced with compression factor 4
	//6 - The file is Imploded
	//7 - Reserved for Tokenizing compression algorithm
	//8 - The file is Deflated
	//9 - Reserved for enhanced Deflating
	//10 - PKWARE Date Compression Library Imploding

	switch(FileList[index].header.CompressionMethod)
	{
	case 0: // no compression
		{
			File->seek(FileList[index].fileDataPosition);
			return createLimitReadFile(FileList[index].simpleFileName.c_str(), File, FileList[index].header.DataDescriptor.UncompressedSize);
		}
	case 8:
		{
  			#ifdef _IRR_COMPILE_WITH_ZLIB_
			
			const u32 uncompressedSize = FileList[index].header.DataDescriptor.UncompressedSize;			
			const u32 compressedSize = FileList[index].header.DataDescriptor.CompressedSize;

			void* pBuf = new c8[ uncompressedSize ];
			if (!pBuf)
			{
				os::Printer::log("Not enough memory for decompressing", FileList[index].simpleFileName.c_str(), ELL_ERROR);
				return 0;
			}

			c8 *pcData = new c8[ compressedSize ];
			if (!pcData)
			{
				os::Printer::log("Not enough memory for decompressing", FileList[index].simpleFileName.c_str(), ELL_ERROR);
				return 0;
			}

			//memset(pcData, 0, compressedSize );
			File->seek(FileList[index].fileDataPosition);
			File->read(pcData, compressedSize );
			
			// Setup the inflate stream.
			z_stream stream;
			s32 err;

			stream.next_in = (Bytef*)pcData;
			stream.avail_in = (uInt)compressedSize;
			stream.next_out = (Bytef*)pBuf;
			stream.avail_out = uncompressedSize;
			stream.zalloc = (alloc_func)0;
			stream.zfree = (free_func)0;

			// Perform inflation. wbits < 0 indicates no zlib header inside the data.
			err = inflateInit2(&stream, -MAX_WBITS);
			if (err == Z_OK)
			{
				err = inflate(&stream, Z_FINISH);
				inflateEnd(&stream);
				if (err == Z_STREAM_END)
					err = Z_OK;
				err = Z_OK;
				inflateEnd(&stream);
			}


			delete[] pcData;
			
			if (err != Z_OK)
			{
				os::Printer::log("Error decompressing", FileList[index].simpleFileName.c_str(), ELL_ERROR);
				delete [] (c8*)pBuf;
				return 0;
			}
			else
				return io::createMemoryReadFile(pBuf, uncompressedSize, FileList[index].zipFileName.c_str(), true);
			
			#else
			return 0; // zlib not compiled, we cannot decompress the data.
			#endif
		}
	default:
		os::Printer::log("file has unsupported compression method.", FileList[index].simpleFileName.c_str(), ELL_ERROR);
		return 0;
	};
}



//! returns count of files in archive
s32 CZipReader::getFileCount()
{
	return FileList.size();
}



//! returns data of file
const SZipFileEntry* CZipReader::getFileInfo(s32 index) const
{
	return &FileList[index];
}



//! deletes the path from a filename
void CZipReader::deletePathFromFilename(core::stringc& filename)
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
s32 CZipReader::findFile(const c8* simpleFilename)
{
	SZipFileEntry entry;
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


// -----------------------------------------------------------------------------
#if 1

class CUnzipReadFile : public CReadFile
{
	public:
		CUnzipReadFile ( const core::stringc &realName, const c8 * hashName )
			: CReadFile( realName.c_str ())
		{
			CallFileName = hashName;
		}
		virtual ~CUnzipReadFile () {}

		virtual const c8* getFileName() const
		{
			return CallFileName.c_str ();
		}

		core::stringc CallFileName;
};

CUnZipReader::CUnZipReader( IFileSystem * parent, const c8* basename, bool ignoreCase, bool ignorePaths)
:CZipReader ( 0, ignoreCase, ignorePaths ), Parent ( parent )
{
	Base = basename;
	if (	Base [ Base.size() - 1 ] != '\\' ||
			Base [ Base.size() - 1 ] != '/'
		)
	{
		Base += "/";
	}

}

void CUnZipReader::buildDirectory ( )
{
}

//! opens a file by file name
IReadFile* CUnZipReader::openFile(const c8* filename)
{
	core::stringc fname;
	fname = Base;
	fname += filename;


	CUnzipReadFile* file = new CUnzipReadFile( fname, filename);
	if (file->isOpen())
		return file;

	file->drop();
	return 0;

}

//! returns fileindex
s32 CUnZipReader::findFile(const c8* filename)
{
	IReadFile *file = openFile ( filename );
	if ( 0 == file )
		return -1;
	file->drop ();
	return 1;
}

#else

CUnZipReader::CUnZipReader( IFileSystem * parent, const c8* basename, bool ignoreCase, bool ignorePaths)
	: CZipReader( 0, ignoreCase, ignorePaths ), Parent ( parent )
{
	strcpy ( Buf, Parent->getWorkingDirectory () );

	Parent->changeWorkingDirectoryTo ( basename );
	buildDirectory ( );
	Parent->changeWorkingDirectoryTo ( Buf );

	FileList.sort();
}

void CUnZipReader::buildDirectory ( )
{
	IFileList * list = new CFileList();

	SZipFileEntry entry;

	const u32 size = list->getFileCount();
	for (u32 i = 0; i!= size; ++i)
	{
		if ( false == list->isDirectory( i ) )
		{
			entry.zipFileName = list->getFullFileName ( i );
			entry.header.FilenameLength = entry.zipFileName.size ();
			extractFilename(&entry);
			FileList.push_back(entry);
		}
		else
		{
			const c8 * rel = list->getFileName ( i );

			if (strcmp( rel, "." ) && strcmp( rel, ".." ))
			{
				Parent->changeWorkingDirectoryTo ( rel );
				buildDirectory ();
				Parent->changeWorkingDirectoryTo ( ".." );
			}
		}
	}

	list->drop ();
}

//! opens a file by file name
IReadFile* CUnZipReader::openFile(const c8* filename)
{
	s32 index = -1;

	if ( IgnorePaths )
	{
		index = findFile(filename);
	}
	else
	if ( FileList.size () )
	{
		const core::stringc search = FileList[0].path + filename;
		index = findFile( search.c_str() );
	}

	if (index == -1)
		return 0;

	return createReadFile(FileList[index].zipFileName.c_str() );
}
#endif


} // end namespace io
} // end namespace irr

