
#ifndef __FONT_CACHE_H__
#define __FONT_CACHE_H__

#include "GLTexture.h"

struct GlyphData {
	tTVPRect position_;	// Texture 内での基準位置、この中に収まるように文字画像をテクスチャに入れる

	tjs_char glyph_;		// 格納されている文字
	tjs_int cellIncX; //!< 一文字進めるの必要なX方向のピクセル数 1/64固定少数
	tjs_int cellIncY; //!< 一文字進めるの必要なY方向のピクセル数 1/64固定少数
	tjs_uint BlackBoxX; //!< 実画像幅 1/64固定少数
	tjs_uint BlackBoxY; //!< 実画像高さ 1/64固定少数
	tjs_int OriginX; //!< 文字Bitmapを描画するascent位置との横オフセット 1/64固定少数
	tjs_int OriginY; //!< 文字Bitmapを描画するascent位置との縦オフセット 1/64固定少数

	tjs_uint index_;	//!< キャッシュ内インデックス
	GlyphData* prev_;	//!< 前の要素
	GlyphData* next_;	//!< 次の要素

	GlyphData() : glyph_(-1) {}
};


// キャッシュといっても、描画前にはテクスチャに描かないといけないので、フォントと一体で使うことになる
// キャッシュ管理方式は、単純に最後に使われたものがリストの先頭に来て
// リストの末尾にあるものから削除(再利用)されていくシンプルな方式
class FontCache {
private:
	GLTexture* texture_;	// 文字画像を保持するテクスチャ
	std::vector<GlyphData>		glyph_cache_;	// グリフデータキャッシュ
	std::map<tjs_char,tjs_uint>	glyph_index_;	// キャッシュ内のインデックスと文字コードの対応

	GlyphData* use_list_;		// 使用中アイテムリスト
	GlyphData* last_item_;		// 使用中の中で一番昔にアクセスされた要素
	GlyphData* remain_list_;	// 未使用アイテムリスト

	FontFace face_;	// フェイス
private:
	/**
	 * キャッシュにない文字をレンダリングして格納する
	 */
	int pushCh( tjs_int ch );
	/**
	 * 未使用キャッシュを得る
	 */
	int findEmpty();
	/**
	 * 一番古いキャッシュを得る
	 */
	int getLastItem();
	/**
	 * 使用中にする
	 */
	void toUseList( GlyphData* data );
	/**
	 * 指定インデックスのグリフ情報を得る
	 */
	const GlyphData& getGlyph( tjs_uint index ) const {
		if( index < glyph_cache_.size() ) {
			return glyph_cache_[index];
		} else {
			throw "Not found glyph";
			return glyph_cache_[0];
		}
	}

public:
	FontCache();
	~FontCache();

	bool loadFaceFromAsset( const char* fontfile, tjs_uint index = 0 );

	void initialize();

	const GlyphData& getGlyphData( tjs_char ch );
};

#endif // __FONT_CACHE_H__
