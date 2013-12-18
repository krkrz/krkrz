// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and/or irrXML.h

#include "irrXML.h"
#include "irrString.h"
#include "irrArray.h"
#include "fast_atof.h"
#include "CXMLReaderImpl.h"

#include <windows.h>
#include "../../../../tp_stub.h"

namespace irr
{
namespace io
{

//! Implementation of the file read callback for ordinary files
class CFileReadCallBack : public IFileReadCallBack
{
public:

	//! construct from filename
	CFileReadCallBack(const char* filename)
		: File(0), Size(0), Close(true)
	{
		// open file
		File = TVPCreateIStream(ttstr(filename), TJS_BS_READ);

		if (File)
			getFileSize();
	}

	//! construct from FILE pointer
	CFileReadCallBack(IStream* file)
		: File(file), Size(0), Close(false)
	{
		if (File)
			getFileSize();
	}

	//! destructor
	virtual ~CFileReadCallBack()
	{
		if (Close && File) {
			File->Release();
			File = NULL;
		}
	}

	//! Reads an amount of bytes from the file.
	virtual int read(void* buffer, int sizeToRead)
	{
		if (!File)
			return 0;

		ULONG len;
		if (File->Read(buffer,sizeToRead,&len) == S_OK) {
			return len;
		} 
		return 9;
	}

	//! Returns size of file in bytes
	virtual long getSize() const
	{
		return Size;
	}

private:

	//! retrieves the file size of the open file
	void getFileSize()
	{
		STATSTG stat;
		File->Stat(&stat, STATFLAG_NONAME);
		Size = (long)stat.cbSize.QuadPart;
	}

	IStream* File;
	long Size;
	bool Close;

}; // end class CFileReadCallBack



// FACTORY FUNCTIONS:


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IRRLICHT_API IrrXMLReader* IRRCALLCONV createIrrXMLReader(const char* filename)
{
	return new CXMLReaderImpl<char, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IRRLICHT_API IrrXMLReader* IRRCALLCONV createIrrXMLReader(IStream* file)
{
	return new CXMLReaderImpl<char, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IRRLICHT_API IrrXMLReader* IRRCALLCONV createIrrXMLReader(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char, IXMLBase>(callback, false); 
}


//! Creates an instance of an UTF-16 xml parser. 
IRRLICHT_API IrrXMLReaderUTF16* IRRCALLCONV createIrrXMLReaderUTF16(const char* filename)
{
	return new CXMLReaderImpl<char16, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UTF-16 xml parser. 
IRRLICHT_API IrrXMLReaderUTF16* IRRCALLCONV createIrrXMLReaderUTF16(IStream* file)
{
	return new CXMLReaderImpl<char16, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UTF-16 xml parser. 
IRRLICHT_API IrrXMLReaderUTF16* IRRCALLCONV createIrrXMLReaderUTF16(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char16, IXMLBase>(callback, false); 
}


//! Creates an instance of an UTF-32 xml parser. 
IRRLICHT_API IrrXMLReaderUTF32* IRRCALLCONV createIrrXMLReaderUTF32(const char* filename)
{
	return new CXMLReaderImpl<char32, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UTF-32 xml parser. 
IRRLICHT_API IrrXMLReaderUTF32* IRRCALLCONV createIrrXMLReaderUTF32(IStream* file)
{
	return new CXMLReaderImpl<char32, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UTF-32 xml parser. 
IRRLICHT_API IrrXMLReaderUTF32* IRRCALLCONV createIrrXMLReaderUTF32(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char32, IXMLBase>(callback, false); 
}


} // end namespace io
} // end namespace irr
