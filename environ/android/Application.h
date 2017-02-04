
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <queue>
#include <string>
#include <assert.h>

#include <mutex>
#include <condition_variable>
#include <thread>

#include <android/sensor.h>
#include <android/configuration.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/input.h>
#include <android/asset_manager.h>

#include "iTVPApplication.h"
#include "TVPScreen.h"
#include "tjsUtils.h"

enum {
	mrOk,
	mrAbort,
	mrCancel,
};

enum {
  mtCustom = 0,
  mtWarning,
  mtError,
  mtInformation,
  mtConfirmation,
  mtStop,
};

enum {
	mbOK = 1,
};
class NativeEventQueueIntarface;
struct NativeEvent;
class TTVPWindowForm;

struct EventCommand {
	NativeEventQueueIntarface*	target;
	NativeEvent*				command;
	EventCommand( NativeEventQueueIntarface* t, NativeEvent* c ) : target(t), command(c) {}
	EventCommand() : target(nullptr), command(nullptr) {}
};

class tTVPApplication : public iTVPApplication {
	JavaVM*			jvm_;
	ANativeWindow*	window_;
	AAssetManager*	asset_manager_;	// set from java
	AConfiguration*	config_;
	jobject			activity_;

	tjs_string title_;

	std::vector<NativeEventQueueIntarface*>	event_handlers_;
	std::vector<NativeEvent*>				command_cache_;
	std::queue<EventCommand>				command_que_;

	std::string language_;
	std::string country_;
	
	bool is_terminate_;

	// tjs_string
	tjs_string internal_data_path_;
	tjs_string external_data_path_;
	tjs_string cache_path_;
	tjs_string package_code_path_;
	tjs_string package_path_;
	tjs_string system_release_version_;

	class tTVPAsyncImageLoader* image_load_thread_;
	class TTVPWindowForm* main_window_;

    pthread_t thread_id_;
	std::mutex event_handlers_mutex_;
	std::mutex command_cache_mutex_;
	std::mutex command_que_mutex_;
	std::condition_variable main_thread_cv_;

	//bool font_list_searched_;
	//std::vector<FontInfo*>	font_list_;
	std::vector<char>		console_cache_;

	tjs_string				startup_path_;
private:
	NativeEvent* createNativeEvent();
	void releaseNativeEvent( NativeEvent* ev );
	void wakeupMainThread();
	void getStringFromJava( const char* methodName, tjs_string& dest ) const;
	void setStringToJava( const char* methodName, const tjs_string& src );
	void callActivityMethod( const char* methodName ) const;

public:
	void setAssetManager( AAssetManager* am ) {
		asset_manager_ = am;
		if( config_ == nullptr ) {
			config_ = AConfiguration_new();
		}
		AConfiguration_fromAssetManager( config_, asset_manager_ );
	}
	static void nativeSetAssetManager(JNIEnv *jenv, jobject obj, jobject assetManager );

	void finishActivity();
	void changeSurfaceSize( int w, int h );

	// platform's SDK version
	tjs_int getSdkVersion() const {
		return AConfiguration_getSdkVersion( const_cast<AConfiguration*>(getConfiguration()) );
	}
	const tjs_string& getSystemVersion() const;
	std::string getLanguage() const {
		char lang[2];
		AConfiguration_getLanguage( const_cast<AConfiguration*>(getConfiguration()), lang );
		return std::string(lang,2);
	}
	std::string getCountry() const {
		char country[2];
		AConfiguration_getCountry( const_cast<AConfiguration*>(getConfiguration()), country );
		return std::string(country,2);
	}
	enum {
		orientUnknown,		// ACONFIGURATION_ORIENTATION_ANY
		orientPortrait,		// ACONFIGURATION_ORIENTATION_PORT
		orientLandscape,	// ACONFIGURATION_ORIENTATION_LAND
		orientSquare		// ACONFIGURATION_ORIENTATION_SQUARE
	};
	tjs_int getOrientation() const {
		return AConfiguration_getOrientation( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_TOUCHSCREEN_ANY  = 0x0000,
		ACONFIGURATION_TOUCHSCREEN_NOTOUCH  = 0x0001,
		ACONFIGURATION_TOUCHSCREEN_STYLUS  = 0x0002,
		ACONFIGURATION_TOUCHSCREEN_FINGER  = 0x0003,
	*/
	tjs_int getTouchscreen() const {
		return AConfiguration_getTouchscreen( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_DENSITY_DEFAULT = 0,
		ACONFIGURATION_DENSITY_LOW = 120,
		ACONFIGURATION_DENSITY_MEDIUM = 160,
		ACONFIGURATION_DENSITY_HIGH = 240,
		ACONFIGURATION_DENSITY_NONE = 0xffff,
		実際の値が返ってくる？
	*/
	tjs_int getDensity() const {
		return AConfiguration_getDensity( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_KEYBOARD_ANY  = 0x0000,
		ACONFIGURATION_KEYBOARD_NOKEYS  = 0x0001,
		ACONFIGURATION_KEYBOARD_QWERTY  = 0x0002,
		ACONFIGURATION_KEYBOARD_12KEY  = 0x0003,
	*/
	tjs_int getKeyboard() const {
		return AConfiguration_getKeyboard( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_NAVIGATION_ANY  = 0x0000,
		ACONFIGURATION_NAVIGATION_NONAV  = 0x0001,
		ACONFIGURATION_NAVIGATION_DPAD  = 0x0002,
		ACONFIGURATION_NAVIGATION_TRACKBALL  = 0x0003,
		ACONFIGURATION_NAVIGATION_WHEEL  = 0x0004,
	*/
	tjs_int getNavigation() const {
		return AConfiguration_getNavigation( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_KEYSHIDDEN_ANY = 0x0000,
		ACONFIGURATION_KEYSHIDDEN_NO = 0x0001,
		ACONFIGURATION_KEYSHIDDEN_YES = 0x0002,
		ACONFIGURATION_KEYSHIDDEN_SOFT = 0x0003,
	*/
	tjs_int getKeysHidden() const {
		return AConfiguration_getKeysHidden( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_NAVHIDDEN_ANY = 0x0000,
		ACONFIGURATION_NAVHIDDEN_NO = 0x0001,
		ACONFIGURATION_NAVHIDDEN_YES = 0x0002,
	*/
	tjs_int getNavHidden() const {
		return AConfiguration_getNavHidden( const_cast<AConfiguration*>(getConfiguration()) );
	}
	tjs_int getCongifSdkVersion() const {
		return AConfiguration_getSdkVersion( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_SCREENSIZE_ANY  = 0x00,
		ACONFIGURATION_SCREENSIZE_SMALL = 0x01,
		ACONFIGURATION_SCREENSIZE_NORMAL = 0x02,
		ACONFIGURATION_SCREENSIZE_LARGE = 0x03,
		ACONFIGURATION_SCREENSIZE_XLARGE = 0x04,
	*/
	tjs_int getScreenSize() const {
		return AConfiguration_getScreenSize( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_SCREENLONG_ANY = 0x00,
		ACONFIGURATION_SCREENLONG_NO = 0x1,
		ACONFIGURATION_SCREENLONG_YES = 0x2,
	*/
	tjs_int getScreenLong() const {
		return AConfiguration_getScreenLong( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_UI_MODE_TYPE_ANY = 0x00,
		ACONFIGURATION_UI_MODE_TYPE_NORMAL = 0x01,
		ACONFIGURATION_UI_MODE_TYPE_DESK = 0x02,
		ACONFIGURATION_UI_MODE_TYPE_CAR = 0x03,
	*/
	tjs_int getUIModeType() const {
		return AConfiguration_getUiModeType( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
		ACONFIGURATION_UI_MODE_NIGHT_ANY = 0x00,
		ACONFIGURATION_UI_MODE_NIGHT_NO = 0x1,
		ACONFIGURATION_UI_MODE_NIGHT_YES = 0x2,
	*/
	tjs_int getUIModeNight() const {
		return AConfiguration_getUiModeNight( const_cast<AConfiguration*>(getConfiguration()) );
	}
	/*
	void setSaveState( const void* data, size_t size ) {
		assert( app_state_ );
		if( app_state_->savedStateSize != size ) {
			if( app_state_->savedState != NULL ) {
				free( app_state_->savedState );
			}
			app_state_->savedState = NULL;
		}
		if( app_state_->savedState == NULL ) {
			app_state_->savedState = malloc(size);
		}
		memcpy( app_state_->savedState, data, size );
		app_state_->savedStateSize = size;
	}
	void clearSaveState() {
		assert( app_state_ );
		if( app_state_->savedState != NULL ) {
			free( app_state_->savedState );
		}
		app_state_->savedState = NULL;
		app_state_->savedStateSize = 0;
	}
	*/
	inline const AAssetManager* getAssetManager() const { return asset_manager_; }
	inline AAssetManager* getAssetManager() { return asset_manager_; }

private:
	inline jobject getActivity() { return activity_; }

	inline const AConfiguration* getConfiguration() const { assert( config_ ); return config_; }
	inline AConfiguration* getConfiguration() { assert( config_ ); return config_; }

	static void handleCommand( struct android_app* state, int32_t cmd );
	void onCommand( struct android_app* state, int32_t cmd );

	static int32_t handleInput( struct android_app* state, AInputEvent* event );
	int32_t onInput( struct android_app* state, AInputEvent* event );

	//bool initCommandPipe();
	//void finalCommandPipe();
	//int8_t readCommand();
	void HandleMessage( struct NativeEvent &ev );

	//static int messagePipeCallBack(int fd, int events, void* user);

	static void* startMainLoopCallback( void* myself );
	// メインループ
	void mainLoop();

	bool appDispatch(NativeEvent& ev);
public:
	tTVPApplication();
	~tTVPApplication();

	const tjs_string& GetInternalDataPath() const;
	const tjs_string& GetExternalDataPath() const;
	const tjs_string* GetCachePath() const;
	const tjs_char* GetPackageName() const;
	const tjs_char* GetPackageCodePath() const;

	bool GetActivating() const { return true; }	// TODO
	void ShowToast( const tjs_char* text );
	//const tTVPScreen& GetScreen() const { return screen_; }

	// for iTVPApplication
	virtual void startApplication( struct android_app* state );
	void initializeApplication();
	void startMainLoop();
	void stopMainLoop();

	void loadSaveState( void* state );
	void handleSensorEvent();
	void tarminateProcess();
	void handleIdle();
	void saveState();
	void initializeWindow();
	void tarminateWindow();
	void gainedFocus();
	void lostFocus();
	void inputChanged();
	void windowResized();
	void windowRedrawNeeded();
	void contentRectChanged();
	void configChanged();
	void lowMemory();
	void onStart();
	void onResume();
	void onPause();
	void onStop();
	void onDestroy();

	void OnTouchDown( float x, float y, float cx, float cy, int32_t id, float pressure, int32_t meta );
	void OnTouchMove( float x, float y, float cx, float cy, int32_t id, float pressure,int32_t meta );
	void OnTouchUp( float x, float y, float cx, float cy, int32_t id, float pressure,int32_t meta );

	// for exception showing
	static int MessageDlg( const tjs_string& string, const tjs_string& caption, int type, int button );

	void postEvent( const struct NativeEvent* ev, NativeEventQueueIntarface* handler = nullptr );

	void addEventHandler( NativeEventQueueIntarface* handler ) {
		std::lock_guard<std::mutex> lock( event_handlers_mutex_ );
		std::vector<NativeEventQueueIntarface*>::iterator it = std::find(event_handlers_.begin(), event_handlers_.end(), handler);
		if( it == event_handlers_.end() ) {
			event_handlers_.push_back( handler );
		}
	}
	void removeEventHandler( NativeEventQueueIntarface* handler ) {
		std::lock_guard<std::mutex> lock( event_handlers_mutex_ );
		std::vector<NativeEventQueueIntarface*>::iterator it = std::remove(event_handlers_.begin(), event_handlers_.end(), handler);
		event_handlers_.erase( it, event_handlers_.end() );
	}

	tjs_string GetTitle() const { return title_; }
	void SetTitle( const tjs_string& caption ) { 
		title_ = caption;
	}
	tjs_string GetActivityCaption();
	void SetActivityCaption( const tjs_string& caption );

	void Terminate() {
		finishActivity();
	}
	/**
	 * 画像の非同期読込み要求
	 */
	void LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name ) {}
	/**
	 * メッセージポンプを回す
	 */
	void ProcessMessages() {}
	void HandleMessage() {}

	void setJavaVM( JavaVM* jvm ) {
		jvm_ = jvm;
	}
	JNIEnv* getJavaEnv( bool& attached ) const;
	void detachJavaEnv() const;

	void setWindow( ANativeWindow* window );
	void SendMessageFromJava( tjs_int message, tjs_int64 wparam, tjs_int64 lparam );
	void SendTouchMessageFromJava( tjs_int type, float x, float y, float c, int id, tjs_int64 tick );
	ANativeWindow* getWindow() { return window_; }
	// for tTVPScreen
	ANativeWindow* getNativeWindow() const { return window_; }
	
	//void setStartupPath( const tjs_string& path ) { startup_path_ = path; }
	//const tjs_string& getStartupPath() const { return startup_path_; }

	static void nativeSetSurface(JNIEnv *jenv, jobject obj, jobject surface);
	static void nativeSetMessageResource(JNIEnv *jenv, jobject obj, jobjectArray mesarray);
	static void nativeToMessage(JNIEnv *jenv, jobject obj, jint mes, jlong wparam, jlong lparam );
	static void nativeSetActivity(JNIEnv *jenv, jobject obj, jobject activity);
	static void nativeInitialize(JNIEnv *jenv, jobject obj);
	static void nativeOnTouch( JNIEnv *jenv, jobject obj, jint type, jfloat x, jfloat y, jfloat c, jint id, jlong tick );
	static void nativeSetStartupPath( JNIEnv *jenv, jobject obj, jstring path );
	static void nativeSetSoundNativeParameter( JNIEnv *jenv, jobject obj, jint rate, jint size );

	void writeBitmapToNative( const void * bits );

	TTVPWindowForm* GetMainWindow() { return main_window_; }
	void AddWindow( TTVPWindowForm* window );

	void PrintConsole( const tjs_char* mes, unsigned long len, bool iserror = false );

};
std::vector<std::string>* LoadLinesFromFile( const tjs_string& path );

extern class tTVPApplication* Application;

#endif // __T_APPLICATION_H__
