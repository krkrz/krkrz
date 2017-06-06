
#include "tjsCommHead.h"
#include "GLFrameBufferObject.h"
#include "DebugIntf.h"
#include "OpenGLScreen.h"
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"
#include "tvpgl.h"

#ifdef ANDROID
extern bool TVPIsSupportGLES3();
#endif

bool GLFrameBufferObject::create( GLuint w, GLuint h ) {
	destory();

	GLint fb;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fb );
	glGenFramebuffers( 1, &framebuffer_id_ );
	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );

	glGenRenderbuffers( 1, &renderbuffer_id_ );
	glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer_id_ );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_id_ );

	glGenTextures( 1, &texture_id_ );
	glBindTexture( GL_TEXTURE_2D, texture_id_ );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id_, 0 );

	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	switch( status ) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		TVPAddLog( TJS_W("Not all framebuffer attachment points are framebuffer attachment complete.") );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
		TVPAddLog( TJS_W("Not all attached images have the same width and height.") );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		TVPAddLog( TJS_W("No images are attached to the framebuffer.") );
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		TVPAddLog( TJS_W("The combination of internal formats of the attached images violates an implementation-dependent set of restrictions. ") );
		break;
	}
	bool result = status == GL_FRAMEBUFFER_COMPLETE;
	if( result == false ) {
		destory();
	} else {
		width_ = w;
		height_ = h;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, fb );
	return result;
}
bool GLFrameBufferObject::readFrameBuffer( tjs_uint x, tjs_uint y, tjs_uint width, tjs_uint height, tjs_uint8* dest, bool front ) {
	glReadBuffer( front ? GL_FRONT : GL_BACK );
	glReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dest );
	return tTVPOpenGLScreen::CheckEGLErrorAndLog();
}
bool GLFrameBufferObject::readTextureToBitmap( class tTJSNI_Bitmap* bmp ) {
	if( !bmp ) {
		TVPAddLog( TJS_W("Bitmap is null.") );
		return false;
	}

	// Bitmap の内部表現が正順(上下反転されていない)ことを前提としているので注意
	bmp->SetSize( width_, height_, false );
	tTVPBaseBitmap* b = bmp->GetBitmap();
	tjs_uint32* dest = reinterpret_cast<tjs_uint32*>(b->GetScanLineForWrite(0));

	// FrameBuffer に一度設定してから読み出す処理をする。
	// OpenGL ES に glGetTextureImage がない悲しさ
	readTextureUseFBO( reinterpret_cast<tjs_uint8*>(dest) );

	tjs_int pitch = b->GetPitchBytes();
	for( tjs_uint y = 0; y < height_; y++ ) {
		TVPRedBlueSwap( dest, width_ );
		dest = reinterpret_cast<tjs_uint32*>(reinterpret_cast<tjs_uint8*>(dest)+pitch);
	}
	// FrameBuffer 内は上下反転しているので上下反転する
	tTVPRect rect( 0, 0, width_, height_ );
	b->UDFlip(rect);

	return true;
}

void GLFrameBufferObject::readTextureUseFBO( GLubyte* pixels ) {
	GLint vp[4];
	glGetIntegerv( GL_VIEWPORT, vp );
	GLint fb;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fb );
	if( fb != framebuffer_id_ ) {
		glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );
	}
	if( vp[2] != width_ || vp[3] != height_ || vp[0] != 0 || vp[1] != 0 ) {
		glViewport( 0, 0, width_, height_ );
	}
	glReadBuffer( GL_BACK );
	glReadPixels( 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	if( fb != framebuffer_id_ ) {
		glBindFramebuffer( GL_FRAMEBUFFER, fb );
	}
	if( vp[2] != width_ || vp[3] != height_ || vp[0] != 0 || vp[1] != 0 ) {
		glViewport( vp[0], vp[1], vp[2], vp[3] );
	}
}
// OpenGL ES 3.0 以降用
/* glGetTextureImage がないと PBO でも解決できない問題であった……
void GLFrameBufferObject::readTextureUsePBO( GLubyte* pixels ) {
	GLuint pbo;
	glGenBuffers( 1, &pbo );
	glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo[0] );
	glBufferData( GL_PIXEL_PACK_BUFFER, width_*height_*4, 0, GL_STREAM_READ );
	glBindBuffer( GL_PIXEL_PACK_BUFFER, g_iPBOSrc );
}
*/
