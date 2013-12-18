// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and irrXML.h

#ifndef __IRR_STRING_H_INCLUDED__
#define __IRR_STRING_H_INCLUDED__

#include "irrTypes.h"
#include "irrAllocator.h"
#include "irrMath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace irr
{
namespace core
{

//! Very simple string class with some useful features.
/** string<c8> and string<wchar_t> work both with unicode AND ascii,
so you can assign unicode to string<c8> and ascii to string<wchar_t>
(and the other way round) if your ever would want to.
Note that the conversation between both is not done using an encoding.

Known bugs:
Special characters like umlauts are ignored in the
methods make_upper, make_lower and equals_ignore_case.
*/
template <typename T, typename TAlloc = irrAllocator<T> >
class string
{
public:

	//! Default constructor
	string()
	: array(0), allocated(1), used(1)
	{
		array = allocator.allocate(1); // new T[1];
		array[0] = 0x0;
	}



	//! Constructor
	string(const string<T>& other)
	: array(0), allocated(0), used(0)
	{
		*this = other;
	}



	//! Constructs a string from a float
	string(const double number)
	: array(0), allocated(0), used(0)
	{
		c8 tmpbuf[255];
		snprintf(tmpbuf, 255, "%0.6f", number);
		*this = tmpbuf;
	}



	//! Constructs a string from an int
	string(int number)
	: array(0), allocated(0), used(0)
	{
		// store if negative and make positive

		bool negative = false;
		if (number < 0)
		{
			number *= -1;
			negative = true;
		}

		// temporary buffer for 16 numbers

		c8 tmpbuf[16];
		tmpbuf[15] = 0;
		u32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (c8)('0' + (number % 10));
			number /= 10;
		}

		// add sign

		if (negative)
		{
			--idx;
			tmpbuf[idx] = '-';
		}

		*this = &tmpbuf[idx];
	}



	//! Constructs a string from an unsigned int
	string(unsigned int number)
	: array(0), allocated(0), used(0)
	{
		// temporary buffer for 16 numbers

		c8 tmpbuf[16];
		tmpbuf[15] = 0;
		u32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (c8)('0' + (number % 10));
			number /= 10;
		}

		*this = &tmpbuf[idx];
	}



	//! Constructor for copying a string from a pointer with a given length
	template <class B>
	string(const B* const c, u32 length)
	: array(0), allocated(0), used(0)
	{
		if (!c)
		{
			// correctly init the string to an empty one
			*this="";
			return;
		}

		allocated = used = length+1;
		array = allocator.allocate(used); // new T[used];

		for (u32 l = 0; l<length; ++l)
			array[l] = (T)c[l];

		array[length] = 0;
	}



	//! Constructor for unicode and ascii strings
	template <class B>
	string(const B* const c)
	: array(0), allocated(0), used(0)
	{
		*this = c;
	}



	//! destructor
	~string()
	{
		allocator.deallocate(array); // delete [] array;
	}



	//! Assignment operator
	string<T>& operator=(const string<T>& other)
	{
		if (this == &other)
			return *this;

		allocator.deallocate(array); // delete [] array;
		allocated = used = other.size()+1;
		array = allocator.allocate(used); //new T[used];

		const T* p = other.c_str();
		for (u32 i=0; i<used; ++i, ++p)
			array[i] = *p;

		return *this;
	}



	//! Assignment operator for strings, ascii and unicode
	template <class B>
	string<T>& operator=(const B* const c)
	{
		if (!c)
		{
			if (!array)
			{
				array = allocator.allocate(1); //new T[1];
				allocated = 1;
			}
			used = 1;
			array[0] = 0x0;
			return *this;
		}

		if ((void*)c == (void*)array)
			return *this;

		u32 len = 0;
		const B* p = c;
		while(*p)
		{
			++len;
			++p;
		}

		// we'll take the old string for a while, because the new
		// string could be a part of the current string.
		T* oldArray = array;

		++len;
		allocated = used = len;
		array = allocator.allocate(used); //new T[used];

		for (u32 l = 0; l<len; ++l)
			array[l] = (T)c[l];

		allocator.deallocate(oldArray); // delete [] oldArray;
		return *this;
	}

	//! Add operator for other strings
	string<T> operator+(const string<T>& other) const
	{
		string<T> str(*this);
		str.append(other);

		return str;
	}

	//! Add operator for strings, ascii and unicode
	template <class B>
	string<T> operator+(const B* const c) const
	{
		string<T> str(*this);
		str.append(c);

		return str;
	}


	//! Direct access operator
	T& operator [](const u32 index)
	{
		_IRR_DEBUG_BREAK_IF(index>=used) // bad index
		return array[index];
	}


	//! Direct access operator
	const T& operator [](const u32 index) const
	{
		_IRR_DEBUG_BREAK_IF(index>=used) // bad index
		return array[index];
	}


	//! Comparison operator
	bool operator ==(const T* const str) const
	{
		if (!str)
			return false;

		u32 i;
		for(i=0; array[i] && str[i]; ++i)
			if (array[i] != str[i])
				return false;

		return !array[i] && !str[i];
	}



	//! Comparison operator
	bool operator ==(const string<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
			if (array[i] != other.array[i])
				return false;

		return used == other.used;
	}


	//! Is smaller operator
	bool operator <(const string<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
		{
			s32 diff = array[i] - other.array[i];
			if ( diff )
				return diff < 0;
/*
			if (array[i] != other.array[i])
				return (array[i] < other.array[i]);
*/
		}

		return used < other.used;
	}



	//! Equals not operator
	bool operator !=(const T* const str) const
	{
		return !(*this == str);
	}



	//! Equals not operator
	bool operator !=(const string<T>& other) const
	{
		return !(*this == other);
	}



	//! Returns length of string
	/** \return Returns length of the string in characters. */
	u32 size() const
	{
		return used-1;
	}



	//! Returns character string
	/** \return Returns pointer to C-style zero terminated string. */
	const T* c_str() const
	{
		return array;
	}



	//! Makes the string lower case.
	void make_lower()
	{
		for (u32 i=0; i<used; ++i)
			array[i] = ansi_lower ( array[i] );
	}



	//! Makes the string upper case.
	void make_upper()
	{
		const T a = (T)'a';
		const T z = (T)'z';
		const T diff = (T)'A' - a;

		for (u32 i=0; i<used; ++i)
		{
			if (array[i]>=a && array[i]<=z)
				array[i] += diff;
		}
	}



	//! Compares the string ignoring case.
	/** \param other: Other string to compare.
	\return Returns true if the string are equal ignoring case. */
	bool equals_ignore_case(const string<T>& other) const
	{
		for(u32 i=0; array[i] && other[i]; ++i)
			if (ansi_lower(array[i]) != ansi_lower(other[i]))
				return false;

		return used == other.used;
	}

	//! Compares the string ignoring case.
	/** \param other: Other string to compare.
	\return Returns true if the string is smaller ignoring case. */
	bool lower_ignore_case(const string<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
		{
			s32 diff = (s32) ansi_lower ( array[i] ) - (s32) ansi_lower ( other.array[i] );
			if ( diff )
				return diff < 0;
		}

		return used < other.used;
	}



	//! compares the first n characters of the strings
	bool equalsn(const string<T>& other, u32 n) const
	{
		u32 i;
		for(i=0; array[i] && other[i] && i < n; ++i)
			if (array[i] != other[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == n) || (used == other.used);
	}


	//! compares the first n characters of the strings
	bool equalsn(const T* const str, u32 n) const
	{
		if (!str)
			return false;
		u32 i;
		for(i=0; array[i] && str[i] && i < n; ++i)
			if (array[i] != str[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == n) || (array[i] == 0 && str[i] == 0);
	}


	//! Appends a character to this string
	/** \param character: Character to append. */
	void append(T character)
	{
		if (used + 1 > allocated)
			reallocate(used + 1);

		++used;

		array[used-2] = character;
		array[used-1] = 0;
	}

	//! Appends a char string to this string
	/** \param other: Char string to append. */
	void append(const T* const other)
	{
		if (!other)
			return;

		u32 len = 0;
		const T* p = other;
		while(*p)
		{
			++len;
			++p;
		}

		if (used + len > allocated)
			reallocate(used + len);

		--used;
		++len;

		for (u32 l=0; l<len; ++l)
			array[l+used] = *(other+l);

		used += len;
	}


	//! Appends a string to this string
	/** \param other: String to append. */
	void append(const string<T>& other)
	{
		--used;
		u32 len = other.size()+1;

		if (used + len > allocated)
			reallocate(used + len);

		for (u32 l=0; l<len; ++l)
			array[used+l] = other[l];

		used += len;
	}


	//! Appends a string of the length l to this string.
	/** \param other: other String to append to this string.
	\param length: How much characters of the other string to add to this one. */
	void append(const string<T>& other, u32 length)
	{
		if (other.size() < length)
		{
			append(other);
			return;
		}

		if (used + length > allocated)
			reallocate(used + length);

		--used;

		for (u32 l=0; l<length; ++l)
			array[l+used] = other[l];
		used += length;

		// ensure proper termination
		array[used]=0;
		++used;
	}


	//! Reserves some memory.
	/** \param count: Amount of characters to reserve. */
	void reserve(u32 count)
	{
		if (count < allocated)
			return;

		reallocate(count);
	}


	//! finds first occurrence of character in string
	/** \param c: Character to search for.
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findFirst(T c) const
	{
		for (u32 i=0; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds first occurrence of a character of a list in string
	/** \param c: List of characters to find. For example if the method
	should find the first occurrence of 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Usually,
	this should be strlen(c)
	\return Returns position where one of the characters has been found,
	or -1 if not found. */
	s32 findFirstChar(const T* const c, u32 count) const
	{
		if (!c)
			return -1;

		for (u32 i=0; i<used; ++i)
			for (u32 j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! Finds first position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Usually,
	this should be strlen(c)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B>
	s32 findFirstCharNotInList(const B* const c, u32 count) const
	{
		for (u32 i=0; i<used-1; ++i)
		{
			u32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! Finds last position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Usually,
	this should be strlen(c)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B>
	s32 findLastCharNotInList(const B* const c, u32 count) const
	{
		for (s32 i=(s32)(used-2); i>=0; --i)
		{
			u32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! finds next occurrence of character in string
	/** \param c: Character to search for.
	\param startPos: Position in string to start searching.
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findNext(T c, u32 startPos) const
	{
		for (u32 i=startPos; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}


	//! finds last occurrence of character in string
	//! \param c: Character to search for.
	//! \param start: start to search reverse ( default = -1, on end )
	//! \return Returns position where the character has been found,
	//! or -1 if not found.
	s32 findLast(T c, s32 start = -1) const
	{
		start = core::clamp ( start < 0 ? (s32)(used) - 1 : start, 0, (s32)(used) - 1 );
		for (s32 i=start; i>=0; --i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds last occurrence of a character of a list in string
	/** \param c: List of strings to find. For example if the method
	should find the last occurrence of 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Usually,
	this should be strlen(c)
	\return Returns position where one of the characters has been found,
	or -1 if not found. */
	s32 findLastChar(const T* const c, u32 count) const
	{
		if (!c)
			return -1;

		for (s32 i=used-1; i>=0; --i)
			for (u32 j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! finds another string in this string
	//! \param str: Another string
	//! \return Returns positions where the string has been found,
	//! or -1 if not found.
	template <class B>
	s32 find(const B* const str) const
	{
		if (str && *str)
		{
			u32 len = 0;

			while (str[len])
				++len;

			if (len > used-1)
				return -1;

			for (u32 i=0; i<used-len; ++i)
			{
				u32 j=0;

				while(str[j] && array[i+j] == str[j])
					++j;

				if (!str[j])
					return i;
			}
		}

		return -1;
	}


	//! Returns a substring
	//! \param begin: Start of substring.
	//! \param length: Length of substring.
	string<T> subString(u32 begin, s32 length) const
	{
		if ((length+begin) > size())
			length = size()-begin;
		if (length <= 0)
			return string<T>("");

		string<T> o;
		o.reserve(length+1);

		for (s32 i=0; i<length; ++i)
			o.array[i] = array[i+begin];

		o.array[length] = 0;
		o.used = o.allocated;

		return o;
	}


	string<T>& operator += (T c)
	{
		append(c);
		return *this;
	}


	string<T>& operator += (const T* const c)
	{
		append(c);
		return *this;
	}


	string<T>& operator += (const string<T>& other)
	{
		append(other);
		return *this;
	}


	string<T>& operator += (const int i)
	{
		append(string<T>(i));
		return *this;
	}


	string<T>& operator += (const unsigned int i)
	{
		append(string<T>(i));
		return *this;
	}


	string<T>& operator += (const long i)
	{
		append(string<T>(i));
		return *this;
	}


	string<T>& operator += (const unsigned long& i)
	{
		append(string<T>(i));
		return *this;
	}


	string<T>& operator += (const double i)
	{
		append(string<T>(i));
		return *this;
	}


	string<T>& operator += (const float i)
	{
		append(string<T>(i));
		return *this;
	}


	//! replaces all characters of a special type with another one
	void replace(T toReplace, T replaceWith)
	{
		for (u32 i=0; i<used; ++i)
			if (array[i] == toReplace)
				array[i] = replaceWith;
	}

	//! trims the string.
	/** Removes whitespace from begin and end of the string. */
	string<T>& trim()
	{
		const c8 whitespace[] = " \t\n\r";
		const u32 whitespacecount = 4;

		// find start and end of real string without whitespace
		s32 begin = findFirstCharNotInList(whitespace, whitespacecount);
		if (begin == -1)
			return (*this="");

		s32 end = findLastCharNotInList(whitespace, whitespacecount);

		return (*this = subString(begin, (end +1) - begin));
	}


	//! Erases a character from the string. May be slow, because all elements
	//! following after the erased element have to be copied.
	//! \param index: Index of element to be erased.
	void erase(u32 index)
	{
		_IRR_DEBUG_BREAK_IF(index>=used) // access violation

		for (u32 i=index+1; i<used; ++i)
			array[i-1] = array[i];

		--used;
	}



private:
/*
	T toLower(const T& t) const
	{
		if (t>=(T)'A' && t<=(T)'Z')
			return t + ((T)'a' - (T)'A');
		else
			return t;
	}
*/
	//! Returns a character converted to lower case
	inline T ansi_lower ( u32 x ) const
	{
		return x >= 'A' && x <= 'Z' ? (T) x + 0x20 : (T) x;
	}


	//! Reallocate the array, make it bigger or smaller
	void reallocate(u32 new_size)
	{
		T* old_array = array;

		array = allocator.allocate(new_size); //new T[new_size];
		allocated = new_size;

		u32 amount = used < new_size ? used : new_size;
		for (u32 i=0; i<amount; ++i)
			array[i] = old_array[i];

		if (allocated < used)
			used = allocated;

		allocator.deallocate(old_array); // delete [] old_array;
	}


	//--- member variables

	T* array;
	u32 allocated;
	u32 used;
	TAlloc allocator;
};


//! Typedef for character strings
typedef string<c8> stringc;

//! Typedef for wide character strings
typedef string<wchar_t> stringw;

} // end namespace core
} // end namespace irr

#endif

