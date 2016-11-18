
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
#include <android_native_app_glue.h>
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

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "krkrz", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "krkrz", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "krkrz", __VA_ARGS__))

static const tjs_int TVP_VERSION_MAJOR = 1;
static const tjs_int TVP_VERSION_MINOR = 0;
static const tjs_int TVP_VERSION_RELEASE = 0;
static const tjs_int TVP_VERSION_BUILD = 1;

tTVPApplication* Application;

extern void TVPRegisterAssetMedia();
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
: jvm_(nullptr), window_(nullptr), asset_manager_(nullptr), config_(nullptr), is_terminate_(true), main_window_(nullptr)
{
}
tTVPApplication::~tTVPApplication() {
}

NativeEvent* tTVPApplication::createNativeEvent() {
	std::lock_guard<std::mutex> lock( main_thread_mutex_ );
	if( command_cache_.empty() ) {
		return new NativeEvent();
	} else {
		NativeEvent* ret = command_cache_.back();
		command_cache_.pop_back();
		return ret;
	}
}
void tTVPApplication::releaseNativeEvent( NativeEvent* ev ) {
	std::lock_guard<std::mutex> lock( main_thread_mutex_ );
	command_cache_.push_back( ev );
}
// コマンドをメインのメッセージループに投げる
void tTVPApplication::postEvent( const NativeEvent* ev, NativeEventQueueIntarface* handler ) {
	std::lock_guard<std::mutex> lock( main_thread_mutex_ );
	NativeEvent* e = createNativeEvent();
	e->Message = ev->Message;
	e->WParam = ev->WParam;
	e->LParam = ev->LParam;
	command_que_.push( EventCommand( handler, e ) );

	// メインスレッドを起こす
	wakeupMainThread();
}
void tTVPApplication::wakeupMainThread() {
	main_thread_cv_.notify_one();
}
void tTVPApplication::mainLoop() {
	while( is_terminate_ ) {
		{	// イベントキューからすべてのイベントをディスパッチ
			std::lock_guard<std::mutex> lock( main_thread_mutex_ );
			while( !command_que_.empty() ) {
				NativeEventQueueIntarface* handler = command_que_.front().target;
				NativeEvent* event = command_que_.front().command;
				command_que_.pop();

				if( handler != nullptr ) {
					// ハンドラ指定付きの場合はハンドラから探して見つからったらディスパッチ
					auto result = std::find_if(event_handlers_.begin(), event_handlers_.end(), [handler](NativeEventQueueIntarface* x) { return x == handler; });
					if( result != event_handlers_.end() ) {
						(*result)->Dispatch( *event );
					}
				} else {
					// ハンドラ指定のない場合はすべてのハンドラでディスパッチ
					for( std::vector<NativeEventQueueIntarface*>::iterator it = event_handlers_.begin(); it != event_handlers_.end(); it++ ) {
						if( (*it) != nullptr ) {
							(*it)->Dispatch( *event );
						}
					}
				}
				releaseNativeEvent( event );
			}
			// アイドル処理
			handleIdle();
		}
		{	// コマンドキューに何か入れられるまで待つ
			std::unique_lock<std::mutex> uniq_lk(main_thread_mutex_);
			main_thread_cv_.wait(uniq_lk, [this]{ return !command_que_.empty();});
		}
	}
}
/*
int8_t tTVPApplication::readCommand() {
	int8_t cmd;
	if( read(user_msg_read_, &cmd, sizeof(cmd)) == sizeof(cmd) ) {
		return cmd;
	} else {
		LOGE("No data on command pipe!");
	}
	return -1;
}
int tTVPApplication::messagePipeCallBack(int fd, int events, void* user) {
	if( user != NULL ) {
		tTVPApplication* app = (tTVPApplication*)user;
		NativeEvent msg;
		while( read(fd, &msg, sizeof(NativeEvent)) == sizeof(NativeEvent) ) {
			app->HandleMessage(msg);
		}
	}
	return 1;
}
 */
void tTVPApplication::HandleMessage( NativeEvent& ev ) {
	for( std::vector<NativeEventQueueIntarface*>::iterator it = event_handlers_.begin(); it != event_handlers_.end(); it++ ) {
		if( (*it) != NULL ) (*it)->Dispatch( ev );
	}
}
// for iTVPApplication
void tTVPApplication::startApplication( struct android_app* state ) {
	/*
	assert( state );
	app_state_ = state;

	state->userData = this;
	state->onAppCmd = tTVPApplication::handleCommand;
	state->onInputEvent = tTVPApplication::handleInput;
	initCommandPipe();

	if( state->savedState != NULL ) {
		// We are starting with a previous saved state; restore from it.
		loadSaveState( state->savedState );
	}
	*/

	//print_font_files();
	// ここから初期化
	
	// try starting the program!
	bool engine_init = false;
	try {
		// TJS2 スクリプトエンジンを初期化してstartup.tjsを呼ぶ。
		TVPInitScriptEngine();
		engine_init = true;

		// banner
		TVPAddImportantLog( TVPFormatMessage(TVPProgramStartedOn, TVPGetOSName(), TVPGetPlatformName()) );
		
		// main loop
		tjs_uint32 tick = TVPGetRoughTickCount32();
		while( 1 ) { // Read all pending events.
			int ident;
			int events;
			//struct android_poll_source* source;
			void* source;
			int timeout = 16;	//16msec周期で動作するようにする
			while( (ident = ALooper_pollAll( timeout/* msec */, NULL, &events, (void**)&source)) != ALOOPER_POLL_TIMEOUT ) {
				// Process this event.
				if( source != NULL ) {
					if( (tTVPApplication*)source == this ) {
						// user_msg_write_ へ投げられたコマンド
					} else {
						struct android_poll_source* ps = (struct android_poll_source*)source;
						ps->process(state, ps);
					}
				}

				// If a sensor has data, process it now.
				/*
				if( ident == LOOPER_ID_USER ) {
					handleSensorEvent();
				}
				*/

				// Check if we are exiting.
				if( state->destroyRequested != 0 ) {
					tarminateProcess();
					return;
				}
				tjs_uint32 curtick = TVPGetRoughTickCount32();
				if( tick > curtick ) {	// 1周回ってしまった場合
					curtick += 0xffffffffUL - tick;
					tick = 0;
				}
				timeout = 16 - (curtick - tick);
				if( timeout < 0 ) timeout = 0;
			}
			handleIdle();
			tick = TVPGetRoughTickCount32();
		}
	} catch(...) {
	}
}
void tTVPApplication::initializeApplication() {
	TVPTerminateCode = 0;

	// TODO Init console(LogCat)
	try {
		TVPRegisterAssetMedia();

		TVPInitScriptEngine();

		// banner
		TVPAddImportantLog( TVPFormatMessage(TVPProgramStartedOn, TVPGetOSName(), TVPGetPlatformName()) );

		TVPInitializeBaseSystems();

		Initialize();

		if(TVPCheckPrintDataPath()) return;
		if(TVPExecuteUserConfig()) return;

		// image_load_thread_ = new tTVPAsyncImageLoader();

		TVPSystemInit();

		if(TVPCheckAbout()) return; // version information dialog box;

		SetTitle( tjs_string(TVPKirikiri) );

		TVPSystemControl = new tTVPSystemControl();

#ifndef TVP_IGNORE_LOAD_TPM_PLUGIN
//		TVPLoadPluigins(); // load plugin module *.tpm
#endif

		// Check digitizer
		CheckDigitizer();

		// start image load thread
		// image_load_thread_->Resume();

		if(TVPProjectDirSelected) TVPInitializeStartupScript();

		// run main loop from activity resume.
	} catch(...) {
	}
}
void tTVPApplication::handleCommand( struct android_app* state, int32_t cmd ) {
	tTVPApplication* app = (tTVPApplication*)(state->userData);
	app->onCommand( state, cmd );
}
int32_t tTVPApplication::handleInput( struct android_app* state, AInputEvent* event ) {
	tTVPApplication* app = (tTVPApplication*)(state->userData);
	return app->onInput( state, event );
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
		pthread_create( &thread_id_, 0, startMainLoopCallback, this );
	}
}
void tTVPApplication::stopMainLoop() {
	is_terminate_ = true;
	wakeupMainThread();
	pthread_join( thread_id_, 0 );
}
void tTVPApplication::onCommand( struct android_app* state, int32_t cmd ) {
	switch( cmd ) {
		case APP_CMD_SAVE_STATE:
			saveState();
			break;
		case APP_CMD_INIT_WINDOW:
			initializeWindow();
			break;
		case APP_CMD_TERM_WINDOW:
			tarminateWindow();
			break;
        case APP_CMD_GAINED_FOCUS:
			gainedFocus();
			break;
		case APP_CMD_LOST_FOCUS:
			lostFocus();
			break;
		case APP_CMD_INPUT_CHANGED:
			inputChanged();
			break;
		case APP_CMD_WINDOW_RESIZED:
			windowResized();
			break;
		case APP_CMD_WINDOW_REDRAW_NEEDED:
			windowRedrawNeeded();
			break;
		case APP_CMD_CONTENT_RECT_CHANGED:
			contentRectChanged();
			break;
		case APP_CMD_CONFIG_CHANGED:
			configChanged();
			break;
		case APP_CMD_LOW_MEMORY:
			lowMemory();
			break;
		case APP_CMD_START:
			onStart();
			break;
		case APP_CMD_RESUME:
			onResume();
			break;
		case APP_CMD_PAUSE:
			onPause();
			break;
		case APP_CMD_STOP:
			onStop();
			break;
		case APP_CMD_DESTROY:
			onDestroy();
			break;
	}
}
int32_t tTVPApplication::onInput( struct android_app* state, AInputEvent* event ) {
	int32_t type = AInputEvent_getType(event);
	if( type == AINPUT_EVENT_TYPE_MOTION ) {
		int32_t src = AInputEvent_getSource(event);	// 入力デバイスの種類
		// src == AINPUT_SOURCE_TOUCHSCREEN タッチスクリーン
		// src == AINPUT_SOURCE_MOUSE AINPUT_SOURCE_TRACKBALL AINPUT_SOURCE_TOUCHPAD
		int32_t action = AMotionEvent_getAction(event);
		int32_t meta = AMotionEvent_getMetaState(event);
		// AMotionEvent_getEventTime(event); // イベント発生時間
		// AMotionEvent_getDownTime(event); // 押されていた時間
		// AMotionEvent_getEdgeFlags(event); // スクリーン端判定
		float x = AMotionEvent_getX(event, 0);
		float y = AMotionEvent_getY(event, 0);
		float cy = AMotionEvent_getTouchMajor(event,0);	// 触れられている長辺 指の形状から縦側にしておく
		float cx = AMotionEvent_getTouchMinor(event,0);	// 触れられている短辺 指の形状から横側にしておく
		float pressure = AMotionEvent_getPressure(event, 0);	// 圧力
		/*
		float size = AMotionEvent_getSize(event, 0);	// 範囲(推定値) デバイス固有値から0-1の範囲に正規化したもの
		float toolmajor = AMotionEvent_getToolMajor(event,0);
		float toolminor = AMotionEvent_getToolMinor(event,0);
		LOGI( "press : %f, size: %f, major : %f, minor : %f\n", pressure, size, toolmajor, toolminor );
		*/
		int32_t id = AMotionEvent_getPointerId(event, 0);
		action &= AMOTION_EVENT_ACTION_MASK;
		switch( action ) {
		case AMOTION_EVENT_ACTION_DOWN:
			OnTouchDown( x, y, cx, cy, id, pressure, meta );
			break;
		case AMOTION_EVENT_ACTION_UP:
			OnTouchUp( x, y, cx, cy, id, pressure, meta );
			break;
		case AMOTION_EVENT_ACTION_CANCEL:	// Down/Up同時発生。ありえるの？
			break;
		case AMOTION_EVENT_ACTION_MOVE:
			OnTouchMove( x, y, cx, cy, id, pressure, meta );
			break;
		case AMOTION_EVENT_ACTION_POINTER_DOWN: {	// multi-touch
			size_t downidx = (action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			size_t count = AMotionEvent_getPointerCount(event);
			if( downidx == 0 ) {
				OnTouchDown( x, y, cx, cy, id, pressure, meta );
			} else {
				OnTouchMove( x, y, cx, cy, id, pressure, meta );
			}
			for( size_t i = 1; i < count; i++ ) {
				x = AMotionEvent_getX(event, i);
				y = AMotionEvent_getY(event, i);
				cy = AMotionEvent_getTouchMajor(event,i);
				cx = AMotionEvent_getTouchMinor(event,i);
				pressure = AMotionEvent_getPressure(event, i);
				id = AMotionEvent_getPointerId(event, i);
				if( i == downidx ) {
					OnTouchDown( x, y, cx, cy, id, pressure, meta );
				} else {
					OnTouchMove( x, y, cx, cy, id, pressure, meta );
				}
			}
			break;
		}
		case AMOTION_EVENT_ACTION_POINTER_UP: {	// multi-touch
			size_t upidx = (action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			size_t count = AMotionEvent_getPointerCount(event);
			if( upidx == 0 ) {
				OnTouchUp( x, y, cx, cy, id, pressure, meta );
			} else {
				OnTouchMove( x, y, cx, cy, id, pressure, meta );
			}
			for( size_t i = 1; i < count; i++ ) {
				x = AMotionEvent_getX(event, i);
				y = AMotionEvent_getY(event, i);
				cy = AMotionEvent_getTouchMajor(event,i);
				cx = AMotionEvent_getTouchMinor(event,i);
				pressure = AMotionEvent_getPressure(event, i);
				id = AMotionEvent_getPointerId(event, i);
				if( i == upidx ) {
					OnTouchUp( x, y, cx, cy, id, pressure, meta );
				} else {
					OnTouchMove( x, y, cx, cy, id, pressure, meta );
				}
			}
			break;
		}
		case AMOTION_EVENT_ACTION_OUTSIDE:
			break;
		}
		return 1;
	} else if( type == AINPUT_EVENT_TYPE_KEY ) { // key events
		int32_t src = AInputEvent_getSource(event);	// 入力デバイスの種類
		// src == AINPUT_SOURCE_KEYBOARD AINPUT_SOURCE_DPAD
		return 1;
	}
	return 0;
}

#if 0
void tTVPApplication::setApplicationState( struct android_app* state ) {
	assert( state );
	app_state_ = state;
	// ここでいろいろと初期化してしまった方がよさげ

	std::string internalDataPath( getInternalDataPath() );
	TVPUtf8ToUtf16( internal_data_path_, internalDataPath );

	std::string externalDataPath( getExternalDataPath() );
	TVPUtf8ToUtf16( external_data_path_, externalDataPath );

	/*
	// Prepare to monitor accelerometer
	sensor_manager_ = ASensorManager_getInstance();
	accelerometer_sensor_ = ASensorManager_getDefaultSensor( sensorManager, ASENSOR_TYPE_ACCELEROMETER );
	sensor_event_queue_ = ASensorManager_createEventQueue( sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL );
	*/
}
#endif
void tTVPApplication::loadSaveState( void* state ) {
}
void tTVPApplication::handleSensorEvent() {
}
void tTVPApplication::tarminateProcess() {
	//screen_.tarminate();
}
void tTVPApplication::handleIdle() {
}
void tTVPApplication::saveState() {
	//clearSaveState();
}
void tTVPApplication::initializeWindow() {
	//screen_.initialize(this);
}
void tTVPApplication::tarminateWindow() {
	//screen_.tarminate();
}
void tTVPApplication::gainedFocus() {
}
void tTVPApplication::lostFocus() {
}
void tTVPApplication::inputChanged() {
}
void tTVPApplication::windowResized() {
}
void tTVPApplication::windowRedrawNeeded() {
}
void tTVPApplication::contentRectChanged() {
}
void tTVPApplication::configChanged() {
}
void tTVPApplication::lowMemory() {
}
void tTVPApplication::onStart() {
}
void tTVPApplication::onResume() {
}
void tTVPApplication::onPause() {
}
void tTVPApplication::onStop() {
}
void tTVPApplication::onDestroy() {
}
void tTVPApplication::OnTouchDown( float x, float y, float cx, float cy, int32_t id, float pressure, int32_t meta ) {
	//screen_.OnTouchDown( x, y, cx, cy, id );
}
void tTVPApplication::OnTouchMove( float x, float y, float cx, float cy, int32_t id, float pressure,int32_t meta ) {
	//screen_.OnTouchMove( x, y, cx, cy, id );
}
void tTVPApplication::OnTouchUp( float x, float y, float cx, float cy, int32_t id, float pressure,int32_t meta ) {
	//screen_.OnTouchUp( x, y, cx, cy, id );
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

extern "C" {
iTVPApplication* CreateApplication() {
	Application = new tTVPApplication();
	return Application;
}
void DestroyApplication( iTVPApplication* app ) {
	delete app;
	Application = NULL;
}
};

// /system/fonts/ フォントが置かれているフォルダ
// 以下のXMLファイルを参照して、lang=ja のデフォルトフォントファイル名を取得する
// /system/etc/fallback_fonts-ja.xml
// /system/etc/fallback_fonts.xml
// /system/etc/fonts.xml
// /etc/fallback_fonts-ja.xml
// /etc/fallback_fonts.xml
// /etc/fonts.xml
// TODO 上記から得たフォントファイルを開いて、フォント名を取得して返す、Androidではファイル名にしてしまった方がいいかもしれない
const tjs_char *TVPGetDefaultFontName() {
	return TJS_W("Noto Sans CJK");
}
// /system/fonts/ フォントが置かれているフォルダから取得するが、読み込んでリスト作るの少し時間かかりそう
void TVPGetAllFontList( std::vector<tjs_string>& list ) {
	list.clear();
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
void tTVPApplication::getStringFromJava( const char* methodName, tjs_string& dest ) const {
	JNIEnv *env;
	if (jvm_->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_OK) {
		jvm_->AttachCurrentThread( &env, nullptr );
		jobject thiz = activity_;
		jclass clazz = env->GetObjectClass(thiz);
		jmethodID mid = env->GetMethodID(clazz, methodName, "()Ljava/lang/String;");
		jstring ret = (jstring) env->CallObjectMethod(thiz, mid, nullptr);
		int jstrlen = env->GetStringLength(ret);
		const jchar* chars = env->GetStringChars( ret, nullptr );
		dest = tjs_string( chars, &chars[jstrlen] );
		env->ReleaseStringChars( ret, chars );
		env->DeleteLocalRef( ret );
		jvm_->DetachCurrentThread();
	}
}
/**
 * Java から送られてきた各種イベントをここで処理する
 * イベントの種類に応じてアプリケーションとして処理するか、Windowに処理させるか判断
 */
void tTVPApplication::SendMessageFromJava( tjs_int message, tjs_int64 wparam, tjs_int64 lparam ) {
	// Main Windowが存在する場合はそのWindowへ送る
	NativeEvent ev(message,lparam,wparam);
	switch( message ) {
	case AM_START:
	case AM_RESTART:
		return;
	case AM_RESUME:
		startMainLoop();
		break;
	case AM_PAUSE:
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

void tTVPApplication::setWindow( ANativeWindow* window ) {
	std::lock_guard<std::mutex> lock( main_thread_mutex_ );
	window_ = window;
}
void tTVPApplication::nativeSetSurface(JNIEnv *jenv, jobject obj, jobject surface) {
	if( surface != 0 ) {
		ANativeWindow* window = ANativeWindow_fromSurface(jenv, surface);
		LOGI("Got window %p", window);
		Application->setWindow(window);
		SendMessageFromJava( AM_SURFACE_CHANGED, 0, 0 );
	} else {
		LOGI("Releasing window");
		ANativeWindow_release(Application->getWindow());
		Application->setWindow(nullptr);
		SendMessageFromJava( AM_SURFACE_DESTORYED, 0, 0 );
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
void tTVPApplication::nativeSetMessageResource(JNIEnv *jenv, jobject obj, jobjectArray mesarray) {
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
}

void tTVPApplication::nativeSetAssetManager(JNIEnv *jenv, jobject obj, jobject assetManager ) {
	AAssetManager* am = AAssetManager_fromJava( jenv, assetManager );
	Application->setAssetManager( am );
}
/**
 * Java からの通知はここに来る
 */
void tTVPApplication::nativeToMessage(JNIEnv *jenv, jobject obj, jint mes, jlong wparam, jlong lparam ) {
	Application->SendMessageFromJava( mes, lparam, wparam );
}
void tTVPApplication::nativeSetActivity(JNIEnv *jenv, jobject obj, jobject activity) {
	Application->activity_ = activity;
}
void tTVPApplication::nativeInitialize(JNIEnv *jenv, jobject obj) {
	Application->initializeApplication();
}
static JNINativeMethod methods[] = {
		// Java側関数名, (引数の型)返り値の型, native側の関数名の順に並べます
		{ "nativeSetSurface", "(LAndroid/view/Surface;)V", (void *)tTVPApplication::nativeSetSurface },
//		{ "nativeSetMessageResource", "([Ljava/lang/String;)V", (void *)tTVPApplication::nativeSetMessageResource },
		{ "nativeSetAssetManager", "([Landroid/content/res/AssetManager;)V", (void *)tTVPApplication::nativeSetAssetManager },
		{ "nativeToMessage", "(IJJ)V", (void*)tTVPApplication::nativeToMessage },
		{ "nativeSetActivity", "([Landroid/app/Activity;)V", (void *)tTVPApplication::nativeSetActivity },
		{ "nativeInitialize", "()V", (void *)tTVPApplication::nativeInitialize},
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
	if( registerNativeMethods(env, "jp/kirikiri/krkrz/MainActivity", methods, NUM_ARRAY_ELEMENTS(methods)) < 0) {
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
