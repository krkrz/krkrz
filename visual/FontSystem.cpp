

#include "tjsCommHead.h"

#include "FontSystem.h"
#include "StringUtil.h"
#include "MsgIntf.h"
#include <vector>
#include "FontRasterizer.h"


extern void TVPGetAllFontList( std::vector<tjs_string>& list );
extern const tjs_char *TVPGetDefaultFontName();
extern void TVPSetDefaultFontName( const tjs_char * name );
extern const ttstr &TVPGetDefaultFaceNames();

void FontSystem::InitFontNames() {
	// enumlate all fonts
	if(FontNamesInit) return;

	std::vector<tjs_string> list;
	TVPGetAllFontList( list );
	size_t count = list.size();
	for( size_t i = 0; i < count; i++ ) {
		AddFont( list[i] );
	}

	FontNamesInit = true;
}
//---------------------------------------------------------------------------
void FontSystem::AddFont( const tjs_string& name ) {
	TVPFontNames.Add( name, 1 );
}
//---------------------------------------------------------------------------
bool FontSystem::FontExists( const tjs_string &name ) {
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
		DefaultFont.Height = -12;
		DefaultFont.Flags = 0;
		DefaultFont.Angle = 0;
		DefaultFont.Face = TVPGetDefaultFaceNames();
	}
}

tjs_string FontSystem::GetBeingFont(tjs_string fonts) {
	// retrieve being font in the system.
	// font candidates are given by "fonts", separated by comma.

	bool vfont;

	if(fonts.c_str()[0] == TJS_W('@')) {     // for vertical writing
		fonts = fonts.c_str() + 1;
		vfont = true;
	} else {
		vfont = false;
	}

	bool prev_empty_name = false;
	while(fonts!=TJS_W("")) {
		tjs_string fontname;
		tjs_string::size_type pos = fonts.find_first_of(TJS_W(","));
		if( pos != std::string::npos ) {
			fontname = Trim( fonts.substr( 0, pos) );
			fonts = fonts.c_str()+pos+1;
		} else {
			fontname = Trim(fonts);
			fonts=TJS_W("");
		}

		// no existing check if previously specified font candidate is empty
		// eg. ",Fontname"

		if(fontname != TJS_W("") && (prev_empty_name || FontExists(fontname) ) ) {
			if(vfont && fontname.c_str()[0] != TJS_W('@')) {
				return  TJS_W("@") + fontname;
			} else {
				return fontname;
			}
		}

		prev_empty_name = (fontname == TJS_W(""));
	}

	if(vfont) {
		return tjs_string(TJS_W("@")) + tjs_string(TVPGetDefaultFontName());
	} else {
		return tjs_string(TVPGetDefaultFontName());
	}
}
//---------------------------------------------------------------------------
void FontSystem::AddExtraFont( const tjs_string& storage, std::vector<ttstr>* faces ) {
	std::vector<tjs_string> loadface;
	if( GetCurrentRasterizer()->AddFont( storage, &loadface ) ) {
		for( auto i = loadface.begin(); i != loadface.end(); ++i ) {
			AddFont( *i );
		}
	}
	if( faces ) {
		for( auto i = loadface.begin(); i != loadface.end(); ++i ) {
			faces->push_back( ttstr( *i ) );
		}
	}
}
//---------------------------------------------------------------------------
const tjs_char* FontSystem::GetDefaultFontName() const {
	return TVPGetDefaultFontName();
}
//---------------------------------------------------------------------------
void FontSystem::SetDefaultFontName( const tjs_char* name ) {
	TVPSetDefaultFontName( name );
	DefaultFont.Face = ttstr(TVPGetDefaultFontName());
}
//---------------------------------------------------------------------------
void FontSystem::GetFontList(std::vector<ttstr> & list, tjs_uint32 flags, const struct tTVPFont & font ) {
	GetCurrentRasterizer()->GetFontList( list, flags, font );
}
//---------------------------------------------------------------------------
