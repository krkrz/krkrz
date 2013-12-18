// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_DIMENSION2D_H_INCLUDED__
#define __IRR_DIMENSION2D_H_INCLUDED__

#include "irrTypes.h"

namespace irr
{
namespace core
{

	//! Specifies a 2 dimensional size.
	template <class T>
	class dimension2d
	{
		public:
			//! Default constructor for empty dimension
			dimension2d() : Width(0), Height(0) {}
			//! Constructor with width and height
			dimension2d(const T& width, const T& height)
				: Width(width), Height(height) {}

			//! Equality operator
			bool operator==(const dimension2d<T>& other) const
			{
				return Width == other.Width && Height == other.Height;
			}

			//! Inequality operator
			bool operator!=(const dimension2d<T>& other) const
			{
				return ! (*this == other);
			}


			//! Set to new values
			dimension2d<T>& set(const T& width, const T& height)
			{
				Width = width;
				Height = height;
				return *this;
			}

			//! Divide width and height by scalar
			dimension2d<T>& operator/=(const T& scale)
			{
				Width /= scale;
				Height /= scale;
				return *this;
			}

			//! Divide width and height by scalar
			dimension2d<T> operator/(const T& scale) const
			{
				return dimension2d<T>(Width/scale, Height/scale);
			}

			//! Multiply width and height by scalar
			dimension2d<T>& operator*=(const T& scale)
			{
				Width *= scale;
				Height *= scale;
				return *this;
			}

			//! Multiply width and height by scalar
			dimension2d<T> operator*(const T& scale) const
			{
				return dimension2d<T>(Width*scale, Height*scale);
			}

			//! Get area
			T getArea() const
			{
				return Width*Height;
			}

			//! Width of the dimension.
			T Width;
			//! Height of the dimension.
			T Height;
	};

	//! Typedef for an f32 dimension.
	typedef dimension2d<f32> dimension2df;
	//! Typedef for an integer dimension.
	typedef dimension2d<s32> dimension2di;

} // end namespace core
} // end namespace irr

#endif

