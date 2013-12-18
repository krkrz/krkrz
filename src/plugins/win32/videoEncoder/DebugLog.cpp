
#include <windows.h>
#include <stdio.h>
#include "DebugLog.h"


void DebugLog( unsigned long type, unsigned long level, const char* format, ... )
{
	va_list	argp;
	char pszBuf[ 512];
	va_start(argp, format);
	vsprintf_s( pszBuf, format, argp);
	va_end(argp);
	::OutputDebugStringA( pszBuf );
}

void DebugLog2( const char* format, ... )
{
	va_list	argp;
	char pszBuf[ 512];
	va_start(argp, format);
	vsprintf_s( pszBuf, format, argp);
	va_end(argp);
	::OutputDebugStringA( pszBuf );
}

