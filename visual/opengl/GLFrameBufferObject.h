#ifndef GLFrameBufferObjectH
#define GLFrameBufferObjectH

#include "OpenGLHeader.h"

class GLFrameBufferObject {
	
protected:
	GLuint texture_id_;
	GLuint framebuffer_id_;
	GLuint renderbuffer_id_;
	GLuint width_;
	GLuint height_;
	// format は GL_RGBA でないと問題が出る GPU があるようなのでそれのみ。

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
	}
	void bindFramebuffer() {
		glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id_ );
	}

	GLuint textureId() const { return texture_id_; }
	GLuint width() const { return width_; }
	GLuint height() const { return height_; }
};


#endif