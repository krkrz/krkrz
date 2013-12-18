// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#include "SoftwareDriver2_compile_config.h"
#include "CDepthBuffer.h"

#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

namespace irr
{
namespace video
{


//! constructor
CDepthBuffer::CDepthBuffer(const core::dimension2d<s32>& size)
: Buffer(0), Size(0,0)
{
	#ifdef _DEBUG
	setDebugName("CDepthBuffer");
	#endif

	setSize(size);
}



//! destructor
CDepthBuffer::~CDepthBuffer()
{
	if (Buffer)
		delete [] Buffer;
}



//! clears the zbuffer
void CDepthBuffer::clear()
{

#ifdef SOFTWARE_DRIVER_2_USE_WBUFFER
	f32 zMax = 0.f;
#else
	f32 zMax = 1.f;
#endif

	u32 zMaxValue;
	zMaxValue = *(u32*) &zMax;

	memset32 ( Buffer, zMaxValue, TotalSize );
}



//! sets the new size of the zbuffer
void CDepthBuffer::setSize(const core::dimension2d<s32>& size)
{
	if (size == Size)
		return;

	Size = size;

	if (Buffer)
		delete [] Buffer;

	TotalSize = size.Width * size.Height * sizeof ( fp24 );
	Buffer = new u8[TotalSize];
}



//! returns the size of the zbuffer
const core::dimension2d<s32>& CDepthBuffer::getSize() const
{
	return Size;
}



//! locks the zbuffer
fp24* CDepthBuffer::lock()
{
	return (fp24*) Buffer;
}



//! unlocks the zbuffer
void CDepthBuffer::unlock()
{
}

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

namespace irr
{
namespace video
{

//! creates a ZBuffer
IDepthBuffer* createDepthBuffer(const core::dimension2d<s32>& size)
{
	#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
	return new CDepthBuffer(size);
	#else
	return 0;
	#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
}
	
} // end namespace video
} // end namespace irr



