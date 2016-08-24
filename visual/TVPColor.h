
#ifndef __TVP_COLOR_H__
#define __TVP_COLOR_H__

enum {
	clScrollBar = 0x80000000,
	clBackground = 0x80000001,
	clActiveCaption = 0x80000002,
	clInactiveCaption = 0x80000003,
	clMenu = 0x80000004,
	clWindow = 0x80000005,
	clWindowFrame = 0x80000006,
	clMenuText = 0x80000007,
	clWindowText = 0x80000008,
	clCaptionText = 0x80000009,
	clActiveBorder = 0x8000000a,
	clInactiveBorder = 0x8000000b,
	clAppWorkSpace = 0x8000000c,
	clHighlight = 0x8000000d,
	clHighlightText = 0x8000000e,
	clBtnFace = 0x8000000f,
	clBtnShadow = 0x80000010,
	clGrayText = 0x80000011,
	clBtnText = 0x80000012,
	clInactiveCaptionText = 0x80000013,
	clBtnHighlight = 0x80000014,
	cl3DDkShadow = 0x80000015,
	cl3DLight = 0x80000016,
	clInfoText = 0x80000017,
	clInfoBk = 0x80000018,
	clNone = 0x1fffffff,
	clAdapt= 0x01ffffff,
	clPalIdx = 0x3000000,
	clAlphaMat = 0x4000000,
};

inline unsigned long ColorToRGB( unsigned long col ) {
#ifdef _WIN32
	if( ((int)col) < 0 ) {
		return ::GetSysColor( (int)(col&0xff) );
	}
	return col;
#else
	switch( col ) {
		case clScrollBar:
			return 0xffc8c8c8;
		case clBackground:
			return 0xff000000;
		case clActiveCaption:
			return 0xff99b4d1;
		case clInactiveCaption:
			return 0xffbfcddb;
		case clMenu:
			return 0xfff0f0f0;
		case clWindow:
			return 0xffffffff;
		case clWindowFrame:
			return 0xff646464;
		case clMenuText:
			return 0xff000000;
		case clWindowText:
			return 0xff000000;
		case clCaptionText:
			return 0xff000000;
		case clActiveBorder:
			return 0xffb4b4b4;
		case clInactiveBorder:
			return 0xfff4f7fc;
		case clAppWorkSpace:
			return 0xffababab;
		case clHighlight:
			return 0xff3399ff;
		case clHighlightText:
			return 0xffffffff;
		case clBtnFace:
			return 0xfff0f0f0;
		case clBtnShadow:
			return 0xffa0a0a0;
		case clGrayText:
			return 0xff6d6d6d;
		case clBtnText:
			return 0xff000000;
		case clInactiveCaptionText:
			return 0xff434e54;
		case clBtnHighlight:
			return 0xffffffff;
		case cl3DDkShadow:
			return 0xff696969;
		case cl3DLight:
			return 0xffe3e3e3;
		case clInfoText:
			return 0xff000000;
		case clInfoBk:
			return 0xffffffe1;
		case clNone: // black for WinNT
			return 0xff000000;
		case clAdapt:
			return clAdapt;
		case clPalIdx:
			return clPalIdx;
		case clAlphaMat:
			return clAlphaMat;
	}
	return col; // unknown, passthru
#endif
}

#endif

