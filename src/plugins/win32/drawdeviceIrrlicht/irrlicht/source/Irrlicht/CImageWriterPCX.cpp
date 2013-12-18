// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageWriterPCX.h"

#ifdef _IRR_COMPILE_WITH_PCX_WRITER_

#include "CImageLoaderPCX.h"
#include "IWriteFile.h"
#include "os.h" // for logging
#include "irrString.h"

namespace irr
{
namespace video
{

IImageWriter* createImageWriterPCX()
{
	return new CImageWriterPCX;
}

CImageWriterPCX::CImageWriterPCX()
{
#ifdef _DEBUG
	setDebugName("CImageWriterPCX");
#endif
}

bool CImageWriterPCX::isAWriteableFileExtension(const c8* fileName) const
{
	return strstr(fileName, ".pcx") != 0;
}

bool CImageWriterPCX::writeImage(io::IWriteFile *file, IImage *image,u32 param) const
{
	os::Printer::log("PCX writer not yet implemented. Image not written.", ELL_WARNING);
	return false;
}

} // namespace video
} // namespace irr

#endif

