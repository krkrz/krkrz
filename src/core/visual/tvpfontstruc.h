//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "tTVPFont" definition
//---------------------------------------------------------------------------

#ifndef __TVPFONTSTRUC_H__
#define __TVPFONTSTRUC_H__

//---------------------------------------------------------------------------
// tTVPFont definition
//---------------------------------------------------------------------------
struct tTVPFont
{
	tjs_int Height; // height of text
	tjs_uint32 Flags;
	tjs_int Angle; // rotation angle ( in tenths of degrees ) 0 .. 1800 .. 3600

	ttstr Face; // font name

	bool operator == (const tTVPFont & rhs) const
	{
		return Height == rhs.Height &&
			Flags == rhs.Flags &&
			Angle == rhs.Angle && 
			Face == rhs.Face;
	}
};


/*[*/
//---------------------------------------------------------------------------
// font ralated constants
//---------------------------------------------------------------------------
#define TVP_TF_ITALIC    0x01
#define TVP_TF_BOLD      0x02
#define TVP_TF_UNDERLINE 0x04
#define TVP_TF_STRIKEOUT 0x08


//---------------------------------------------------------------------------
#define TVP_FSF_FIXEDPITCH   1      // fsfFixedPitch
#define TVP_FSF_SAMECHARSET  2      // fsfSameCharSet
#define TVP_FSF_NOVERTICAL   4      // fsfNoVertical
#define TVP_FSF_TRUETYPEONLY 8      // fsfTrueTypeOnly
#define TVP_FSF_USEFONTFACE  0x100  // fsfUseFontFace

/*]*/

//---------------------------------------------------------------------------
#endif
