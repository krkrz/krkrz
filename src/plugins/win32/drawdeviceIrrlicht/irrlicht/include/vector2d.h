// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_POINT_2D_H_INCLUDED__
#define __IRR_POINT_2D_H_INCLUDED__

#include "irrMath.h"

namespace irr
{
namespace core
{


//! 2d vector template class with lots of operators and methods.
template <class T>
class vector2d
{
public:

	vector2d() : X(0), Y(0) {}
	vector2d(T nx, T ny) : X(nx), Y(ny) {}
	vector2d(const vector2d<T>& other) : X(other.X), Y(other.Y) {}

	// operators

	vector2d<T> operator-() const { return vector2d<T>(-X, -Y); }

	vector2d<T>& operator=(const vector2d<T>& other) { X = other.X; Y = other.Y; return *this; }

	vector2d<T> operator+(const vector2d<T>& other) const { return vector2d<T>(X + other.X, Y + other.Y); }
	vector2d<T>& operator+=(const vector2d<T>& other) { X+=other.X; Y+=other.Y; return *this; }
	vector2d<T> operator+(const T v) const { return vector2d<T>(X + v, Y + v); }
	vector2d<T>& operator+=(const T v) { X+=v; Y+=v; return *this; }

	vector2d<T> operator-(const vector2d<T>& other) const { return vector2d<T>(X - other.X, Y - other.Y); }
	vector2d<T>& operator-=(const vector2d<T>& other) { X-=other.X; Y-=other.Y; return *this; }
	vector2d<T> operator-(const T v) const { return vector2d<T>(X - v, Y - v); }
	vector2d<T>& operator-=(const T v) { X-=v; Y-=v; return *this; }

	vector2d<T> operator*(const vector2d<T>& other) const { return vector2d<T>(X * other.X, Y * other.Y); }
	vector2d<T>& operator*=(const vector2d<T>& other) { X*=other.X; Y*=other.Y; return *this; }
	vector2d<T> operator*(const T v) const { return vector2d<T>(X * v, Y * v); }
	vector2d<T>& operator*=(const T v) { X*=v; Y*=v; return *this; }

	vector2d<T> operator/(const vector2d<T>& other) const { return vector2d<T>(X / other.X, Y / other.Y); }
	vector2d<T>& operator/=(const vector2d<T>& other) { X/=other.X; Y/=other.Y; return *this; }
	vector2d<T> operator/(const T v) const { return vector2d<T>(X / v, Y / v); }
	vector2d<T>& operator/=(const T v) { X/=v; Y/=v; return *this; }

	bool operator<=(const vector2d<T>&other) const { return X<=other.X && Y<=other.Y; }
	bool operator>=(const vector2d<T>&other) const { return X>=other.X && Y>=other.Y; }

	bool operator<(const vector2d<T>&other) const { return X<other.X && Y<other.Y; }
	bool operator>(const vector2d<T>&other) const { return X>other.X && Y>other.Y; }

	bool operator==(const vector2d<T>& other) const { return other.X==X && other.Y==Y; }
	bool operator!=(const vector2d<T>& other) const { return other.X!=X || other.Y!=Y; }

	// functions

	//! Checks if this vector equals the other one.
	/** Takes floating point rounding errors into account.
	\param other Vector to compare with.
	\return True if the two vector are (almost) equal, else false. */
	bool equals(const vector2d<T>& other) const
	{
		return core::equals(X, other.X) && core::equals(Y, other.Y);
	}

	void set(T nx, T ny) {X=nx; Y=ny; }
	void set(const vector2d<T>& p) { X=p.X; Y=p.Y;}

	//! Gets the length of the vector.
	/** \return The length of the vector. */
	T getLength() const { return (T)sqrt((f64)(X*X + Y*Y)); }

	//! Get the squared length of this vector
	/** This is useful because it is much faster than getLength().
	\return The squared length of the vector. */
	T getLengthSQ() const { return X*X + Y*Y; }

	//! Get the dot product of this vector with another.
	/** \param other Other vector to take dot product with.
	\return The dot product of the two vectors. */
	T dotProduct(const vector2d<T>& other) const
	{
		return X*other.X + Y*other.Y;
	}

	//! Gets distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Distance from other point. */
	T getDistanceFrom(const vector2d<T>& other) const
	{
		return vector2d<T>(X - other.X, Y - other.Y).getLength();
	}

	//! Returns squared distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Squared distance from other point. */
	T getDistanceFromSQ(const vector2d<T>& other) const
	{
		return vector2d<T>(X - other.X, Y - other.Y).getLengthSQ();
	}

	//! rotates the point around a center by an amount of degrees.
	/** \param degrees Amount of degrees to rotate by.
	\param center Rotation center. */
	void rotateBy(f64 degrees, const vector2d<T>& center)
	{
		degrees *= DEGTORAD64;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);

		X -= center.X;
		Y -= center.Y;

		set(X*cs - Y*sn, X*sn + Y*cs);

		X += center.X;
		Y += center.Y;
	}

	//! Normalize the vector.
	/** The null vector is left untouched.
	\return Reference to this vector, after normalization. */
	vector2d<T>& normalize()
	{
		T l = X*X + Y*Y;
		if (l == 0)
			return *this;
		l = core::reciprocal_squareroot ( (f32)l );
		X *= l;
		Y *= l;
		return *this;
	}

	//! Calculates the angle of this vector in degrees in the trigonometric sense.
	/** 0 is to the left (9 o'clock), values increase clockwise.
	This method has been suggested by Pr3t3nd3r.
	\return Returns a value between 0 and 360. */
	f64 getAngleTrig() const
	{
		if (X == 0)
			return Y < 0 ? 270 : 90;
		else
		if (Y == 0)
			return X < 0 ? 180 : 0;

		if ( Y > 0)
			if (X > 0)
				return atan(Y/X) * RADTODEG64;
			else
				return 180.0-atan(Y/-X) * RADTODEG64;
		else
			if (X > 0)
				return 360.0-atan(-Y/X) * RADTODEG64;
			else
				return 180.0+atan(-Y/-X) * RADTODEG64;
	}

	//! Calculates the angle of this vector in degrees in the counter trigonometric sense.
	/** 0 is to the right (3 o'clock), values increase counter-clockwise.
	\return Returns a value between 0 and 360. */
	inline f64 getAngle() const
	{
		if (Y == 0) // corrected thanks to a suggestion by Jox
			return X < 0 ? 180 : 0;
		else if (X == 0)
			return Y < 0 ? 90 : 270;

		f64 tmp = Y / getLength();
		tmp = atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;

		if (X>0 && Y>0)
			return tmp + 270;
		else
		if (X>0 && Y<0)
			return tmp + 90;
		else
		if (X<0 && Y<0)
			return 90 - tmp;
		else
		if (X<0 && Y>0)
			return 270 - tmp;

		return tmp;
	}

	//! Calculates the angle between this vector and another one in degree.
	/** \param b Other vector to test with.
	\return Returns a value between 0 and 90. */
	inline f64 getAngleWith(const vector2d<T>& b) const
	{
		f64 tmp = X*b.X + Y*b.Y;

		if (tmp == 0.0)
			return 90.0;

		tmp = tmp / sqrt((f64)((X*X + Y*Y) * (b.X*b.X + b.Y*b.Y)));
		if (tmp < 0.0)
			tmp = -tmp;

		return atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;
	}

	//! Returns if this vector interpreted as a point is on a line between two other points.
	/** It is assumed that the point is on the line.
	\param begin Beginning vector to compare between.
	\param end Ending vector to compare between.
	\return True if this vector is between begin and end, false if not. */
	bool isBetweenPoints(const vector2d<T>& begin, const vector2d<T>& end) const
	{
		T f = (end - begin).getLengthSQ();
		return getDistanceFromSQ(begin) < f &&
			getDistanceFromSQ(end) < f;
	}

	//! Get the interpolated vector
	/** \param other Other vector to interpolate with.
	\param d Value between 0.0f and 1.0f.
	\return Interpolated vector. */
	vector2d<T> getInterpolated(const vector2d<T>& other, f32 d) const
	{
		T inv = (T) 1.0 - d;
		return vector2d<T>(other.X*inv + X*d, other.Y*inv + Y*d);
	}

	//! Returns (quadratically) interpolated vector between this and the two given ones.
	/** \param v2 Second vector to interpolate with
	\param v3 Third vector to interpolate with
	\param d Value between 0.0f and 1.0f.
	\return Interpolated vector. */
	vector2d<T> getInterpolated_quadratic(const vector2d<T>& v2, const vector2d<T>& v3, const T d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const T inv = (T) 1.0 - d;
		const T mul0 = inv * inv;
		const T mul1 = (T) 2.0 * d * inv;
		const T mul2 = d * d;

		return vector2d<T> ( X * mul0 + v2.X * mul1 + v3.X * mul2,
					Y * mul0 + v2.Y * mul1 + v3.Y * mul2);
	}

	//! Sets this vector to the linearly interpolated vector between a and b.
	/** \param a first vector to interpolate with
	\param b second vector to interpolate with
	\param t value between 0.0f and 1.0f. */
	void interpolate(const vector2d<T>& a, const vector2d<T>& b, const f32 t)
	{
		X = b.X + ( ( a.X - b.X ) * t );
		Y = b.Y + ( ( a.Y - b.Y ) * t );
	}

	//! X coordinate of vector.
	T X;
	//! Y coordinate of vector.
	T Y;
};

	//! Typedef for f32 2d vector.
	typedef vector2d<f32> vector2df;
	//! Typedef for integer 2d vector.
	typedef vector2d<s32> vector2di;

	template<class S, class T> vector2d<T> operator*(const S scalar, const vector2d<T>& vector) { return vector*scalar; }

} // end namespace core
} // end namespace irr

#endif

