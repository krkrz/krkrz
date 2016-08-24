
#ifndef __FONT_FACE_H__
#define __FONT_FACE_H__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

class FontFace {
	FT_Face face_;
	tjs_uint32 options_; //!< フラグ

	tjs_uint (*UnicodeToLocalChar)(tjs_char in); //!< SJISなどをUnicodeに変換する関数
	tjs_char (*LocalCharToUnicode)(tjs_uint in); //!< UnicodeをSJISなどに変換する関数

private:
	static unsigned long asset_read_func( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count );
	static void asset_close_func( FT_Stream stream );
	bool openFaceByIndex( tjs_uint index, FT_Face& face, FT_StreamRec& stream );
	bool loadGlyphSlotFromCharcode( tjs_char code );

public:
	FontFace();

	/**
	 * asset からファイル指定で開く
	 * @param fontfile フォルトファイル名
	 * @param index : フォントファイル内のフェイスインデックス
	 */
	bool loadFaceFromAsset( const char* fontfile, tjs_uint index = 0 );

	/**
	 * 詳細なサイズを指定する。
	 * 固定少数で指定できるので画面サイズと想定サイズが違う場合に使いやすい
	 * @param height : 26.6固定少数での高さ(point)を指定
	 * @param hresolution : 横解像度(dpi)
	 * @param vresolution : 縦解像度(dpi)
	 */
	bool setSize( tjs_int height, tjs_uint hresolution=300, tjs_uint vresolution=300 ) {
		return FT_Set_Char_Size( face_, 0, height, hresolution, vresolution ) == 0;
	}

	/**
	 * 文字をレンダリングして、文字のグリフビットマップを得られるようにする
	 * アンチエイリアスなしでレンダリングしている
	 * @param code : 文字コード
	 */
	bool renderGlyph( tjs_char code );

	/**
	 * レンダリングした文字の画像を得る
	 * 事前にrenderGlyphをコールしておくこと
	 */
	const FT_Bitmap* getBitmap() const { return &(face_->glyph->bitmap); }

	/**
	 * レンダリングしたグリフを得る
	 * 事前にrenderGlyphをコールしておくこと
	 */
	const FT_GlyphSlot* getGlyph() const { return &(face_->glyph); }

	/**
	 * Faceを得る
	 */
	const FT_Face getFace() const { return face_; }

	// 26.6固定少数を整数にする
	static inline tjs_int posToInt( tjs_int x ) { return (((x) + (1 << 5)) >> 6); }
	// 整数を26.6固定少数にする
	static inline tjs_int intToPos( tjs_int x ) { return x << 6; }
};

#endif // __FONT_FACE_H__
