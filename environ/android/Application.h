
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
//#include <android_native_app_glue.h>
#include <android/input.h>
#include <android/asset_manager.h>

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

class iTVPAndroidActivityEventHandler {
public:
	virtual void onResume() = 0;
	virtual void onPause() = 0;
};


class tTVPApplication {
	JavaVM*			jvm_;
	ANativeWindow*	window_;
	AAssetManager*	asset_manager_;	// set from java
	std::unique_ptr<AConfiguration,decltype(&AConfiguration_delete)>		config_;
	std::unique_ptr<AConfiguration,decltype(&AConfiguration_delete)>		old_config_;
	jobject			activity_;

	tjs_string title_;

	std::vector<NativeEventQueueIntarface*>	event_handlers_;
	std::vector<NativeEvent*>				command_cache_;
	std::queue<EventCommand>				command_que_;
	std::vector<iTVPAndroidActivityEventHandler*>	activity_ev_handlers_;

	std::string language_;
	std::string country_;
	
	bool is_terminate_;

	// tjs_string
	tjs_string internal_data_path_;
	tjs_string external_data_path_;
	tjs_string cache_path_;
	tjs_string external_cache_path_;
	tjs_string package_code_path_;
	tjs_string package_resource_path_;
	tjs_string package_path_;
	tjs_string system_release_version_;
	tjs_string so_path_;
	tjs_string obb_path_;
	tjs_string data_directory_;
	tjs_string download_cache_directory_;
	tjs_string external_storage_directory_;
	tjs_string external_public_music_path_;
	tjs_string external_public_podcasts_path_;
	tjs_string external_public_ringtones_path_;
	tjs_string external_public_alarams_path_;
	tjs_string external_public_notifications_path_;
	tjs_string external_public_pictures_path_;
	tjs_string external_public_movies_path_;
	tjs_string external_public_downloads_path_;
	tjs_string external_public_dcim_path_;
	tjs_string external_public_documents_path_;
	tjs_string root_directory_;

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
	bool					activity_active_;
private:
	NativeEvent* createNativeEvent();
	void releaseNativeEvent( NativeEvent* ev );
	void wakeupMainThread();
	void getStringFromJava( const char* methodName, tjs_string& dest ) const;
	void setStringToJava( const char* methodName, const tjs_string& src );
    void getLongToJava( const char* methodName, tjs_int64 src );
    void getFloatToJava( const char* methodName, float src );
	void callActivityMethod( const char* methodName ) const;
	void getIntegerFromJava( const char* methodName, tjs_int& dest ) const;
	void getBooleanFromJava( const char* methodName, bool& dest ) const;
    void getLongFromJava( const char* methodName, tjs_int64& dest ) const;
    void getFloatFromJava( const char* methodName, float& dest ) const;

public:
	void setAssetManager( AAssetManager* am ) {
		asset_manager_ = am;
		if( !config_ ) {
			config_.reset( AConfiguration_new() );
		} else {
			// update configuration
			if( !old_config_ ) {
				old_config_.reset( AConfiguration_new() );
			}
			AConfiguration_copy( old_config_.get(), config_.get() );
		}
		AConfiguration_fromAssetManager( config_.get(), asset_manager_ );
		if( old_config_ ) {
			// update config
			checkConfigurationUpdate();
		}
	}
	void checkConfigurationUpdate();
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

	inline const AAssetManager* getAssetManager() const { return asset_manager_; }
	inline AAssetManager* getAssetManager() { return asset_manager_; }

private:
	inline jobject getActivity() { return activity_; }

	inline const AConfiguration* getConfiguration() const { assert( config_ ); return config_.get(); }
	inline AConfiguration* getConfiguration() { assert( config_ ); return config_.get(); }

	void HandleMessage( struct NativeEvent &ev );

	static void* startMainLoopCallback( void* myself );
	// メインループ
	void mainLoop();

	bool appDispatch(NativeEvent& ev);

	void handleIdle();
public:
	tTVPApplication();
	~tTVPApplication();

	const tjs_string& GetInternalDataPath() const;
	const tjs_string& GetExternalDataPath() const;
	const tjs_string* GetCachePath() const;
	const tjs_string* GetExternalCachePath() const;
	const tjs_string* GetObbPath() const;
	const tjs_char* GetPackageName() const;
	const tjs_char* GetPackageCodePath() const;
	const tjs_char* GetPackageResourcePath() const;
	const tjs_char* GetSoPath() const;
	const tjs_string* GetDataDirectory() const;
	const tjs_string* GetDownloadCacheDirectory() const;
	const tjs_string* GetExternalStorageDirectory() const;
	const tjs_string* GetExternalPublicMusicPath() const;
	const tjs_string* GetExternalPublicPodcastsPath() const;
	const tjs_string* GetExternalPublicRingtonesPath() const;
	const tjs_string* GetExternalPublicAlaramsPath() const;
	const tjs_string* GetExternalPublicNotificationsPath() const;
	const tjs_string* GetExternalPublicPicturesPath() const;
	const tjs_string* GetExternalPublicMoviesPath() const;
	const tjs_string* GetExternalPublicDownloadsPath() const;
	const tjs_string* GetExternalPublicDCIMPath() const;
	const tjs_string* GetExternalPublicDocumentsPath() const;
	const tjs_string* GetRootDirectory() const;

	tjs_string GetExternalStorageState() const;
	bool IsExternalStorageEmulated() const;
	bool IsExternalStorageRemovable() const;

    /**** 動画関係 ****/
	/** 動画を開く **/
	void OpenMovie( const tjs_char* path );
    /** 動画再生 **/
    void PlayMovie();
    /** 動画停止 **/
    void StopMovie();
	/** 動画一時停止 **/
	void PauseMovie();
    /** 動画の現在位置(msec)を取得する **/
    tjs_int64 GetMovieCurrentPosition() const;
    /** 動画の現在位置(msec)を設定する **/
    void SetMovieCurrentPosition( tjs_int64 pos );
    /** 動画の長さ(msec)を取得する **/
    tjs_int64 GetMovieDuration() const;
    /** 動画の音量を取得する **/
    float GetMovieVolume() const;
    /** 動画の音量を設定する **/
    void SetMovieVolume( float vol );
	/** 動画の表示/非表示を取得する **/
	bool GetMovieVisible() const;

	/** ディスプレイの回転角度を取得する **/
	tjs_int GetDisplayRotate() const;
	// Main View の幅を取得する
	tjs_int GetMainViewWidth() const;
	// Main View の高さを取得する
	tjs_int GetMainViewHeight() const;
	tjs_int GetActivityWidth() const;
	tjs_int GetActivityHeight() const;
	tjs_int GetScreenWidth() const;
	tjs_int GetScreenHeight() const;

	// アクティブかどうか
	bool GetActivating() const { return activity_active_; }
	// Android では非アクティブの時は最初化とみなす
	bool GetNotMinimizing() const { return !GetActivating(); }
	// Toastを使って文字列を表示する
	void ShowToast( const tjs_char* text );

	void initializeApplication();
	void startMainLoop();
	void stopMainLoop();

	int GetOpenGLESVersionCode() const;

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
	void addActivityEventHandler( iTVPAndroidActivityEventHandler* handler ) {
		auto it = std::find(activity_ev_handlers_.begin(), activity_ev_handlers_.end(), handler);
		if( it == activity_ev_handlers_.end() ) {
			activity_ev_handlers_.push_back( handler );
		}
	}
	void removeActivityEventHandler( iTVPAndroidActivityEventHandler* handler ) {
		auto it = std::remove(activity_ev_handlers_.begin(), activity_ev_handlers_.end(), handler);
		activity_ev_handlers_.erase( it, activity_ev_handlers_.end() );
	}
	void callActivityEventResume();
	void callActivityEventPause();

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
	void LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name );
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

	tjs_int GetSurfaceWidth() const {
		if( window_ ) {
			return ANativeWindow_getWidth( window_ );
		}
		return 0;
	}
	tjs_int GetSurfaceHeight() const {
		if( window_ ) {
			return ANativeWindow_getHeight ( window_ );
		}
		return 0;
	}
	
	// for tTVPScreen
	//ANativeWindow* getNativeWindow() const { return window_; }
	
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
	static void nativeStartScript( JNIEnv *jenv, jobject obj );

	void writeBitmapToNative( const void * bits );

	TTVPWindowForm* GetMainWindow() { return main_window_; }
	void AddWindow( TTVPWindowForm* window );

	void PrintConsole( const tjs_char* mes, unsigned long len, bool iserror = false );

};
std::vector<std::string>* LoadLinesFromFile( const tjs_string& path );

extern class tTVPApplication* Application;

#endif // __T_APPLICATION_H__
