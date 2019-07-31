
#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__

#include "OpenGLHeader.h"

struct GLTextreImageSet {
	GLuint width;
	GLuint height;
	const GLvoid* bits;
	GLTextreImageSet( GLuint w, GLuint h, const GLvoid* b ) : width( w ), height( h ), bits( b ) {}
};
extern int TVPOpenGLESVersion;
class GLTexture {
protected:
	GLuint texture_id_;
	GLint format_;
	GLuint width_;
	GLuint height_;

	GLenum stretchType_;
	GLenum wrapS_;
	GLenum wrapT_;
	bool hasMipmap_ = false;
public:
	GLTexture() : texture_id_(0), width_(0), height_(0), format_(0), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {}
	GLTexture( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) : width_(w), height_(h), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {
		create( w, h, bits, format );
	}
	~GLTexture() {
		destory();
	}

	void create( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) {
		glPixelStorei( GL_UNPACK_ALIGNMENT, format == GL_RGBA ? 4 : 1 );
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
	/**
	* ミップマップを持つテクスチャを生成する
	* 今のところ GL_RGBA 固定
	*/
	void createMipmapTexture( std::vector<GLTextreImageSet>& img ) {
		if( img.size() > 0 ) {
			GLuint w = img[0].width;
			GLuint h = img[0].height;
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
			glGenTextures( 1, &texture_id_ );
			glBindTexture( GL_TEXTURE_2D, texture_id_ );

			GLint count = img.size();
			if( count > 1 ) hasMipmap_ = true;

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stretchType_ );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getMipmapFilter( stretchType_ ) );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_ );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_ );
			// ミップマップの最小と最大レベルを指定する、これがないと存在しないレベルを参照しようとすることが発生しうる
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1 );
			if( TVPOpenGLESVersion == 200 ) {
				// OpenGL ES2.0 の時は、glGenerateMipmap しないと正しくミップマップ描画できない模様
				GLTextreImageSet& tex = img[0];
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
				glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
				glGenerateMipmap( GL_TEXTURE_2D );
				// 自前で生成したものに一部置き換える
				for( GLint i = 1; i < count; i++ ) {
					GLTextreImageSet& tex = img[i];
					glTexSubImage2D( GL_TEXTURE_2D, i, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
				}
			} else {
				for( GLint i = 0; i < count; i++ ) {
					GLTextreImageSet& tex = img[i];
					glTexImage2D( GL_TEXTURE_2D, i, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
				}

			}
			glBindTexture( GL_TEXTURE_2D, 0 );
			format_ = GL_RGBA;
			width_ = w;
			height_ = h;
		}
	}
	void destory() {
		if( texture_id_ != 0 ) {
			glDeleteTextures( 1, &texture_id_ );
			texture_id_ = 0;
			hasMipmap_ = false;
		}
	}
	/**
	 * フィルタタイプに応じたミップマップテクスチャフィルタを返す
	 * GL_NEAREST_MIPMAP_LINEAR/GL_LINEAR_MIPMAP_LINEAR は使用していない
	 */
	static GLint getMipmapFilter( GLint filter ) {
		switch( filter ) {
		case GL_NEAREST:
			return GL_NEAREST_MIPMAP_NEAREST;
		case GL_LINEAR:
			return GL_LINEAR_MIPMAP_NEAREST;
		case GL_NEAREST_MIPMAP_NEAREST:
			return GL_NEAREST_MIPMAP_NEAREST;
		case GL_LINEAR_MIPMAP_NEAREST:
			return GL_LINEAR_MIPMAP_NEAREST;
		case GL_NEAREST_MIPMAP_LINEAR:
			return GL_NEAREST_MIPMAP_LINEAR;
		case GL_LINEAR_MIPMAP_LINEAR:
			return GL_LINEAR_MIPMAP_LINEAR;
		default:
			return GL_LINEAR_MIPMAP_NEAREST;
		}
	}
	void copyImage( GLint x, GLint y, GLint w, GLint h, const GLvoid* bits ) {
		glPixelStorei( GL_UNPACK_ALIGNMENT, format_ == GL_RGBA ? 4 : 1 );
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
			if( hasMipmap_ == false ) {
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s );
			} else {
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getMipmapFilter(s) );
			}
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
