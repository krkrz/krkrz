// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:


// TODO: reference additional headers your program requires here
# if (defined(WIN32) || defined(WINCE))
#include <windows.h>
# define LOOG_INT64 signed __int64
# define LOOG_UINT64 unsigned __int64
#else
# define LOOG_INT64 int64_t
# define LOOG_UINT64 uint64_t
#endif