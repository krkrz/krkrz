/****************************************************************************/
/*! @file
@brief 

-----------------------------------------------------------------------------
	Copyright (C) 2007 T.Imoto ( http://www.kaede-software.com/ )
-----------------------------------------------------------------------------
@author		T.Imoto
@date		2007/09/20
@note
*****************************************************************************/

#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__


//! デバッグログのレベル
enum DebugLogLevel {
	DebugLogLevelHighest,
	DebugLogLevelHigher,
	DebugLogLevelNormal,
	DebugLogLevelLower,
	DebugLogLevelLowest,
	DebugLogLevelEOT
};

#if defined(_DEBUG) || defined(DEBUG)
// Debugのとき
#define Trace(x)				DebugLog(0,DebugLogLevelLowest,x)
#define Trace0(x)				DebugLog(0,DebugLogLevelLowest,x)
#define Trace1(x, a)			DebugLog(0,DebugLogLevelLowest,x, a)
#define Trace2(x, a, b)			DebugLog(0,DebugLogLevelLowest,x, a, b)
#define Trace3(x, a, b, c)		DebugLog(0,DebugLogLevelLowest,x, a, b, c)
#define Trace4(x, a, b, c, d)	DebugLog(0,DebugLogLevelLowest,x, a, b, c, d)
#else
// Releaseのとき
#define Trace(x)
#define Trace0(x)
#define Trace1(x, a)
#define Trace2(x, a, b)
#define Trace3(x, a, b, c)
#define Trace4(x, a, b, c, d)
#endif


void DebugLog( unsigned long type, unsigned long level, const char* format, ... );
void DebugLog2( const char* format, ... );


#endif // __DEBUG_LOG_H__

