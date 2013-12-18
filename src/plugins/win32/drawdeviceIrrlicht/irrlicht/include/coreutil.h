// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_CORE_UTIL_H_INCLUDED__
#define __IRR_CORE_UTIL_H_INCLUDED__

#include "irrString.h"

namespace irr
{
namespace core
{

/*! \file irrxml.h
	\brief File containing useful basic utility functions
*/

// ----------- some basic quite often used string functions -----------------

//! cut the filename extension from a string
inline stringc& cutFilenameExtension ( stringc &dest, const stringc &source )
{
	s32 endPos = source.findLast ( '.' );
	dest = source.subString ( 0, endPos < 0 ? source.size () : endPos );
	return dest;
}

//! get the filename extension from a string
inline stringc& getFileNameExtension ( stringc &dest, const stringc &source )
{
	s32 endPos = source.findLast ( '.' );
	if ( endPos < 0 )
		dest = "";
	else
		dest = source.subString ( endPos, source.size () );
	return dest;
}

//! some standard function ( to remove dependencies )
#undef isdigit
#undef isspace
#undef isupper
inline s32 isdigit(s32 c) { return c >= '0' && c <= '9'; }
inline s32 isspace(s32 c) { return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'; }
inline s32 isupper(s32 c) { return c >= 'A' && c <= 'Z'; }


} // end namespace core
} // end namespace irr

#endif

