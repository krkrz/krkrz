// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

#include "SoftwareDriver2_compile_config.h"
#include "SoftwareDriver2_helper.h"
#include "CSoftwareTexture2.h"
#include "os.h"

namespace irr
{
namespace video  
{

//! constructor
CSoftwareTexture2::CSoftwareTexture2(IImage* image, const char* name, bool generateMipLevels, bool isRenderTarget)
: ITexture(name), MipMapLOD(0), HasMipMaps(generateMipLevels), IsRenderTarget(isRenderTarget)
{
	#ifndef SOFTWARE_DRIVER_2_MIPMAPPING
		HasMipMaps = false;
	#endif

	memset32 ( MipMap, 0, sizeof ( MipMap ) );

	if (image)
	{
		
		core::dimension2d<s32> optSize;
		core::dimension2d<s32> origSize = image->getDimension();
		OrigSize = origSize;

		optSize.Width = getTextureSizeFromSurfaceSize(origSize.Width);
		optSize.Height = getTextureSizeFromSurfaceSize(origSize.Height);
		
		if ( origSize == optSize )
		{
			MipMap[0] = new CImage(ECF_SOFTWARE2, image);
		}
		else
		{
			MipMap[0] = new CImage(ECF_SOFTWARE2, optSize);

			// temporary CImage needed
			CImage * temp = new CImage ( ECF_SOFTWARE2, image );
			temp->copyToScaling(MipMap[0]);
			temp->drop ();
		}
	}

	regenerateMipMapLevels ();
	setCurrentMipMapLOD ( 0 );
}


//! destructor
CSoftwareTexture2::~CSoftwareTexture2()
{
	for ( s32 i = 0; i!= SOFTWARE_DRIVER_2_MIPMAPPING_MAX; ++i )
	{
		if ( MipMap[i] )
			MipMap[i]->drop ();
	}
}


//! returns the size of a texture which would be the optimize size for rendering it
inline s32 CSoftwareTexture2::getTextureSizeFromSurfaceSize(s32 size) const
{
	s32 ts = 0x01;
	while(ts < size)
		ts <<= 1;

/*
	if (ts > size && ts > 256 )
		ts >>= 1;
*/
	return ts;
}


//! Regenerates the mip map levels of the texture. Useful after locking and 
//! modifying the texture
void CSoftwareTexture2::regenerateMipMapLevels()
{
	if ( !HasMipMaps )
		return;

	s32 i;

	// release
	for ( i = 1; i!= SOFTWARE_DRIVER_2_MIPMAPPING_MAX; ++i )
	{
		if ( MipMap[i] )
			MipMap[i]->drop ();
	}

	core::dimension2d<s32> newSize;
	core::dimension2d<s32> currentSize;

	i = 1;
	CImage * c = MipMap[0];
	while ( i < SOFTWARE_DRIVER_2_MIPMAPPING_MAX )
	{
		currentSize = c->getDimension();
		newSize.Width = core::s32_max ( 1, currentSize.Width >> 1 );
		newSize.Height = core::s32_max ( 1, currentSize.Height >> 1 );

		MipMap[i] = new CImage(ECF_SOFTWARE2, newSize);
		MipMap[0]->copyToScalingBoxFilter ( MipMap[i], 0 );
		c = MipMap[i];
		i += 1;
	}
}


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

