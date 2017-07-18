//---------------------------------------------------------------------------
/*
	DocumentFile系のストレージを扱うメディア
	Android で SDカードやGoogleDrive等の読み書きに使われる
*/
//---------------------------------------------------------------------------
// content:// media
//---------------------------------------------------------------------------

#include "tjsCommHead.h"

#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <android/asset_manager.h>

#include "Application.h"
#include "StorageIntf.h"

//---------------------------------------------------------------------------
// tTVPContentFileStream
//---------------------------------------------------------------------------
class tTVPContentFileStream : public tTJSBinaryStream
{
private:
	static jmethodID getFd_func_;

	jobject ParcelFileDescriptor_;
	int fd_;

public:
	tTVPContentFileStream( int fd, jobject dsc ) : fd_(fd), ParcelFileDescriptor_(dsc) {}
	virtual ~tTVPContentFileStream() {
		if( ParcelFileDescriptor_ ) {
			bool attached;
			JNIEnv *env = Application->getJavaEnv(attached);
			env->DeleteGlobalRef( ParcelFileDescriptor_ );
			ParcelFileDescriptor_ = nullptr;
			if( attached ) Application->detachJavaEnv();
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
		if( offset >= 0x100000000LL ) return 0;
		int s = lseek( fd_, offset, orgin );
		if( s >= 0 ) return s;
		TVPThrowExceptionMessage(TVPSeekError);
		return 0;	// seek error
	}

	tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size) {
		int r = read( fd_, buffer, read_size );
		if( r >= 0 ) return r;
		// read error
		TVPThrowExceptionMessage(TVPReadError);
        return 0;
	}
	tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint write_size) {
		int w = write( fd_, buffer, write_size );
		if( w >= 0 ) return w;
		// write error
		TVPThrowExceptionMessage(TVPWriteError);
		return 0;
	}

	void TJS_INTF_METHOD SetEndOfStorage() {
        off_t ret = lseek( fd_, 0, SEEK_END );
		if( ret < 0 ) TVPThrowExceptionMessage(TVPSeekError);
	}

	tjs_uint64 TJS_INTF_METHOD GetSize() {
		struct stat st = {0};
		if(  fstat( fd_, &st ) == 0 ) {
			return st.st_size;
		}
		return 0;
	}

    static tTVPContentFileStream* create( JNIEnv *env, jobject pfdsc ) {
		if( getFd_func_ == nullptr ) {
			if( init( env ) == JNI_FALSE ) {
				return nullptr;
			}
		}
    	if( pfdsc == nullptr ) {
    		return nullptr;
    	}
		jint fd = env->CallIntMethod( pfdsc, getFd_func_ );
		pfdsc = env->NewGlobalRef(pfdsc);
		return new tTVPContentFileStream( fd, pfdsc );
	}
	static jboolean init( JNIEnv *env ) {
		jclass clsj = env->FindClass("android/os/ParcelFileDescriptor");
		if( clsj != nullptr ) {
			getFd_func_ = env->GetMethodID(clsj, "getFd", "()I");
			env->DeleteLocalRef(clsj);
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
};
jmethodID tTVPContentFileStream::getFd_func_ = nullptr;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTVPContentMedia
//---------------------------------------------------------------------------
class tTVPContentMedia : public iTVPStorageMedia {
	tjs_uint RefCount;

	jclass clazz_;
	static jmethodID isExist_func_;
	static jmethodID open_func_;
	static jmethodID getFileList_func_;
	static jmethodID getFullUri_func_;
    static jmethodID getFileListInDir_func;

private:
	bool GetFullUri(const ttstr& path, const ttstr& name, ttstr &uri) {
		bool result = false;
		bool attached;
		JNIEnv *env = Application->getJavaEnv(attached);
		jstring jpath = env->NewString( reinterpret_cast<const jchar*>(path.c_str()), path.length() );
		jstring jname = env->NewString( reinterpret_cast<const jchar*>(name.c_str()), name.length() );
		jstring ret = (jstring)env->CallStaticObjectMethod( clazz_, getFullUri_func_, jpath, jname );
		if( ret != nullptr ) {
			int jstrlen = env->GetStringLength( ret );
			const jchar* chars = env->GetStringChars( ret, nullptr );
			uri = ttstr(reinterpret_cast<const tjs_char*>(chars),jstrlen);
			env->ReleaseStringChars( ret, chars );
			env->DeleteLocalRef( ret );
			result = true;
		}
		env->DeleteLocalRef( jname );
		env->DeleteLocalRef( jpath );
		if( attached ) Application->detachJavaEnv();
		return result;
	}

public:
	tTVPContentMedia() {
		RefCount = 1;

		bool attached;
		JNIEnv *env = Application->getJavaEnv(attached);
		jclass clazz = env->FindClass("jp/kirikiri/krkrz/ContentMedia");
		clazz_ = reinterpret_cast<jclass>(env->NewGlobalRef(reinterpret_cast<jobject>(clazz) ));
		if( isExist_func_ == nullptr || open_func_ == nullptr || getFileList_func_ == nullptr || getFullUri_func_ == nullptr || getFileListInDir_func == nullptr ) {
			isExist_func_ = env->GetStaticMethodID(clazz_, "isExist", "(Ljava/lang/String;)Z");
			open_func_ = env->GetStaticMethodID(clazz_, "open", "(Ljava/lang/String;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;");
			getFileList_func_ = env->GetStaticMethodID(clazz_, "getFileList", "(Ljava/lang/String;)[Ljava/lang/String;");
			getFullUri_func_ = env->GetStaticMethodID(clazz_, "getFullUri", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
            getFileListInDir_func = env->GetStaticMethodID(clazz_, "getFileListInDir", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;");
		}
		env->DeleteLocalRef(clazz);
		if( attached ) Application->detachJavaEnv();
	}
	~tTVPContentMedia() {
		bool attached;
		JNIEnv *env = Application->getJavaEnv(attached);
		env->DeleteGlobalRef( clazz_ );
		if( attached ) Application->detachJavaEnv();
	}

	void TJS_INTF_METHOD AddRef() { RefCount ++; }
	void TJS_INTF_METHOD Release() {
		if(RefCount == 1)
			delete this;
		else
			RefCount --;
	}
	void TJS_INTF_METHOD GetName(ttstr &name) { name = TJS_W("content"); }

	void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) {
		// normalize domain name
		// 何もしない。content://以下のパスはケースセンシティブ
	}
	void TJS_INTF_METHOD NormalizePathName(ttstr &name) {
		// 何もしない。content://以下のパスはケースセンシティブ
	}
	bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
		if(name.IsEmpty()) return false;
		ttstr _name(name);
		//GetLocalName(_name);
		GetLocallyAccessibleName(_name);
		if( _name.IsEmpty() ) return false;	// 存在しないファイルへの変換はできない

		bool attached;
		JNIEnv *env = Application->getJavaEnv(attached);
		jstring path = env->NewString( reinterpret_cast<const jchar*>(_name.c_str()), _name.length() );
		jboolean ret = env->CallStaticBooleanMethod( clazz_, isExist_func_, path );
		env->DeleteLocalRef( path );
		if( attached ) Application->detachJavaEnv();
		return ret ? true : false;
	}
	tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) {
		// open storage named "name".
		// currently only local/network(by OS) storage systems are supported.
		if(name.IsEmpty())
			TVPThrowExceptionMessage(TVPCannotOpenStorage, TJS_W("\"\""));

		flags = flags & TJS_BS_ACCESS_MASK;
		ttstr flag = TJS_W("r");
		switch( flags ) {
			case TJS_BS_READ:
				flag = TJS_W("r");
				break;
			case TJS_BS_WRITE:
				flag = TJS_W("w");
				break;
			case TJS_BS_APPEND:
				flag = TJS_W("rw");
				break;
			case TJS_BS_UPDATE:
				flag = TJS_W("rwt");
				break;
		}

		ttstr origname = name;
		ttstr _name(name);
		GetLocalName(_name);

		bool attached;
		JNIEnv *env = Application->getJavaEnv(attached);
		jstring path = env->NewString( reinterpret_cast<const jchar*>(_name.c_str()), _name.length() );
		jstring mode = env->NewString( reinterpret_cast<const jchar*>(flag.c_str()), flag.length() );
		jobject  obj = env->CallStaticObjectMethod( clazz_, open_func_, path, mode );
		tTVPContentFileStream* ret = tTVPContentFileStream::create(env, obj);
		env->DeleteLocalRef( path );
		env->DeleteLocalRef( mode );
		env->DeleteLocalRef( obj );
		if( attached ) Application->detachJavaEnv();
		return ret;
	}
    void GetListAtInternal(const ttstr &name, iTVPStorageLister *lister) {
        bool attached;
        JNIEnv *env = Application->getJavaEnv(attached);
        jstring path = env->NewString( reinterpret_cast<const jchar*>(name.c_str()), name.length() );
        jobjectArray obj = (jobjectArray)env->CallStaticObjectMethod( clazz_, getFileList_func_, path );
        int len  = env->GetArrayLength(obj);
        for( int i = 0; i < len; i++ ) {
            jstring string = (jstring) env->GetObjectArrayElement( obj, i);
            int jstrlen = env->GetStringLength( string );
            const jchar* chars = env->GetStringChars( string, nullptr );
            ttstr file(reinterpret_cast<const tjs_char*>(chars),jstrlen);
            lister->Add( file );
            env->ReleaseStringChars( string, chars );
            env->DeleteLocalRef( string );
        }
        env->DeleteLocalRef( obj );
        env->DeleteLocalRef( path );
        if( attached ) Application->detachJavaEnv();
    }
    void GetListAtPath(const ttstr &name, const ttstr& dir, iTVPStorageLister *lister) {
        bool attached;
        JNIEnv *env = Application->getJavaEnv(attached);
        jstring root = env->NewString( reinterpret_cast<const jchar*>(name.c_str()), name.length() );
        jstring path = env->NewString( reinterpret_cast<const jchar*>(dir.c_str()), dir.length() );
        jobjectArray obj = (jobjectArray)env->CallStaticObjectMethod( clazz_, getFileListInDir_func, root, path );
        int len  = env->GetArrayLength(obj);
        for( int i = 0; i < len; i++ ) {
            jstring string = (jstring) env->GetObjectArrayElement( obj, i);
            int jstrlen = env->GetStringLength( string );
            const jchar* chars = env->GetStringChars( string, nullptr );
            ttstr file(reinterpret_cast<const tjs_char*>(chars),jstrlen);
            lister->Add( file );
            env->ReleaseStringChars( string, chars );
            env->DeleteLocalRef( string );
        }
        env->DeleteLocalRef( obj );
        env->DeleteLocalRef( path );
        env->DeleteLocalRef( root );
        if( attached ) Application->detachJavaEnv();
    }

	void TJS_INTF_METHOD GetListAt(const ttstr &_name, iTVPStorageLister *lister) {
		ttstr name(_name);
        // 末尾が/の場合、その/を削除
        const tjs_char *ptr = name.c_str();
        const tjs_char *ep = ptr + name.length() - 1;
        if( *ep == TJS_W('/') ) {
            name = ttstr( ptr, name.length() - 1 );
        }
        LocalAccess( name, lister );
	}
    /**
     * Get local path or list files.
     * @param name
     * @param lister
     */
    void LocalAccess( ttstr& name, iTVPStorageLister *lister = nullptr ) {
        ttstr newname;
        const tjs_char *ptr = name.c_str();
        bool appendshceme = false;
        if( name.length() < 11 ||
            ptr[0] != TJS_W('c') ||
            ptr[1] != TJS_W('o') ||
            ptr[2] != TJS_W('n') ||
            ptr[3] != TJS_W('t') ||
            ptr[4] != TJS_W('e') ||
            ptr[5] != TJS_W('n') ||
            ptr[6] != TJS_W('t') ||
            ptr[7] != TJS_W(':') ||
            ptr[8] != TJS_W('/') ||
            ptr[9] != TJS_W('/') ) {
            appendshceme = true;
            if( *ptr == TJS_W('.') ) ptr++;
            while( *ptr == TJS_W('/') || *ptr == TJS_W('\\') ) ptr++;
            newname = ttstr(TJS_W("content://")) + ttstr(ptr);
        } else {
            newname = ttstr(ptr);
        }
        // change path delimiter to '/'
        tjs_char *pp = newname.Independ();
        tjs_char *sp = pp;
        while(*pp)
        {
            if(*pp == TJS_W('\\')) *pp = TJS_W('/');
            pp++;
        }
        // find %2F and after '/'
        if( name.length() > 5 ) {
            bool found2f = false;
            tjs_char *p2f = pp;
            p2f -= 2;
            while( sp != p2f ) {
                p2f--;
                if( p2f[0] == TJS_W('%') && p2f[1] == TJS_W('2') && (p2f[2] == TJS_W('F') || p2f[2] == TJS_W('f') ) ) {
                    found2f = true;
                    break;
                }
            }
            if( found2f ) {
                p2f += 3;
                while( p2f != pp ) {
                    p2f++;
                    if( *p2f == TJS_W('/') ) {
                        pp = p2f;
                        break;
                    }
                }

            }
        }
        name.Clear();
        ttstr filename;
        ttstr path;
        if( sp != pp ) {
            // found '/'
            filename = ttstr(&(pp[1]));
            path = ttstr(sp,static_cast<int>(pp-sp));
            if( !lister ) {
                ttstr uri;
                if (GetFullUri(path, filename, uri)) {
                    name = uri;
                }
            } else {
                GetListAtPath( path, filename, lister );
            }
        }
        // else not found local path
    }
	void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) {
        LocalAccess( name );
	}

	void TJS_INTF_METHOD GetLocalName(ttstr &name) {
		ttstr tmp = name;
		GetLocallyAccessibleName(tmp);
		if(tmp.IsEmpty()) TVPThrowExceptionMessage(TVPCannotGetLocalName, name);
		name = tmp;
	}
};
jmethodID tTVPContentMedia::isExist_func_ = nullptr;
jmethodID tTVPContentMedia::open_func_ = nullptr;
jmethodID tTVPContentMedia::getFileList_func_ = nullptr;
jmethodID tTVPContentMedia::getFullUri_func_ = nullptr;
jmethodID tTVPContentMedia::getFileListInDir_func = nullptr;
//---------------------------------------------------------------------------
void TVPRegisterContentMedia()
{
	iTVPStorageMedia *contentmedia = new tTVPContentMedia();
	TVPRegisterStorageMedia( contentmedia );
	contentmedia->Release();
}
//---------------------------------------------------------------------------
