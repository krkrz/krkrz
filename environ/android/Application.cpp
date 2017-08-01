/**
 * メモ
 *
 * JNIとのやり取りに関して、各種寿命など以下のリンク文書が参考になる
 * Android開発者のためのJNI入門
 * http://techbooster.jpn.org/andriod/application/7264/
 * Javaを呼出して動かす（jobject、jstring、jclass）
 * http://simple-asta.blogspot.jp/p/javajobjectjstringjclass.html
 */
#include "tjsCommHead.h"

#include "tjsError.h"
#include "tjsDebug.h"

#include <jni.h>
#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <android/sensor.h>
#include <android/log.h>
//#include <android_native_app_glue.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include "Application.h"

#include "ScriptMgnIntf.h"
#include "SystemIntf.h"
#include "DebugIntf.h"
#include "TickCount.h"
#include "NativeEventQueue.h"
#include "CharacterSet.h"
#include "WindowForm.h"
#include "SysInitImpl.h"
#include "SystemControl.h"
#include "ActivityEvents.h"
#include "MsgIntf.h"
#include "FontSystem.h"
#include "GraphicsLoadThread.h"
#include "MsgLoad.h"

#include <ft2build.h>
#include FT_TRUETYPE_UNPATENTED_H
#include FT_SYNTHESIS_H
#include FT_BITMAP_H
extern FT_Library FreeTypeLibrary;
extern void TVPInitializeFont();

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "krkrz", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "krkrz", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "krkrz", __VA_ARGS__))

static const tjs_int TVP_VERSION_MAJOR = 1;
static const tjs_int TVP_VERSION_MINOR = 0;
static const tjs_int TVP_VERSION_RELEASE = 0;
static const tjs_int TVP_VERSION_BUILD = 1;

tTVPApplication* Application;

extern void TVPRegisterAssetMedia();
extern void TVPRegisterContentMedia();
extern void TVPSetSoundNativeParameter( int rate, int buffSize );
/**
 * Android 版のバージョン番号はソースコードに埋め込む
 * パッケージのバージョン番号はアプリのバージョンであって、エンジンのバージョンではないため
 * apk からバージョン番号を取得するのは好ましくない。
 */
void TVPGetFileVersionOf( tjs_int& major, tjs_int& minor, tjs_int& release, tjs_int& build ) {
	major = TVP_VERSION_MAJOR;
	minor = TVP_VERSION_MINOR;
	release = TVP_VERSION_RELEASE;
	build = TVP_VERSION_BUILD;
}

/**
 * WM_... のように AM_... でメッセージを作ると理解早いかな
 *
 * 初期化は複数段階で行う必要がある
 * native 初期化 > Java 側初期化 > Java側からnativeへ諸設定 > nativeスレッド本格始動
 */

tTVPApplication::tTVPApplication()
: jvm_(nullptr), window_(nullptr), asset_manager_(nullptr),
 config_(nullptr,AConfiguration_delete), old_config_(nullptr,AConfiguration_delete),
 is_terminate_(true), main_window_(nullptr),
 console_cache_(1024), image_load_thread_(nullptr), thread_id_(-1),
 activity_active_(false)
{
}
tTVPApplication::~tTVPApplication() {
}

NativeEvent* tTVPApplication::createNativeEvent() {
	std::lock_guard<std::mutex> lock( command_cache_mutex_ );
	if( command_cache_.empty() ) {
		return new NativeEvent();
	} else {
		NativeEvent* ret = command_cache_.back();
		command_cache_.pop_back();
		return ret;
	}
}
void tTVPApplication::releaseNativeEvent( NativeEvent* ev ) {
	std::lock_guard<std::mutex> lock( command_cache_mutex_ );
	command_cache_.push_back( ev );
}
// コマンドをメインのメッセージループに投げる
void tTVPApplication::postEvent( const NativeEvent* ev, NativeEventQueueIntarface* handler ) {
	NativeEvent* e = createNativeEvent();
	e->Message = ev->Message;
	e->WParam = ev->WParam;
	e->LParam = ev->LParam;
	{
		std::lock_guard<std::mutex> lock( command_que_mutex_ );
		command_que_.push( EventCommand( handler, e ) );
	}

	// メインスレッドを起こす
	wakeupMainThread();
}
void tTVPApplication::wakeupMainThread() {
	main_thread_cv_.notify_one();
}
void tTVPApplication::handleIdle() {
}
void tTVPApplication::mainLoop() {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);	// attach thread to java
	// ここの env は、TJS VM のメインスレッド内共通なので、スレッドIDと共に保持して、各種呼び出し時に使いまわす方が効率的か
	while( is_terminate_ == false ) {
		{	// イベントキューからすべてのイベントをディスパッチ
			NativeEventQueueIntarface* handler;
			NativeEvent* event;
			do {
				handler = nullptr;
				event = nullptr;
				{
					std::lock_guard<std::mutex> lock( command_que_mutex_ );
					if( !command_que_.empty() ) {
						handler = command_que_.front().target;
						event = command_que_.front().command;
						command_que_.pop();
					}
				}
				if( event ) {
					if (handler != nullptr) {
						// ハンドラ指定付きの場合はハンドラから探して見つからったらディスパッチ
						std::lock_guard<std::mutex> lock(event_handlers_mutex_);
						auto result = std::find_if(event_handlers_.begin(), event_handlers_.end(),
												   [handler](NativeEventQueueIntarface *x) {
													   return x == handler;
												   });
						if (result != event_handlers_.end()) {
							(*result)->Dispatch(*event);
						}
					} else {
						if (appDispatch(*event) == false) {
							// ハンドラ指定のない場合でアプリでディスパッチしないものは、すべてのハンドラでディスパッチ
							std::lock_guard<std::mutex> lock(event_handlers_mutex_);
							for (std::vector<NativeEventQueueIntarface *>::iterator it = event_handlers_.begin();
								 it != event_handlers_.end(); it++) {
								if ((*it) != nullptr) {
									(*it)->Dispatch(*event);
								}
							}
						}
					}
					releaseNativeEvent(event);
				}
			} while( event );
			// アイドル処理
			handleIdle();
		}
		{	// コマンドキューに何か入れられるまで待つ
			std::unique_lock<std::mutex> uniq_lk(command_que_mutex_);
			main_thread_cv_.wait(uniq_lk, [this]{ return !command_que_.empty();});
		}
	}

	try {
		// image_load_thread_->ExitRequest();
		delete image_load_thread_;
		image_load_thread_ = nullptr;
	} catch(...) {
		// ignore errors
	}

	if( attached ) detachJavaEnv();
}
bool tTVPApplication::appDispatch(NativeEvent& ev) {
	switch( ev.Message ) {
		case AM_STARTUP_SCRIPT:
			TVPInitializeStartupScript();
			return true;
	}
	return false;
}
void tTVPApplication::HandleMessage( NativeEvent& ev ) {
	std::lock_guard<std::mutex> lock( event_handlers_mutex_ );
	for( std::vector<NativeEventQueueIntarface*>::iterator it = event_handlers_.begin(); it != event_handlers_.end(); it++ ) {
		if( (*it) != NULL ) (*it)->Dispatch( ev );
	}
}

void tTVPApplication::initializeApplication() {
	TVPTerminateCode = 0;

	try {
		// asset:// を登録
		TVPRegisterAssetMedia();

		// content:// を登録
		TVPRegisterContentMedia();

		// スクリプトエンジンを初期化し各種クラスを登録
		TVPInitScriptEngine();

		// ログへOS名等出力
		TVPAddImportantLog( TVPFormatMessage(TVPProgramStartedOn, TVPGetOSName(), TVPGetPlatformName()) );

		// アーカイブデリミタ、カレントディレクトリ、msgmap.tjsの実行 と言った初期化処理
		TVPInitializeBaseSystems();

		// -userconf 付きで起動されたかどうかチェックする。Android だと Activity 分けた方が賢明
		// if(TVPExecuteUserConfig()) return;

		// 非同期画像読み込みは後で実装する
		image_load_thread_ = new tTVPAsyncImageLoader();

		TVPSystemInit();

		SetTitle( tjs_string(TVPKirikiri) );

		TVPSystemControl = new tTVPSystemControl();

#ifndef TVP_IGNORE_LOAD_TPM_PLUGIN
//		TVPLoadPluigins(); // load plugin module *.tpm
#endif

		// start image load thread
		image_load_thread_->StartTread();

		// if(TVPProjectDirSelected) TVPInitializeStartupScript();

		// run main loop from activity resume.
	} catch(...) {
	}
}

void* tTVPApplication::startMainLoopCallback( void* myself ) {
	tTVPApplication* app = reinterpret_cast<tTVPApplication*>(myself);
	app->mainLoop();
	pthread_exit(0);
	return nullptr;
}
void tTVPApplication::startMainLoop() {
	if( is_terminate_ ) {
		is_terminate_ = false;
		pthread_attr_t attr;
		if( pthread_attr_init( &attr ) == 0 ) {
			pthread_attr_setstacksize( &attr, 64*1024 );
			pthread_create( &thread_id_, &attr, startMainLoopCallback, this );
			pthread_attr_destroy( &attr );
		} else {
			pthread_create( &thread_id_, 0, startMainLoopCallback, this );
		}
	}
}
void tTVPApplication::stopMainLoop() {
	if( is_terminate_ == false ) {
		is_terminate_ = true;
		wakeupMainThread();
		pthread_join( thread_id_, 0 );
	}
}
void tTVPApplication::checkConfigurationUpdate() {
	if( !config_ || !old_config_ ) return;

	int32_t diff = AConfiguration_diff( old_config_.get(), config_.get() );
	
	// Mobile Country Code(電気通信事業者運用地域)
	//if( diff & ACONFIGURATION_MCC ){}

	// Mobile Network Code(電気通信事業者識別コード)
	//if( diff & ACONFIGURATION_MNC ){}

	// ロケール
	//if( diff & ACONFIGURATION_LOCALE ){}

	// タッチスクリーン
	//if( diff & ACONFIGURATION_TOUCHSCREEN ){}

	// キーボード
	//if( diff & ACONFIGURATION_KEYBOARD ){}

	// キーボード
	// if( diff & ACONFIGURATION_KEYBOARD_HIDDEN ){}

	// ナビゲーション
	// if( diff & ACONFIGURATION_NAVIGATION ){}

	if( diff & ACONFIGURATION_ORIENTATION ) {
		int32_t density = AConfiguration_getDensity( config_.get() );
		tjs_int orient = orientUnknown;
		switch( AConfiguration_getOrientation( config_.get() ) ) {
		case tTVPApplication::orientUnknown:
			orient = orientUnknown;
		case tTVPApplication::orientPortrait:
			orient = orientPortrait;
		case tTVPApplication::orientLandscape:
			orient = orientLandscape;
		case tTVPApplication::orientSquare:	// not used
			orient = orientUnknown;
		}
		SendMessageFromJava( AM_DISPLAY_ROTATE, orient, density );
	}

	// 解像度
	// if( diff & ACONFIGURATION_DENSITY ){}

	// スクリーンサイズ
	// if( diff & ACONFIGURATION_SCREEN_SIZE ){}

	// プラットフォームバージョン
	// if( diff & ACONFIGURATION_VERSION ){}

	// スクリーンレイアウト
	// if( diff & ACONFIGURATION_SCREEN_LAYOUT ){}

	// UIモード
	// if( diff & ACONFIGURATION_UI_MODE ){}

	// 最小画面幅
	// if( diff & ACONFIGURATION_SMALLEST_SCREEN_SIZE ){}

	// レイアウトの向き
	// if( diff & ACONFIGURATION_LAYOUTDIR ){}
}
//-----------------------------

std::vector<std::string>* LoadLinesFromFile( const tjs_string& path ) {
	std::string npath;
	if( TVPUtf16ToUtf8( npath, path ) == false ) {
		return nullptr;
	}
	FILE *fp = fopen( npath.c_str(), "r");
    if( fp == nullptr ) {
		return nullptr;
    }
	char buff[1024];
	std::vector<std::string>* ret = new std::vector<std::string>();
    while( fgets(buff, 1024, fp) != nullptr ) {
		ret->push_back( std::string(buff) );
    }
    fclose(fp);
	return ret;
}
void tTVPApplication::writeBitmapToNative( const void * src ) {
	int32_t format = ANativeWindow_getFormat( window_ );
	ARect dirty;
	dirty.left = 0;
	dirty.top = 0;
	dirty.right = ANativeWindow_getWidth( window_ );
	dirty.bottom = ANativeWindow_getHeight( window_ );
	ANativeWindow_Buffer buffer;
	ANativeWindow_lock( window_ , &buffer, &dirty );
	unsigned char* bits = (unsigned char*)buffer.bits;
	for( int32_t y = 0; y < buffer.height; y++ ) {
		unsigned char* lines = bits;
		for( int32_t x = 0; x < buffer.width; x++ ) {
			// src を書き込む
			lines[0] = 0xff;
			lines[1] = 0xff;
			lines[2] = 0;
			lines[3] = 0xff;
			lines += 4;
		}
		bits += buffer.stride*sizeof(int32_t);
	}
	ANativeWindow_unlockAndPost( window_  );
}
void tTVPApplication::AddWindow( TTVPWindowForm* window ) {
	if( main_window_ ) {
		TVPThrowExceptionMessage(TJS_W("Cannot add window. Window has already been added."));	// TODO move to resource
	}
	main_window_ = window;
}
void tTVPApplication::PrintConsole( const tjs_char* mes, unsigned long len, bool iserror ) {
	if( console_cache_.size() < (len*3+1) ) {
		console_cache_.resize(len*3+1);
	}
	tjs_int u8len = TVPWideCharToUtf8String( mes, &(console_cache_[0]) );
	console_cache_[u8len] = '\0';
	if( iserror ) {
		__android_log_print(ANDROID_LOG_ERROR, "krkrz", "%s", &(console_cache_[0]) );
	} else {
		__android_log_print(ANDROID_LOG_INFO, "krkrz", "%s", &(console_cache_[0]) );
	}
}

// /system/fonts/ フォントが置かれているフォルダから取得する(Nexus5で約50msかかる)
// フォントが最初に使われる時にFontSystem::InitFontNames経由で呼ばれる
extern void TVPAddSystemFontToFreeType( const std::string& storage, std::vector<tjs_string>* faces );
extern void TVPGetSystemFontListFromFreeType( std::vector<tjs_string>& faces );
static bool TVPIsGetAllFontList = false;
void TVPGetAllFontList( std::vector<tjs_string>& list ) {
	TVPInitializeFont();
	if( TVPIsGetAllFontList ) {
		TVPGetSystemFontListFromFreeType( list );
	}

	DIR* dr;
	if( ( dr = opendir("/system/fonts/") ) != nullptr ) {
		struct dirent* entry;
		while( ( entry = readdir( dr ) ) != nullptr ) {
			if( entry->d_type == DT_REG ) {
				std::string path(entry->d_name);
				std::string::size_type extp = path.find_last_of(".");
				if( extp != std::string::npos ) {
					std::string ext = path.substr(extp);
					if( ext == std::string(".ttf") || ext == std::string(".ttc") || ext == std::string(".otf") ) {
						// .ttf | .ttc | .otf
						std::string fullpath( std::string("/system/fonts/") + path );
						TVPAddSystemFontToFreeType( fullpath, &list );
					}
				}
			}
		}
		closedir( dr );
		TVPIsGetAllFontList = true;
	}
#if 0
	for( std::list<std::string>::const_iterator i = fontfiles.begin(); i != fontfiles.end(); ++i ) {
		FT_Face face = nullptr;
		std::string fullpath( std::string("/system/fonts/") + *i );
		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		args.flags = FT_OPEN_PATHNAME;
		args.pathname = fullpath.c_str();
		tjs_uint face_num = 1;
		std::list<std::string> facenames;
		for( tjs_uint f = 0; f < face_num; f++ ) {
			FT_Error err = FT_Open_Face( FreeTypeLibrary, &args, 0, &face);
			if( err == 0 ) {
				facenames.push_back( std::string(face->family_name) );
				std::string(face->style_name);	// スタイル名
				if( face->face_flags & FT_FACE_FLAG_SCALABLE ) {
					// 可変サイズフォントのみ採用
					if( face->num_glyphs > 2965 ) {
						// JIS第一水準漢字以上のグリフ数
						if( face->style_flags & FT_STYLE_FLAG_ITALIC ) {}
						if( face->style_flags & FT_STYLE_FLAG_BOLD ) {}
						face_num = face->num_faces;
						int numcharmap = face->num_charmaps;
						for( int c = 0; c < numcharmap; c++ ) {
							FT_Encoding enc = face->charmaps[c]->encoding;
							if( enc == FT_ENCODING_SJIS ) {
								// mybe japanese
							}
							if( enc == FT_ENCODING_UNICODE ) {
							}
						}
					}
				}
			}
			if(face) FT_Done_Face(face), face = nullptr;
		}
	}
#endif
}
static bool IsInitDefalutFontName = false;
static bool SelectFont( const std::vector<tjs_string>& faces, tjs_string& face ) {
	std::vector<tjs_string> fonts;
	TVPGetAllFontList( fonts );
	for( auto i = faces.begin(); i != faces.end(); ++i ) {
		auto found = std::find( fonts.begin(), fonts.end(), *i );
		if( found != fonts.end() ) {
			face = *i;
			return true;
		}
	}
	return false;
}
const tjs_char *TVPGetDefaultFontName() {
	if( IsInitDefalutFontName ) {
		return TVPDefaultFontName;
	}
	TVPDefaultFontName.AssignMessage(TJS_W("Droid Sans Mono"));
	IsInitDefalutFontName =  true;

	// コマンドラインで指定がある場合、そのフォントを使用する
	tTJSVariant opt;
	if(TVPGetCommandLine(TJS_W("-deffont"), &opt)) {
		ttstr str(opt);
		TVPDefaultFontName.AssignMessage( str.c_str() );
	} else {
		std::string lang( Application->getLanguage() );
		tjs_string face;
		if( lang == std::string("ja" ) ) {
			std::vector<tjs_string> facenames{tjs_string(TJS_W("Noto Sans JP")),tjs_string(TJS_W("Noto Sans CJK JP")),tjs_string(TJS_W("MotoyaLMaru")),
				tjs_string(TJS_W("MotoyaLCedar")),tjs_string(TJS_W("Droid Sans Japanese")),tjs_string(TJS_W("Droid Sans Mono"))};
			if( SelectFont( facenames, face ) ) {
				TVPDefaultFontName.AssignMessage( face.c_str() );
			}
		} else if( lang == std::string("zh" ) ) {
			std::vector<tjs_string> facenames{tjs_string(TJS_W("Noto Sans SC")),tjs_string(TJS_W("Droid Sans Mono"))};
			if( SelectFont( facenames, face ) ) {
				TVPDefaultFontName.AssignMessage( face.c_str() );
			}
		} else if( lang == std::string("ko" ) ) {
			std::vector<tjs_string> facenames{tjs_string(TJS_W("Noto Sans KR")),tjs_string(TJS_W("Droid Sans Mono"))};
			if( SelectFont( facenames, face ) ) {
				TVPDefaultFontName.AssignMessage( face.c_str() );
			}
		} else {
			std::vector<tjs_string> facenames{tjs_string(TJS_W("Droid Sans Mono"))};
			if( SelectFont( facenames, face ) ) {
				TVPDefaultFontName.AssignMessage( face.c_str() );
			}
		}
	}
	return TVPDefaultFontName;
}
void TVPSetDefaultFontName( const tjs_char * name ) {
	TVPDefaultFontName.AssignMessage( name );
}
static ttstr TVPDefaultFaceNames;
/**
 * Androidの場合、デフォルトフォントだと各地域固有の文字のみしか入っていないので、Roboto,Droid Sans Monoも候補として返す
 */
const ttstr &TVPGetDefaultFaceNames() {
	if( !TVPDefaultFaceNames.IsEmpty() ) {
		return TVPDefaultFaceNames;
	} else {
		TVPDefaultFaceNames = ttstr( TVPGetDefaultFontName() );
		std::string lang( Application->getLanguage() );
		if( lang == std::string("ja" ) ) {
			// TODO:存在確認などしてもうちょっと最適なフェイスリストを作る用にした方がいい
			TVPDefaultFaceNames += ttstr(TJS_W("Noto Sans,MotoyaLMaru,Roboto"));
		} else {
			TVPDefaultFaceNames += ttstr(TJS_W(",Roboto"));
		}
		return TVPDefaultFaceNames;
	}
}

void tTVPApplication::getStringFromJava( const char* methodName, tjs_string& dest ) const {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "()Ljava/lang/String;");
		jstring ret = (jstring) env->CallObjectMethod(thiz, mid, nullptr);
		int jstrlen = env->GetStringLength(ret);
		const jchar* chars = env->GetStringChars( ret, nullptr );
		dest = tjs_string( chars, &chars[jstrlen] );
		env->ReleaseStringChars( ret, chars );
		env->DeleteLocalRef( ret );
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
void tTVPApplication::setStringToJava( const char* methodName, const tjs_string& src ) {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "(Ljava/lang/String;)V");
		jstring arg = env->NewString( reinterpret_cast<const jchar *>(src.c_str()), src.length() );
		env->CallVoidMethod(thiz, mid, arg);
		env->DeleteLocalRef( arg );
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
void tTVPApplication::getLongToJava( const char* methodName, tjs_int64 src ) {
    bool attached;
    JNIEnv *env = getJavaEnv(attached);
    if ( env != nullptr ) {
        jobject thiz = activity_;
        jclass clazz = env->GetObjectClass(thiz);
        jmethodID mid = env->GetMethodID(clazz, methodName, "(J)V");
        env->CallVoidMethod(thiz, mid, src);
        env->DeleteLocalRef(clazz);
        if( attached ) detachJavaEnv();
    }
}
void tTVPApplication::getFloatToJava( const char* methodName, float src ) {
    bool attached;
    JNIEnv *env = getJavaEnv(attached);
    if ( env != nullptr ) {
        jobject thiz = activity_;
        jclass clazz = env->GetObjectClass(thiz);
        jmethodID mid = env->GetMethodID(clazz, methodName, "(F)V");
        env->CallVoidMethod(thiz, mid, src );
        env->DeleteLocalRef(clazz);
        if( attached ) detachJavaEnv();
    }
}
void tTVPApplication::callActivityMethod( const char* methodName ) const {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "()V");
		env->CallVoidMethod(thiz, mid, nullptr);
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
void tTVPApplication::getIntegerFromJava( const char* methodName, tjs_int& dest ) const {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "()I");
		int ret = (int)env->CallIntMethod(thiz, mid, nullptr);
		dest = ret;
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
void tTVPApplication::getBooleanFromJava( const char* methodName, bool& dest ) const {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "()Z");
		jboolean ret = (jboolean)env->CallBooleanMethod(thiz,mid, nullptr);
		dest = ret == JNI_TRUE;
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
void tTVPApplication::getLongFromJava( const char* methodName, tjs_int64& dest ) const {
    bool attached;
    JNIEnv *env = getJavaEnv(attached);
    if ( env != nullptr ) {
        jobject thiz = activity_;
        jclass clazz = env->GetObjectClass(thiz);
        jmethodID mid = env->GetMethodID(clazz, methodName, "()J");
        jlong ret = (int)env->CallLongMethod(thiz, mid, nullptr);
        dest = ret;
        env->DeleteLocalRef(clazz);
        if( attached ) detachJavaEnv();
    }
}
void tTVPApplication::getFloatFromJava( const char* methodName, float& dest ) const {
    bool attached;
    JNIEnv *env = getJavaEnv(attached);
    if ( env != nullptr ) {
        jobject thiz = activity_;
        jclass clazz = env->GetObjectClass(thiz);
        jmethodID mid = env->GetMethodID(clazz, methodName, "()F");
        float ret = (int)env->CallFloatMethod(thiz, mid, nullptr);
        dest = ret;
        env->DeleteLocalRef(clazz);
        if( attached ) detachJavaEnv();
    }
}
const tjs_string& tTVPApplication::GetInternalDataPath() const {
	if( internal_data_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getInternalDataPath"), const_cast<tjs_string&>(internal_data_path_) );
	}
	return internal_data_path_;
}
const tjs_string& tTVPApplication::GetExternalDataPath() const {
	if( external_data_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalDataPath"), const_cast<tjs_string&>(external_data_path_) );
	}
	return external_data_path_;
}
const tjs_string* tTVPApplication::GetCachePath() const {
	if( cache_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getCachePath"), const_cast<tjs_string&>(cache_path_) );
	}
	return &cache_path_;
}
const tjs_string* tTVPApplication::GetExternalCachePath() const {
	if( external_cache_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalCachePath"), const_cast<tjs_string&>(external_cache_path_) );
	}
	return &external_cache_path_;
}
const tjs_string* tTVPApplication::GetObbPath() const {
	if( obb_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getObbPath"), const_cast<tjs_string&>(obb_path_) );
	}
	return &obb_path_;
}
const tjs_char* tTVPApplication::GetPackageName() const {
	if( package_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getPackageName"), const_cast<tjs_string&>(package_path_) );
	}
	return package_path_.c_str();
}
const tjs_char* tTVPApplication::GetPackageCodePath() const {
	if( package_code_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getPackageCodePath"), const_cast<tjs_string&>(package_code_path_) );
	}
	return package_code_path_.c_str();
}
const tjs_char* tTVPApplication::GetPackageResourcePath() const {
	if( package_resource_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getPackageResourcePath"), const_cast<tjs_string&>(package_resource_path_) );
	}
	return package_resource_path_.c_str();
}
const tjs_char* tTVPApplication::GetSoPath() const {
	if( so_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getSoPath"), const_cast<tjs_string&>(so_path_) );
	}
	return so_path_.c_str();
}
const tjs_string* tTVPApplication::GetDataDirectory() const {
	if( data_directory_.empty() ) {
		getStringFromJava( static_cast<const char*>("getDataDirectory"), const_cast<tjs_string&>(data_directory_) );
	}
	return &data_directory_;
}
const tjs_string* tTVPApplication::GetDownloadCacheDirectory() const {
	if( download_cache_directory_.empty() ) {
		getStringFromJava( static_cast<const char*>("getDownloadCacheDirectory"), const_cast<tjs_string&>(download_cache_directory_) );
	}
	return &download_cache_directory_;
}
const tjs_string* tTVPApplication::GetExternalStorageDirectory() const {
	if( external_storage_directory_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStorageDirectory"), const_cast<tjs_string&>(external_storage_directory_) );
	}
	return &external_storage_directory_;
}
const tjs_string* tTVPApplication::GetExternalPublicMusicPath() const {
	if( external_public_music_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryMusic"), const_cast<tjs_string&>(external_public_music_path_) );
	}
	return &external_public_music_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicPodcastsPath() const {
	if( external_public_podcasts_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryPodcasts"), const_cast<tjs_string&>(external_public_podcasts_path_) );
	}
	return &external_public_podcasts_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicRingtonesPath() const {
	if( external_public_ringtones_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryRingtones"), const_cast<tjs_string&>(external_public_ringtones_path_) );
	}
	return &external_public_ringtones_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicAlaramsPath() const {
	if( external_public_alarams_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryAlarms"), const_cast<tjs_string&>(external_public_alarams_path_) );
	}
	return &external_public_alarams_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicNotificationsPath() const {
	if( external_public_notifications_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryNotifications"), const_cast<tjs_string&>(external_public_notifications_path_) );
	}
	return &external_public_notifications_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicPicturesPath() const {
	if( external_public_pictures_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryPictures"), const_cast<tjs_string&>(external_public_pictures_path_) );
	}
	return &external_public_pictures_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicMoviesPath() const {
	if( external_public_movies_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryMovies"), const_cast<tjs_string&>(external_public_movies_path_) );
	}
	return &external_public_movies_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicDownloadsPath() const {
	if( external_public_downloads_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryDownloads"), const_cast<tjs_string&>(external_public_downloads_path_) );
	}
	return &external_public_downloads_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicDCIMPath() const {
	if( external_public_dcim_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryDCIM"), const_cast<tjs_string&>(external_public_dcim_path_) );
	}
	return &external_public_dcim_path_;
}
const tjs_string* tTVPApplication::GetExternalPublicDocumentsPath() const {
	if( external_public_documents_path_.empty() ) {
		getStringFromJava( static_cast<const char*>("getExternalStoragePublicDirectoryDocuments"), const_cast<tjs_string&>(external_public_documents_path_) );
	}
	return &external_public_documents_path_;
}
const tjs_string* tTVPApplication::GetRootDirectory() const {
	if( root_directory_.empty() ) {
		getStringFromJava( static_cast<const char*>("getRootDirectory"), const_cast<tjs_string&>(root_directory_) );
	}
	return &root_directory_;
}
tjs_string tTVPApplication::GetExternalStorageState() const {
	tjs_string ret;
	getStringFromJava( static_cast<const char*>("getExternalStorageState"), ret );
	return ret;
}
bool tTVPApplication::IsExternalStorageEmulated() const {
	bool ret;
	getBooleanFromJava( static_cast<const char*>("isExternalStorageEmulated"), ret );
	return ret;
}
bool tTVPApplication::IsExternalStorageRemovable() const {
	bool ret;
	getBooleanFromJava( static_cast<const char*>("isExternalStorageRemovable"), ret );
	return ret;
}

// 動画オープン
void tTVPApplication::OpenMovie( const tjs_char* path ) {
	setStringToJava( "postOpenMovie", tjs_string(path) );
}
// 動画再生
void tTVPApplication::PlayMovie() {
	callActivityMethod( "postPlayMovie" );
}
// 動画停止
void tTVPApplication::StopMovie() {
    callActivityMethod( "postStopMovie" );
}
// 動画停止
void tTVPApplication::PauseMovie() {
	callActivityMethod( "postPauseMovie" );
}
tjs_int64 tTVPApplication::GetMovieCurrentPosition() const {
    tjs_int64 ret = 0;
    getLongFromJava( "getMovieCurrentPosition", ret );
    return ret;
}
void tTVPApplication::SetMovieCurrentPosition( tjs_int64 pos ) {
    getLongToJava( "setMovieCurrentPosition", pos );
}
tjs_int64 tTVPApplication::GetMovieDuration() const {
    tjs_int64 ret = 0;
    getLongFromJava( "getMovieDuration", ret );
    return ret;
}
float tTVPApplication::GetMovieVolume() const {
    float ret = 0.0f;
    getFloatFromJava( "getMovieVolume", ret );
    return ret;
}
void tTVPApplication::SetMovieVolume( float vol ) {
    getFloatToJava( "setMovieVolume", vol );
}
bool tTVPApplication::GetMovieVisible() const {
	bool v = false;
	getBooleanFromJava( "getMovieVisible", v );
	return v;
}
tjs_int tTVPApplication::GetDisplayRotate() const {
	tjs_int rot = -1;
	getIntegerFromJava( static_cast<const char*>("getDisplayRotate"), rot );
	return rot;
}
tjs_int tTVPApplication::GetMainViewWidth() const {
	tjs_int w = 0;
	getIntegerFromJava( static_cast<const char*>("getMainViewWidth"), w );
	return w;
}
tjs_int tTVPApplication::GetMainViewHeight() const {
	tjs_int h = 0;
	getIntegerFromJava( static_cast<const char*>("getMainViewHeight"), h );
	return h;
}
tjs_int tTVPApplication::GetActivityWidth() const {
	tjs_int w = 0;
	getIntegerFromJava( static_cast<const char*>("getActivityWidth"), w );
	return w;
}
tjs_int tTVPApplication::GetActivityHeight() const {
	tjs_int h = 0;
	getIntegerFromJava( static_cast<const char*>("getActivityHeight"), h );
	return h;
}
tjs_int tTVPApplication::GetScreenWidth() const {
    tjs_int w = 0;
    getIntegerFromJava( static_cast<const char*>("getScreenWidth"), w );
    return w;
}
tjs_int tTVPApplication::GetScreenHeight() const {
    tjs_int h = 0;
    getIntegerFromJava( static_cast<const char*>("getScreenHeight"), h );
    return h;
}
int tTVPApplication::GetOpenGLESVersionCode() const {
	int ret;
	getIntegerFromJava( static_cast<const char*>("getOpenGLESVersionCode"), ret );
	return ret;
}
void tTVPApplication::finishActivity() {
	callActivityMethod( "postFinish" );
	stopMainLoop();
}
void tTVPApplication::changeSurfaceSize( int w, int h ) {
	bool attached;
	JNIEnv *env = getJavaEnv(attached);
	if ( env != nullptr ) {
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, "postChangeSurfaceSize", "(II)V");
		env->CallVoidMethod(thiz, mid, w, h );
		env->DeleteLocalRef(clazz);
		if( attached ) detachJavaEnv();
	}
}
const tjs_string& tTVPApplication::getSystemVersion() const {
	if( system_release_version_.empty() ) {
		bool attached;
		JNIEnv *env = getJavaEnv(attached);
		if ( env != nullptr ) {
			jclass versionClass = env->FindClass("android/os/Build$VERSION" );
			jfieldID releaseFieldID = env->GetStaticFieldID(versionClass, "RELEASE", "Ljava/lang/String;" );
			jstring ret = (jstring)env->GetStaticObjectField(versionClass, releaseFieldID );
			int jstrlen = env->GetStringLength(ret);
			const jchar* chars = env->GetStringChars( ret, nullptr );
			tjs_string& dest = const_cast<tjs_string&>(system_release_version_);
			dest = tjs_string( chars, &chars[jstrlen] );
			env->ReleaseStringChars( ret, chars );
			env->DeleteLocalRef( ret );
			env->DeleteLocalRef(versionClass);
			if( attached ) detachJavaEnv();
		}
	}
	return system_release_version_;
}
tjs_string tTVPApplication::GetActivityCaption() {
	tjs_string caption;
	getStringFromJava( "getCaption", caption );
	return caption;
}
void tTVPApplication::SetActivityCaption( const tjs_string& caption ) {
	setStringToJava( "postChangeCaption", caption );
}
void tTVPApplication::ShowToast( const tjs_char* text ) {
	setStringToJava( "postShowToastMessage", tjs_string(text) );
}
int tTVPApplication::MessageDlg( const tjs_string& string, const tjs_string& caption, int type, int button ) {
	Application->ShowToast( string.c_str() );
	return 0;
}
void tTVPApplication::LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name ) {
	if( image_load_thread_ ) {
		image_load_thread_->LoadRequest( owner, bmp, name );
	}
}
void tTVPApplication::callActivityEventResume() {
	for( auto ev : activity_ev_handlers_ ){
		ev->onResume();
	}
}
void tTVPApplication::callActivityEventPause() {
	for( auto ev : activity_ev_handlers_ ){
		ev->onPause();
	}
}
/**
 * Java から送られてきた各種イベントをここで処理する
 * イベントの種類に応じてアプリケーションとして処理するか、Windowに処理させるか判断
 */
void tTVPApplication::SendMessageFromJava( tjs_int message, tjs_int64 wparam, tjs_int64 lparam ) {
	// Main Windowが存在する場合はそのWindowへ送る
	NativeEvent ev(message,wparam,lparam);
	switch( message ) {
	case AM_STARTUP_SCRIPT:
		postEvent( &ev, nullptr );
		return;
	case AM_START:
	case AM_RESTART:
		return;
	case AM_RESUME:
		activity_active_ = true;
		callActivityEventResume();
		break;
	case AM_PAUSE:
		activity_active_ = false;
		callActivityEventPause();
		break;
	case AM_STOP:
		break;
	case AM_DESTROY:
		stopMainLoop();
		return;
	case AM_SURFACE_CHANGED:
	case AM_SURFACE_CREATED:
	case AM_SURFACE_DESTORYED:
		break;
	}
	TTVPWindowForm* win = GetMainWindow();
	if( win ) {
		postEvent( &ev, win->GetEventHandler() );
	} else {
		postEvent( &ev, nullptr );
	}
}
static const int TOUCH_DOWN = 0;
static const int TOUCH_MOVE = 1;
static const int TOUCH_UP = 2;
void tTVPApplication::SendTouchMessageFromJava( tjs_int type, float x, float y, float c, int id, tjs_int64 tick ) {
	NativeEvent ev;
	switch( type ) {
	case TOUCH_DOWN:
		ev.Message = AM_TOUCH_DOWN;
		break;
	case TOUCH_MOVE:
		ev.Message = AM_TOUCH_MOVE;
		break;
	case TOUCH_UP:
		ev.Message = AM_TOUCH_UP;
		break;
	default:
		return;
	}
	ev.WParamf0 = x;
	ev.WParamf1 = y;
	ev.LParamf0 = c;
	ev.LParam1 = id;
	ev.Result = tick;
	TTVPWindowForm* win = GetMainWindow();
	if( win ) {
		postEvent( &ev, win->GetEventHandler() );
	} else {
		postEvent( &ev, nullptr );
	}
}
void tTVPApplication::setWindow( ANativeWindow* window ) {
	// std::lock_guard<std::mutex> lock( main_thread_mutex_ );
	if( window_ ) {
		// release previous window reference
		ANativeWindow_release( window_ );
	}

	window_ = window;

	if( window ) {
		// acquire window reference
		ANativeWindow_acquire( window );
	}
}
void tTVPApplication::nativeSetSurface(JNIEnv *jenv, jobject obj, jobject surface) {
	if( surface != 0 ) {
		ANativeWindow* window = ANativeWindow_fromSurface(jenv, surface);
		LOGI("Got window %p", window);
        if( window != nullptr && Application->getWindow() != window ) {
            Application->setWindow(window);
            Application->SendMessageFromJava( AM_SURFACE_CHANGED, 0, 0 );
        }
	} else {
		LOGI("Releasing window");
		Application->setWindow(nullptr);
		Application->SendMessageFromJava( AM_SURFACE_DESTORYED, 0, 0 );
	}
	return;
}

void jstrcpy_maxlen(tjs_char *d, const jchar *s, size_t len)
{
	tjs_char ch;
	len++;
	while((ch=*s)!=0 && --len) *(d++) = ch, s++;
	*d = 0;
}
class tTVPMessageResourceProvider : public iTVPMessageResourceProvider {
	JNIEnv* env_;
	jobjectArray array_;
	jstring work_;
	tjs_int count_;
	tjs_int last_index_;
public:
	tTVPMessageResourceProvider( JNIEnv *env, jobjectArray a ) : env_(env), array_(a), last_index_(-1) {
		count_ = env->GetArrayLength(a);
	}
	virtual const tjs_char* GetMessage( tjs_int index, tjs_uint& length ) {
		if( index >= count_ ) {
			length = 0;
			return nullptr;
		}
		work_ = (jstring)env_->GetObjectArrayElement( array_, index );
		length = env_->GetStringLength( work_ );
		last_index_ = index;
		return reinterpret_cast<const tjs_char*>(env_->GetStringChars( work_, nullptr ));
	}
	virtual void ReleaseMessage( const tjs_char* mes, tjs_int index ) {
		assert( last_index_ == index );
		env_->ReleaseStringChars( work_, reinterpret_cast<const jchar*>(mes) );
		env_->DeleteLocalRef( work_ );
		work_ = nullptr;
	}
};
void tTVPApplication::nativeSetMessageResource(JNIEnv *jenv, jobject obj, jobjectArray mesarray) {
	tTVPMessageResourceProvider provider( jenv, mesarray );
	TVPLoadMessage( &provider );
	/*
	int stringCount = jenv->GetArrayLength(mesarray);
	for( int i = 0; i < stringCount; i++ ) {
		jstring string = (jstring) jenv->GetObjectArrayElement( mesarray, i);
		int jstrlen = jenv->GetStringLength( string );
		const jchar* chars = jenv->GetStringChars( string, nullptr );
		// copy message. 解放しない
		tjs_char* mesres = new tjs_char[jstrlen+1];
		jstrcpy_maxlen( mesres, chars, jstrlen );
		mesres[jstrlen] = TJS_W('\0');
		jenv->ReleaseStringChars( string, chars );
		jenv->DeleteLocalRef( string );
	}
	 */
}

void tTVPApplication::nativeSetAssetManager(JNIEnv *jenv, jobject obj, jobject assetManager ) {
	AAssetManager* am = AAssetManager_fromJava( jenv, assetManager );
	Application->setAssetManager( am );
}
/**
 * Java からの通知はここに来る
 */
void tTVPApplication::nativeToMessage(JNIEnv *jenv, jobject obj, jint mes, jlong wparam, jlong lparam ) {
	Application->SendMessageFromJava( mes, wparam, lparam );
}
void tTVPApplication::nativeSetActivity(JNIEnv *env, jobject obj, jobject activity) {
	if( activity != nullptr ) {
		jobject globalactivity = env->NewGlobalRef(activity);
		Application->activity_ = globalactivity;
		// Activity の jclass や 必要となるメソッドIDはここで一気に取得してしまっていた方がいいかもしれない
		// jclass は頻繁に必要になるのでここで、メソッドIDは必要になった初回に取得が妥当か
	} else {
		if( Application->activity_ != nullptr ) {
			env->DeleteGlobalRef( Application->activity_ );
			Application->activity_ = nullptr;
		}
	}
}
void tTVPApplication::nativeInitialize(JNIEnv *jenv, jobject obj) {
	Application->initializeApplication();
}
void tTVPApplication::nativeOnTouch( JNIEnv *jenv, jobject obj, jint type, jfloat x, jfloat y, jfloat c, jint id, jlong tick ) {
	Application->SendTouchMessageFromJava( type, x, y, c, id, tick );
}
// サウンドHW/ドライバが最終出力するサンプリングレートとバッファサイズをJavaがで取得し、それをここで設定する
void tTVPApplication::nativeSetSoundNativeParameter( JNIEnv *jenv, jobject obj, jint rate, jint size )
{
	TVPSetSoundNativeParameter( rate, size );
}
// 起動パスを渡された時呼び出される
extern void TVPSetProjectPath( const ttstr& path );
void tTVPApplication::nativeSetStartupPath( JNIEnv *jenv, jobject obj, jstring jpath ) {
	int jstrlen = jenv->GetStringLength( jpath );
	const jchar* chars = jenv->GetStringChars( jpath, nullptr );
	ttstr path(reinterpret_cast<const tjs_char*>(chars),jstrlen);
	jenv->ReleaseStringChars( jpath, chars );

	TVPSetProjectPath( path );
	Application->SendMessageFromJava( AM_STARTUP_SCRIPT, 0, 0 );
	Application->startMainLoop();
}
void tTVPApplication::nativeStartScript( JNIEnv *jenv, jobject obj ) {
	Application->SendMessageFromJava( AM_STARTUP_SCRIPT, 0, 0 );
	Application->startMainLoop();
}
JNIEnv* tTVPApplication::getJavaEnv( bool& attached ) const {
	attached = false;
	JNIEnv *env = nullptr;
	jint status = jvm_->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
	if( status == JNI_EDETACHED ) {
		jint astatus = jvm_->AttachCurrentThread( &env, nullptr );
		if( astatus != JNI_OK ) {
			// throw error
			TVPThrowExceptionMessage(TJS_W("Cannot attach java thread."));
			return nullptr;
		}
		attached = true;
	} else if( status != JNI_OK ) {
		TVPThrowExceptionMessage(TJS_W("Cannot retrieve java Env."));
	}
	return env;
}
void tTVPApplication::detachJavaEnv() const {
	jvm_->DetachCurrentThread();
}
static JNINativeMethod methods[] = {
		// Java側関数名, (引数の型)返り値の型, native側の関数名の順に並べます
		{ "nativeSetSurface", "(Landroid/view/Surface;)V", (void *)tTVPApplication::nativeSetSurface },
		{ "nativeSetMessageResource", "([Ljava/lang/String;)V", (void *)tTVPApplication::nativeSetMessageResource },
		{ "nativeSetAssetManager", "(Landroid/content/res/AssetManager;)V", (void *)tTVPApplication::nativeSetAssetManager },
		{ "nativeToMessage", "(IJJ)V", (void*)tTVPApplication::nativeToMessage },
		{ "nativeSetActivity", "(Landroid/app/Activity;)V", (void *)tTVPApplication::nativeSetActivity },
		{ "nativeInitialize", "()V", (void *)tTVPApplication::nativeInitialize},
		{ "nativeOnTouch", "(IFFFIJ)V", (void *)tTVPApplication::nativeOnTouch},
		{ "nativeSetStartupPath", "(Ljava/lang/String;)V", (void *)tTVPApplication::nativeSetStartupPath},
		{ "nativeSetSoundNativeParameter", "(II)V", (void*)tTVPApplication::nativeSetSoundNativeParameter},
		{ "nativeStartScript", "()V", (void*)tTVPApplication::nativeStartScript},
};

jint registerNativeMethods( JNIEnv* env, const char *class_name, JNINativeMethod *methods, int num_methods ) {
	int result = 0;
	jclass clazz = env->FindClass(class_name);
	if (clazz) {
		int result = env->RegisterNatives(clazz, methods, num_methods);
		if (result < 0) {
			LOGE("registerNativeMethods failed(class=%s)", class_name);
		}
	} else {
		LOGE("registerNativeMethods: class'%s' not found", class_name);
	}
	return result;
}
#define	NUM_ARRAY_ELEMENTS(p) ((int) sizeof(p) / sizeof(p[0]))
int registerJavaMethod( JNIEnv *env)  {
	if( registerNativeMethods(env, "jp/kirikiri/krkrz/BaseMainActivity", methods, NUM_ARRAY_ELEMENTS(methods)) < 0) {
		return -1;
	}
	return 0;
}

extern "C" jint JNI_OnLoad( JavaVM *vm, void *reserved ) {
	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	}
	Application = new tTVPApplication();
	Application->setJavaVM( vm );

	// register native methods
	int res = registerJavaMethod(env);

	return JNI_VERSION_1_6;
}
