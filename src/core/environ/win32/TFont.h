

#ifndef __T_FONT_H__
#define __T_FONT_H__
#include "FontSystem.h"

extern FontSystem* TVPFontSystem;

class TFont {
	HFONT hFont_;
	HFONT hOldFont_;
	HDC hMemDC_;
	HBITMAP hBmp_;
	HBITMAP hOldBmp_;

public:
	TFont() : hFont_(INVALID_HANDLE_VALUE), hOldFont_(INVALID_HANDLE_VALUE), hMemDC_(INVALID_HANDLE_VALUE),
		hBmp_(INVALID_HANDLE_VALUE), hOldBmp_(INVALID_HANDLE_VALUE) {
		BITMAPINFO bmpinfo;
		ZeroMemory( &bmpinfo, sizeof(bmpinfo) );
		bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpinfo.bmiHeader.biBitCount = 32;
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biWidth = 32;
		bmpinfo.bmiHeader.biHeight = 32;

		hMemDC_ = ::CreateCompatibleDC( NULL );
		char * Bits;
		hBmp_ = ::CreateDIBSection( NULL, &bmpinfo, DIB_RGB_COLORS, (void **)(&Bits), NULL, 0 );
		hOldBmp_ = ::SelectObject( hMemDC_, hBmp_ );

		// デフォルトの LOGFONT 指定した方が良さそう
		HFONT hFont = (HFONT)::GetStockObject( ANSI_FIXED_FONT );
		LOGFONT logfont={0};
		::GetObject( hFont_, sizeof(LOGFONT), &logfont );
		logfont.lfHeight = -12;
		logfont.lfWidth = 0;
		logfont.lfCharSet = SHIFTJIS_CHARSET;
		TJS_strncpy_s( logfont.lfFaceName, LF_FACESIZE, L"ＭＳ Ｐゴシック", LF_FACESIZE );
		logfont.lfItalic = FALSE;
		logfont.lfUnderline = FALSE;
		logfont.lfStrikeOut = FALSE;
		ApplyFont( &logfont );
	}
	TFont( const tTVPFont &font ) : hFont_(INVALID_HANDLE_VALUE), hOldFont_(INVALID_HANDLE_VALUE), hMemDC_(INVALID_HANDLE_VALUE),
		hBmp_(INVALID_HANDLE_VALUE), hOldBmp_(INVALID_HANDLE_VALUE) {
		BITMAPINFO bmpinfo;
		ZeroMemory( &bmpinfo, sizeof(bmpinfo) );
		bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpinfo.bmiHeader.biBitCount = 32;
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biWidth = 32;
		bmpinfo.bmiHeader.biHeight = 32;

		hMemDC_ = ::CreateCompatibleDC( NULL );
		char * Bits;
		hBmp_ = ::CreateDIBSection( NULL, &bmpinfo, DIB_RGB_COLORS, (void **)(&Bits), NULL, 0 );
		hOldBmp_ = ::SelectObject( hMemDC_, hBmp_ );

		// デフォルトの LOGFONT 指定した方が良さそう
		HFONT hFont = (HFONT)::GetStockObject( ANSI_FIXED_FONT );
		LOGFONT LogFont={0};
		LogFont.lfHeight = -std::abs(font.Height);
		LogFont.lfItalic = (font.Flags & TVP_TF_ITALIC) ? TRUE:FALSE;
		LogFont.lfWeight = (font.Flags & TVP_TF_BOLD) ? 700 : 400;
		LogFont.lfUnderline = (font.Flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
		LogFont.lfStrikeOut = (font.Flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
		LogFont.lfEscapement = LogFont.lfOrientation = font.Angle;
		LogFont.lfCharSet = SHIFTJIS_CHARSET; // TODO: i18n
		LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		LogFont.lfQuality = DEFAULT_QUALITY;
		LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		std::wstring face = TVPFontSystem->GetBeingFont(font.Face.AsStdString());
		TJS_strncpy_s(LogFont.lfFaceName, LF_FACESIZE, face.c_str(), LF_FACESIZE -1);
		LogFont.lfFaceName[LF_FACESIZE-1] = 0;

		ApplyFont( &LogFont );
	}
	~TFont() {
		::SelectObject( hMemDC_, hOldBmp_ );
		if( INVALID_HANDLE_VALUE != hOldFont_ ) {
			::SelectObject( hMemDC_, hOldFont_ );
		}
		if( hFont_ != INVALID_HANDLE_VALUE ) {
			::DeleteObject( hFont_ );
		}
		::DeleteObject( hBmp_ );
		::DeleteDC( hMemDC_ );
	}

	int GetAscentHeight() {
		int otmSize = ::GetOutlineTextMetrics( hMemDC_, 0, NULL );
		char *otmBuf = new char[otmSize];
		OUTLINETEXTMETRIC *otm = (OUTLINETEXTMETRIC*)otmBuf;
		::GetOutlineTextMetrics( hMemDC_, otmSize, otm );
		int result = otm->otmAscent;
		delete[] otmBuf;
		return result;
	}

	void Assign( const TFont* font ) {
		LOGFONT logfont = {0};
		font->GetFont( &logfont );
		ApplyFont( &logfont );
	}
	void Assign( const tTVPFont &font ) {
		LOGFONT LogFont={0};
		LogFont.lfHeight = -std::abs(font.Height);
		LogFont.lfItalic = (font.Flags & TVP_TF_ITALIC) ? TRUE:FALSE;
		LogFont.lfWeight = (font.Flags & TVP_TF_BOLD) ? 700 : 400;
		LogFont.lfUnderline = (font.Flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
		LogFont.lfStrikeOut = (font.Flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
		LogFont.lfEscapement = LogFont.lfOrientation = font.Angle;
		LogFont.lfCharSet = SHIFTJIS_CHARSET; // TODO: i18n
		LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		LogFont.lfQuality = DEFAULT_QUALITY;
		LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		std::wstring face = TVPFontSystem->GetBeingFont(font.Face.AsStdString());
		TJS_strncpy_s(LogFont.lfFaceName, LF_FACESIZE, face.c_str(), LF_FACESIZE -1);
		LogFont.lfFaceName[LF_FACESIZE-1] = 0;
		ApplyFont( &LogFont );
	}
	void ApplyFont( const LOGFONT* info ) {
		HFONT hFont = ::CreateFontIndirect( info );
		if( hFont_ != INVALID_HANDLE_VALUE ) {
			HFONT hOld = ::SelectObject( hMemDC_, hFont );
			//assert( hOld == hFont_ );
			::DeleteObject( hOld );
			hFont_ = hFont;
		} else {
			hOldFont_ = ::SelectObject( hMemDC_, hFont );
			hFont_ = hFont;
		}
	}
	void GetFont( LOGFONT* font ) const {
		::GetObject( hFont_, sizeof(LOGFONT), font );
	}
	/*
	void GetGlyph( UINT code ) {
		GLYPHMETRICS gm;
		ZeroMemory(&gm, sizeof(gm));
		MAT2 *transmat;
		DWORD size = ::GetGlyphOutline( hMemDC_, code, GGO_GRAY8_BITMAP, 0, NULL, transmat );
	}
	*/
	HDC GetDC() { return hMemDC_; }
};

void TVPGetFontList(std::vector<std::wstring> & list, tjs_uint32 flags, const tTVPFont & font );
#endif // __T_FONT_H__
