//===========================================================================
//Copyright (C) 2003, 2004 Zentaro Kavanagh
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//- Neither the name of Zentaro Kavanagh nor the names of contributors 
//  may be used to endorse or promote products derived from this software 
//  without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <atlbase.h>
#include <atlcom.h>

#include <libOOOgg/libOOOgg.h>

#include "AbstractTransformFilter.h"
#include "AbstractTransformInputPin.h"
#include "AbstractTransformOutputPin.h"

#include "VorbisDecodeInputPin.h"
#include "VorbisDecodeOutputPin.h"
#include "VorbisDecodeFilter.h"

#include "libilliCore/iLE_Math.h"
#include "libOOOgg/OggPacket.h"

#include "common/Log.h"

#ifndef LOOG_INT64
# ifdef WIN32
#  define LOOG_INT64 signed __int64
# else  /* assume POSIX */
#  define LOOG_INT64 int64_t
# endif
#endif

#ifndef VORBISDECODER_DLL
#define LIBOOOGG_API
#else
#ifdef LIBOOOGG_EXPORTS
#define LIBOOOGG_API __declspec(dllexport)
#else
#define LIBOOOGG_API __declspec(dllimport)
#endif
#endif
