// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _C_IMAGE_WRITER_PPM_H_INCLUDED__
#define _C_IMAGE_WRITER_PPM_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_PPM_WRITER_

#include "IImageWriter.h"

namespace irr
{
namespace video
{

class CImageWriterPPM : public IImageWriter
{
public:
	//! constructor
	CImageWriterPPM();

	//! return true if this writer can write a file with the given extension
	virtual bool isAWriteableFileExtension(const c8* fileName) const;

	//! write image to file
	virtual bool writeImage(io::IWriteFile *file, IImage *image, u32 param) const;
};

} // namespace video
} // namespace irr

#endif // _C_IMAGE_WRITER_PPM_H_INCLUDED__
#endif

