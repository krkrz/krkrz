// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_POINT_3D_H_INCLUDED__
#define __IRR_POINT_3D_H_INCLUDED__

#include "irrMath.h"

namespace irr
{
namespace core
{

	//! 3d vector template class with lots of operators and methods.
	template <class T>
	class vector3d
	{
	public:
		//! Default constructor (null vector).
		vector3d() : X(0), Y(0), Z(0) {}
		vector3d(T nx, T ny, T nz) : X(nx), Y(ny), Z(nz) {}
		vector3d(const vector3d<T>& other) : X(other.X), Y(other.Y), Z(other.Z) {}

		// operators

		vector3d<T> operator-() const { return vector3d<T>(-X, -Y, -Z); }

		vector3d<T>& operator=(const vector3d<T>& other) { X = other.X; Y = other.Y; Z = other.Z; return *this; }

		vector3d<T> operator+(const vector3d<T>& other) const { return vector3d<T>(X + other.X, Y + other.Y, Z + other.Z); }
		vector3d<T>& operator+=(const vector3d<T>& other) { X+=other.X; Y+=other.Y; Z+=other.Z; return *this; }

		vector3d<T> operator-(const vector3d<T>& other) const { return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z); }
		vector3d<T>& operator-=(const vector3d<T>& other) { X-=other.X; Y-=other.Y; Z-=other.Z; return *this; }

		vector3d<T> operator*(const vector3d<T>& other) const { return vector3d<T>(X * other.X, Y * other.Y, Z * other.Z); }
		vector3d<T>& operator*=(const vector3d<T>& other) { X*=other.X; Y*=other.Y; Z*=other.Z; return *this; }
		vector3d<T> operator*(const T v) const { return vector3d<T>(X * v, Y * v, Z * v); }
		vector3d<T>& operator*=(const T v) { X*=v; Y*=v; Z*=v; return *this; }

		vector3d<T> operator/(const vector3d<T>& other) const { return vector3d<T>(X / other.X, Y / other.Y, Z / other.Z); }
		vector3d<T>& operator/=(const vector3d<T>& other) { X/=other.X; Y/=other.Y; Z/=other.Z; return *this; }
		vector3d<T> operator/(const T v) const { T i=(T)1.0/v; return vector3d<T>(X * i, Y * i, Z * i); }
		vector3d<T>& operator/=(const T v) { T i=(T)1.0/v; X*=i; Y*=i; Z*=i; return *this; }

		bool operator<=(const vector3d<T>&other) const { return X<=other.X && Y<=other.Y && Z<=other.Z;}
		bool operator>=(const vector3d<T>&other) const { return X>=other.X && Y>=other.Y && Z>=other.Z;}
		bool operator<(const vector3d<T>&other) const { return X<other.X && Y<other.Y && Z<other.Z;}
		bool operator>(const vector3d<T>&other) const { return X>other.X && Y>other.Y && Z>other.Z;}

		//! use weak float compare
		bool operator==(const vector3d<T>& other) const
		{
			return this->equals(other);
		}

		bool operator!=(const vector3d<T>& other) const
		{
			return !this->equals(other);
		}

		// functions

		//! returns if this vector equals the other one, taking floating point rounding errors into account
		bool equals(const vector3d<T>& other, const T tolerance = (T)ROUNDING_ERROR_32 ) const
		{
			return core::equals(X, other.X, tolerance) &&
				core::equals(Y, other.Y, tolerance) &&
				core::equals(Z, other.Z, tolerance);
		}

		void set(const T nx, const T ny, const T nz) {X=nx; Y=ny; Z=nz; }
		void set(const vector3d<T>& p) { X=p.X; Y=p.Y; Z=p.Z;}

		//! Get length of the vector.
		T getLength() const { return (T) sqrt((f64)(X*X + Y*Y + Z*Z)); }

		//! Get squared length of the vector.
		/** This is useful because it is much faster than getLength().
		\return Squared length of the vector. */
		T getLengthSQ() const { return X*X + Y*Y + Z*Z; }

		//! Get the dot product with another vector.
		T dotProduct(const vector3d<T>& other) const
		{
			return X*other.X + Y*other.Y + Z*other.Z;
		}

		//! Get distance from another point.
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFrom(const vector3d<T>& other) const
		{
			return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z).getLength();
		}

		//! Returns squared distance from another point.
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFromSQ(const vector3d<T>& other) const
		{
			return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z).getLengthSQ();
		}

		//! Calculates the cross product with another vector.
		/** \param p Vector to multiply with.
		\return Crossproduct of this vector with p. */
		vector3d<T> crossProduct(const vector3d<T>& p) const
		{
			return vector3d<T>(Y * p.Z - Z * p.Y, Z * p.X - X * p.Z, X * p.Y - Y * p.X);
		}

		//! Returns if this vector interpreted as a point is on a line between two other points.
		/** It is assumed that the point is on the line.
		\param begin Beginning vector to compare between.
		\param end Ending vector to compare between.
		\return True if this vector is between begin and end, false if not. */
		bool isBetweenPoints(const vector3d<T>& begin, const vector3d<T>& end) const
		{
			T f = (end - begin).getLengthSQ();
			return getDistanceFromSQ(begin) < f &&
				getDistanceFromSQ(end) < f;
		}

		//! Normalizes the vector.
		/** In case of the 0 vector the result is still 0, otherwise
		the length of the vector will be 1.
		TODO: 64 Bit template doesnt work.. need specialized template
		\return Reference to this vector after normalization. */
		vector3d<T>& normalize()
		{
			T l = X*X + Y*Y + Z*Z;
			if (l == 0)
				return *this;
			l = (T) reciprocal_squareroot ( (f32)l );
			X *= l;
			Y *= l;
			Z *= l;
			return *this;
		}

		//! Sets the length of the vector to a new value
		void setLength(T newlength)
		{
			normalize();
			*this *= newlength;
		}

		//! Inverts the vector.
		void invert()
		{
			X *= -1.0f;
			Y *= -1.0f;
			Z *= -1.0f;
		}

		//! Rotates the vector by a specified number of degrees around the Y axis and the specified center.
		/** \param degrees Number of degrees to rotate around the Y axis.
		\param center The center of the rotation. */
		void rotateXZBy(f64 degrees, const vector3d<T>& center)
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			X -= center.X;
			Z -= center.Z;
			set(X*cs - Z*sn, Y, X*sn + Z*cs);
			X += center.X;
			Z += center.Z;
		}

		//! Rotates the vector by a specified number of degrees around the Z axis and the specified center.
		/** \param degrees: Number of degrees to rotate around the Z axis.
		\param center: The center of the rotation. */
		void rotateXYBy(f64 degrees, const vector3d<T>& center)
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			X -= center.X;
			Y -= center.Y;
			set(X*cs - Y*sn, X*sn + Y*cs, Z);
			X += center.X;
			Y += center.Y;
		}

		//! Rotates the vector by a specified number of degrees around the X axis and the specified center.
		/** \param degrees: Number of degrees to rotate around the X axis.
		\param center: The center of the rotation. */
		void rotateYZBy(f64 degrees, const vector3d<T>& center)
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			Z -= center.Z;
			Y -= center.Y;
			set(X, Y*cs - Z*sn, Y*sn + Z*cs);
			Z += center.Z;
			Y += center.Y;
		}

		//! Returns interpolated vector.
		/** \param other Other vector to interpolate between
		\param d Value between 0.0f and 1.0f. */
		vector3d<T> getInterpolated(const vector3d<T>& other, const T d) const
		{
			const T inv = (T) 1.0 - d;
			return vector3d<T>(other.X*inv + X*d, other.Y*inv + Y*d, other.Z*inv + Z*d);
		}

		//! Returns interpolated vector. ( quadratic )
		/** \param v2 Second vector to interpolate with
		\param v3 Third vector to interpolate with
		\param d Value between 0.0f and 1.0f. */
		vector3d<T> getInterpolated_quadratic(const vector3d<T>& v2, const vector3d<T>& v3, const T d) const
		{
			// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
			const T inv = (T) 1.0 - d;
			const T mul0 = inv * inv;
			const T mul1 = (T) 2.0 * d * inv;
			const T mul2 = d * d;

			return vector3d<T> ( X * mul0 + v2.X * mul1 + v3.X * mul2,
					Y * mul0 + v2.Y * mul1 + v3.Y * mul2,
					Z * mul0 + v2.Z * mul1 + v3.Z * mul2);
		}

		//! Gets the Y and Z rotations of a vector.
		/** Thanks to Arras on the Irrlicht forums to add this method.
		\return A vector representing the rotation in degrees of
		this vector. The Z component of the vector will always be 0. */
		vector3d<T> getHorizontalAngle()
		{
			vector3d<T> angle;

			angle.Y = (T)atan2(X, Z);
			angle.Y *= (f32)RADTODEG64;

			if (angle.Y < 0.0f) angle.Y += 360.0f;
			if (angle.Y >= 360.0f) angle.Y -= 360.0f;

			f32 z1 = sqrtf(X*X + Z*Z);

			angle.X = (T)atan2(z1, Y);
			angle.X *= (f32)RADTODEG64;
			angle.X -= 90.0f;

			if (angle.X < 0.0f) angle.X += 360.0f;
			if (angle.X >= 360.0f) angle.X -= 360.0f;

			return angle;
		}

		//! Fills an array of 4 values with the vector data (usually floats).
		/** Useful for setting in shader constants for example. The fourth value
		will always be 0. */
		void getAs4Values(T* array) const
		{
			array[0] = X;
			array[1] = Y;
			array[2] = Z;
			array[3] = 0;
		}


		//! X coordinate of the vector
		T X;
		//! Y coordinate of the vector
		T Y;
		//! Z coordinate of the vector
		T Z;
	};


	//! Typedef for a f32 3d vector.
	typedef vector3d<f32> vector3df;
	//! Typedef for an integer 3d vector.
	typedef vector3d<s32> vector3di;

	//! Function multiplying a scalar and a vector component-wise.
	template<class S, class T> vector3d<T> operator*(const S scalar, const vector3d<T>& vector) { return vector*scalar; }

} // end namespace core
} // end namespace irr

#endif

