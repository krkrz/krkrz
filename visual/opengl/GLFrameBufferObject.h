#ifndef GLFrameBufferObjectH
#define GLFrameBufferObjectH

#include "OpenGLHeader.h"
#include "ComplexRect.h"

class GLFrameBufferObject {
	
protected:
	GLuint texture_id_;
	GLuint framebuffer_id_;
	GLuint renderbuffer_id_;
	GLuint width_;
	GLuint height_;
	// format は GL_RGBA でないと問題が出る GPU があるようなのでそれのみ。

	void readTextureUseFBO( GLubyte* pixels );
	//void readTextureUsePBO( GLubyte* pixels );
public:
	GLFrameBufferObject() : texture_id_(0), framebuffer_id_(0), renderbuffer_id_(0), width_(0), height_(0) {}
	~GLFrameBufferObject() {
		destory();
	}
	bool create( GLuint w, GLuint h );
	void destory() {
		if( texture_id_ != 0 ) {
			glDeleteTextures( 1, &texture_id_ );
			texture_id_ = 0;
		}
		if( renderbuffer_id_ != 0 ) {
			glDeleteRenderbuffers( 1, &renderbuffer_id_ );
			renderbuffer_id_ = 0;
		}
		if( framebuffer_id_ != 0 ) {
			glDeleteFramebuffers( 1, &framebuffer_id_ );
			framebuffer_id_ = 0;
		}
		width_ = height_ = 0;
	}
	void bindFramebuffer();
	void copyImage( GLint x, GLint y, GLint w, GLint h, const GLvoid* bits ) {
		 glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, bits );
	}
	bool exchangeTexture( GLuint tex_id );

	GLuint textureId() const { return texture_id_; }
	GLuint width() const { return width_; }
	GLuint height() const { return height_; }
	GLint format() const { return GL_RGBA; }

	static bool readFrameBuffer( tjs_uint x, tjs_uint y, tjs_uint width, tjs_uint height, tjs_uint8* dest, bool front );

	bool readTextureToBitmap( class tTVPBaseBitmap* bmp );
	bool readTextureToBitmap( class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int dleft, tjs_int dtop );
};


#endif