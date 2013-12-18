// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_LINE_2D_H_INCLUDED__
#define __IRR_LINE_2D_H_INCLUDED__

#include "irrTypes.h"
#include "vector2d.h"

namespace irr
{
namespace core
{

//! 2D line between two points with intersection methods.
template <class T>
class line2d
{
	public:
		//! Default constructor for line going from (0,0) to (1,1).
		line2d() : start(0,0), end(1,1) {}
		//! Constructor for line between the two points.
		line2d(T xa, T ya, T xb, T yb) : start(xa, ya), end(xb, yb) {}
		//! Constructor for line between the two points given as vectors.
		line2d(const vector2d<T>& start, const vector2d<T>& end) : start(start), end(end) {}
		//! Copy constructor.
		line2d(const line2d<T>& other) : start(other.start), end(other.end) {}

		// operators

		line2d<T> operator+(const vector2d<T>& point) const { return line2d<T>(start + point, end + point); }
		line2d<T>& operator+=(const vector2d<T>& point) { start += point; end += point; return *this; }

		line2d<T> operator-(const vector2d<T>& point) const { return line2d<T>(start - point, end - point); }
		line2d<T>& operator-=(const vector2d<T>& point) { start -= point; end -= point; return *this; }

		bool operator==(const line2d<T>& other) const
		{ return (start==other.start && end==other.end) || (end==other.start && start==other.end);}
		bool operator!=(const line2d<T>& other) const
		{ return !(start==other.start && end==other.end) || (end==other.start && start==other.end);}

		// functions
		//! Set this line to new line going through the two points.
		void setLine(const T& xa, const T& ya, const T& xb, const T& yb){start.set(xa, ya); end.set(xb, yb);}
		//! Set this line to new line going through the two points.
		void setLine(const vector2d<T>& nstart, const vector2d<T>& nend){start.set(nstart); end.set(nend);}
		//! Set this line to new line given as parameter.
		void setLine(const line2d<T>& line){start.set(line.start); end.set(line.end);}

		//! Get length of line
		/** \return Length of the line. */
		f64 getLength() const { return start.getDistanceFrom(end); }

		//! Get squared length of the line
		/** \return Squared length of line. */
		T getLengthSQ() const { return start.getDistanceFromSQ(end); }

		//! Get middle of the line
		/** \return center of the line. */
		vector2d<T> getMiddle() const
		{
			return (start + end) * (T)0.5;
		}

		//! Get the vector of the line.
		/** \return The vector of the line. */
		vector2d<T> getVector() const { return vector2d<T>(start.X - end.X, start.Y - end.Y); }

		//! Tests if this line intersects with another line.
		/** \param l: Other line to test intersection with.
		\param out: If there is an intersection, the location of the
		intersection will be stored in this vector.
		\return True if there is an intersection, false if not. */
		bool intersectWith(const line2d<T>& l, vector2d<T>& out) const
		{
			bool found=false;

			f32 a1,a2,b1,b2;

			// calculate slopes, deal with infinity
			if (end.X-start.X == 0)
				b1 = (f32)1e+10;
			else
				b1 = (end.Y-start.Y)/(end.X-start.X);
			if (l.end.X-l.start.X == 0)
				b2 = (f32)1e+10;
			else
				b2 = (l.end.Y-l.start.Y)/(l.end.X-l.start.X);

			// calculate position
			a1 = start.Y   - b1 * start.X;
			a2 = l.start.Y - b2 * l.start.X;
			out.X = - (a1-a2)/(b1-b2);
			out.Y = a1 + b1*out.X;

			// did the lines cross?
			if ((start.X-out.X) *(out.X-end.X) >= -ROUNDING_ERROR_32 &&
				(l.start.X-out.X)*(out.X-l.end.X)>= -ROUNDING_ERROR_32 &&
				(start.Y-out.Y)  *(out.Y-end.Y)  >= -ROUNDING_ERROR_32 &&
				(l.start.Y-out.Y)*(out.Y-l.end.Y)>= -ROUNDING_ERROR_32 )
			{
				found = true;
			}
			return found;
		}

		//! Get unit vector of the line.
		/** \return Unit vector of this line. */
		vector2d<T> getUnitVector() const
		{
			T len = (T)(1.0 / getLength());
			return vector2d<T>((end.X - start.X) * len, (end.Y - start.Y) * len);
		}

		//! Get angle between this line and given line.
		/** \param l Other line for test.
		\return Angle in degrees. */
		f64 getAngleWith(const line2d<T>& l) const
		{
			vector2d<T> vect = getVector();
			vector2d<T> vect2 = l.getVector();
			return vect.getAngleWith(vect2);
		}

		//! Tells us if the given point lies to the left, right, or on the line.
		/** \return 0 if the point is on the line
		<0 if to the left, or >0 if to the right. */
		T getPointOrientation(const vector2d<T>& point) const
		{
			return ( (end.X - start.X) * (point.Y - start.Y) -
					(point.X - start.X) * (end.Y - start.Y) );
		}

		//! Check if the given point is a member of the line
		/** \return True if point is between start and end, else false. */
		bool isPointOnLine(const vector2d<T>& point) const
		{
			T d = getPointOrientation(point);
			return (d == 0 && point.isBetweenPoints(start, end));
		}

		//! Check if the given point is between start and end of the line.
		/** Assumes that the point is already somewhere on the line. */
		bool isPointBetweenStartAndEnd(const vector2d<T>& point) const
		{
			return point.isBetweenPoints(start, end);
		}

		//! Get the closest point on this line to a point
		vector2d<T> getClosestPoint(const vector2d<T>& point) const
		{
			vector2d<T> c = point - start;
			vector2d<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			T t = v.dotProduct(c);

			if (t < (T)0.0) return start;
			if (t > d) return end;

			v *= t;
			return start + v;
		}

		//! Start point of the line.
		vector2d<T> start;
		//! End point of the line.
		vector2d<T> end;
};

	//! Typedef for an f32 line.
	typedef line2d<f32> line2df;
	//! Typedef for an integer line.
	typedef line2d<s32> line2di;

} // end namespace core
} // end namespace irr

#endif

