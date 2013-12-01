
#define _USE_MATH_DEFINES
#include "FreeTypeFontRasterizer.h"
#include "LayerBitmapIntf.h"
#include "FreeType.h"
#include <math.h>
#include "MsgIntf.h"

extern void TVPUninitializeFreeFont();

FreeTypeFontRasterizer::FreeTypeFontRasterizer() : RefCount(0), Face(NULL), LastBitmap(NULL) {
	AddRef();
}
FreeTypeFontRasterizer::~FreeTypeFontRasterizer() {
	if( Face ) delete Face;
	Face = NULL;
	TVPUninitializeFreeFont();
}
void FreeTypeFontRasterizer::AddRef() {
	RefCount++;
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::Release() {
	RefCount--;
	LastBitmap = NULL;
	if( RefCount == 0 ) {
		if( Face ) delete Face;
		Face = NULL;

		delete this;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::ApplyFont( class tTVPNativeBaseBitmap *bmp, bool force ) {
	if( bmp != LastBitmap || force ) {
		ApplyFont( bmp->GetFont() );
		LastBitmap = bmp;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::ApplyFont( const tTVPFont& font ) {
	CurrentFont = font;
	ttstr name = font.Face;
	std::wstring stdname = name.AsStdString();
	// TVP_FACE_OPTIONS_NO_ANTIALIASING
	// TVP_FACE_OPTIONS_NO_HINTING
	// TVP_FACE_OPTIONS_FORCE_AUTO_HINTING
	tjs_uint32 opt = 0;
	opt |= (font.Flags & TVP_TF_ITALIC) ? TVP_TF_ITALIC : 0;
	opt |= (font.Flags & TVP_TF_BOLD) ? TVP_TF_BOLD : 0;
	opt |= (font.Flags & TVP_TF_UNDERLINE) ? TVP_TF_UNDERLINE : 0;
	opt |= (font.Flags & TVP_TF_STRIKEOUT) ? TVP_TF_STRIKEOUT : 0;
	opt |= (font.Flags & TVP_TF_FONTFILE) ? TVP_FACE_OPTIONS_FILE : 0;
	bool recreate = false;
	if( Face ) {
		if( Face->GetFontName() != stdname ) {
			delete Face;
			Face = new tFreeTypeFace( stdname, opt );
			recreate = true;
		}
	} else {
		Face = new tFreeTypeFace( stdname, opt );
		recreate = true;
	}
	Face->SetHeight( font.Height < 0 ? -font.Height : font.Height );
	if( recreate == false ) {
		if( font.Flags & TVP_TF_ITALIC ) {
			Face->SetOption(TVP_TF_ITALIC);
		} else {
			Face->ClearOption(TVP_TF_ITALIC);
		}
		if( font.Flags & TVP_TF_BOLD ) {
			Face->SetOption(TVP_TF_BOLD);
		} else {
			Face->ClearOption(TVP_TF_BOLD);
		}
		if( font.Flags & TVP_TF_UNDERLINE ) {
			Face->SetOption(TVP_TF_UNDERLINE);
		} else {
			Face->ClearOption(TVP_TF_UNDERLINE);
		}
		if( font.Flags & TVP_TF_STRIKEOUT ) {
			Face->SetOption(TVP_TF_STRIKEOUT);
		} else {
			Face->ClearOption(TVP_TF_STRIKEOUT);
		}
	}
	LastBitmap = NULL;
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::GetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h) {
	if( Face ) {
		tGlyphMetrics metrics;
		if( Face->GetGlyphSizeFromCharcode( ch, metrics) ) {
			w = metrics.CellIncX;
			h = metrics.CellIncY;
		}
	}
}
tjs_int FreeTypeFontRasterizer::GetAscentHeight() {
	if( Face ) return Face->GetAscent();
	return 0;
}
tTVPCharacterData* FreeTypeFontRasterizer::GetBitmap( const tTVPFontAndCharacterData & font, tjs_int aofsx, tjs_int aofsy ) {
	if( font.Antialiased ) {
		Face->ClearOption( TVP_FACE_OPTIONS_NO_ANTIALIASING );
	} else {
		Face->SetOption( TVP_FACE_OPTIONS_NO_ANTIALIASING );
	}
	if( font.Hinting ) {
		Face->ClearOption( TVP_FACE_OPTIONS_NO_HINTING );
		//Face->SetOption( TVP_FACE_OPTIONS_FORCE_AUTO_HINTING );
	} else {
		Face->SetOption( TVP_FACE_OPTIONS_NO_HINTING );
		//Face->ClearOption( TVP_FACE_OPTIONS_FORCE_AUTO_HINTING );
	}
	tTVPCharacterData* data = Face->GetGlyphFromCharcode(font.Character);
	if( data == NULL ) {
		data = Face->GetGlyphFromCharcode( Face->GetDefaultChar() );
		if( data == NULL ) {
			TVPThrowExceptionMessage( TVPFontRasterizeError );
		}
	}

	int cx = data->Metrics.CellIncX;
	int cy = data->Metrics.CellIncY;
	if( font.Font.Angle == 0 ) {
		data->Metrics.CellIncX = cx;
		data->Metrics.CellIncY = 0;
	} else if(font.Font.Angle == 2700) {
		data->Metrics.CellIncX = 0;
		data->Metrics.CellIncY = cx;
	} else {
		double angle = font.Font.Angle * (M_PI/1800);
		data->Metrics.CellIncX = static_cast<tjs_int>(  std::cos(angle) * cx);
		data->Metrics.CellIncY = static_cast<tjs_int>(- std::sin(angle) * cx);
	}

	data->Antialiased = font.Antialiased;
	data->FullColored = false;
	data->Blured = font.Blured;
	data->BlurWidth = font.BlurWidth;
	data->BlurLevel = font.BlurLevel;
	return data;
}


