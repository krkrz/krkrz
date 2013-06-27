
#define _USE_MATH_DEFINES
#include "FreeTypeFontRasterizer.h"
#include "LayerBitmapIntf.h"
#include "FreeType.h"
#include <math.h>

FreeTypeFontRasterizer::FreeTypeFontRasterizer() : RefCount(0), Face(NULL), BoldFace(NULL), LastBitmap(NULL) {
}
FreeTypeFontRasterizer::~FreeTypeFontRasterizer() {
	if( Face ) delete Face;
	Face = NULL;
	if( BoldFace ) delete BoldFace;
	BoldFace = NULL;
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
		if( BoldFace ) delete BoldFace;
		BoldFace = NULL;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::ApplyFont( class tTVPNativeBaseBitmap *bmp, bool force ) {
	if( bmp != LastBitmap || force ) {
		const tTVPFont &font = bmp->GetFont();
		CurrentFont = font;
		ttstr name = font.Face;
		std::wstring stdname = name.AsStdString();
		// TVP_FACE_OPTIONS_NO_ANTIALIASING
		// TVP_FACE_OPTIONS_NO_HINTING
		// TVP_FACE_OPTIONS_FORCE_AUTO_HINTING
		if( Face ) {
			if( Face->GetFontName() != stdname ) {
				delete Face;
				delete BoldFace;
				Face = new tFreeTypeFace( stdname, 0 );
				BoldFace = new tFreeTypeFace( stdname, TVP_TF_BOLD );
			}
		} else {
			Face = new tFreeTypeFace( stdname, 0 );
			BoldFace = new tFreeTypeFace( stdname, TVP_TF_BOLD );
		}
		Face->SetHeight( font.Height );
		BoldFace->SetHeight( font.Height );
		LastBitmap = bmp;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::GetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h) {
	if( Face ) {
		tGlyphMetrics metrics;
		if( Face->GetGlyphMetricsFromCharcode( ch, metrics) ) {
			w = metrics.CellIncX >> 6;
			h = metrics.CellIncX >> 6;
		}
	}
	if( CurrentFont.Flags & TVP_TF_BOLD ) {
		w += (int)(h / 50) + 1; // calculate bold font size
	}
}
tjs_int FreeTypeFontRasterizer::GetAscentHeight() {
	if( Face ) return Face->GetAscent();
	return 0;
}
tTVPCharacterData* FreeTypeFontRasterizer::GetBitmap( const tTVPFontAndCharacterData & font, tjs_int aofsx, tjs_int aofsy ) {
	tTVPCharacterData* data = NULL;
	if( font.Font.Flags & TVP_TF_BOLD ) {
		data = BoldFace->GetGlyphFromCharcode(font.Character);
	} else {
		data = Face->GetGlyphFromCharcode(font.Character);
	}

	int cx = data->Metrics.CellIncX >> 6;
	int cy = data->Metrics.CellIncY >> 6;
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


