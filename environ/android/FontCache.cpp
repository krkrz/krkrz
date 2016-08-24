

#include "tjsCommHead.h"

#include "FontCache.h"



FontCache::FontCache() : use_list_(NULL), remain_list_(NULL), last_item_(NULL) {
	// テクスチャ生成
	int maxsize = GLTexture::getMaxTextureSize();
	tjs_uint8* temp = new tjs_uint8[maxsize*maxsize];
	texture_ = new GLTexture( maxsize, maxsize, temp, GL_ALPHA );
	delete[] temp;
}
FontCache::~FontCache() {
	delete texture_;
	texture_ = NULL;
}


// アンチエイリアスはシェーダーに任せる
void FontCache::initialize() {
	// 縦と横の格納可能数を計算する
	const FT_Face f = face_.getFace();
	const FT_BBox& box = f.bbox;
	tjs_int gw = FontFace::posToInt( box.xMax - box.xMin ) + 1;
	tjs_int gh = FontFace::posToInt( box.yMax - box.yMin ) + 1;
	int num_x = texture_->width() / gw;
	int num_y = texture_->height() / gh;

	// bool Antialiased;
	// int style_;	// underline/strikeout

	// 初期化
	// 各要素の位置の確定とリンクの接続を行う
	int cacheCount = num_x * num_y;
	glyph_cache_.resize(cacheCount);
	int idx = 0;
	GlyphData* prev = NULL;
	for( int y = 0; y < num_y; y++ ) {
		int posy = y * gh;
		for( int x = 0; x < num_x; x++ ) {
			int posx = x * gw;
			int l = posx;
			int r = l + gw;
			int t = posy;
			int b = t + gh;
			glyph_cache_[idx].position_.set( l, t, r, b );
			glyph_cache_[idx].index_ = idx;
			glyph_cache_[idx].prev_ = prev;
			glyph_cache_[idx].next_ = glyph_cache_[idx+1];
			prev = &(glyph_cache_[idx]);
			idx++;
		}
	}
	glyph_cache_[cacheCount-1].next_ = NULL;
	remain_list_ = &(glyph_cache_[0]);
	use_list_ = NULL;
	last_item_ = NULL;
}
void FontCache::toUseList( GlyphData* data ) {
	if( use_list_ == NULL ) {
		use_list_ = data;
		data->next_ = NULL
		data->prev_ = NULL;
	} else {
		use_list_->prev_ = data;
		data->next_ = use_list_;
		data->prev_ = NULL;
		use_list_ = data;
	}
	if( last_item_ == NULL ) {
		// 使用されたアイテムがまだない時は、今回のを入れる
		last_item_ = data;
	}
}
// 一番古い要素を得る
// 一番古い要素は、使われることになるので、一番新しい要素となる
int FontCache::getLastItem() {
	if( last_item_ ) {
		int result = last_item_->index_;
		assert( last_item_->next_ );
		// 最後の手前の要素が最後の要素になる
		GlyphData* prev = last_item_->prev_;
		prev->next_ = NULL;

		// 現在の最後のアイテムを先頭に移動する
		GlyphData* first = last_item_;
		first->prev_ = NULL;
		first->next_ = use_list_;
		use_list_->prev_ = first;
		use_list_ = first;

		last_item_ = prev;	// 最後の手前だった要素が最後の要素
		return result;
	} else {
		// 使用している古いものがない、プログラムエラー？
		return -1;
	}
}
int FontCache::findEmpty() {
	int result = -1;
	if( remain_list_ != NULL ) {
		GlyphData* first = remain_list_;
		result = remain_list_->index_;

		if( remain_list_->next_ ) {
			// 次の要素がある場合はそれを先頭に
			remain_list_ = remain_list_->next_;
			remain_list_->prev_ = NULL;
		} else {
			remain_list_ = NULL;
		}
		toUseList( first );
	}
	return result;
}
int FontCache::pushCh( tjs_int ch ) {
	int index = findEmpty();
	if( index < 0 ) {
		// 未使用のがない時は、一番古いのを使う
		index = getLastItem();
		if( index >= 0 ) {
			// 使っていたものはmapから消す
			tjs_char ch = glyph_cache_[index].glyph_;
			glyph_index_.erase( ch );
		}
	}
	if( face_.renderGlyph( ch ) ) {
		GlyphData& glyph = glyph_cache_[index];
		
		face_.
		
		// レンダリングしたものをmapへ
		glyph_index_.insert( std::map<tjs_char,tjs_int>::value_type( ch, index ) );
	}
}
const GlyphData& FontCache::getGlyphData( tjs_char ch ) {
	std::map<tjs_char,tjs_int>::iterator i = glyph_index_.find( ch );
	if( i != glyph_index_.end() ) {
		// キャッシュにあるので、それをそのまま使う
		return getGlyph( i->second );
	} else {
		// キャッシュにないので新たにレンダリング
		int index = pushCh( ch );
		if( index < 0 ) {
			throw "font rendaring error";
		}
		return getGlyph( (tjs_uint)index );
	}
}

bool FontCache::loadFaceFromAsset( const char* fontfile, tjs_uint index = 0 ) {
	if( face_.loadFaceFromAsset( fontfile, index ) ) {
		return true;
	}
	return false;
}

/*

	// 実際の画面サイズと想定サイズが違っても、できるだけ実画面サイズで綺麗に描画する
	// realsize : 実際の画面サイズ(幅or高さ)
	// referencesize : 想定している画面サイズ(幅or高さ)
	realsize, referencesize, x, y, fontsize


*/

