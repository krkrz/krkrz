
#include <jni.h>
#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include "iTVPApplication.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "krkrz", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "krkrz", __VA_ARGS__))

static iTVPApplication* (*proc_CreateApplication)();
static void (*proc_DestroyApplication)( iTVPApplication* app );

template<typename T>
void auto_load( T& a, void* ptr ) {
	a = reinterpret_cast<T>( ptr );
}
#define get_proc_0( image, func_name ) auto_load( proc_##func_name, dlsym( image, #func_name ) )
#define get_proc( func_name ) get_proc_0( LoadApplication, func_name )

static void* LoadApplication = NULL;
static void FinalizeApplication() {
	if( LoadApplication ){
		dlclose( LoadApplication );
		LoadApplication = NULL;
	}
}
static iTVPApplication* InitializeApplication( const char* internalDataPath ) {
	// 前回残っていた物があったら解放する
	FinalizeApplication();
	// so を読み込んで初期化
	std::string path(internalDataPath);
	std::string::size_type pos = path.find_last_of( TJS_W('/') );
	if( pos != std::string::npos ) {
		std::string dllpath = path.substr( 0, pos+1 ) + std::string("lib/libkrkrz.so");
		LoadApplication = dlopen( dllpath.c_str(), RTLD_LAZY );
	}
	if( !LoadApplication ) { // error
		return NULL;
	}
	get_proc( CreateApplication );
	get_proc( DestroyApplication );
	return proc_CreateApplication();
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 * onCreate を通る場合、android_main から始まるので、ここで初期化すれば
 * メモリの問題は回避できるか。
 */
void android_main(struct android_app* state) {
	app_dummy();
	iTVPApplication* application = InitializeApplication( state->activity->internalDataPath );
	if( application != NULL ) {
		application->startApplication( state );
		proc_DestroyApplication( application );
	}
	FinalizeApplication();
}

