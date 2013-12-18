//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Regular Expression Support
//---------------------------------------------------------------------------
#ifndef tjsRegExpH
#define tjsRegExpH

#ifdef TJS_USE_XPRESSIVE
#include <boost/xpressive/xpressive_dynamic.hpp>
#ifdef BOOST_XPRESSIVE_NO_WREGEX
#error "wcregex is not usable! see details at tjsRegExp.h"
#endif
#else // !defined(TJS_USE_XPRESSIVE)
#include <boost/regex.hpp>
#ifdef BOOST_NO_WREGEX
#error "wregex is not usable! see details at tjsRegExp.h"

gcc g++ has a lack of wstring.

you must modify:

include/g++-3/string: uncomment this:
typedef basic_string <wchar_t> wstring;


include/g++-3/std/bastring.h: 
    { if (length () == 0) return ""; terminate (); return data (); }

is not usable in wstring; fix like

    { static charT zero=(charT)0; if (length () == 0) return &zero; terminate (); return data (); }


boost/config/stdlib/libstdcpp3.hpp: insert this to the beginning of the file:
#define _GLIBCPP_USE_WCHAR_T

or uncomment in sgi.hpp

#     define BOOST_NO_STD_WSTRING

#endif
#endif // TJS_USE_XPRESSIVE

#ifndef __TP_STUB_H__
#include "tjsNative.h"
#endif

namespace TJS
{
#ifdef TJS_USE_XPRESSIVE
typedef boost::xpressive::wcregex tTJSRegEx;
typedef boost::xpressive::wcmatch tTJSRegExMatch;
typedef boost::xpressive::wcregex_iterator tTJSRegExIterator;
#else
typedef boost::basic_regex<tjs_char> tTJSRegEx;
typedef boost::match_results<const tjs_char *> tTJSRegExMatch;
#endif
//---------------------------------------------------------------------------
// tTJSNI_RegExp
//---------------------------------------------------------------------------
class tTJSNI_RegExp : public tTJSNativeInstance
{
public:
	tTJSNI_RegExp();
	tTJSRegEx RegEx;
	tjs_uint32 Flags;
	tjs_uint Start;
	tTJSVariant Array;
	tjs_uint Index;
	ttstr Input;
	tjs_uint LastIndex;
	ttstr LastMatch;
	ttstr LastParen;
	ttstr LeftContext;
	ttstr RightContext;

private:

public:
	void Split(iTJSDispatch2 ** array, const ttstr &target, bool purgeempty);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNC_RegExp
//---------------------------------------------------------------------------
class tTJSNC_RegExp : public tTJSNativeClass
{
public:
	tTJSNC_RegExp();

	static void Compile(tjs_int numparam, tTJSVariant **param, tTJSNI_RegExp *_this);
	static bool Match(tTJSRegExMatch& what, ttstr target, tTJSNI_RegExp *_this);
	static bool Exec(tTJSRegExMatch& what, ttstr target, tTJSNI_RegExp *_this);
	static iTJSDispatch2 * GetResultArray(bool matched, tTJSNI_RegExp *_this,
		const tTJSRegExMatch& what);

private:
	tTJSNativeInstance *CreateNativeInstance();

public:
	static tjs_uint32 ClassID;

//	static tTJSVariant LastRegExp;
	static tTJSVariant& LastRegExp();
};
//---------------------------------------------------------------------------
extern iTJSDispatch2 * TJSCreateRegExpClass();
//---------------------------------------------------------------------------

} // namespace TJS

#endif
