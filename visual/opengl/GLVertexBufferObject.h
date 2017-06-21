
#ifndef __GL_VERTEX_BUFFER_OBJECT_H__
#define __GL_VERTEX_BUFFER_OBJECT_H__

#include "OpenGLHeader.h"


class GLVertexBufferObject {
	GLuint vbo_id_;
	GLenum target_;

public:
	GLVertexBufferObject() : vbo_id_(0), target_(0) {}

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
		target_ = GL_ARRAY_BUFFER;
	}

	void bindBuffer() const {
		if( vbo_id_ ) {
			glBindBuffer( target_, vbo_id_ );
		}
	}
	void unbindBuffer() const {
		glBindBuffer( target_, 0 );
	}
	bool isCreated() const { return vbo_id_ != 0; }

	GLuint id() const { return vbo_id_; }
};


#endif
