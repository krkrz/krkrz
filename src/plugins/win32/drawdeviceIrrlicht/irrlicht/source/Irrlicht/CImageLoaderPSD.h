// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IMAGE_LOADER_PSD_H_INCLUDED__
#define __C_IMAGE_LOADER_PSD_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_PSD_LOADER_

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
#	error compiler not supported
#endif

	struct PsdHeader
	{
		c8 signature [4];	// Always equal to 8BPS.
		u16 version;		// Always equal to 1
		c8 reserved [6];	// Must be zero
		u16 channels;		// Number of any channels inc. alphas
		u32 height;		// Rows Height of image in pixel
		u32 width;		// Colums Width of image in pixel
		u16 depth;		// Bits/channel
		u16 mode;		// Color mode of the file (Bitmap/Grayscale..)
	} PACK_STRUCT;


// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT

/*!
	Surface Loader for psd images
*/
class CImageLoaderPSD : public IImageLoader
{
public:

	//! constructor
	CImageLoaderPSD();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName) const;

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(io::IReadFile* file) const;

	//! creates a surface from the file
	virtual IImage* loadImage(io::IReadFile* file) const;

private:

	bool readRawImageData(io::IReadFile* file, const PsdHeader& header, u32* imageData) const;
	bool readRLEImageData(io::IReadFile* file, const PsdHeader& header, u32* imageData) const;
	s16 getShiftFromChannel(c8 channelNr, const PsdHeader& header) const;
};


} // end namespace video
} // end namespace irr

#endif
#endif

