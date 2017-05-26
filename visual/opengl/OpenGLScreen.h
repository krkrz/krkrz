
#ifndef OpenGLScreenH
#define OpenGLScreenH

#include "OpenGLHeader.h"

class tTVPOpenGLScreen {
	void* NativeHandle;

	EGLConfig mConfig;
	EGLDisplay mDisplay;
	EGLSurface mSurface;
	EGLContext mContext;
	EGLint mSwapInterval;

	int mRedBits;
	int mGreenBits;
	int mBlueBits;
	int mAlphaBits;
	int mDepthBits;
	int mStencilBits;
	bool mMultisample;

#ifdef WIN32
	//EGLNativeWindowType mParentWindow;	// HWND
	//EGLNativeWindowType mNativeWindow;	// HWND
	EGLNativeDisplayType mNativeDisplay;// HDC
#endif
public:
	tTVPOpenGLScreen( void* nativeHandle );

	bool Initialize();
	void Destory();
	bool IsInitialized() const;

	void Swap() { eglSwapBuffers( mDisplay, mSurface ); }
	EGLConfig GetConfig() const { return mConfig; }
	EGLDisplay GetDisplay() const { return mDisplay; }
	EGLSurface GetSurface() const { return mSurface; }
	EGLContext GetContext() const { return mContext; }
};



#endif
