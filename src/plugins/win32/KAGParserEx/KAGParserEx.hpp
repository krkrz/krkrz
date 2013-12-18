#ifndef KAGParserExHPP
#define KAGParserExHPP

#include <windows.h>
#include "tp_stub.h"
#include "tjsHashSearch.h"
using namespace TJS;
#include <vector>

#define TVP_KAGPARSER_EX_PLUGIN
#define TVP_KAGPARSER_EX_CLASSNAME TJS_W("KAGParser")

#define TVP_KAGPARSER_CONCAT(a,b) a ## b
#define TVP_KAGPARSER_MESSAGEMAP(name) (TJSGetMessageMapMessage(TVP_KAGPARSER_CONCAT(L,#name)).c_str())


// from tjsConfig.h
#define TJS_strchr			wcschr
#define TJS_strcmp			wcscmp
#define TJS_strncpy			wcsncpy

// from MsgIntf.h (and MESSAGEMAP modified)
#define TVPThrowInternalError TVPThrowExceptionMessage(TVP_KAGPARSER_MESSAGEMAP(TVPInternalError), __FILE__,  __LINE__)

#include "KAGParser.h"
#endif
