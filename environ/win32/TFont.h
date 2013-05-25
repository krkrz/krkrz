

#ifndef __T_FONT_H__
#define __T_FONT_H__


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

		// ƒfƒtƒHƒ‹ƒg‚Ì LOGFONT Žw’è‚µ‚½•û‚ª—Ç‚³‚»‚¤
		HFONT hFont = (HFONT)::GetStockObject( ANSI_FIXED_FONT );
		LOGFONT logfont={0};
		::GetObject( hFont_, sizeof(LOGFONT), &logfont );
		logfont.lfHeight = -12;
		logfont.lfWidth = 0;
		logfont.lfCharSet = SHIFTJIS_CHARSET;
		_tcsncpy_s( logfont.lfFaceName, LF_FACESIZE, _T("‚l‚r ‚oƒSƒVƒbƒN"), LF_FACESIZE );
		logfont.lfItalic = FALSE;
		logfont.lfUnderline = FALSE;
		logfont.lfStrikeOut = FALSE;
		ApplyFont( &logfont );
	}
	~TFont() {
		::SelectObject( hMemDC_, hOldBmp_ );
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

void TVPGetFontList(std::vector<std::string> & list, tjs_uint32 flags, TFont * refcanvas);
#endif // __T_FONT_H__
