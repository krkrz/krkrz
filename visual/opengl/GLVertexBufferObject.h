
#ifndef __GL_VERTEX_BUFFER_OBJECT_H__
#define __GL_VERTEX_BUFFER_OBJECT_H__

#include "OpenGLHeader.h"


class GLVertexBufferObject {
	GLuint vbo_id_;
	GLenum target_;
	GLenum usage_;
	tjs_int size_;

public:
	GLVertexBufferObject() : vbo_id_(0), target_(0), usage_(GL_STATIC_DRAW) {}

	~GLVertexBufferObject() {
		destory();
	}
	void destory() {
		if( vbo_id_ ) {
			glDeleteBuffers( 1, &vbo_id_ );
			vbo_id_ = 0;
		}
	}

	void createStaticVertex( const GLfloat* vtx, tjs_int byteSize ) {
		destory();

		glGenBuffers( 1, &vbo_id_ );
		glBindBuffer( GL_ARRAY_BUFFER, vbo_id_ );
		glBufferData( GL_ARRAY_BUFFER, byteSize, (const void *)vtx, GL_STATIC_DRAW );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		target_ = GL_ARRAY_BUFFER;
		usage_ = GL_STATIC_DRAW;
		size_ = byteSize;
	}
	void createVertexBuffer( tjs_int byteSize, GLenum usage, bool isIndex = false, const void* data = nullptr ) {
		destory();
		if( isIndex ) {
			target_ = GL_ELEMENT_ARRAY_BUFFER;
		} else {
			target_ = GL_ARRAY_BUFFER;
		}

		glGenBuffers( 1, &vbo_id_ );
		glBindBuffer( target_, vbo_id_ );
		glBufferData( target_, byteSize, data, usage );
		glBindBuffer( target_, 0 );
		usage_ = usage;
		size_ = byteSize;
	}
	void copyBuffer( GLintptr offset, GLsizeiptr size,  const GLvoid * data) {
		if( vbo_id_ ) {
			glBindBuffer( target_, vbo_id_ );
			glBufferSubData( target_, offset, size, data );
			glBindBuffer( target_, 0 );
		}
	}

	void bindBuffer() const {
		if( vbo_id_ ) {
			glBindBuffer( target_, vbo_id_ );
		}
	}
	void unbindBuffer() const {
		glBindBuffer( target_, 0 );
	}
	void* mapBuffer() {
		void* result = nullptr;
		if( vbo_id_ ) {
			glBindBuffer( target_, vbo_id_ );
			result = glMapBufferRange( target_, 0, size_, GL_MAP_READ_BIT|GL_MAP_WRITE_BIT );
			glBindBuffer( target_, 0 );
		}
		return result;
	}
	void unmapBuffer() {
		glUnmapBuffer( target_ );
	}
	bool isCreated() const { return vbo_id_ != 0; }

	GLuint id() const { return vbo_id_; }
	GLenum usage() const { return usage_; }
	GLenum target() const { return target_; }
	tjs_int size() const { return  size_; }

	bool isIndex() const { return target_ == GL_ELEMENT_ARRAY_BUFFER; }
};


#endif
