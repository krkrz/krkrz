#include "tjsCommHead.h"

#include <android/asset_manager.h>
#include "Application.h"
#include "StorageIntf.h"

//---------------------------------------------------------------------------
// tTVPAssetFileStream
//---------------------------------------------------------------------------
class tTVPAssetFileStream : public tTJSBinaryStream
{
private:
	AAsset* asset_;

public:
	tTVPAssetFileStream( AAssetManager* mgr, const ttstr &origname, const ttstr & localname, tjs_uint32 flag) {
		tjs_uint32 access = flag & TJS_BS_ACCESS_MASK;
		if( access != TJS_BS_READ ) {	// asset へは書き込めない
			TVPThrowExceptionMessage(TVPCannotOpenStorage, origname);
		}
		asset_ = AAssetManager_open( mgr, localname.AsNarrowStdString().c_str(), AASSET_MODE_RANDOM );
        if( asset_ == NULL ) {
			TVPThrowExceptionMessage(TVPCannotOpenStorage, origname);
		}
	}
	~tTVPAssetFileStream() {
		if( asset_ ) {
			AAsset_close( asset_ );
			asset_ = NULL;
		}
	}

	tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence) {
		int orgin;
		switch(whence)
		{
		case TJS_BS_SEEK_SET:	orgin = SEEK_SET;	break;
		case TJS_BS_SEEK_CUR:	orgin = SEEK_CUR;	break;
		case TJS_BS_SEEK_END:	orgin = SEEK_END;	break;
		default:				orgin = SEEK_SET;	break; // may be enough
		}
		return AAsset_seek64( asset_, offset, orgin );
	}

	tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size) {
		return AAsset_read( asset_, buffer, read_size );
	}
	tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint write_size) {
		// cannot write to asset
		TVPThrowExceptionMessage(TVPWriteError);
		return 0;
	}

	void TJS_INTF_METHOD SetEndOfStorage() {
		// cannot write to asset
		TVPThrowExceptionMessage(TVPWriteError);
	}

	tjs_uint64 TJS_INTF_METHOD GetSize() {
		return AAsset_getLength64( asset_ );
	}
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTVPFileMedia
//---------------------------------------------------------------------------
class tTVPAssetMedia : public iTVPStorageMedia {
	tjs_uint RefCount;

public:
	tTVPAssetMedia() { RefCount = 1; }
	~tTVPAssetMedia() {;}

	void TJS_INTF_METHOD AddRef() { RefCount ++; }
	void TJS_INTF_METHOD Release() {
		if(RefCount == 1)
			delete this;
		else
			RefCount --;
	}
	void TJS_INTF_METHOD GetName(ttstr &name) { name = TJS_W("asset"); }

	void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) {
		// normalize domain name
        // 何もしない。asset://以下のパスはケースセンシティブ
	}
	void TJS_INTF_METHOD NormalizePathName(ttstr &name) {
		// normalize path name
        // 何もしない。asset://以下のパスはケースセンシティブ
	}
	bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
		if(name.IsEmpty()) return false;
		ttstr _name(name);
		GetLocalName(_name);
		AAsset* asset = AAssetManager_open( Application->getAssetManager(), _name.AsNarrowStdString().c_str(), AASSET_MODE_UNKNOWN);
		bool result = asset != NULL;
		if( result ) {
			AAsset_close( asset );
		}
		return result;
	}
	tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) {
		// open storage named "name".
		// currently only local/network(by OS) storage systems are supported.
		if(name.IsEmpty())
			TVPThrowExceptionMessage(TVPCannotOpenStorage, TJS_W("\"\""));

		ttstr origname = name;
		ttstr _name(name);
		GetLocalName(_name);
		return new tTVPAssetFileStream( Application->getAssetManager(), origname, _name, flags );
	}
	// Assetはキャッシングした方がいいな
	void TJS_INTF_METHOD GetListAt(const ttstr &_name, iTVPStorageLister *lister) {
		ttstr name(_name);
		GetLocalName(name);

        if( name.GetLen() == 1 && name[0] == TJS_W('/') ) name = ttstr(TJS_W(""));
        if( name[name.length()-1] == TJS_W('/') ) name = ttstr( name, name.length()-1 );
		AAssetDir* dir = AAssetManager_openDir( Application->getAssetManager(), name.AsNarrowStdString().c_str() );
		if( dir ) {
			const char* filename = nullptr;
			do {
				filename = AAssetDir_getNextFileName( dir );
				if( filename ) {
					ttstr file( filename );
                    /*
					tjs_char *p = file.Independ();
					while(*p) {
						// make all characters small
						if(*p >= TJS_W('A') && *p <= TJS_W('Z'))
							*p += TJS_W('a') - TJS_W('A');
						p++;
					}
                    */
					lister->Add( file );
				}
			} while( filename );
			AAssetDir_close( dir );
		}
	}
	void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) {
		ttstr newname;
		const tjs_char *ptr = name.c_str();
		if( *ptr == TJS_W('.') ) ptr++;
		if( *ptr == TJS_W('/') ) ptr++;
		while( (*ptr == TJS_W('/') || *ptr == TJS_W('\\')) && (ptr[1] == TJS_W('/') || ptr[1] == TJS_W('\\')) ) ptr++;
		newname = ttstr(ptr);
		// change path delimiter to '/'
		tjs_char *pp = newname.Independ();
		while(*pp)
		{
			if(*pp == TJS_W('\\')) *pp = TJS_W('/');
			pp++;
		}
		name = newname;
	}

	void TJS_INTF_METHOD GetLocalName(ttstr &name) {
		ttstr tmp = name;
		GetLocallyAccessibleName(tmp);
		if(tmp.IsEmpty()) TVPThrowExceptionMessage(TVPCannotGetLocalName, name);
		name = tmp;
	}
};
//---------------------------------------------------------------------------
void TVPRegisterAssetMedia()
{
	iTVPStorageMedia *assetmedia = new tTVPAssetMedia();
	TVPRegisterStorageMedia( assetmedia );
	assetmedia->Release();
}
//---------------------------------------------------------------------------


class AssetCache {
	class AssetDirectory {
		bool is_file_;
		std::string name_;
		std::map<std::string,AssetDirectory*> files_;
	public:
		AssetDirectory() : is_file_(false) {}
		AssetDirectory( bool isfile, const char* name ) : is_file_(isfile), name_(name) {}
		AssetDirectory( bool isfile, const std::string& name ) : is_file_(isfile), name_(name) {}
		~AssetDirectory() {
			std::map<std::string,AssetDirectory*>::iterator i = files_.begin();
			for( ; i != files_.end(); ++i ) {
				delete (*i).second;
			}
			files_.clear();
		}
		void pushFile( const std::string& name, AssetDirectory* dir ) {
			files_.insert( std::map<std::string,AssetDirectory*>::value_type( name, dir ) );
		}
		AssetDirectory* getFile( const std::string& name ) {
			std::map<std::string,AssetDirectory*>::iterator i = files_.find( name );
			if( i != files_.end() ) {
				return (*i).second;
			} else {
				return nullptr;
			}
		}
		const std::string& getName() const { return name_; }
		bool isFile() const { return is_file_; }
	};
	AssetDirectory root_;
public:
	void initialize() {
		AAssetManager* mgr = Application->getAssetManager();
		AAssetDir* dir = AAssetManager_openDir( mgr, "" );
		if( dir ) {
			searchDir( mgr, dir, root_, std::string("") );
			AAssetDir_close( dir );
		}
	}
	void searchDir( AAssetManager* mgr, AAssetDir* dir, AssetDirectory& current, const std::string& base ) {
		const char* filename = nullptr;
		do {
			filename = AAssetDir_getNextFileName( dir );
			if( filename ) {
				std::string curfile(filename);
				std::string path( base + curfile );
				AAssetDir* newdir = AAssetManager_openDir( mgr, path.c_str() );
				if( newdir ) {
					AssetDirectory* finddir = new AssetDirectory( false, curfile );
					current.pushFile( curfile, finddir );
					// 再帰
					searchDir( mgr, newdir, *finddir, path + std::string("/") );
					AAssetDir_close( newdir );
				} else {
					// ファイルだった
					current.pushFile( curfile, new AssetDirectory( true, curfile ) );
				}
			}
		} while( filename );
	}
};


