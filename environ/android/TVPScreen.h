
#ifndef __TVP_SCREEN_H__
#define __TVP_SCREEN_H__

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "MsgIntf.h"

class iTVPSurface;
class tTVPScreen {
	EGLDisplay display_;
	EGLContext context_;
	EGLSurface surface_;
	EGLint width_;
	EGLint height_;
	EGLint format_;

	GLuint program_;
	GLuint positionHandle_;
	GLuint uvHandle_;
	GLuint texHandle_;

	class iTVPSurface* root_;

	static const char VERTEX_CODE[];
	static const char FRAGMENT_CODE[];

	void setupProgram();
	GLuint loadShader( GLenum shaderType, const char* source );
	void printShaderCompileLog( GLuint shader );
	void printShaderLinkLog( GLuint program );
public:
	tTVPScreen();
	~tTVPScreen();

	void attachRoot( iTVPSurface* node ) {
		if( root_ != NULL ) {
			TVPThrowExceptionMessage(TJS_W("Already exist root node."));
		}
		root_ = node;
	}
	void detachRoot( iTVPSurface* node ) {
		if( root_ == node ) {
			root_ = NULL;
		}
	}

	bool initialize( class tTVPApplication* app );
	void tarminate();
	void drawFrame();

	void beginDraw();
	void drawTexture( int x, int y, int w, int h, GLuint textureId );
	void endDraw();

	tjs_int getWidth() const { return width_; }
	tjs_int getHeight() const { return height_; }

	GLuint createProgram( const char* vertexCode, const char* fragmentCode );
/*
	static int GetWidth();
	static int GetHeight();
	static void GetDesktopRect( RECT& r );
	static int GetDesktopLeft();
	static int GetDesktopTop();
	static int GetDesktopWidth();
	static int GetDesktopHeight();
*/
	void OnActivate();
	void OnDeactivate();
/*
	void OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	void OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);
*/
	void OnKeyDown(tjs_uint key, tjs_uint32 shift);
	void OnKeyUp(tjs_uint key, tjs_uint32 shift);
	void OnKeyPress(tjs_char key);

	void OnTouchDown( float x, float y, float cx, float cy, tjs_uint32 id );
	void OnTouchUp( float x, float y, float cx, float cy, tjs_uint32 id );
	void OnTouchMove( float x, float y, float cx, float cy, tjs_uint32 id );
	
	void OnTouchScaling( float startdist, float curdist, float cx, float cy, tjs_int flag );
	void OnTouchRotate( float startangle, float curangle, float dist, float cx, float cy, tjs_int flag );
	void OnMultiTouch();

	void OnDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution );
};

#endif // __TVP_SCREEN_H__
