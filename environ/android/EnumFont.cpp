
#include "tjsCommHead.h"

#include <stdio.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <android/log.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_UNPATENTED_H
#include FT_SYNTHESIS_H
#include FT_BITMAP_H
#include FT_SYSTEM_H


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "krkrz", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "krkrz", __VA_ARGS__))


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
static unsigned long read_func( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count ) {
	FILE* fp = static_cast<FILE*>(stream->descriptor.pointer);
	unsigned long result;
	if( count == 0 ) {
		result = 0;
		fseek( fp, offset, SEEK_SET );
	} else {
		fseek( fp, offset, SEEK_SET );
		fread( buffer, 1, count, fp );
		result = count;
	}
	return result;
}
static void close_func( FT_Stream stream ) {
	fclose( static_cast<FILE*>(stream->descriptor.pointer) );
}
//---------------------------------------------------------------------------
/**
 * 指定インデックスのFaceを開く
 * @param index	開くindex
 * @param face	FT_Face 変数への参照
 * @return	Faceを開ければ true そうでなければ false
 * @note	初めて Face を開く場合は face で指定する変数には null を入れておくこと
 */
bool OpenFaceByIndex( tjs_uint index, FT_Face& face, FT_StreamRec& stream )
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
struct Face {
	std::string file_name_;
	std::string family_name_;
	std::string style_name_;
	long face_flags_;
	long style_flags_;
	int face_index_;
	int num_glyphs_;
};
static void read_face_names( const char* filename ) {
	std::string path(std::string("/system/fonts/") + std::string(filename));
	FILE* fp = fopen( path.c_str(), "rb" );
	if( fp == NULL ) return;

	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );

	FT_StreamRec fsr;
	fsr.base = 0;
	fsr.size = static_cast<unsigned long>(len);
	fsr.pos = 0;
	fsr.descriptor.pointer = fp;
	fsr.pathname.pointer = NULL;
	fsr.read = read_func;
	fsr.close = close_func;

	std::vector<std::string> FaceNames;
	// Face をそれぞれ開き、Face名を取得して FaceNames に格納する
	tjs_uint face_num = 1;
	FT_Face face = NULL;
	for( tjs_uint i = 0; i < face_num; i++ ) {
		if( !OpenFaceByIndex( i, face, fsr ) ) {
			FaceNames.push_back(std::string());
		} else {
			const char * name = face->family_name;
			FaceNames.push_back( std::string(name) );
			face_num = face->num_faces;
		}
	}
	if( face ) FT_Done_Face(face), face = NULL;

	LOGI( "Font file : %s\n", filename );
	std::vector<std::string>::iterator i = FaceNames.begin();
	for( ; i != FaceNames.end(); ++i ) {
		LOGI( "  Face name : %s\n", i->c_str() );
	}
	// FT_ENCODING_SJIS or FT_ENCODING_UNICODE
	/*
	非漢字：1183文字 ＋ 39文字分の未使用領域
	・第１水準漢字：2965文字
	・第２水準漢字：3390文字
	7538 この辺りのグリフ数以上が候補
	この中で日本語グリフが得られるものを後で振り分けるのが無難か
	いくつかの文字でグリフ取得できるか試すのが無難かなぁ
	
	フォントリスト取得する時にリストアップしていなかったらそこで初めてリストアップ。
	デフォルトは、assetsに入れてそれを使うのが無難か
	*/
}

/**
 * 以下のようにして、ttfファイルを取得すれば、フォントを使えそうではある。
 * ただ、全部開いてフェイス名キャッシュして～となると、少し時間かかりそう。
 * アーカイブ内にフォント入れて、それを読んで表示するのが確実か。
 * 初回起動時に調べて、以降キャッシュするのが確実かな
 */
void print_font_files() {
	TVPInitializeFont();
	DIR *dir;
	if( (dir=opendir("/system/fonts")) != NULL) {
		struct dirent *dp;
		for( dp = readdir(dir); dp != NULL; dp = readdir(dir) ) {
			if( dp->d_type == DT_REG ) {
				read_face_names( dp->d_name );
			}
		}
		closedir(dir);
	}
	TVPUninitializeFreeFont();
}



