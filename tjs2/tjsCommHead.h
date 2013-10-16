//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// tjs common header
//---------------------------------------------------------------------------


/*
	Add headers that would not be frequently changed.
*/
#ifndef tjsCommHeadH
#define tjsCommHeadH

#ifdef TJS_SUPPORT_VCL
#include <vcl.h>
#endif

#ifdef __WIN32__
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include "targetver.h"
#include <windows.h>
#endif


#include <string.h>
#ifndef __USE_UNIX98
#define __USE_UNIX98
#endif
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <vector>
#include <string>
#include <stdexcept>

#include "tjsConfig.h"
#include "tjs.h"


#ifdef TJS_SUPPORT_VCL
	#pragma intrinsic strcpy
	#pragma intrinsic strcmp  // why these are needed?
#endif

//---------------------------------------------------------------------------
#endif


