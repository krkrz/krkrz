// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_Z_BUFFER_H_INCLUDED__
#define __C_Z_BUFFER_H_INCLUDED__

#include "IDepthBuffer.h"

namespace irr
{
namespace video
{

	class CDepthBuffer : public IDepthBuffer
	{
	public:

		//! constructor
		CDepthBuffer(const core::dimension2d<s32>& size);

		//! destructor
		virtual ~CDepthBuffer();

		//! clears the zbuffer
		virtual void clear();

		//! sets the new size of the zbuffer
		virtual void setSize(const core::dimension2d<s32>& size);

		//! returns the size of the zbuffer
		virtual const core::dimension2d<s32>& getSize() const;

		//! locks the zbuffer
		virtual fp24* lock();

		//! unlocks the zbuffer
		virtual void unlock();

	private:

		u8* Buffer;
		core::dimension2d<s32> Size;
		u32 TotalSize;
	};
	
} // end namespace video
} // end namespace irr

#endif

