
#ifndef __T_STRING_H__
#define __T_STRING_H__

#include <string>
#ifdef WIN32
#include <tchar.h>
#endif

#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

#ifndef _T
#ifdef UNICODE
#define _T(x) L##x
#else
#define _T(x) x
#endif
#endif


#ifndef TCHAR
#ifdef UNICODE
#define TCHAR wchar_t
#else
#define TCHAR char
#endif
#endif

#endif // __T_STRING_H__
