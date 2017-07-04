//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief FreeType フォントドライバ
//---------------------------------------------------------------------------
#ifndef _FREETYPE_H_
#define _FREETYPE_H_


#include "CharacterData.h"
#include "FreeTypeFace.h"
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4819)
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//---------------------------------------------------------------------------
#define	TVP_GET_FACE_INDEX_FROM_OPTIONS(x) ((x) & 0xff) //!< オプション整数からFaceインデックスを取り出すマクロ
#define	TVP_FACE_OPTIONS_FACE_INDEX(x)		((x) & 0xff) //!< Faceインデックスをオプション整数に変換するマクロ
#define	TVP_FACE_OPTIONS_FILE				0x00010000 //!< フォント名ではなくてファイル名によるフォントの指定を行う
#define TVP_FACE_OPTIONS_NO_HINTING			0x00020000 //!< ヒンティングを行わない
#define TVP_FACE_OPTIONS_FORCE_AUTO_HINTING	0x00020000 //!< 強制的に auto hinting を行う
#define TVP_FACE_OPTIONS_NO_ANTIALIASING	0x00040000 //!< アンチエイリアスを行わない

//---------------------------------------------------------------------------
/**
 * 存在しないグリフを補うために複数のフォントを指定可能なように、複数のフェイス情報を保持できるように
 * 各フェイスに必要な情報を保持する。
 */
struct FaceSet {
	/** フォント名 */
	tjs_string FontName;

	/** Face オブジェクト */
	std::unique_ptr<tBaseFreeTypeFace> Face;

	/** FreeType Face オブジェクト */
	FT_Face FTFace;

	typedef std::vector<FT_ULong> tGlyphIndexToCharcodeVector;

	/** グリフインデックスから文字コードへの変換マップ */
	std::unique_ptr<tGlyphIndexToCharcodeVector> GlyphIndexToCharcodeVector;

	/** SJISなどをUnicodeに変換する関数 */
	tjs_uint (*UnicodeToLocalChar)(tjs_char in);

	/** UnicodeをSJISなどに変換する関数 */
	tjs_char (*LocalCharToUnicode)(tjs_uint in);

	FaceSet() : UnicodeToLocalChar(nullptr), LocalCharToUnicode(nullptr) {}
};


//---------------------------------------------------------------------------
/**
 * FreeType フォント face
 */
class tFreeTypeFace
{
/*
	tjs_string FontName;		//!< フォント名
	tBaseFreeTypeFace * Face; //!< Face オブジェクト
	FT_Face FTFace; //!< FreeType Face オブジェクト

	typedef std::vector<FT_ULong> tGlyphIndexToCharcodeVector;
	tGlyphIndexToCharcodeVector * GlyphIndexToCharcodeVector;		//!< グリフインデックスから文字コードへの変換マップ

	tjs_uint (*UnicodeToLocalChar)(tjs_char in); //!< SJISなどをUnicodeに変換する関数
	tjs_char (*LocalCharToUnicode)(tjs_uint in); //!< UnicodeをSJISなどに変換する関数
*/

	tjs_uint32 Options; //!< フラグ
	tjs_int Height;		//!< フォントサイズ(高さ) in pixel

	std::vector<std::unique_ptr<FaceSet> >	Faces;

	static inline tjs_int FT_PosToInt( tjs_int x ) { return (((x) + (1 << 5)) >> 6); }

private:
	void GetUnderline( tjs_int& pos, tjs_int& thickness, tjs_int index ) const {
		FT_Face face = Faces[index]->FTFace;
		tjs_int ppem = face->size->metrics.y_ppem;
		tjs_int upe = face->units_per_EM;
		tjs_int liney = 0; //下線の位置
		tjs_int height = FT_PosToInt( face->size->metrics.height );
		liney = ((face->ascender - face->underline_position) * ppem) / upe;
		thickness = (face->underline_thickness * ppem) / upe;
		if( thickness < 1 ) thickness = 1;
		if( liney > height ) {
			liney = height - 1;
		}
		pos = liney;
	}
	void GetStrikeOut( tjs_int& pos, tjs_int& thickness, tjs_int index ) const {
		FT_Face face = Faces[index]->FTFace;
		tjs_int ppem = face->size->metrics.y_ppem;
		tjs_int upe = face->units_per_EM;
		thickness = face->underline_thickness * ppem / upe;
		if( thickness < 1 ) thickness = 1;
		pos = face->ascender * 7 * ppem / (10 * upe);
	}

public:
	tFreeTypeFace(const std::vector<tjs_string> &fontname, tjs_uint32 options);
	~tFreeTypeFace();

	//tjs_uint GetGlyphCount();
	//tjs_char GetCharcodeFromGlyphIndex(tjs_uint index);

	void GetFaceNameList(std::vector<tjs_string> &dest);

	const tjs_string& GetFontName() const { return Faces[0]->FontName; }

	tjs_int GetHeight() { return Height; }
	void SetHeight(int height);

	void SetOption( tjs_uint32 opt ) {
		Options |= opt;
	}
	void ClearOption( tjs_uint32 opt ) {
		Options &= ~opt;
	}
	bool GetOption( tjs_uint32 opt ) const {
		return (Options&opt) == opt;
	}
	tjs_char GetDefaultChar() const {
		return Faces[0]->Face->GetDefaultChar();
	}
	tjs_char GetFirstChar() {
		FT_UInt gindex;
		return static_cast<tjs_char>( FT_Get_First_Char( Faces[0]->FTFace, &gindex ) );
	}

	tjs_int GetAscent() const {
		FT_Face face = Faces[0]->FTFace;
		tjs_int ppem = face->size->metrics.y_ppem;
		tjs_int upe = face->units_per_EM;
		return face->ascender * ppem / upe;
	}
	tTVPCharacterData * GetGlyphFromCharcode(tjs_char code);
	bool GetGlyphRectFromCharcode(struct tTVPRect& rt, tjs_char code, tjs_int& advancex, tjs_int& advancey );
	bool GetGlyphSizeFromCharcode(tjs_char code, tGlyphMetrics & metrics);

private:
	tjs_int GetGlyphMetricsFromCharcode(tjs_char code, tGlyphMetrics & metrics);
	tjs_int LoadGlyphSlotFromCharcode(tjs_char code);
};
//---------------------------------------------------------------------------

#endif /*_FREETYPE_H_*/
