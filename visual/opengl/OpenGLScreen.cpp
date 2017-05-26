
#include "tjsCommHead.h"
#include "OpenGLScreen.h"
#include <assert.h>



tTVPOpenGLScreen::tTVPOpenGLScreen( void* nativeHandle ) : NativeHandle(nativeHandle),
mDisplay(EGL_NO_DISPLAY),
mSurface(EGL_NO_SURFACE),
mContext(EGL_NO_CONTEXT),
mSwapInterval(1),
mRedBits(-1),
mGreenBits(-1),
mBlueBits(-1),
mAlphaBits(-1),
mDepthBits(-1),
mStencilBits(-1),
mMultisample(false)
{
}

bool tTVPOpenGLScreen::Initialize() {
#ifdef WIN32
// ANGLE_ENABLE_D3D11 を有効に
// ANGLE_ENABLE_D3D9 を無効に
	// OpenGL ES 3.0 を使用する
	EGLint displayAttributes[] = {
		EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 3,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 0,
		EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
		EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
		EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
		EGL_NONE
	};
	mNativeDisplay = ::GetDC( (HWND)NativeHandle );
	mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>(mNativeDisplay), displayAttributes);
	eglBindAPI( EGL_OPENGL_ES_API );
	if( eglGetError() != EGL_SUCCESS ) {
		Destory();
		return false;
	}
	const EGLint configAttributes[] = {
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		EGL_DEPTH_SIZE,     EGL_DONT_CARE,
		EGL_STENCIL_SIZE,   EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
		EGL_NONE
	};
	EGLint configCount;
	if( !eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount ) || (configCount != 1) ) {
		Destory();
		return false;
	}
	eglGetConfigAttrib( mDisplay, mConfig, EGL_RED_SIZE, &mRedBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_GREEN_SIZE, &mGreenBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_BLUE_SIZE, &mBlueBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_ALPHA_SIZE, &mAlphaBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_DEPTH_SIZE, &mDepthBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_STENCIL_SIZE, &mStencilBits );
	EGLint surfaceAttributes[] = {
//		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_NONE };
	mSurface = eglCreateWindowSurface( mDisplay, mConfig, (HWND)NativeHandle, surfaceAttributes );
	if( eglGetError() != EGL_SUCCESS ) {
		Destory();
		return false;
	}
	assert( mSurface != EGL_NO_SURFACE );
	EGLint contextAttributes[] = { EGL_NONE };
	mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
	if( eglGetError() != EGL_SUCCESS ) {
		Destory();
		return false;
	}
	eglMakeCurrent( mDisplay, mSurface, mSurface, mContext );
	if( eglGetError() != EGL_SUCCESS ) {
		Destory();
		return false;
	}
	eglSwapInterval( mDisplay, mSwapInterval );	// V-sync wait?
	return true;
#elif defined( ANDROID )
	
#endif
}
void tTVPOpenGLScreen::Destory() {
#ifdef WIN32
	if( mSurface != EGL_NO_SURFACE ) {
		assert( mDisplay != EGL_NO_DISPLAY );
		eglDestroySurface( mDisplay, mSurface );
		mSurface = EGL_NO_SURFACE;
	}
	if( mContext != EGL_NO_CONTEXT ) {
		assert( mDisplay != EGL_NO_DISPLAY );
		eglDestroyContext( mDisplay, mContext );
		mContext = EGL_NO_CONTEXT;
	}
	if( mDisplay != EGL_NO_DISPLAY ) {
		eglMakeCurrent( mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		eglTerminate( mDisplay );
		mDisplay = EGL_NO_DISPLAY;
	}
	if( mNativeDisplay ) {
		::ReleaseDC( (HWND)NativeHandle, mNativeDisplay );
		mNativeDisplay = 0;
	}
#elif define( ANDROID )
#endif
}
bool tTVPOpenGLScreen::IsInitialized() const {
	return mSurface != EGL_NO_SURFACE && mContext != EGL_NO_CONTEXT && mDisplay != EGL_NO_DISPLAY;
}



