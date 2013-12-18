// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageLoaderPCX.h"

#ifdef _IRR_COMPILE_WITH_PCX_LOADER_

#include "IReadFile.h"
#include "SColor.h"
#include "CColorConverter.h"
#include "CImage.h"
#include "os.h"
#include "irrString.h"


namespace irr
{
namespace video
{


//! constructor
CImageLoaderPCX::CImageLoaderPCX()
{
	#ifdef _DEBUG
	setDebugName("CImageLoaderPCX");
	#endif
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderPCX::isALoadableFileExtension(const c8* fileName) const
{
	return (strstr(fileName, ".PCX") != 0) || (strstr(fileName, ".pcx") != 0);
}



//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderPCX::isALoadableFileFormat(io::IReadFile* file) const
{
	u8 headerID;
	file->read(&headerID, sizeof(headerID));
	return headerID == 0x0a;
}


//! creates a image from the file
IImage* CImageLoaderPCX::loadImage(io::IReadFile* file) const
{
	SPCXHeader header;
	s32* paletteData = 0;

	file->read(&header, sizeof(header));
	#ifdef __BIG_ENDIAN__
		header.XMin = os::Byteswap::byteswap(header.XMin);
		header.YMin = os::Byteswap::byteswap(header.YMin);
		header.XMax = os::Byteswap::byteswap(header.XMax);
		header.YMax = os::Byteswap::byteswap(header.YMax);
		header.HorizDPI = os::Byteswap::byteswap(header.HorizDPI);
		header.VertDPI = os::Byteswap::byteswap(header.VertDPI);
		header.BytesPerLine = os::Byteswap::byteswap(header.BytesPerLine);
		header.PaletteType = os::Byteswap::byteswap(header.PaletteType);
		header.HScrSize = os::Byteswap::byteswap(header.HScrSize);
		header.VScrSize = os::Byteswap::byteswap(header.VScrSize);
	#endif

	//! return if the header is wrong
	if (header.Manufacturer != 0x0a && header.Encoding != 0x01)
		return 0;

	// return if this isn't a supported type
	if( (header.BitsPerPixel != 8) && (header.BitsPerPixel != 24) )
	{
		os::Printer::log("Unsupported bits per pixel in PCX file.",
			file->getFileName(), irr::ELL_WARNING);
		return 0;
	}

	// read palette
	if( header.BitsPerPixel == 8 )
	{
		// the palette indicator (usually a 0x0c is found infront of the actual palette data)
		// is ignored because some exporters seem to forget to write it. This would result in
		// no image loaded before, now only wrong colors will be set.
		const long pos = file->getPos();
		file->seek( file->getSize()-256*3, false );

		u8 *tempPalette = new u8[768];
		paletteData = new s32[256];
		memset(paletteData, 0xFF, 256*sizeof(s32));
		file->read( tempPalette, 768 );

		for( s32 i=0; i<256; i++ )
		{
			paletteData[i] = (tempPalette[i*3+0] << 16) |
					 (tempPalette[i*3+1] << 8) | 
					 (tempPalette[i*3+2] );
		}

		delete [] tempPalette;

		file->seek( pos );
	}
	else if( header.BitsPerPixel == 4 )
	{
		paletteData = new s32[16];
		memset(paletteData, 0, 16*sizeof(s32));
		for( s32 i=0; i<256; i++ )
		{
			paletteData[i] = (header.Palette[i*3+0] << 16) |
					 (header.Palette[i*3+1] << 8) | 
					 (header.Palette[i*3+2]);
		}
	}

	// read image data
	s32 width, height, imagebytes;
	width = header.XMax - header.XMin + 1;
	height = header.YMax - header.YMin + 1;
	imagebytes = header.BytesPerLine * height * header.Planes * header.BitsPerPixel / 8;
	u8* PCXData = new u8[imagebytes];

	u8 cnt, value;
	for( s32 offset = 0; offset < imagebytes; )
	{
		file->read( &cnt, 1 );
		if( !((cnt & 0xc0) == 0xc0) )
		{
			value = cnt;
			cnt = 1;
		}
		else
		{
			cnt &= 0x3f;
			file->read( &value, 1 );
		}
		memset(PCXData+offset, value, cnt);
		offset += cnt;
	}

	// create image
	video::IImage* image = 0;
	s32 pitch = header.BytesPerLine - width * header.Planes * header.BitsPerPixel / 8;

	if (pitch < 0)
		pitch = -pitch;

	switch(header.BitsPerPixel) // TODO: Other formats
	{
	case 8:
		image = new CImage(ECF_A1R5G5B5, core::dimension2d<s32>(width, height));
		if (image)
			CColorConverter::convert8BitTo16Bit(PCXData, (s16*)image->lock(), width, height, paletteData, pitch);
		break;
	case 24:
		image = new CImage(ECF_R8G8B8, core::dimension2d<s32>(width, height));
		if (image)
			CColorConverter::convert24BitTo24Bit(PCXData, (u8*)image->lock(), width, height, pitch);
		break;
	};
	if (image)
		image->unlock();

	// clean up

	delete [] paletteData;
	delete [] PCXData;

	return image;
}


//! creates a loader which is able to load pcx images
IImageLoader* createImageLoaderPCX()
{
	return new CImageLoaderPCX();
}



} // end namespace video
} // end namespace irr

#endif

