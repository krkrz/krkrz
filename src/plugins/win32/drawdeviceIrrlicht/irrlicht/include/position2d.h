// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_POSITION_H_INCLUDED__
#define __IRR_POSITION_H_INCLUDED__

#include "irrTypes.h"
#include "dimension2d.h"

namespace irr
{
namespace core
{

	//! Simple class for holding 2d coordinates.
	/** Not supposed for doing geometric calculations.
	use vector2d instead for things like that.
	*/
	template <class T>
	class position2d
	{
		public:
			//! Default constructor for (0,0).
			position2d() : X(0), Y(0) {}
			position2d(T x, T y) : X(x), Y(y) {}
			position2d(const position2d<T>& other)
				: X(other.X), Y(other.Y) {}

			bool operator == (const position2d<T>& other) const
			{
				return X == other.X && Y == other.Y;
			}

			bool operator != (const position2d<T>& other) const
			{
				return X != other.X || Y != other.Y;
			}

			const position2d<T>& operator+=(const position2d<T>& other)
			{
				X += other.X;
				Y += other.Y;
				return *this;
			}

			const position2d<T>& operator-=(const position2d<T>& other)
			{
				X -= other.X;
				Y -= other.Y;
				return *this;
			}

			const position2d<T>& operator+=(const dimension2d<T>& other)
			{
				X += other.Width;
				Y += other.Height;
				return *this;
			}

			const position2d<T>& operator-=(const dimension2d<T>& other)
			{
				X -= other.Width;
				Y -= other.Height;
				return *this;
			}

			position2d<T> operator-(const position2d<T>& other) const
			{
				return position2d<T>(X-other.X, Y-other.Y);
			}

			position2d<T> operator+(const position2d<T>& other) const
			{
				return position2d<T>(X+other.X, Y+other.Y);
			}

			position2d<T> operator*(const position2d<T>& other) const
			{
				return position2d<T>(X*other.X, Y*other.Y);
			}

			position2d<T> operator*(const T& scalar) const
			{
				return position2d<T>(X*scalar, Y*scalar);
			}

			position2d<T> operator+(const dimension2d<T>& other) const
			{
				return position2d<T>(X+other.Width, Y+other.Height);
			}

			position2d<T> operator-(const dimension2d<T>& other) const
			{
				return position2d<T>(X-other.Width, Y-other.Height);
			}

			const position2d<T>& operator=(const position2d<T>& other)
			{
				X = other.X;
				Y = other.Y;
				return *this;
			}

			//! X coordinate of the position.
			T X;
			//! Y coordinate of the position.
			T Y;
	};

	//! Typedef for an f32 position.
	typedef position2d<f32> position2df;
	//! Typedef for an integer position.
	typedef position2d<s32> position2di;

} // end namespace core
} // end namespace irr

#endif

