
#include "tjsCommHead.h"
#include "GLFrameBufferObject.h"
#include "DebugIntf.h"
#include "OpenGLScreen.h"
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"
#include "tvpgl.h"
#include <memory>


bool GLFrameBufferObject::create( GLuint w, GLuint h ) {
	destory();

	GLint fb;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fb );
	glGenFramebuffers( 1, &framebuffer_id_ );
	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );

	glGenRenderbuffers( 1, &renderbuffer_id_ );
	glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer_id_ );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_id_ );

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
bool GLFrameBufferObject::exchangeTexture( GLuint tex_id ) {
	GLint fb;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fb );

	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 );

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
	if( result ) {
		texture_id_ = tex_id;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, fb );

	return result;
}
bool GLFrameBufferObject::readFrameBuffer( tjs_uint x, tjs_uint y, tjs_uint width, tjs_uint height, tjs_uint8* dest, bool front ) {
	glReadBuffer( front ? GL_FRONT : GL_BACK );
	glReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dest );
	return tTVPOpenGLScreen::CheckEGLErrorAndLog();
}
bool GLFrameBufferObject::readTextureToBitmap( tTVPBaseBitmap* bmp ) {
	if( !bmp ) {
		TVPAddLog( TJS_W("Bitmap is null.") );
		return false;
	}

	// Bitmap の内部表現が逆順前提
	bmp->SetSize( width_, height_, false );
	tjs_uint32* dest = reinterpret_cast<tjs_uint32*>(bmp->GetScanLineForWrite(height_-1));

	// FrameBuffer に一度設定してから読み出す処理をする。
	// OpenGL ES に glGetTextureImage がない悲しさ
	readTextureUseFBO( reinterpret_cast<tjs_uint8*>(dest) );

	// 赤と青を入れ替えながらコピー
	tjs_int pitch = bmp->GetPitchBytes();
	for( tjs_uint y = 0; y < height_; y++ ) {
		TVPRedBlueSwap( dest, width_ );
		dest = reinterpret_cast<tjs_uint32*>(reinterpret_cast<tjs_uint8*>(dest)+pitch);
	}

	return true;
}
bool GLFrameBufferObject::readTextureToBitmap( class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int left, tjs_int top ) {
	// clip src texture area
	tTVPRect clip( srcRect );
	if( clip.right > (tjs_int)width_ ) clip.right = width_;
	if( clip.bottom > (tjs_int)height_ ) clip.bottom = height_;
	if( clip.left < 0 ) clip.left = 0;
	if( clip.top < 0 ) clip.top = 0;

	// clip dest bitmap area
	if( left < 0 ) {
		clip.left += -left;
		left = 0;
	}
	if( top < 0 ) {
		clip.top += -top;
		top = 0;
	}
	if( (tjs_int)bmp->GetWidth() < (left+clip.get_width()) ) clip.set_width( bmp->GetWidth() - left );
	if( (tjs_int)bmp->GetHeight() < (top+clip.get_height()) ) clip.set_height( bmp->GetHeight() - top );

	// has copy area?
	if( clip.get_width() <= 0 || clip.get_height() <= 0 ) {
		TVPAddLog(TJS_W("out of area"));
		return false;
	}
	if( clip.left >= (tjs_int)bmp->GetWidth() || clip.top >= (tjs_int)bmp->GetHeight() ) {
		TVPAddLog(TJS_W("out of area"));
		return false;
	}
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
	std::unique_ptr<tjs_uint32[]> buffer(new tjs_uint32[clip.get_width()*clip.get_height()]);
	glReadBuffer( GL_BACK );
	glReadPixels( clip.left, clip.top, clip.get_width(), clip.get_height(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buffer.get() );
	if( fb != framebuffer_id_ ) {
		glBindFramebuffer( GL_FRAMEBUFFER, fb );
	}
	if( vp[2] != width_ || vp[3] != height_ || vp[0] != 0 || vp[1] != 0 ) {
		glViewport( vp[0], vp[1], vp[2], vp[3] );
	}

	// 赤と青を入れ替えながらコピー
	//for( tjs_int y = 0, line=top+clip.get_height()-1; y < clip.get_height(); y++, line-- ) {
	for( tjs_int y = 0, line=top; y < clip.get_height(); y++, line++ ) {
		tjs_uint32* dest = reinterpret_cast<tjs_uint32*>(bmp->GetScanLineForWrite(line)) + left;
		TVPRedBlueSwapCopy( dest, &buffer[y*clip.get_width()], clip.get_width() );
	}

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
void GLFrameBufferObject::bindFramebuffer() {
	if( framebuffer_id_ ) {
		glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );
		glViewport( 0, 0, width_, height_ );
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
