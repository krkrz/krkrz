
#ifndef __FONT_SYSTEM_H__
#define __FONT_SYSTEM_H__

#include "tjsCommHead.h"
#include "tvpfontstruc.h"
#include "tjsHashSearch.h"
#include <string>

class tTVPWStringHash {
public:
	static tjs_uint32 Make(const std::wstring &val)
	{
		const wchar_t* ptr = val.c_str();
		if(*ptr == 0) return 0;
		tjs_uint32 v = 0;
		while(*ptr)
		{
			v += *ptr;
			v += (v << 10);
			v ^= (v >> 6);
			ptr++;
		}
		v += (v << 3);
		v ^= (v >> 11);
		v += (v << 15);
		if(!v) v = (tjs_uint32)-1;
		return v;
	}
};

/*
struct tTVPFontSearchStruct
{
	const char *Face;
	bool Found;
};
*/
class FontSystem {
	bool FontNamesInit;
	tTJSHashTable<std::wstring, tjs_int, tTVPWStringHash> TVPFontNames;

	static const TCHAR * const TVPDefaultFontName;
	tTVPFont DefaultFont;
	bool DefaultLOGFONTCreated;

	// win32
	LOGFONT DefaultLOGFONT;
	// win32

	void InitFontNames();
	//---------------------------------------------------------------------------
	void AddFont( const std::wstring& name );
	//---------------------------------------------------------------------------
	bool FontExists( const std::wstring &name );

	void ConstructDefaultFont();

public:
	FontSystem();
	std::wstring GetBeingFont(std::wstring fonts);
	const tTVPFont& GetDefaultFont() const {
		return DefaultFont;
	}
};

#endif // __FONT_SYSTEM_H__

