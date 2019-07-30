
#include "tjsCommHead.h"
#include "OpenGLScreen.h"
#include <assert.h>
#include "DebugIntf.h"
#include "GLTexture.h"

#ifdef WIN32
#include <d3d9.h>
#include <atlbase.h>
#include "CDLLLoader.h"
#endif

extern int TVPOpenGLESVersion;

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
mMaxSwapInterval(1),
mDefaultFrameBufferId(0)
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
		TVPAddLog( TJS_W( "Failed to retrieve DC." ) );
		Destroy();
		return false;
	}

	mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>(mNativeDisplay), displayAttributes);
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
		Destroy();
		return false;
	}
	EGLint esClientVersion = 3;
	EGLint majorVersion, minorVersion;
	if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
		TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
		CheckEGLErrorAndLog();
		// 要求を下げて再試行
		Destroy();
		mNativeDisplay = ::GetDC( (HWND)NativeHandle );
		EGLint displayAttributes2[] = {
			EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
			EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
			EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
			EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
			EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
			EGL_NONE
		};

		mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>( mNativeDisplay ), displayAttributes2 );
		if( !CheckEGLErrorAndLog() ) {
			TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
			if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
				Destroy();
				return false;
			}
		}
		if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
			TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
			CheckEGLErrorAndLog();
			// 要求を下げて再試行
			Destroy();
			mNativeDisplay = ::GetDC( (HWND)NativeHandle );
			EGLint displayAttributes3[] = {
				EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE,
				EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
				EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
				EGL_NONE
			};

			mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>( mNativeDisplay ), displayAttributes3 );
			if( !CheckEGLErrorAndLog() ) {
				TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
				if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
					Destroy();
					return false;
				}
			}
			if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
				TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
				CheckEGLErrorAndLog();
				Destroy();
				return false;
			}
		}
		esClientVersion = 2;	// DirectX 9 の時は、ES 2.0 でないと動かない
		TVPOpenGLESVersion = 200;
	}
	TVPAddLog( ttstr( TJS_W( "(info) Support EGL") ) + to_tjs_string( majorVersion ) + ttstr( TJS_W( "." ) ) + to_tjs_string( minorVersion ) );
#if 0
	// OpenGL ES2.0 を選択して、ES2.0 環境で問題なく動くか確認する
	esClientVersion = 2;	// DirectX 9 の時は、ES 2.0 でないと動かない
	TVPOpenGLESVersion = 200;
#endif
	eglBindAPI( EGL_OPENGL_ES_API );

	const EGLint configAttributesAny[] = { EGL_RED_SIZE, EGL_DONT_CARE, EGL_GREEN_SIZE, EGL_DONT_CARE, EGL_BLUE_SIZE, EGL_DONT_CARE, EGL_ALPHA_SIZE, EGL_DONT_CARE, EGL_NONE };
	EGLint configSize;
	mConfig = nullptr;
	if( eglChooseConfig( mDisplay, configAttributesAny, NULL, 0, &configSize ) != EGL_FALSE && configSize > 0 ) {
		std::vector<EGLConfig> configs( configSize );
		EGLint configCount;
		EGLBoolean ret = eglChooseConfig( mDisplay, configAttributesAny, &( configs[0] ), configSize, &configCount );
		if( ret != EGL_FALSE ) {
			if( configCount == 1 ) {
				// 1個しか見付からないのなら、それでもう確定
				mConfig = configs[0];
			}
			EGLint r, g, b, a;
			if( mConfig == nullptr ) {
				// 最初にRGBA全て8のものを探す
				for( EGLint i = 0; i < configCount; i++ ) {
					EGLConfig config = configs[i];
					eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &r );
					eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &g );
					eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &b );
					eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &a );
					if( r == 8 && g == 8 && b == 8 && a == 8 ) {
						mConfig = config;
						break;
					}
				}
			}
			if( mConfig == nullptr ) {
				// 見付からなかったようなので、Alphaは気にしないことにして探す
				for( EGLint i = 0; i < configCount; i++ ) {
					EGLConfig config = configs[i];
					eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &r );
					eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &g );
					eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &b );
					eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &a );
					if( r == 8 && g == 8 && b == 8 ) {
						mConfig = config;
						break;
					}
				}
			}
			if( mConfig == nullptr ) {
				// RGB888 がないのなら、もう気にせず最初のやつに決定してしまう
				mConfig = configs[0];
			}
		}
	} else {
		// 1個も見付からない時は、失敗。起動できない
		TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
#if 0
	// 全て DONT CARE でコールしているが、eglGetConfigAttrib を用いて、設定可能なものを列挙した後選択する形の方がより好ましい
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
	if( result == EGL_FALSE || (configCount == 0) ) {
		TVPAddLog( TJS_W( "Failed to call eglChooseConfig. try EGL_DONT_CARE." ) );
		const EGLint configAttributes2[] = {
			EGL_RED_SIZE,       EGL_DONT_CARE,
			EGL_GREEN_SIZE,     EGL_DONT_CARE,
			EGL_BLUE_SIZE,      EGL_DONT_CARE,
			EGL_ALPHA_SIZE,     EGL_DONT_CARE,
			EGL_DEPTH_SIZE,     EGL_DONT_CARE,
			EGL_STENCIL_SIZE,   EGL_DONT_CARE,
			EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
			//EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
		};
		EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
		if( result == EGL_FALSE || ( configCount == 0 ) ) {
			TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
			CheckEGLErrorAndLog();
			Destroy();
			return false;
		}
	}
#endif
	GetConfigAttribute( mConfig );
	EGLint surfaceAttributes[] = {
//		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_NONE };
	mSurface = eglCreateWindowSurface( mDisplay, mConfig, (HWND)NativeHandle, surfaceAttributes );
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglCreateWindowSurface." ) );
		Destroy();
		return false;
	}
	assert( mSurface != EGL_NO_SURFACE );
	EGLint contextAttributes[] = {
		//EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_CONTEXT_CLIENT_VERSION, esClientVersion,
		EGL_CONTEXT_MINOR_VERSION, 0,
		EGL_NONE };
	mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
#if 0
		if( mSurface != EGL_NO_SURFACE ) {
			eglDestroySurface( mDisplay, mSurface );
			mSurface = EGL_NO_SURFACE;
		}
		if( mContext != EGL_NO_CONTEXT ) {
			eglDestroyContext( mDisplay, mContext );
			mContext = EGL_NO_CONTEXT;
		}
		const EGLint configAttributes2[] = {
			EGL_RED_SIZE,       EGL_DONT_CARE,
			EGL_GREEN_SIZE,     EGL_DONT_CARE,
			EGL_BLUE_SIZE,      EGL_DONT_CARE,
			EGL_ALPHA_SIZE,     EGL_DONT_CARE,
			EGL_DEPTH_SIZE,     EGL_DONT_CARE,
			EGL_STENCIL_SIZE,   EGL_DONT_CARE,
			EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
			EGL_NONE
		};
		EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
		if( result == EGL_FALSE || ( configCount == 0 ) ) {
			TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
			CheckEGLErrorAndLog();
			Destroy();
			return false;
		}
		GetConfigAttribute( mConfig );
		mSurface = eglCreateWindowSurface( mDisplay, mConfig, (HWND)NativeHandle, surfaceAttributes );
		if( !CheckEGLErrorAndLog() ) {
			TVPAddLog( TJS_W( "Failed to call eglCreateWindowSurface." ) );
			Destroy();
			return false;
		}
		mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
		if( !CheckEGLErrorAndLog() ) {
			TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
			Destroy();
			return false;
		}
#else
		TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
		Destroy();
		return false;
#endif
	}
	eglMakeCurrent( mDisplay, mSurface, mSurface, mContext );
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglMakeCurrent." ) );
		Destroy();
		return false;
	}
	mSwapInterval = mMinSwapInterval;
	eglSwapInterval( mDisplay, mSwapInterval );	// V-sync wait?

	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &mDefaultFrameBufferId );
#if 0
	// 古い環境のための情報確認(確認範囲では動きそうだが、動かない)
	int maxtexsize = GLTexture::getMaxTextureSize();
	TVPAddLog( ttstr( TJS_W( "(info) Max Texture Size :" ) ) + to_tjs_string( maxtexsize ) );
	{
		// Check D3D9 caps
		CDLLLoader d3ddll;
		if( d3ddll.Load( TJS_W( "d3d9.dll" ) ) ) {
			typedef IDirect3D9 *( WINAPI *FuncDirect3DCreate9 )( UINT SDKVersion );
			FuncDirect3DCreate9 pDirect3DCreate9 = (FuncDirect3DCreate9)d3ddll.GetProcAddress( "Direct3DCreate9" );
			if( pDirect3DCreate9 != nullptr ) {
				CComPtr<IDirect3D9> d3d;
				if( nullptr != ( d3d = pDirect3DCreate9( D3D_SDK_VERSION ) ) ) {
					D3DCAPS9	d3dcaps;
					if( D3D_OK == d3d->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps ) ) {
						if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP ) {
							TVPAddLog( TJS_W( "(info) Support Mipmap Texture." ) );
						} else {
							TVPAddLog( TJS_W( "(info) Not Support Mipmap Texture." ) );
						}
						if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 ) {
							TVPAddLog( TJS_W( "(info) Texture requere pow2." ) );
						}
						if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL ) {
							TVPAddLog( TJS_W( "(info) D3D Texture NONPOW2CONDITIONAL ." ) );
						}
					}
				}
			}
		}
	}
#endif
	return true;
#elif defined( ANDROID )
	ANativeWindow* window = reinterpret_cast<ANativeWindow*>(NativeHandle);	// Surface(SurfaceView)

	if( (mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY )) == EGL_NO_DISPLAY ) {
		CheckEGLErrorAndLog();
		return false;
	}
	if( !eglInitialize( mDisplay, 0, 0 ) ) {
		CheckEGLErrorAndLog();
		return false;
	}
	const EGLint configAttributes[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE,      8,
		EGL_GREEN_SIZE,     8,
		EGL_RED_SIZE,       8,
		EGL_STENCIL_SIZE,   8,
		EGL_NONE
    };
	EGLint configCount;
	if( !eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount ) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	EGLint format;
	if( !eglGetConfigAttrib( mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &format ) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	ANativeWindow_setBuffersGeometry( window, 0, 0, format );
	if( !(mSurface = eglCreateWindowSurface( mDisplay, mConfig, window, 0)) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	const EGLint contextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	if( !(mContext = eglCreateContext( mDisplay, mConfig, EGL_NO_CONTEXT, contextAttributes)) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	if( !eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}

	mSwapInterval = mMinSwapInterval;
	eglSwapInterval( mDisplay, mSwapInterval );	// V-sync wait?

	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &mDefaultFrameBufferId );
	return true;
#endif
}
void tTVPOpenGLScreen::GetConfigAttribute( EGLConfig config ) {
	eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &mRedBits );
	eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &mGreenBits );
	eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &mBlueBits );
	eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &mAlphaBits );
	eglGetConfigAttrib( mDisplay, config, EGL_DEPTH_SIZE, &mDepthBits );
	eglGetConfigAttrib( mDisplay, config, EGL_STENCIL_SIZE, &mStencilBits );
	eglGetConfigAttrib( mDisplay, config, EGL_MIN_SWAP_INTERVAL, &mMinSwapInterval );
	eglGetConfigAttrib( mDisplay, config, EGL_MAX_SWAP_INTERVAL, &mMaxSwapInterval );
	ttstr displog = ttstr( TJS_W( "(info) EGL config :" ) );
	displog += ttstr( TJS_W( " R:" ) ) + to_tjs_string( mRedBits );
	displog += ttstr( TJS_W( " G:" ) ) + to_tjs_string( mGreenBits );
	displog += ttstr( TJS_W( " B:" ) ) + to_tjs_string( mBlueBits );
	displog += ttstr( TJS_W( " A:" ) ) + to_tjs_string( mAlphaBits );
	displog += ttstr( TJS_W( " Depth:" ) ) + to_tjs_string( mDepthBits );
	displog += ttstr( TJS_W( " Stencil:" ) ) + to_tjs_string( mStencilBits );
	TVPAddLog( displog );
}
void tTVPOpenGLScreen::ReleaseSurface() {
#ifdef ANDROID
    if( mSurface != EGL_NO_SURFACE ) {
        assert( mDisplay != EGL_NO_DISPLAY );
        eglDestroySurface( mDisplay, mSurface );
        mSurface = EGL_NO_SURFACE;
    }
#endif
}
void tTVPOpenGLScreen::UpdateWindowSurface( void* nativeHandle ) {
	if( mSurface != EGL_NO_SURFACE ) {
		return;
	}
#ifdef ANDROID
	NativeHandle = nativeHandle;
	ANativeWindow* window = reinterpret_cast<ANativeWindow*>(NativeHandle);
	EGLint format;
	if( !eglGetConfigAttrib( mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &format ) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return;
	}
	ANativeWindow_setBuffersGeometry( window, 0, 0, format );
	if( !(mSurface = eglCreateWindowSurface( mDisplay, mConfig, window, 0)) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return;
	}
	if( !eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) ) {
		CheckEGLErrorAndLog();
		Destroy();
		return;
	}
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
#elif defined( ANDROID )
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
#endif
}
bool tTVPOpenGLScreen::IsInitialized() const {
	return mSurface != EGL_NO_SURFACE && mContext != EGL_NO_CONTEXT && mDisplay != EGL_NO_DISPLAY;
}

void tTVPOpenGLScreen::Swap() {
	if( mSurface != EGL_NO_SURFACE ) {
		eglSwapBuffers( mDisplay, mSurface );
	}
}
EGLint tTVPOpenGLScreen::GetSurfaceWidth() const {
	EGLint result = 0;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_WIDTH, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}
EGLint tTVPOpenGLScreen::GetSurfaceHeight() const {
	EGLint result = 0;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_HEIGHT, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}
bool tTVPOpenGLScreen::CheckEGLErrorAndLog() {
	GLenum error_code = eglGetError();
	if( error_code == EGL_SUCCESS ) return true;
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
	default: TVPAddLog( (tjs_string(TJS_W( "ANGLE Error : " )) + to_tjs_string( (unsigned long)error_code )).c_str() ); break;
	}
	return false;
}
bool tTVPOpenGLScreen::CheckGLErrorAndLog(const tjs_char* funcname) {
	GLenum error_code = glGetError();
	if( error_code == GL_NO_ERROR ) return true;
	if( funcname != nullptr ) {
		TVPAddLog( ttstr(funcname) );
	}
	switch( error_code ) {
	case GL_INVALID_ENUM: TVPAddLog( TJS_W( "GL error : GL_INVALID_ENUM." ) ); break;
	case GL_INVALID_VALUE: TVPAddLog( TJS_W( "GL error : GL_INVALID_VALUE." ) ); break;
	case GL_INVALID_OPERATION: TVPAddLog( TJS_W( "GL error : GL_INVALID_OPERATION." ) ); break;
	case GL_OUT_OF_MEMORY: TVPAddLog( TJS_W( "GL error : GL_OUT_OF_MEMORY." ) ); break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: TVPAddLog( TJS_W( "GL error : GL_INVALID_FRAMEBUFFER_OPERATION." ) ); break;
	default: TVPAddLog( (tjs_string(TJS_W( "GL error code : " )) + to_tjs_string( (unsigned long)error_code )).c_str() ); break;
	}
	return false;
}
void tTVPOpenGLScreen::SetScissorRect( const tTVPRect& rect ) {
	// Scissorは左下原点
	EGLint height;
	if( eglQuerySurface( mDisplay, mSurface, EGL_HEIGHT, &height ) == EGL_TRUE ) {
		glScissor( rect.left, height-rect.bottom, rect.get_width(), rect.get_height() );
		glEnable( GL_SCISSOR_TEST );
	} else {
		CheckEGLErrorAndLog();
	}
}
void tTVPOpenGLScreen::DisableScissorRect() {
	glDisable( GL_SCISSOR_TEST );
}
void tTVPOpenGLScreen::SetWaitVSync( bool b ) {
	if( b ) {
		mSwapInterval = 1;
		eglSwapInterval( mDisplay, mSwapInterval );
	} else {
		mSwapInterval = mMinSwapInterval;
		eglSwapInterval( mDisplay, mSwapInterval );
	}
}
