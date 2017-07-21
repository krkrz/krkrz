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

#include "tjsCommHead.h"
#include "FreeType.h"
#ifdef _WIN32
#include "NativeFreeTypeFace.h"
#endif
#include "uni_cp932.h"
#include "cp932_uni.h"

#include "BinaryStream.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"
#include "ComplexRect.h"

#include <algorithm>
#include <vector>
#include <map>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4819)
#endif
#include <ft2build.h>
#include FT_TRUETYPE_UNPATENTED_H
#include FT_SYNTHESIS_H
#include FT_BITMAP_H
#ifdef _MSC_VER
#pragma warning(pop)
#endif

extern bool TVPEncodeUTF8ToUTF16( tjs_string &output, const std::string &source );

//---------------------------------------------------------------------------

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
/**
 * path と filename はどちらかしか入っていない
 * path は Android でシステムフォントを追加するために存在する
 * filename はアーカイブやassetから追加された時に保持される
 * つまり、システムフォントとアーカイブ/assetのフォントは別物扱いされる
 */
struct FontInfo {
	tjs_string	facename;
	std::string	path;		// 
	tjs_string	filename;
	tjs_uint	index;
	std::string	stylename;
	tjs_uint	num_glyphs;
	tjs_uint	style_flags;
	tjs_uint	face_flags;
	tjs_uint	enc_flags;
	FontInfo() : index( 0 ), num_glyphs( 0 ), style_flags( 0 ), face_flags( 0 ), enc_flags( 0 ) {}
};
enum TVPEncodingFlags {
	TVP_ENC_SJIS			= 0x01 << 0,
	TVP_ENC_UNICODE			= 0x01 << 1,
	TVP_ENC_NONE			= 0x01 << 2,
	TVP_ENC_MS_SYMBOL		= 0x01 << 3,
	TVP_ENC_GB2312			= 0x01 << 4,
	TVP_ENC_BIG5			= 0x01 << 5,
	TVP_ENC_WANSUNG			= 0x01 << 6,
	TVP_ENC_JOHAB			= 0x01 << 7,
	TVP_ENC_ADOBE_STANDARD	= 0x01 << 8,
	TVP_ENC_ADOBE_EXPERT	= 0x01 << 9,
	TVP_ENC_ADOBE_CUSTOM	= 0x01 << 10,
	TVP_ENC_ADOBE_LATIN_1	= 0x01 << 11,
	TVP_ENC_APPLE_ROMAN		= 0x01 << 12
};
struct FaceKey {
	tjs_string name_;
	tjs_uint flag_;
	FaceKey( const tjs_string& n, tjs_uint f ) : name_(n), flag_(f&(TVP_TF_ITALIC|TVP_TF_BOLD)) {}
	bool operator < (const FaceKey& rhs) const {
		if( name_ == rhs.name_ ) {
			return flag_ < rhs.flag_;
		} else {
			return name_ < rhs.name_;
		}
	}
	bool operator == ( const FaceKey& rhs) const {
		return (name_ == rhs.name_) && (flag_==rhs.flag_);
	}
};
class tTVPFreeTypeFaceList {
	std::vector<FontInfo*>	faces_;
	std::vector<FontInfo*>	systemfaces_;
	std::map<FaceKey,FontInfo*>	systemfont_;

private:
	static bool CheckFaceFlag( FontInfo& face, tjs_uint flag );

public:
	~tTVPFreeTypeFaceList();
	bool LoadFont( tjs_string filename, std::vector<tjs_string>* faces );
	void LoadSystemFont( std::string path, std::vector<tjs_string>* faces );
	void GetSystemFontList( std::vector<tjs_string>& faces );

	tBaseFreeTypeFace* GetFace( const tjs_string& facename, tjs_uint32 options ) const;
	void GetFontList(std::vector<ttstr> & list, tjs_uint32 flags, const tTVPFont & font );
};

//---------------------------------------------------------------------------
/**
 * ファイルシステム経由でのFreeType Face クラス
 */
class tGenericFreeTypeFace : public tBaseFreeTypeFace
{
protected:
	FT_Face Face;	//!< FreeType face オブジェクト
	tTJSBinaryStream* File;	 //!< tTJSBinaryStream オブジェクト
	std::vector<tjs_string> FaceNames; //!< Face名を列挙した配列
	//std::unique_ptr<tjs_uint8[]> FontImage;

private:
	FT_StreamRec Stream;

public:
	tGenericFreeTypeFace(const ttstr &fontname, tjs_uint32 options);
	tGenericFreeTypeFace(const std::string& path, const ttstr &fontname, tjs_uint32 index );
	tGenericFreeTypeFace(const ttstr &filename, std::vector<FontInfo*>& fonst, std::vector<tjs_string>* faces );
	virtual ~tGenericFreeTypeFace();

	virtual FT_Face GetFTFace() const;
	virtual void GetFaceNameList(std::vector<tjs_string> & dest) const;
	virtual tjs_char GetDefaultChar() const { return TJS_W(' '); }

private:
	void Clear();
	static unsigned long IoFunc( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count );
	static void CloseFunc( FT_Stream  stream );

	bool OpenFaceByIndex(tjs_uint index, FT_Face & face);
};
//---------------------------------------------------------------------------



static tTVPFreeTypeFaceList* FreeTypeFaceList = nullptr;
static inline void TVPInitializeFaceList() {
	if( FreeTypeFaceList == nullptr ) {
		FreeTypeFaceList = new tTVPFreeTypeFaceList();
	}
}
static void TVPClearFaceList() { if( FreeTypeFaceList ) delete FreeTypeFaceList, FreeTypeFaceList = nullptr; }
static tTVPAtExit TVPClearFaceListAtExit
	(TVP_ATEXIT_PRI_SHUTDOWN, TVPClearFaceList);
//---------------------------------------------------------------------------
bool TVPAddFontToFreeType( const ttstr& storage, std::vector<tjs_string>* faces ) {
	TVPInitializeFaceList();
	return FreeTypeFaceList->LoadFont( storage.AsStdString(), faces );
}
//---------------------------------------------------------------------------
void TVPAddSystemFontToFreeType( const std::string& storage, std::vector<tjs_string>* faces ) {
	TVPInitializeFaceList();
	FreeTypeFaceList->LoadSystemFont( storage, faces );
}
//---------------------------------------------------------------------------
void TVPGetFontListFromFreeType(std::vector<ttstr> & list, tjs_uint32 flags, const tTVPFont & font ) {
	TVPInitializeFaceList();
	FreeTypeFaceList->GetFontList( list, flags, font );
}
//---------------------------------------------------------------------------
void TVPGetSystemFontListFromFreeType( std::vector<tjs_string>& faces ) {
	TVPInitializeFaceList();
	FreeTypeFaceList->GetSystemFontList( faces );
}
//---------------------------------------------------------------------------
static void TVPLoadFont( FT_Open_Args& arg, std::vector<FontInfo*>& fonts, std::map<FaceKey,FontInfo*>* fontmap, std::vector<tjs_string>* faces, const std::string* path, const tjs_string* filename ) {
	FT_Face face = nullptr;
	tjs_uint face_num = 1;
	for( tjs_uint f = 0; f < face_num; f++ ) {
		FT_Error err = FT_Open_Face( FreeTypeLibrary, &arg, f, &face);
		if( err == 0 ) {
			face_num = face->num_faces;
			std::string familyname(face->family_name);
			FontInfo* info = new FontInfo();
			fonts.push_back( info );
			tjs_string wname;
			TVPEncodeUTF8ToUTF16( wname, familyname );
			if( faces ) faces->push_back( wname );
			if( fontmap ) {
				tjs_uint styleflag = (face->style_flags&FT_STYLE_FLAG_ITALIC) ? TVP_TF_ITALIC : 0;
				styleflag |= (face->style_flags&FT_STYLE_FLAG_BOLD ) ? TVP_TF_BOLD : 0;
				fontmap->insert( std::map<FaceKey, FontInfo*>::value_type( FaceKey(wname,styleflag), info ) );
			}
			info->facename = wname;
			if( path ) info->path = *path;
			if( filename ) info->filename = *filename;
			info->index = f;
			info->stylename = std::string(face->style_name);
			info->num_glyphs = face->num_glyphs;
			info->face_flags = face->face_flags;
			info->style_flags = face->style_flags;
			info->enc_flags = 0;
			int numcharmap = face->num_charmaps;
			for (int c = 0; c < numcharmap; c++) {
				FT_Encoding enc = face->charmaps[c]->encoding;
				if (enc == FT_ENCODING_SJIS) {
					info->enc_flags |= TVP_ENC_SJIS;
				} else if (enc == FT_ENCODING_UNICODE) {
					info->enc_flags |= TVP_ENC_UNICODE;
				} else if (enc == FT_ENCODING_NONE) {
					info->enc_flags |= TVP_ENC_NONE;
				} else if (enc == FT_ENCODING_MS_SYMBOL) {
					info->enc_flags |= TVP_ENC_MS_SYMBOL;
				} else if (enc == FT_ENCODING_GB2312) {
					info->enc_flags |= TVP_ENC_GB2312;
				} else if (enc == FT_ENCODING_BIG5) {
					info->enc_flags |= TVP_ENC_BIG5;
				} else if (enc == FT_ENCODING_WANSUNG) {
					info->enc_flags |= TVP_ENC_WANSUNG;
				} else if (enc == FT_ENCODING_JOHAB) {
					info->enc_flags |= TVP_ENC_JOHAB;
				} else if (enc == FT_ENCODING_ADOBE_STANDARD) {
					info->enc_flags |= TVP_ENC_ADOBE_STANDARD;
				} else if (enc == FT_ENCODING_ADOBE_EXPERT) {
					info->enc_flags |= TVP_ENC_ADOBE_EXPERT;
				} else if (enc == FT_ENCODING_ADOBE_CUSTOM) {
					info->enc_flags |= TVP_ENC_ADOBE_CUSTOM;
				} else if (enc == FT_ENCODING_ADOBE_LATIN_1) {
					info->enc_flags |= TVP_ENC_ADOBE_LATIN_1;
				} else if (enc == FT_ENCODING_APPLE_ROMAN) {
					info->enc_flags |= TVP_ENC_APPLE_ROMAN;
				}
			}
		}
		if(face) FT_Done_Face(face), face = nullptr;
	}
}
//---------------------------------------------------------------------------
tTVPFreeTypeFaceList::~tTVPFreeTypeFaceList() {
	for( auto i = faces_.begin(); i != faces_.end(); i++ ) {
		delete (*i);
	}
	faces_.clear();
	for( auto i = systemfaces_.begin(); i != systemfaces_.end(); i++ ) {
		delete (*i);
	}
	systemfaces_.clear();
	systemfont_.clear();
}
//---------------------------------------------------------------------------
bool tTVPFreeTypeFaceList::LoadFont( tjs_string filename, std::vector<tjs_string>* faces ) {
	TVPInitializeFont();

	// 読み込み済みの中にあるかチェックする
	bool found = false;
	for( auto i = faces_.begin(); i != faces_.end(); i++ ) {
		if( (*i)->filename == filename ) {
			if( faces ) faces->push_back( (*i)->facename );
			else return false;
			found = true;
		}
	}
	if( found ) return false;

	// フェイス名一覧を取得する
	tGenericFreeTypeFace ftf( filename, faces_, faces );
	return true;
}
//---------------------------------------------------------------------------
void tTVPFreeTypeFaceList::LoadSystemFont( std::string path, std::vector<tjs_string>* faces ) {
	TVPInitializeFont();

	// 読み込み済みの中にあるかチェックする
	bool found = false;
	for( auto i = systemfaces_.begin(); i != systemfaces_.end(); i++ ) {
		if( (*i)->path == path ) {
			if( faces ) faces->push_back( (*i)->facename );
			found = true;
		}
	}
	if( found ) return;

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));
	args.flags = FT_OPEN_PATHNAME;
	args.pathname = const_cast<char*>(path.c_str());
	TVPLoadFont( args, systemfaces_, &systemfont_, faces, &path, nullptr );
}
//---------------------------------------------------------------------------
void tTVPFreeTypeFaceList::GetSystemFontList( std::vector<tjs_string>& faces ) {
	faces.clear();
	for( auto i = systemfaces_.begin(); i != systemfaces_.end(); i++ ) {
		faces.push_back( (*i)->facename );
	}
}
//---------------------------------------------------------------------------
tBaseFreeTypeFace* tTVPFreeTypeFaceList::GetFace( const tjs_string& facename, tjs_uint32 options ) const {
	if( !faces_.empty() ) {
		// まずはスタイル指定で検索する
		tjs_uint styleflag = options&TVP_TF_ITALIC ? FT_STYLE_FLAG_ITALIC : 0;
		styleflag |= options&TVP_TF_BOLD ? FT_STYLE_FLAG_BOLD : 0;
		auto f = std::find_if( faces_.begin(), faces_.end(), [facename, styleflag]( const FontInfo* x ) {
			return ( x->facename == facename && ( x->style_flags&( FT_STYLE_FLAG_ITALIC | FT_STYLE_FLAG_BOLD ) ) == styleflag );
		} );
		if( f != faces_.end() ) {
			FontInfo* font = *f;
			return new tGenericFreeTypeFace( ttstr( font->filename ), TVP_FACE_OPTIONS_FACE_INDEX( font->index ) );
		}
		// スタイル気にせず検索する
		f = std::find_if( faces_.begin(), faces_.end(), [facename]( const FontInfo* x ) { return ( x->facename == facename ); } );
		if( f != faces_.end() ) {
			FontInfo* font = *f;
			return new tGenericFreeTypeFace( ttstr( font->filename ), TVP_FACE_OPTIONS_FACE_INDEX( font->index ) );
		}
	} else {
		FaceKey key( facename, options&( TVP_TF_ITALIC | TVP_TF_BOLD ) );
		auto i = systemfont_.find( key );
		if( i != systemfont_.end() ) {
			FontInfo* font = i->second;
			return new tGenericFreeTypeFace( font->path, ttstr( facename ), font->index );
		}
	}
	return nullptr;
}
//---------------------------------------------------------------------------
bool tTVPFreeTypeFaceList::CheckFaceFlag( FontInfo& face, tjs_uint flags ) {
	if( flags & TVP_FSF_FIXEDPITCH ) { // fixed pitch only ?
		if( (face.face_flags & FT_FACE_FLAG_FIXED_SIZES) == 0 ) return false;
	}
	// if( flags & TVP_FSF_SAMECHARSET ) // キャラセットはFreeTypeでは無視
	if( flags & TVP_FSF_IGNORESYMBOL ) { // 不完全ではあるが、MS Symbolのみ除外
		if( (face.enc_flags & TVP_ENC_MS_SYMBOL) ) return false;
	}

	if( flags & TVP_FSF_NOVERTICAL ) { // not to list vertical fonts up ?
		if( (face.face_flags & FT_FACE_FLAG_VERTICAL) ) return false;
	}
	// if( flags & TVP_FSF_TRUETYPEONLY ) // TrueTypeOnlyはFreeTypeでは無視
	return true;
}
//---------------------------------------------------------------------------
void tTVPFreeTypeFaceList::GetFontList(std::vector<ttstr> & list, tjs_uint32 flags, const tTVPFont & font ) {
	for( auto i = faces_.begin(); i != faces_.end(); i++ ) {
		if( CheckFaceFlag( *(*i), flags ) ) {
			ttstr facename( (*i)->facename );
			if(std::find(list.begin(), list.end(), facename) == list.end())
				list.push_back( facename );
		}
	}
	for( auto i = systemfaces_.begin(); i != systemfaces_.end(); i++ ) {
		if( CheckFaceFlag( *(*i), flags ) ) {
			ttstr facename( (*i)->facename );
			if(std::find(list.begin(), list.end(), facename) == list.end())
				list.push_back( facename );
		}
	}
}
//---------------------------------------------------------------------------
/**
 * コンストラクタ
 * @param fontname	フォント名
 * @param options	オプション(TVP_TF_XXXX 定数かTVP_FACE_OPTIONS_XXXX定数の組み合わせ)
 */
tGenericFreeTypeFace::tGenericFreeTypeFace(const ttstr &fontname, tjs_uint32 options) : File(NULL)
{
	// フィールドの初期化
	Face = NULL;
	memset(&Stream, 0, sizeof(Stream));

	try {
		if(File) {
			delete File;
			File = NULL;
		} 

		// ファイルを開く
		File = TVPCreateBinaryStreamForRead(fontname,TJS_W("") );
		if( File == NULL ) {
			TVPThrowExceptionMessage( TVPCannotOpenFontFile, fontname );
		}

		// FT_StreamRec の各フィールドを埋める
		FT_StreamRec * fsr = &Stream;
//#ifndef ANDROID
#if 1
		fsr->base = 0;
		fsr->size = static_cast<unsigned long>(File->GetSize());
		fsr->pos = 0;
		fsr->descriptor.pointer = this;
		fsr->pathname.pointer = NULL;
		fsr->read = IoFunc;
		fsr->close = CloseFunc;
#else
		tjs_uint32 fileSize = (tjs_uint32)File->GetSize();
		FontImage.reset( new tjs_uint8[fileSize] );
		File->SetPosition( 0 );
		File->ReadBuffer( (void*)FontImage.get(), fileSize );
		delete File;
		File = nullptr;

		fsr->base = FontImage.get();
		fsr->size = fileSize;
		fsr->pos = 0;
		fsr->descriptor.pointer = nullptr;
		fsr->pathname.pointer = nullptr;
		fsr->read = nullptr;
		fsr->close = nullptr;
#endif
		// Face をそれぞれ開き、Face名を取得して FaceNames に格納する
		tjs_uint face_num = 1;

		FT_Face face = NULL;

		for(tjs_uint i = 0; i < face_num; i++)
		{
			if(!OpenFaceByIndex(i, face))
			{
				FaceNames.push_back(tjs_string());
			}
			else
			{
				const char * name = face->family_name;
				tjs_string wname;
				TVPEncodeUTF8ToUTF16( wname, std::string(name) );
				FaceNames.push_back( wname );
				face_num = face->num_faces;
			}
		}

		if(face) FT_Done_Face(face), face = NULL;


		// FreeType エンジンでファイルを開こうとしてみる
		tjs_uint index = TVP_GET_FACE_INDEX_FROM_OPTIONS(options);
		if(!OpenFaceByIndex(index, Face)) {
			// フォントを開けなかった
			TVPThrowExceptionMessage(TVPFontCannotBeUsed, fontname );
		}
	}
	catch(...)
	{
		throw;
	}
}
//---------------------------------------------------------------------------
tGenericFreeTypeFace::tGenericFreeTypeFace(const std::string& path, const ttstr &fontname, tjs_uint32 index ) : File(nullptr), Face(nullptr)
{
	memset(&Stream, 0, sizeof(Stream));
	try {
		FT_Parameter parameters[1];
		parameters[0].tag = FT_PARAM_TAG_UNPATENTED_HINTING; // Appleの特許回避を行う
		parameters[0].data = NULL;

		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		args.flags = FT_OPEN_PATHNAME;
		args.pathname = const_cast<char*>(path.c_str());
		args.num_params = 1;
		args.params = parameters;
		FT_Error err = FT_Open_Face( FreeTypeLibrary, &args, index, &Face);
		if( err != 0 ) {
			TVPThrowExceptionMessage(TVPFontCannotBeUsed, fontname );
		}
		FaceNames.push_back( fontname.AsStdString() );
	}
	catch(...)
	{
		throw;
	}
}
//---------------------------------------------------------------------------
/**
 * コンストラクタ
 * フォントの情報を取得するためのコンストラクタ
 * @param fontname	フォント名
 * @param fonts		フォント情報格納先
 * @param faces		フェイス名リスト格納先
 */
tGenericFreeTypeFace::tGenericFreeTypeFace(const ttstr &filename, std::vector<FontInfo*>& fonts, std::vector<tjs_string>* faces ) : File(nullptr), Face(nullptr)
{
	// フィールドの初期化
	memset(&Stream, 0, sizeof(Stream));

	try {
		// ファイルを開く
		File = TVPCreateBinaryStreamForRead( filename,TJS_W("") );
		if( File == NULL ) {
			TVPThrowExceptionMessage( TVPCannotOpenFontFile, filename );
		}

		// FT_StreamRec の各フィールドを埋める
		FT_StreamRec * fsr = &Stream;
		fsr->base = 0;
		fsr->size = static_cast<unsigned long>(File->GetSize());
		fsr->pos = 0;
		fsr->descriptor.pointer = this;
		fsr->pathname.pointer = NULL;
		fsr->read = IoFunc;
		fsr->close = CloseFunc;

		// Face をそれぞれ開き、Face名を取得して FaceNames に格納する
		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		args.flags = FT_OPEN_STREAM;
		args.stream = &Stream;
		tjs_string fontname = filename.AsStdString();
		TVPLoadFont( args, fonts, nullptr, faces, nullptr, &fontname );
	}
	catch(...)
	{
		throw;
	}
}

//---------------------------------------------------------------------------
/**
 * デストラクタ
 */
tGenericFreeTypeFace::~tGenericFreeTypeFace()
{
	if(Face) FT_Done_Face(Face), Face = NULL;
	if(File) {
		delete File;
		File = NULL;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * FreeType の Face オブジェクトを返す
 */
FT_Face tGenericFreeTypeFace::GetFTFace() const
{
	return Face;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * このフォントファイルが持っているフォントを配列として返す
 */
void tGenericFreeTypeFace::GetFaceNameList(std::vector<tjs_string> & dest) const
{
	dest = FaceNames;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
/**
 * FreeType 用 ストリーム読み込み関数
 */
unsigned long tGenericFreeTypeFace::IoFunc( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count )
{
	tGenericFreeTypeFace * _this =
		static_cast<tGenericFreeTypeFace*>(stream->descriptor.pointer);

	size_t result;
	if(count == 0)
	{
		// seek
		result = 0;
		_this->File->SetPosition( offset );
	}
	else
	{
		// read
		_this->File->SetPosition( offset );
		_this->File->ReadBuffer(buffer, count);
		result = count;
	}

	return (unsigned long)result;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * FreeType 用 ストリーム削除関数
 */
void tGenericFreeTypeFace::CloseFunc( FT_Stream  stream )
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * 指定インデックスのFaceを開く
 * @param index	開くindex
 * @param face	FT_Face 変数への参照
 * @return	Faceを開ければ true そうでなければ false
 * @note	初めて Face を開く場合は face で指定する変数には null を入れておくこと
 */
bool tGenericFreeTypeFace::OpenFaceByIndex(tjs_uint index, FT_Face & face)
{
	if(face) FT_Done_Face(face), face = NULL;

	FT_Parameter parameters[1];
	parameters[0].tag = FT_PARAM_TAG_UNPATENTED_HINTING; // Appleの特許回避を行う
	parameters[0].data = NULL;

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));
	args.flags = FT_OPEN_STREAM;
	args.stream = &Stream;
	args.driver = 0;
	args.num_params = 1;
	args.params = parameters;

	FT_Error err = FT_Open_Face( FreeTypeLibrary, &args, index, &face);

	return err == 0;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
/**
 * コンストラクタ
 * @param fontname	フォント名
 * @param options	オプション
 */
tFreeTypeFace::tFreeTypeFace(const std::vector<tjs_string> &fontname, tjs_uint32 options)
{
	TVPInitializeFont();
	TVPInitializeFaceList();

	// フィールドをクリア
	Options = options;
	Height = 10;

	tjs_size count = fontname.size();
	for( tjs_int i = 0; i < (tjs_int)count; i++ ) {
		Faces.push_back( std::unique_ptr<FaceSet>( new FaceSet() ) );
		Faces[i]->FontName = fontname[i];

		// フォントを開く
		if( ( options & TVP_FACE_OPTIONS_FILE ) == 0 )
		{
			Faces[i]->Face.reset( FreeTypeFaceList->GetFace( fontname[i], options ) );
		}
#ifdef _WIN32
		if( Faces[i]->Face == nullptr )
		{
			if( options & TVP_FACE_OPTIONS_FILE )
			{
				// ファイルを開く
				Faces[i]->Face.reset( new tGenericFreeTypeFace( fontname[i], options ) );
				// 例外がここで発生する可能性があるので注意
			} else
			{
				// ネイティブのフォント名による指定 (プラットフォーム依存)
				Faces[i]->Face.reset( new tNativeFreeTypeFace( fontname[i], options ) );
				// 例外がここで発生する可能性があるので注意
			}
		}
#else
		if( Faces[i]->Face == nullptr )
		{
			if( options & TVP_FACE_OPTIONS_FILE )
			{
				Faces[i]->Face.reset( new tGenericFreeTypeFace( fontname[i], options ) );
			}
		}
#endif
		Faces[i]->FTFace = Faces[i]->Face->GetFTFace();

		// マッピングを確認する
		if( Faces[i]->FTFace->charmap == nullptr )
		{
			// FreeType は自動的に UNICODE マッピングを使用するが、
			// フォントが UNICODE マッピングの情報を含んでいない場合は
			// 自動的な文字マッピングの選択は行われない。
			// (日本語環境に限って言えば) SJIS マッピングしかもってない
			// フォントが多いのでSJISを選択させてみる。
			FT_Error err = FT_Select_Charmap( Faces[i]->FTFace, FT_ENCODING_SJIS );
			if( !err )
			{
				// SJIS への切り替えが成功した
				// 変換関数をセットする
				Faces[i]->UnicodeToLocalChar = UnicodeToSJIS;
				Faces[i]->LocalCharToUnicode = SJISToUnicode;
			} else
			{
				// とりあえず何か割り当てる、正しく表示されないかもしれないが
				int numcharmap = Faces[i]->FTFace->num_charmaps;
				for( int j = 0; j < numcharmap; j++ )
				{
					FT_Encoding enc = Faces[i]->FTFace->charmaps[j]->encoding;
					if( enc != FT_ENCODING_NONE && enc != FT_ENCODING_APPLE_ROMAN )
					{
						err = FT_Select_Charmap( Faces[i]->FTFace, enc );
						if( !err ) {
							break;
						}
					}
				}
			}
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * デストラクタ
 */
tFreeTypeFace::~tFreeTypeFace()
{
}
//---------------------------------------------------------------------------

#if 0
//---------------------------------------------------------------------------
/**
 * このFaceが保持しているglyphの数を得る
 * @return	このFaceが保持しているglyphの数
 */
tjs_uint tFreeTypeFace::GetGlyphCount()
{
	FT_Face face = Faces[0]->FTFace;
	if(!face) return 0;

	// FreeType が返してくるグリフの数は、実際に文字コードが割り当てられていない
	// グリフをも含んだ数となっている
	// ここで、実際にフォントに含まれているグリフを取得する
	// TODO:スレッド保護されていないので注意！！！！！！
	if(!Faces[0]->GlyphIndexToCharcodeVector)
	{
		// マップが作成されていないので作成する
		Faces[0]->GlyphIndexToCharcodeVector.reset( new FaceSet::tGlyphIndexToCharcodeVector() );
		FT_ULong  charcode;
		FT_UInt   gindex;
		charcode = FT_Get_First_Char( face, &gindex );
		while ( gindex != 0 )
		{
			FT_ULong code;
			if( Faces[0]->LocalCharToUnicode)
				code = Faces[0]->LocalCharToUnicode(charcode);
			else
				code = charcode;
			Faces[0]->GlyphIndexToCharcodeVector->push_back(code);
			charcode = FT_Get_Next_Char( face, charcode, &gindex );
		}
		std::sort(
			Faces[0]->GlyphIndexToCharcodeVector->begin(),
			Faces[0]->GlyphIndexToCharcodeVector->end()); // 文字コード順で並び替え
	}

	return (tjs_uint)Faces[0]->GlyphIndexToCharcodeVector->size();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
/**
 * Glyph インデックスから対応する文字コードを得る
 * @param index	インデックス(FreeTypeの管理している文字indexとは違うので注意)
 * @return	対応する文字コード(対応するコードが無い場合は 0)
 */
tjs_char tFreeTypeFace::GetCharcodeFromGlyphIndex(tjs_uint index)
{
	tjs_uint size = GetGlyphCount(); // グリフ数を得るついでにマップを作成する

	if(!Faces[0]->GlyphIndexToCharcodeVector) return 0;
	if(index >= size) return 0;

	return static_cast<tjs_char>((*(Faces[0]->GlyphIndexToCharcodeVector.get()))[index]);
}
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
/**
 * このフォントに含まれるFace名のリストを得る
 * @param dest	格納先配列
 */
void tFreeTypeFace::GetFaceNameList(std::vector<tjs_string> &dest)
{
	Faces[0]->Face->GetFaceNameList(dest);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * フォントの高さを設定する
 * @param height	フォントの高さ(ピクセル単位)
 */
void tFreeTypeFace::SetHeight(int height)
{
	Height = height;
	for( auto face = Faces.begin(); face != Faces.end(); face++ ) {
		FT_Error err = FT_Set_Pixel_Sizes( (*face)->FTFace, 0, Height );
		if(err)
		{
			// TODO: Error ハンドリング
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * 指定した文字コードに対するグリフビットマップを得る
 * @param code	文字コード
 * @return	新規作成されたグリフビットマップオブジェクトへのポインタ
 *			NULL の場合は変換に失敗した場合
 */
tTVPCharacterData * tFreeTypeFace::GetGlyphFromCharcode(tjs_char code)
{
	// グリフスロットにグリフを読み込み、寸法を取得する
	tjs_int index;
	tGlyphMetrics metrics;
	if( (index = GetGlyphMetricsFromCharcode(code, metrics)) < 0 )
		return nullptr;

	// 文字をレンダリングする
	FT_Error err;

	FT_Face face = Faces[index]->FTFace;
	if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
	{
		FT_Render_Mode mode;
		if(!(Options & TVP_FACE_OPTIONS_NO_ANTIALIASING))
			mode = FT_RENDER_MODE_NORMAL;
		else
			mode = FT_RENDER_MODE_MONO;
		err = FT_Render_Glyph(face->glyph, mode);
			// note: デフォルトのレンダリングモードは FT_RENDER_MODE_NORMAL (256色グレースケール)
			//       FT_RENDER_MODE_MONO は 1bpp モノクローム
		if(err) return nullptr;
	}

	// 一応ビットマップ形式をチェック
	FT_Bitmap *ft_bmp = &(face->glyph->bitmap);
	FT_Bitmap new_bmp;
	bool release_ft_bmp = false;
	tTVPCharacterData * glyph_bmp = nullptr;
	try
	{
		if(ft_bmp->rows && ft_bmp->width)
		{
			// ビットマップがサイズを持っている場合
			if(ft_bmp->pixel_mode != ft_pixel_mode_grays)
			{
				// ft_pixel_mode_grays ではないので ft_pixel_mode_grays 形式に変換する
				FT_Bitmap_New(&new_bmp);
				release_ft_bmp = true;
				ft_bmp = &new_bmp;
				err = FT_Bitmap_Convert(face->glyph->library,
					&(face->glyph->bitmap),
					&new_bmp, 1);
					// 結局 tGlyphBitmap 形式に変換する際にアラインメントをし直すので
					// ここで指定する alignment は 1 でよい
				if(err)
				{
					if(release_ft_bmp) FT_Bitmap_Done(face->glyph->library, ft_bmp);
					return nullptr;
				}
			}

			if(ft_bmp->num_grays != 256)
			{
				// gray レベルが 256 ではない
				// 256 になるように乗算を行う
				tjs_int32 multiply =
					static_cast<tjs_int32>((static_cast<tjs_int32> (1) << 30) - 1) /
						(ft_bmp->num_grays - 1);
				for(tjs_int y = ft_bmp->rows - 1; y >= 0; y--)
				{
					unsigned char * p = ft_bmp->buffer + y * ft_bmp->pitch;
					for(tjs_int x = ft_bmp->width - 1; x >= 0; x--)
					{
						tjs_int32 v = static_cast<tjs_int32>((*p * multiply)  >> 22);
						*p = static_cast<unsigned char>(v);
						p++;
					}
				}
			}
		}
		// 64倍されているものを解除する
		metrics.CellIncX = FT_PosToInt( metrics.CellIncX );
		metrics.CellIncY = FT_PosToInt( metrics.CellIncY );

		// tGlyphBitmap を作成して返す
		//int baseline = (int)(face->height + face->descender) * face->size->metrics.y_ppem / face->units_per_EM;
		int baseline = (int)( face->ascender ) * face->size->metrics.y_ppem / face->units_per_EM;

		glyph_bmp = new tTVPCharacterData(
			ft_bmp->buffer,
			ft_bmp->pitch,
			  face->glyph->bitmap_left,
			  baseline - face->glyph->bitmap_top,
			  ft_bmp->width,
			  ft_bmp->rows,
			metrics);
		glyph_bmp->Gray = 256;

		
		if( Options & TVP_TF_UNDERLINE ) {
			tjs_int pos = -1, thickness = -1;
			GetUnderline( pos, thickness, index );
			if( pos >= 0 && thickness > 0 ) {
				glyph_bmp->AddHorizontalLine( pos, thickness, 255 );
			}
		}
		if( Options & TVP_TF_STRIKEOUT ) {
			tjs_int pos = -1, thickness = -1;
			GetStrikeOut( pos, thickness, index );
			if( pos >= 0 && thickness > 0 ) {
				glyph_bmp->AddHorizontalLine( pos, thickness, 255 );
			}
		}
	}
	catch(...)
	{
		if(release_ft_bmp) FT_Bitmap_Done(face->glyph->library, ft_bmp);
		throw;
	}
	if(release_ft_bmp) FT_Bitmap_Done(face->glyph->library, ft_bmp);

	return glyph_bmp;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * 指定した文字コードに対する描画領域を得る
 * @param code	文字コード
 * @return	レンダリング領域矩形へのポインタ
 *			NULL の場合は変換に失敗した場合
 */
bool tFreeTypeFace::GetGlyphRectFromCharcode( tTVPRect& rt, tjs_char code, tjs_int& advancex, tjs_int& advancey )
{
	advancex = advancey = 0;
	tjs_int index;
	if( (index = LoadGlyphSlotFromCharcode(code)) < 0 )
		return false;

	FT_Face face = Faces[index]->FTFace;
	int baseline = (int)( face->ascender ) * face->size->metrics.y_ppem / face->units_per_EM;
	/*
	FT_Render_Glyph でレンダリングしないと以下の各値は取得できない
	tjs_int t = baseline - face->glyph->bitmap_top;
	tjs_int l = face->glyph->bitmap_left;
	tjs_int w = face->glyph->bitmap.width;
	tjs_int h = face->glyph->bitmap.rows;
	*/
	tjs_int t = baseline - FT_PosToInt( face->glyph->metrics.horiBearingY );
	tjs_int l = FT_PosToInt( face->glyph->metrics.horiBearingX );
	tjs_int w = FT_PosToInt( face->glyph->metrics.width );
	tjs_int h = FT_PosToInt( face->glyph->metrics.height );
	advancex = FT_PosToInt( face->glyph->advance.x );
	advancey = FT_PosToInt( face->glyph->advance.y );
	rt = tTVPRect(l,t,l+w,t+h);
	if( Options & TVP_TF_UNDERLINE ) {
		tjs_int pos = -1, thickness = -1;
		GetUnderline( pos, thickness, index );
		if( pos >= 0 && thickness > 0 ) {
			if( rt.left > 0 ) rt.left = 0;
			if( rt.right < advancex ) rt.right = advancex;
			if( pos < rt.top ) rt.top = pos;
			if( (pos+thickness) >= rt.bottom ) rt.bottom = pos+thickness+1;

		}
	}
	if( Options & TVP_TF_STRIKEOUT ) {
		tjs_int pos = -1, thickness = -1;
		GetStrikeOut( pos, thickness, index );
		if( pos >= 0 && thickness > 0 ) {
			if( rt.left > 0 ) rt.left = 0;
			if( rt.right < advancex ) rt.right = advancex;
			if( pos < rt.top ) rt.top = pos;
			if( (pos+thickness) >= rt.bottom ) rt.bottom = pos+thickness+1;
		}
	}
	return true;
}

//---------------------------------------------------------------------------
/**
 * 指定した文字コードに対するグリフの寸法を得る(文字を進めるためのサイズ)
 * @param code		文字コード
 * @param metrics	寸法
 * @return	成功の場合フェイスインデックス、失敗の場合負の値
 */
tjs_int tFreeTypeFace::GetGlyphMetricsFromCharcode(tjs_char code,
	tGlyphMetrics & metrics)
{
	tjs_int index;
	if( (index = LoadGlyphSlotFromCharcode(code)) < 0 ) return -1;

	// メトリック構造体を作成
	// CellIncX や CellIncY は ピクセル値が 64 倍された値なので注意
	// これはもともと FreeType の仕様だけれども、Risaでも内部的には
	// この精度で CellIncX や CellIncY を扱う
	metrics.CellIncX =  Faces[index]->FTFace->glyph->advance.x;
	metrics.CellIncY =  Faces[index]->FTFace->glyph->advance.y;

	return index;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * 指定した文字コードに対するグリフのサイズを得る(文字の大きさ)
 * @param code		文字コード
 * @param metrics	サイズ
 * @return	成功の場合真、失敗の場合偽
 */
bool tFreeTypeFace::GetGlyphSizeFromCharcode(tjs_char code, tGlyphMetrics & metrics)
{
	tjs_int index;
	if( (index = LoadGlyphSlotFromCharcode(code)) < 0 ) return false;

	// メトリック構造体を作成
	metrics.CellIncX = FT_PosToInt( Faces[index]->FTFace->glyph->metrics.horiAdvance );
	metrics.CellIncY = FT_PosToInt( Faces[index]->FTFace->glyph->metrics.vertAdvance );

	return true;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * 指定した文字コードに対するグリフをグリフスロットに設定する
 * @param code	文字コード
 * @return	成功の場合フェイスインデックス、失敗の場合負の値
 */
tjs_int tFreeTypeFace::LoadGlyphSlotFromCharcode(tjs_char code)
{
	// TODO: スレッド保護

	tjs_size count = Faces.size();
	for( tjs_size i = 0; i < count; i++ ) {
		std::unique_ptr<FaceSet>& faceset = Faces[i];
		// 文字コードを得る
		FT_ULong localcode;
		if(faceset->UnicodeToLocalChar == NULL)
			localcode = code;
		else
			localcode = faceset->UnicodeToLocalChar(code);

		// 文字コードから index を得る
		FT_UInt glyph_index = FT_Get_Char_Index(faceset->FTFace, localcode);
		if(glyph_index == 0)
			continue;

		// グリフスロットに文字を読み込む
		FT_Int32 load_glyph_flag = 0;
		if(!(Options & TVP_FACE_OPTIONS_NO_ANTIALIASING))
			load_glyph_flag |= FT_LOAD_NO_BITMAP;
		else
			load_glyph_flag |= FT_LOAD_TARGET_MONO;
				// note: ビットマップフォントを読み込みたくない場合は FT_LOAD_NO_BITMAP を指定

		if(Options & TVP_FACE_OPTIONS_NO_HINTING)
			load_glyph_flag |= FT_LOAD_NO_HINTING|FT_LOAD_NO_AUTOHINT;
		if(Options & TVP_FACE_OPTIONS_FORCE_AUTO_HINTING)
			load_glyph_flag |= FT_LOAD_FORCE_AUTOHINT;

		FT_Error err;
		err = FT_Load_Glyph(faceset->FTFace, glyph_index, load_glyph_flag);

		if(err) continue;

		// フォントの変形を行う
		if( Options & TVP_TF_BOLD ) FT_GlyphSlot_Embolden( faceset->FTFace->glyph );
		if( Options & TVP_TF_ITALIC ) FT_GlyphSlot_Oblique( faceset->FTFace->glyph );

		return (tjs_int)i;
	}

	return -1;
}
//---------------------------------------------------------------------------

