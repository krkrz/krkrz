
#include "tjsCommHead.h"
#include "OpenGLScreen.h"
#include <assert.h>
#include "DebugIntf.h"


tTVPOpenGLScreen::tTVPOpenGLScreen( void* nativeHandle ) : NativeHandle(nativeHandle),
mDisplay(EGL_NO_DISPLAY),
mSurface(EGL_NO_SURFACE),
mContext(EGL_NO_CONTEXT),
mSwapInterval(0),
mRedBits(-1),
mGreenBits(-1),
mBlueBits(-1),
mAlphaBits(-1),
mDepthBits(-1),
mStencilBits(-1),
mMultisample(false),
mMinSwapInterval(1),
mMaxSwapInterval(1)
{
}

bool tTVPOpenGLScreen::Initialize() {
#ifdef WIN32
// ANGLE_ENABLE_D3D11 を有効に
// ANGLE_ENABLE_D3D9 を無効に
	// OpenGL ES 3.0 を使用する
	EGLint displayAttributes[] = {
		EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
		EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
		EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
		EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
		EGL_NONE
	};
#if 0
	WNDCLASSEX childWindowClass = { 0 };
	childWindowClass.cbSize = sizeof( WNDCLASSEX );
	childWindowClass.style = CS_OWNDC;
	childWindowClass.lpfnWndProc = ::DefWindowProc;
	childWindowClass.cbClsExtra = 0;
	childWindowClass.cbWndExtra = 0;
	childWindowClass.hInstance = GetModuleHandle( nullptr );
	childWindowClass.hIcon = nullptr;
	//childWindowClass.hCursor = LoadCursorA( NULL, idcArrow );
	childWindowClass.hbrBackground = 0;
	childWindowClass.lpszMenuName = nullptr;
	childWindowClass.lpszClassName = TJS_W("ANGLE Window");

	WNDCLASSEX tmpwc = { sizeof( WNDCLASSEX ) };
	RECT ownerRect;
	::GetClientRect( (HWND)NativeHandle, &ownerRect );
	BOOL ClassRegistered = ::GetClassInfoEx( childWindowClass.hInstance, childWindowClass.lpszClassName, &tmpwc );
	if( ClassRegistered == 0 ) {
		if( !RegisterClassEx( &childWindowClass ) ) {
			return false;
		}
	}
	mNativeWindow = CreateWindowEx( 0, TJS_W( "ANGLE Window" ), TJS_W(""), WS_CHILD,
		0, 0, static_cast<int>( ownerRect.right- ownerRect.left ), static_cast<int>( ownerRect.bottom- ownerRect.top ),
		(HWND)NativeHandle, nullptr, GetModuleHandle( nullptr ), this );
#endif
	mNativeDisplay = ::GetDC( (HWND)NativeHandle );
	if( !mNativeDisplay ) {
		Destroy();
		return false;
	}

	mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>(mNativeDisplay), displayAttributes);
	eglBindAPI( EGL_OPENGL_ES_API );
	if( !CheckEGLErrorAndLog() ) {
		Destroy();
		return false;
	}
	EGLint majorVersion, minorVersion;
	if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
		Destroy();
		return false;
	}
	const EGLint configAttributes[] = {
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		/*
		EGL_RED_SIZE,       EGL_DONT_CARE,
		EGL_GREEN_SIZE,     EGL_DONT_CARE,
		EGL_BLUE_SIZE,      EGL_DONT_CARE,
		EGL_ALPHA_SIZE,     EGL_DONT_CARE,
		*/
		EGL_DEPTH_SIZE,     EGL_DONT_CARE,
		EGL_STENCIL_SIZE,   EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
		//EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLint configCount;
	EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
	if( result == EGL_FALSE || (configCount != 1) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	eglGetConfigAttrib( mDisplay, mConfig, EGL_RED_SIZE, &mRedBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_GREEN_SIZE, &mGreenBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_BLUE_SIZE, &mBlueBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_ALPHA_SIZE, &mAlphaBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_DEPTH_SIZE, &mDepthBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_STENCIL_SIZE, &mStencilBits );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_MIN_SWAP_INTERVAL, &mMinSwapInterval );
	eglGetConfigAttrib( mDisplay, mConfig, EGL_MAX_SWAP_INTERVAL, &mMaxSwapInterval );
	EGLint surfaceAttributes[] = {
//		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_NONE };
	mSurface = eglCreateWindowSurface( mDisplay, mConfig, (HWND)NativeHandle, surfaceAttributes );
	if( !CheckEGLErrorAndLog() ) {
		Destroy();
		return false;
	}
	assert( mSurface != EGL_NO_SURFACE );
	EGLint contextAttributes[] = {
		//EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 0,
		EGL_NONE };
	mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
	if( !CheckEGLErrorAndLog() ) {
		Destroy();
		return false;
	}
	eglMakeCurrent( mDisplay, mSurface, mSurface, mContext );
	if( !CheckEGLErrorAndLog() ) {
		Destroy();
		return false;
	}
	eglSwapInterval( mDisplay, mSwapInterval );	// V-sync wait?
	return true;
#elif defined( ANDROID )
	
#endif
}
void tTVPOpenGLScreen::Destroy() {
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

EGLint tTVPOpenGLScreen::GetSurfaceWidth() const {
	EGLint result;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_WIDTH, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}
EGLint tTVPOpenGLScreen::GetSurfaceHeight() const {
	EGLint result;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_HEIGHT, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}
bool tTVPOpenGLScreen::CheckEGLErrorAndLog() {
	GLenum error_code = eglGetError();
	switch( error_code ) {
/*
	case EGL_BAD_DISPLAY: TVPAddLog( TJS_W( "Display is not an EGL display connection." ) ); break;
	case EGL_BAD_ATTRIBUTE: TVPAddLog( TJS_W( "Attribute_list contains an invalid frame buffer configuration attribute or an attribute value that is unrecognized or out of range." ) ); break;
	case EGL_NOT_INITIALIZED: TVPAddLog( TJS_W( "Display has not been initialized." ) ); break;
	case EGL_BAD_PARAMETER: TVPAddLog( TJS_W( "Num_config is NULL." ) ); break;
*/
	//case EGL_SUCCESS: TVPAddLog( TJS_W( "The last function succeeded without error." ) ); break;
	case EGL_NOT_INITIALIZED: TVPAddLog( TJS_W( "EGL is not initialized, or could not be initialized, for the specified EGL display connection." ) ); break;
	case EGL_BAD_ACCESS: TVPAddLog( TJS_W( "EGL cannot access a requested resource( for example a context is bound in another thread )." ) ); break;
	case EGL_BAD_ALLOC: TVPAddLog( TJS_W( "EGL failed to allocate resources for the requested operation." ) ); break;
	case EGL_BAD_ATTRIBUTE: TVPAddLog( TJS_W( "An unrecognized attribute or attribute value was passed in the attribute list." ) ); break;
	case EGL_BAD_CONTEXT: TVPAddLog( TJS_W( "An EGLContext argument does not name a valid EGL rendering context." ) ); break;
	case EGL_BAD_CONFIG: TVPAddLog( TJS_W( "An EGLConfig argument does not name a valid EGL frame buffer configuration." ) ); break;
	case EGL_BAD_CURRENT_SURFACE: TVPAddLog( TJS_W( "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid." ) ); break;
	case EGL_BAD_DISPLAY: TVPAddLog( TJS_W( "An EGLDisplay argument does not name a valid EGL display connection." ) ); break;
	case EGL_BAD_SURFACE: TVPAddLog( TJS_W( "An EGLSurface argument does not name a valid surface( window, pixel buffer or pixmap ) configured for GL rendering." ) ); break;
	case EGL_BAD_MATCH: TVPAddLog( TJS_W( "Arguments are inconsistent( for example, a valid context requires buffers not supplied by a valid surface )." ) ); break;
	case EGL_BAD_PARAMETER: TVPAddLog( TJS_W( "One or more argument values are invalid." ) ); break;
	case EGL_BAD_NATIVE_PIXMAP: TVPAddLog( TJS_W( "A NativePixmapType argument does not refer to a valid native pixmap." ) ); break;
	case EGL_BAD_NATIVE_WINDOW: TVPAddLog( TJS_W( "A NativeWindowType argument does not refer to a valid native window." ) ); break;
	case EGL_CONTEXT_LOST: TVPAddLog( TJS_W( "A power management event has occurred.The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering." ) ); break;
	}
	return error_code == EGL_SUCCESS;
}


