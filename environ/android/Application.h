
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <assert.h>

#include <android/sensor.h>
#include <android/configuration.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include "iTVPApplication.h"
#include "TVPScreen.h"

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

class tTVPApplication : public iTVPApplication {
	JavaVM* jvm_;
	ANativeWindow* window_;

	struct android_app* app_state_;
	tTVPScreen screen_;
	std::wstring title_;

	int user_msg_read_;
	int user_msg_write_;
	//ALooper user_looper_;
	// コマンドごとにキューを設けないと意図した動作にならないか
	//std::queue<NativeEvent> event_queue_;
	std::vector<NativeEventQueueIntarface*> event_handlers_;

	/*
	// センサー系はまとめた方が良さそう
	// Screen クラスでイベントとして処理してしまった方がいいかな
	ASensorManager* sensor_manager_;
	const ASensor* accelerometer_sensor_;
	ASensorEventQueue* sensor_event_queue_;
	*/

	std::wstring internal_data_path_;
	std::wstring external_data_path_;

public:
	// platform's SDK version
	tjs_int getSdkVersion() const {
		return getActivity()->sdkVersion;
	}
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
	// for tTVPScreen
	ANativeWindow* getNativeWindow() const {
		return getAppState()->window;
	}
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
	inline AAssetManager* getAssetManager() {
		ANativeActivity* activity = getActivity();
		assert( activity->assetManager );
		return activity->assetManager;
	}
private:
	inline const android_app* getAppState() const {
		assert( app_state_ );
		return app_state_;
	}
	inline android_app* getAppState() {
		assert( app_state_ );
		return app_state_;
	}
	inline const ANativeActivity* getActivity() const {
		const android_app* state = getAppState();
		assert( state->activity );
		return state->activity;
	}
	inline ANativeActivity* getActivity() {
		const android_app* state = getAppState();
		assert( state->activity );
		return state->activity;
	}
	inline const AAssetManager* getAssetManager() const {
		const ANativeActivity* activity = getActivity();
		assert( activity->assetManager );
		return activity->assetManager;
	}
	inline const char* getInternalDataPath() const {
		const char* path = getActivity()->internalDataPath;
		assert( path );
		return path;
	}
	inline const char* getExternalDataPath() const {
		const char* path = getActivity()->externalDataPath;
		assert( path );
		return path;
	}
	inline const AConfiguration* getConfiguration() const {
		const android_app* state = getAppState();
		assert( state->config );
		return state->config;
	}
	inline AConfiguration* getConfiguration() {
		const android_app* state = getAppState();
		assert( state->config );
		return state->config;
	}

	static void handleCommand( struct android_app* state, int32_t cmd );
	void onCommand( struct android_app* state, int32_t cmd );

	static int32_t handleInput( struct android_app* state, AInputEvent* event );
	int32_t onInput( struct android_app* state, AInputEvent* event );

	bool initCommandPipe();
	void finalCommandPipe();
	int8_t readCommand();
	void HandleMessage( struct NativeEvent &ev );

	static int messagePipeCallBack(int fd, int events, void* user);

public:
	tTVPApplication();
	~tTVPApplication();

	const std::wstring& GetInternalDataPath() const { return internal_data_path_; }
	const std::wstring& GetExternalDataPath() const { return external_data_path_; }
	const std::string* GetCachePath() const { return nullptr; } // TODO キャッシュディレクトリを返すように実装する
	const std::string GetPackageName() const { return std::string(); }

	bool GetActivating() const { return true; }	// TODO
	void ShowToast( const tjs_char* text ) {}	// TODO
	const tTVPScreen& GetScreen() const { return screen_; }

	// for iTVPApplication
	virtual void startApplication( struct android_app* state );

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
	static int MessageDlg( const std::wstring& string, const std::wstring& caption, int type, int button ) {
		return 0;
	}
	void postEvent( const struct NativeEvent* ev );

	void addEventHandler( NativeEventQueueIntarface* handler ) {
		std::vector<NativeEventQueueIntarface*>::iterator it = std::find(event_handlers_.begin(), event_handlers_.end(), handler);
		if( it == event_handlers_.end() ) {
			event_handlers_.push_back( handler );
		}
	}
	void removeEventHandler( NativeEventQueueIntarface* handler ) {
		std::vector<NativeEventQueueIntarface*>::iterator it = std::remove(event_handlers_.begin(), event_handlers_.end(), handler);
		event_handlers_.erase( it, event_handlers_.end() );
	}

	std::wstring GetTitle() const { return title_; }
	void SetTitle( const std::wstring& caption ) { 
		title_ = caption;
	}
	void Terminate() {
		exit(0);
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
	void setWindow( ANativeWindow* window ) {
		window_ = window;
	}
	ANativeWindow* getWindow() { return window_; }
	static void nativeOnStart(JNIEnv *jenv, jobject obj);
	static void nativeOnResume(JNIEnv *jenv, jobject obj);
	static void nativeOnPause(JNIEnv *jenv, jobject obj);
	static void nativeOnStop(JNIEnv *jenv, jobject obj);
	static void nativeSetSurface(JNIEnv *jenv, jobject obj, jobject surface);
	void writeBitmapToNative( const void * bits );
};
std::vector<std::string>* LoadLinesFromFile( const std::wstring& path );

extern class tTVPApplication* Application;

#endif // __T_APPLICATION_H__
