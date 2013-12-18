// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IMAGE_LOADER_TGA_H_INCLUDED__
#define __C_IMAGE_LOADER_TGA_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_TGA_LOADER_

#include "IImageLoader.h"


namespace irr
{
namespace video
{


// byte-align structures
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	define PACK_STRUCT
#endif

	// these structs are also used in the TGA writer
	struct STGAHeader{
		u8 IdLength;
		u8 ColorMapType;
		u8 ImageType;
		u8 FirstEntryIndex[2];
		u16 ColorMapLength;
		u8 ColorMapEntrySize;
		u8 XOrigin[2];
		u8 YOrigin[2];
		u16 ImageWidth;
		u16 ImageHeight;
		u8 PixelDepth;
		u8 ImageDescriptor;
	} PACK_STRUCT;

	struct STGAFooter
	{
		u32 ExtensionOffset;
		u32 DeveloperOffset;
		c8  Signature[18];
	} PACK_STRUCT;

// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT

/*!
	Surface Loader for targa images
*/
class CImageLoaderTGA : public IImageLoader
{
public:

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName) const;

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(io::IReadFile* file) const;

	//! creates a surface from the file
	virtual IImage* loadImage(io::IReadFile* file) const;

private:

	//! loads a compressed tga. Was written and sent in by Jon Pry, thank you very much!
	u8* loadCompressedImage(io::IReadFile *file, const STGAHeader& header) const;
};


} // end namespace video
} // end namespace irr

#endif
#endif

