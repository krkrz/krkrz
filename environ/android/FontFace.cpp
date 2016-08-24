
#include "tjsCommHead.h"
#include "FontFace.h"
#include "Application.h"
#include "tvpfontstruc.h"
#include "uni_cp932.h"
#include "cp932_uni.h"
#include <android/asset_manager.h>

#include <ft2build.h>
#include FT_TRUETYPE_UNPATENTED_H
#include FT_SYNTHESIS_H
#include FT_SYSTEM_H

FT_Library FreeTypeLibrary = NULL;	//!< FreeType ライブラリ
void TVPInitializeFont() {
	if( FreeTypeLibrary == NULL ) {
		FT_Error err = FT_Init_FreeType( &FreeTypeLibrary );
	}
}
void TVPUninitializeFreeFont() {
	if( FreeTypeLibrary ) {
		FT_Done_FreeType( FreeTypeLibrary );
		FreeTypeLibrary = NULL;
	}
}

unsigned long FontFace::asset_read_func( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count ) {
	AAsset* asset = static_cast<AAsset*>(stream->descriptor.pointer);
	unsigned long result;
	if( count == 0 ) {
		result = 0;
		AAsset_seek( asset, offset, SEEK_SET );
	} else {
		AAsset_seek( asset, offset, SEEK_SET );
		AAsset_read( asset, buffer, count );
		result = count;
	}
	return result;
}
void FontFace::asset_close_func( FT_Stream stream ) {
	AAsset_close(  static_cast<AAsset*>(stream->descriptor.pointer) );
}
//---------------------------------------------------------------------------
/**
 * 指定インデックスのFaceを開く
 * @param index	開くindex
 * @param face	FT_Face 変数への参照
 * @return	Faceを開ければ true そうでなければ false
 * @note	初めて Face を開く場合は face で指定する変数には null を入れておくこと
 */
bool FontFace::openFaceByIndex( tjs_uint index, FT_Face& face, FT_StreamRec& stream )
{
	if( face ) FT_Done_Face(face), face = NULL;

	FT_Parameter parameters[1];
	parameters[0].tag = FT_PARAM_TAG_UNPATENTED_HINTING; // Appleの特許回避を行う
	parameters[0].data = NULL;

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));
	args.flags = FT_OPEN_STREAM;
	args.stream = &stream;
	args.driver = 0;
	args.num_params = 1;
	args.params = parameters;
	FT_Error err = FT_Open_Face( FreeTypeLibrary, &args, index, &face );
	return err == 0;
}

FontFace::FontFace() : face_(NULL) {
}
bool FontFace::loadFaceFromAsset( const char* fontfile, tjs_uint index ) {
	AAsset* asset = AAssetManager_open( Application->getAssetManager(), fontfile, AASSET_MODE_RANDOM );
	if( asset == NULL ) return false;

	off_t len = AAsset_getLength( asset );

	FT_StreamRec fsr;
	fsr.base = 0;
	fsr.size = static_cast<unsigned long>(len);
	fsr.pos = 0;
	fsr.descriptor.pointer = asset;
	fsr.pathname.pointer = NULL;
	fsr.read = asset_read_func;
	fsr.close = asset_close_func;
/*
	tjs_uint face_num = 1;
	FT_Face face = NULL;
	for( tjs_uint i = 0; i < face_num; i++ ) {
		if( !openFaceByIndex( i, face, fsr ) ) {
		} else {
			const char * name = face->family_name;
			face_num = face->num_faces;
		}
	}
	if( face ) FT_Done_Face(face), face = NULL;
*/
	if( openFaceByIndex( index, face_, fsr ) ) {
		if( face_->charmap == NULL ) {
			FT_Error err = FT_Select_Charmap( face_, FT_ENCODING_SJIS );
			if( !err ) {
				// SJIS への切り替えが成功した。変換関数をセットする
				UnicodeToLocalChar = UnicodeToSJIS;
				LocalCharToUnicode = SJISToUnicode;
			} else {
				int numcharmap = face_->num_charmaps;
				for( int i = 0; i < numcharmap; i++ ) {
					FT_Encoding enc = face_->charmaps[i]->encoding;
					if( enc != FT_ENCODING_NONE && enc != FT_ENCODING_APPLE_ROMAN ) {
						err = FT_Select_Charmap( face_, enc );
						if( !err ) {
							break;
						}
					}
				}
			}
		}
	}
}
bool FontFace::loadGlyphSlotFromCharcode( tjs_char code ) {
	// 文字コードを得る
	FT_ULong localcode;
	if( UnicodeToLocalChar == NULL )
		localcode = code;
	else
		localcode = UnicodeToLocalChar( code );

	// 文字コードから index を得る
	FT_UInt glyph_index = FT_Get_Char_Index( face_, localcode );
	if( glyph_index == 0 )
		return false;

	// グリフスロットに文字を読み込む、アンチエイリアスはシェーダーで頑張る
	FT_Int32 load_glyph_flag = FT_LOAD_TARGET_MONO | FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT;
	FT_Error err = FT_Load_Glyph( face_, glyph_index, load_glyph_flag );
	if( err ) return false;

	// フォントの変形を行う
	if( options_ & TVP_TF_BOLD ) FT_GlyphSlot_Embolden( face_->glyph );
	if( options_ & TVP_TF_ITALIC ) FT_GlyphSlot_Oblique( face_->glyph );

	return true;
}
bool FontFace::renderGlyph( tjs_char code ) {
	if( !loadGlyphSlotFromCharcode(code) ) return false;
	FT_Error err;
	if( face_->glyph->format != FT_GLYPH_FORMAT_BITMAP ) {
		err = FT_Render_Glyph( face_->glyph, FT_RENDER_MODE_MONO ); // アンチエイリアスはシェーダーで頑張る
		if( err ) return false;
	}
	return true;
}
#if 0
tTVPGlyphData* FontFace::getGlyphFromCharcode( tjs_char code ) {
	if( !loadGlyphSlotFromCharcode(code) ) return NULL;

	// face_->glyph->advance.x;
	// face_->glyph->advance.y;
	FT_Error err;
	if( face_->glyph->format != FT_GLYPH_FORMAT_BITMAP ) {
		err = FT_Render_Glyph( face_->glyph, FT_RENDER_MODE_MONO ); // アンチエイリアスはシェーダーで頑張る
		if( err ) return NULL;
	}
	
	// ビットマップ形式をチェック
	FT_Bitmap *ft_bmp = &(face_->glyph->bitmap);
	FT_Bitmap new_bmp;
	bool release_ft_bmp = false;
	try {
		if( ft_bmp->rows && ft_bmp->width ) {
			// ビットマップがサイズを持っている場合
			if( ft_bmp->pixel_mode != ft_pixel_mode_grays ) {
				// ft_pixel_mode_grays ではないので ft_pixel_mode_grays 形式に変換する
				FT_Bitmap_New(&new_bmp);
				release_ft_bmp = true;
				ft_bmp = &new_bmp;
				err = FT_Bitmap_Convert( face_->glyph->library, &(face_->glyph->bitmap), &new_bmp, 1 );
				if( err ) {
					if( release_ft_bmp ) FT_Bitmap_Done( face_->glyph->library, ft_bmp );
					return NULL;
				}
			}
			if( ft_bmp->num_grays != 256 ) {
				// gray レベルが 256 ではない時は 256 になるように乗算を行う
				tjs_int32 multiply = static_cast<tjs_int32>((static_cast<tjs_int32> (1) << 30) - 1) / (ft_bmp->num_grays - 1);
				for( tjs_int y = ft_bmp->rows - 1; y >= 0; y-- ) {
					unsigned char * p = ft_bmp->buffer + y * ft_bmp->pitch;
					for( tjs_int x = ft_bmp->width - 1; x >= 0; x-- ) {
						tjs_int32 v = static_cast<tjs_int32>((*p * multiply)  >> 22);
						*p = static_cast<unsigned char>(v);
						p++;
					}
				}
			}
		}
		// 64倍されているものを解除する
		// metrics.CellIncX = FT_PosToInt( metrics.CellIncX );
		// metrics.CellIncY = FT_PosToInt( metrics.CellIncY );

		// tGlyphBitmap を作成して返す
		//int baseline = (int)(face_->height + face_->descender) * face_->size->metrics.y_ppem / face_->units_per_EM;
		int baseline = (int)( face_->ascender ) * face_->size->metrics.y_ppem / face_->units_per_EM;

		glyph_bmp = new tTVPCharacterData(
			ft_bmp->buffer,
			ft_bmp->pitch,
			  face_->glyph->bitmap_left,
			  baseline - face_->glyph->bitmap_top,
			  ft_bmp->width,
			  ft_bmp->rows,
			metrics);
		glyph_bmp->Gray = 256;

		if( Options & TVP_TF_UNDERLINE ) {
			tjs_int pos = -1, thickness = -1;
			GetUnderline( pos, thickness );
			if( pos >= 0 && thickness > 0 ) {
				glyph_bmp->AddHorizontalLine( pos, thickness, 255 );
			}
		}
		if( Options & TVP_TF_STRIKEOUT ) {
			tjs_int pos = -1, thickness = -1;
			GetStrikeOut( pos, thickness );
			if( pos >= 0 && thickness > 0 ) {
				glyph_bmp->AddHorizontalLine( pos, thickness, 255 );
			}
		}
	}
	catch(...)
	{
		if(release_ft_bmp) FT_Bitmap_Done(FTFace->glyph->library, ft_bmp);
		throw;
	}
	if(release_ft_bmp) FT_Bitmap_Done(FTFace->glyph->library, ft_bmp);
}
#endif

