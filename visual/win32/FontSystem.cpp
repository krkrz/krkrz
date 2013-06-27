
#include "FontSystem.h"
#include "StringUtil.h"
#include <vector>

extern void TVPGetAllFontList( std::vector<std::wstring>& list );

const TCHAR * const FontSystem::TVPDefaultFontName = _T("‚l‚r ‚oƒSƒVƒbƒN"); // TODO: i18n

void FontSystem::InitFontNames() {
	// enumlate all fonts
	if(FontNamesInit) return;

	std::vector<std::wstring> list;
	TVPGetAllFontList( list );
	int count = list.size();
	for( int i = 0; i < count; i++ ) {
		AddFont( list[i] );
	}

	FontNamesInit = true;
}
//---------------------------------------------------------------------------
void FontSystem::AddFont( const std::wstring& name ) {
	TVPFontNames.Add( name, 1 );
}
//---------------------------------------------------------------------------
bool FontSystem::FontExists( const std::wstring &name ) {
	// check existence of font
	InitFontNames();

	int * t = TVPFontNames.Find(name);
	return t != NULL;
}

FontSystem::FontSystem() : FontNamesInit(false), DefaultLOGFONTCreated(false) {
	ConstructDefaultFont();
}

void FontSystem::ConstructDefaultFont() {
	if( !DefaultLOGFONTCreated ) {
		DefaultLOGFONTCreated = true;
		LOGFONT &l = DefaultLOGFONT;
		l.lfHeight = -12;
		l.lfWidth = 0;
		l.lfEscapement = 0;
		l.lfOrientation = 0;
		l.lfWeight = 400;
		l.lfItalic = FALSE;
		l.lfUnderline = FALSE;
		l.lfStrikeOut = FALSE;
		l.lfCharSet = SHIFTJIS_CHARSET; // TODO: i18n
		l.lfOutPrecision = OUT_DEFAULT_PRECIS;
		l.lfQuality = DEFAULT_QUALITY;
		l.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy(l.lfFaceName, TVPDefaultFontName); // TODO: i18n

		DefaultFont.Height = l.lfHeight;
		DefaultFont.Flags = 0;
		DefaultFont.Angle = 0;
		DefaultFont.Face = ttstr(l.lfFaceName);
	}
}

std::wstring FontSystem::GetBeingFont(std::wstring fonts) {
	// retrieve being font in the system.
	// font candidates are given by "fonts", separated by comma.

	bool vfont;

	if(fonts.c_str()[0] == '@') {     // for vertical writing
		fonts = fonts.c_str() + 1;
		vfont = true;
	} else {
		vfont = false;
	}

	bool prev_empty_name = false;
	while(fonts!=_T("")) {
		std::wstring fontname;
		int pos = fonts.find_first_of(_T(","));
		if( pos != std::string::npos ) {
			fontname = Trim( fonts.substr( 0, pos) );
			fonts = fonts.c_str()+pos;
		} else {
			fontname = Trim(fonts);
			fonts=_T("");
		}

		// no existing check if previously specified font candidate is empty
		// eg. ",Fontname"

		if(fontname != _T("") && (prev_empty_name || FontExists(fontname) ) ) {
			if(vfont && fontname.c_str()[0] != '@') {
				return  _T("@") + fontname;
			} else {
				return fontname;
			}
		}

		prev_empty_name = (fontname == _T(""));
	}

	if(vfont) {
		return std::wstring(_T("@")) + TVPDefaultFontName;
	} else {
		return TVPDefaultFontName;
	}
}
