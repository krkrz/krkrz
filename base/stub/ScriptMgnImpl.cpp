//---------------------------------------------------------------------------
// TJS2 Script Managing
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Hash Map Object を書き出すためのサブプロセスとして起動しているかどうか
// チェックする
// Windows 以外では、ないものとして扱ってもいいか
bool TVPCheckProcessLog() { return false; }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Script system initialization script
//---------------------------------------------------------------------------
static const tjs_nchar * TVPInitTJSScript =
	// note that this script is stored as narrow string
TJS_N("const\
\
/* constants */\
 /* tTVPBorderStyle */ bsNone=0,  bsSingle=1,  bsSizeable=2,  bsDialog=3,  bsToolWindow=4,  bsSizeToolWin=5,\
 /* tTVPUpdateType */ utNormal=0,  utEntire =1,\
 /* tTVPMouseButton */  mbLeft=0,  mbRight=1,  mbMiddle=2, mbX1=3, mbX2=4,\
 /* tTVPMouseCursorState */ mcsVisible=0, mcsTempHidden=1, mcsHidden=2,\
 /* tTVPImeMode */ imDisable=0, imClose=1, imOpen=2, imDontCare=3, imSAlpha=4, imAlpha=5, imHira=6, imSKata=7, imKata=8, imChinese=9, imSHanguel=10, imHanguel=11,\
 /* Set of shift state */  ssShift=(1<<0),  ssAlt=(1<<1),  ssCtrl=(1<<2),  ssLeft=(1<<3),  ssRight=(1<<4),  ssMiddle=(1<<5),  ssDouble =(1<<6),  ssRepeat = (1<<7),\
 /* TVP_FSF_???? */ fsfFixedPitch=1, fsfSameCharSet=2, fsfNoVertical=4, \
	fsfTrueTypeOnly=8, fsfUseFontFace=0x100, fsfIgnoreSymbol=0x10,\
 /* tTVPLayerType */ ltBinder=0, ltCoverRect=1, ltOpaque=1, ltTransparent=2, ltAlpha=2, ltAdditive=3, ltSubtractive=4, ltMultiplicative=5, ltEffect=6, ltFilter=7, ltDodge=8, ltDarken=9, ltLighten=10, ltScreen=11, ltAddAlpha = 12,\
	ltPsNormal = 13, ltPsAdditive = 14, ltPsSubtractive = 15, ltPsMultiplicative = 16, ltPsScreen = 17, ltPsOverlay = 18, ltPsHardLight = 19, ltPsSoftLight = 20, ltPsColorDodge = 21, ltPsColorDodge5 = 22, ltPsColorBurn = 23, ltPsLighten = 24, ltPsDarken = 25, ltPsDifference = 26, ltPsDifference5 = 27, ltPsExclusion = 28, \
 /* tTVPBlendOperationMode */ omPsNormal = ltPsNormal,omPsAdditive = ltPsAdditive,omPsSubtractive = ltPsSubtractive,omPsMultiplicative = ltPsMultiplicative,omPsScreen = ltPsScreen,omPsOverlay = ltPsOverlay,omPsHardLight = ltPsHardLight,omPsSoftLight = ltPsSoftLight,omPsColorDodge = ltPsColorDodge,omPsColorDodge5 = ltPsColorDodge5,omPsColorBurn = ltPsColorBurn,omPsLighten = ltPsLighten,omPsDarken = ltPsDarken,omPsDifference = ltPsDifference,omPsDifference5 = ltPsDifference5,omPsExclusion = ltPsExclusion, \
	omAdditive=ltAdditive, omSubtractive=ltSubtractive, omMultiplicative=ltMultiplicative, omDodge=ltDodge, omDarken=ltDarken, omLighten=ltLighten, omScreen=ltScreen, omAddAlpha=ltAddAlpha, omOpaque=ltOpaque, omAlpha=ltAlpha, omAuto = 128,\
 /* tTVPDrawFace */ dfBoth=0, dfAlpha = dfBoth, dfAddAlpha = 4, dfMain=1, dfOpaque = dfMain, dfMask=2, dfProvince=3, dfAuto=128,\
 /* tTVPHitType */ htMask=0, htProvince=1,\
 /* tTVPScrollTransFrom */ sttLeft=0, sttTop=1, sttRight=2, sttBottom=3,\
 /* tTVPScrollTransStay */ ststNoStay=0, ststStayDest=1, ststStaySrc=2, \
 /* tTVPKAGDebugLevel */ tkdlNone=0, tkdlSimple=1, tkdlVerbose=2, \
 /* tTVPAsyncTriggerMode */	atmNormal=0, atmExclusive=1, atmAtIdle=2, \
 /* tTVPBBStretchType */ stNearest=0, stFastLinear=1, stLinear=2, stCubic=3, stSemiFastLinear = 4, stFastCubic = 5, stLanczos2 = 6, stFastLanczos2 = 7, stLanczos3 = 8, stFastLanczos3 = 9, stSpline16 = 10, stFastSpline16 = 11, stSpline36 = 12, stFastSpline36 = 13, stAreaAvg = 14, stFastAreaAvg = 15, stGaussian = 16, stFastGaussian = 17, stBlackmanSinc = 18, stFastBlackmanSinc = 19, stRefNoClip = 0x10000,\
 /* tTVPClipboardFormat */ cbfText = 1,\
 /* TVP_COMPACT_LEVEL_???? */ clIdle = 5, clDeactivate = 10, clMinimize = 15, clAll = 100,\
 /* tTVPVideoOverlayMode Add: T.Imoto */ vomOverlay=0, vomLayer=1, vomMixer=2, vomMFEVR=3,\
 /* tTVPPeriodEventReason */ perLoop = 0, perPeriod = 1, perPrepare = 2, perSegLoop = 3,\
 /* tTVPSoundGlobalFocusMode */ sgfmNeverMute = 0, sgfmMuteOnMinimize = 1, sgfmMuteOnDeactivate = 2,\
 /* tTVPTouchDevice */ tdNone=0, tdIntegratedTouch=0x01, tdExternalTouch=0x02, tdIntegratedPen=0x04, tdExternalPen=0x08, tdMultiInput=0x40, tdDigitizerReady=0x80,\
    tdMouse=0x0100, tdMouseWheel=0x0200,\
 /* Display Orientation */ oriUnknown=0, oriPortrait=1, oriLandscape=2,\
\
/* file attributes */\
 faReadOnly=0x01, faHidden=0x02, faSysFile=0x04, faVolumeID=0x08, faDirectory=0x10, faArchive=0x20, faAnyFile=0x3f,\
/* mouse cursor constants */\
 crDefault = 0x0,\
 crNone = -1,\
 crArrow = -2,\
 crCross = -3,\
 crIBeam = -4,\
 crSize = -5,\
 crSizeNESW = -6,\
 crSizeNS = -7,\
 crSizeNWSE = -8,\
 crSizeWE = -9,\
 crUpArrow = -10,\
 crHourGlass = -11,\
 crDrag = -12,\
 crNoDrop = -13,\
 crHSplit = -14,\
 crVSplit = -15,\
 crMultiDrag = -16,\
 crSQLWait = -17,\
 crNo = -18,\
 crAppStart = -19,\
 crHelp = -20,\
 crHandPoint = -21,\
 crSizeAll = -22,\
 crHBeam = 1,\
/* color constants */\
 clScrollBar = 0x80000000,\
 clBackground = 0x80000001,\
 clActiveCaption = 0x80000002,\
 clInactiveCaption = 0x80000003,\
 clMenu = 0x80000004,\
 clWindow = 0x80000005,\
 clWindowFrame = 0x80000006,\
 clMenuText = 0x80000007,\
 clWindowText = 0x80000008,\
 clCaptionText = 0x80000009,\
 clActiveBorder = 0x8000000a,\
 clInactiveBorder = 0x8000000b,\
 clAppWorkSpace = 0x8000000c,\
 clHighlight = 0x8000000d,\
 clHighlightText = 0x8000000e,\
 clBtnFace = 0x8000000f,\
 clBtnShadow = 0x80000010,\
 clGrayText = 0x80000011,\
 clBtnText = 0x80000012,\
 clInactiveCaptionText = 0x80000013,\
 clBtnHighlight = 0x80000014,\
 cl3DDkShadow = 0x80000015,\
 cl3DLight = 0x80000016,\
 clInfoText = 0x80000017,\
 clInfoBk = 0x80000018,\
 clNone = 0x1fffffff,\
 clAdapt= 0x01ffffff,\
 clPalIdx = 0x3000000,\
 clAlphaMat = 0x4000000,\
/* for Menu.trackPopup (see winuser.h) */\
 tpmLeftButton      = 0x0000,\
 tpmRightButton     = 0x0002,\
 tpmLeftAlign       = 0x0000,\
 tpmCenterAlign     = 0x0004,\
 tpmRightAlign      = 0x0008,\
 tpmTopAlign        = 0x0000,\
 tpmVCenterAlign    = 0x0010,\
 tpmBottomAlign     = 0x0020,\
 tpmHorizontal      = 0x0000,\
 tpmVertical        = 0x0040,\
 tpmNoNotify        = 0x0080,\
 tpmReturnCmd       = 0x0100,\
 tpmRecurse         = 0x0001,\
 tpmHorPosAnimation = 0x0400,\
 tpmHorNegAnimation = 0x0800,\
 tpmVerPosAnimation = 0x1000,\
 tpmVerNegAnimation = 0x2000,\
 tpmNoAnimation     = 0x4000,\
/* for Pad.showScrollBars (see Vcl/stdctrls.hpp :: enum TScrollStyle) */\
 ssNone       = 0,\
 ssHorizontal = 1,\
 ssVertical   = 2,\
 ssBoth       = 3,\
/* virtual keycodes */\
 VK_LBUTTON =0x01,\
 VK_RBUTTON =0x02,\
 VK_CANCEL =0x03,\
 VK_MBUTTON =0x04,\
 VK_BACK =0x08,\
 VK_TAB =0x09,\
 VK_CLEAR =0x0C,\
 VK_RETURN =0x0D,\
 VK_SHIFT =0x10,\
 VK_CONTROL =0x11,\
 VK_MENU =0x12,\
 VK_PAUSE =0x13,\
 VK_CAPITAL =0x14,\
 VK_KANA =0x15,\
 VK_HANGEUL =0x15,\
 VK_HANGUL =0x15,\
 VK_JUNJA =0x17,\
 VK_FINAL =0x18,\
 VK_HANJA =0x19,\
 VK_KANJI =0x19,\
 VK_ESCAPE =0x1B,\
 VK_CONVERT =0x1C,\
 VK_NONCONVERT =0x1D,\
 VK_ACCEPT =0x1E,\
 VK_MODECHANGE =0x1F,\
 VK_SPACE =0x20,\
 VK_PRIOR =0x21,\
 VK_NEXT =0x22,\
 VK_END =0x23,\
 VK_HOME =0x24,\
 VK_LEFT =0x25,\
 VK_UP =0x26,\
 VK_RIGHT =0x27,\
 VK_DOWN =0x28,\
 VK_SELECT =0x29,\
 VK_PRINT =0x2A,\
 VK_EXECUTE =0x2B,\
 VK_SNAPSHOT =0x2C,\
 VK_INSERT =0x2D,\
 VK_DELETE =0x2E,\
 VK_HELP =0x2F,\
 VK_0 =0x30,\
 VK_1 =0x31,\
 VK_2 =0x32,\
 VK_3 =0x33,\
 VK_4 =0x34,\
 VK_5 =0x35,\
 VK_6 =0x36,\
 VK_7 =0x37,\
 VK_8 =0x38,\
 VK_9 =0x39,\
 VK_A =0x41,\
 VK_B =0x42,\
 VK_C =0x43,\
 VK_D =0x44,\
 VK_E =0x45,\
 VK_F =0x46,\
 VK_G =0x47,\
 VK_H =0x48,\
 VK_I =0x49,\
 VK_J =0x4A,\
 VK_K =0x4B,\
 VK_L =0x4C,\
 VK_M =0x4D,\
 VK_N =0x4E,\
 VK_O =0x4F,\
 VK_P =0x50,\
 VK_Q =0x51,\
 VK_R =0x52,\
 VK_S =0x53,\
 VK_T =0x54,\
 VK_U =0x55,\
 VK_V =0x56,\
 VK_W =0x57,\
 VK_X =0x58,\
 VK_Y =0x59,\
 VK_Z =0x5A,\
 VK_LWIN =0x5B,\
 VK_RWIN =0x5C,\
 VK_APPS =0x5D,\
 VK_NUMPAD0 =0x60,\
 VK_NUMPAD1 =0x61,\
 VK_NUMPAD2 =0x62,\
 VK_NUMPAD3 =0x63,\
 VK_NUMPAD4 =0x64,\
 VK_NUMPAD5 =0x65,\
 VK_NUMPAD6 =0x66,\
 VK_NUMPAD7 =0x67,\
 VK_NUMPAD8 =0x68,\
 VK_NUMPAD9 =0x69,\
 VK_MULTIPLY =0x6A,\
 VK_ADD =0x6B,\
 VK_SEPARATOR =0x6C,\
 VK_SUBTRACT =0x6D,\
 VK_DECIMAL =0x6E,\
 VK_DIVIDE =0x6F,\
 VK_F1 =0x70,\
 VK_F2 =0x71,\
 VK_F3 =0x72,\
 VK_F4 =0x73,\
 VK_F5 =0x74,\
 VK_F6 =0x75,\
 VK_F7 =0x76,\
 VK_F8 =0x77,\
 VK_F9 =0x78,\
 VK_F10 =0x79,\
 VK_F11 =0x7A,\
 VK_F12 =0x7B,\
 VK_F13 =0x7C,\
 VK_F14 =0x7D,\
 VK_F15 =0x7E,\
 VK_F16 =0x7F,\
 VK_F17 =0x80,\
 VK_F18 =0x81,\
 VK_F19 =0x82,\
 VK_F20 =0x83,\
 VK_F21 =0x84,\
 VK_F22 =0x85,\
 VK_F23 =0x86,\
 VK_F24 =0x87,\
 VK_NUMLOCK =0x90,\
 VK_SCROLL =0x91,\
 VK_LSHIFT =0xA0,\
 VK_RSHIFT =0xA1,\
 VK_LCONTROL =0xA2,\
 VK_RCONTROL =0xA3,\
 VK_LMENU =0xA4,\
 VK_RMENU =0xA5,\
/* VK_PADXXXX are KIRIKIRI specific */\
 VK_PADLEFT =0x1B5,\
 VK_PADUP =0x1B6,\
 VK_PADRIGHT =0x1B7,\
 VK_PADDOWN =0x1B8,\
 VK_PAD1 =0x1C0,\
 VK_PAD2 =0x1C1,\
 VK_PAD3 =0x1C2,\
 VK_PAD4 =0x1C3,\
 VK_PAD5 =0x1C4,\
 VK_PAD6 =0x1C5,\
 VK_PAD7 =0x1C6,\
 VK_PAD8 =0x1C7,\
 VK_PAD9 =0x1C8,\
 VK_PAD10 =0x1C9,\
 VK_PADANY = 0x1DF,\
 VK_PROCESSKEY =0xE5,\
 VK_ATTN =0xF6,\
 VK_CRSEL =0xF7,\
 VK_EXSEL =0xF8,\
 VK_EREOF =0xF9,\
 VK_PLAY =0xFA,\
 VK_ZOOM =0xFB,\
 VK_NONAME =0xFC,\
 VK_PA1 =0xFD,\
 VK_OEM_CLEAR =0xFE,\
 frFreeType=0,\
 frGDI=1,\
/* graphic cache system */\
 gcsAuto=-1,\
/* image 'mode' tag (mainly is generated by image format converter) constants */\
 imageTagLayerType = %[\
opaque		:%[type:ltOpaque			],\
rect		:%[type:ltOpaque			],\
alpha		:%[type:ltAlpha				],\
transparent	:%[type:ltAlpha				],\
addalpha	:%[type:ltAddAlpha			],\
add			:%[type:ltAdditive			],\
sub			:%[type:ltSubtractive		],\
mul			:%[type:ltMultiplicative	],\
dodge		:%[type:ltDodge				],\
darken		:%[type:ltDarken			],\
lighten		:%[type:ltLighten			],\
screen		:%[type:ltScreen			],\
psnormal	:%[type:ltPsNormal			],\
psadd		:%[type:ltPsAdditive		],\
pssub		:%[type:ltPsSubtractive		],\
psmul		:%[type:ltPsMultiplicative	],\
psscreen	:%[type:ltPsScreen			],\
psoverlay	:%[type:ltPsOverlay			],\
pshlight	:%[type:ltPsHardLight		],\
psslight	:%[type:ltPsSoftLight		],\
psdodge		:%[type:ltPsColorDodge		],\
psdodge5	:%[type:ltPsColorDodge5		],\
psburn		:%[type:ltPsColorBurn		],\
pslighten	:%[type:ltPsLighten			],\
psdarken	:%[type:ltPsDarken			],\
psdiff		:%[type:ltPsDifference		],\
psdiff5		:%[type:ltPsDifference5		],\
psexcl		:%[type:ltPsExclusion		],\
],\
/* draw thread num */\
 dtnAuto=0\
;");
//---------------------------------------------------------------------------
ttstr TVPGetSystemInitializeScript()
{
	return ttstr(TVPInitTJSScript);
}
//---------------------------------------------------------------------------
