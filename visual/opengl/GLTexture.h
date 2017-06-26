
#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__

#include "OpenGLHeader.h"

#ifdef ANDROID
extern bool TVPIsSupportGLES3();
#endif

class GLTexture {
protected:
	GLuint texture_id_;
	GLint format_;
	GLuint width_;
	GLuint height_;

	GLenum stretchType_;
	GLenum wrapS_;
	GLenum wrapT_;
public:
	GLTexture() : texture_id_(0), width_(0), height_(0), format_(0), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {}
	GLTexture( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) : width_(w), height_(h), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {
		create( w, h, bits, format );
	}
	~GLTexture() {
		destory();
	}

	void create( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) {
		glGenTextures( 1, &texture_id_ );
		glBindTexture( GL_TEXTURE_2D, texture_id_ );
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,stretchType_);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,stretchType_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_);
		glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
		glBindTexture( GL_TEXTURE_2D, 0 );
		format_ = format;
		width_ = w;
		height_ = h;
	}
	void destory() {
		if( texture_id_ != 0 ) {
			glDeleteTextures( 1, &texture_id_ );
			texture_id_ = 0;
		}
	}
	void copyImage( GLint x, GLint y, GLint w, GLint h, const GLvoid* bits ) {
		 glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, format_, GL_UNSIGNED_BYTE, bits );
	}
	static int getMaxTextureSize() {
		GLint maxTex;
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTex );
		return maxTex;
	}
	GLuint width() const { return width_; }
	GLuint height() const { return height_; }
	GLuint id() const { return texture_id_; }
	GLint format() const { return format_; }

	GLenum stretchType() const { return stretchType_; }
	void setStretchType( GLenum s ) {
		if( texture_id_ && stretchType_ != s ) {
			glBindTexture( GL_TEXTURE_2D, texture_id_ );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s );
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
		stretchType_ = s;
	}
	GLenum wrapS() const { return wrapS_; }
	void setWrapS( GLenum s ) {
		if( texture_id_ && wrapS_ != s ) {
			glBindTexture( GL_TEXTURE_2D, texture_id_ );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s );
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
		wrapS_ = s;
	}
	GLenum wrapT() const { return wrapT_; }
	void setWrapT( GLenum s ) {
		if( texture_id_ && wrapT_ != s ) {
			glBindTexture( GL_TEXTURE_2D, texture_id_ );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s );
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
		wrapT_ = s;
	}

	friend class tTJSNI_Offscreen;
};


#endif // __GL_TEXTURE_H__
