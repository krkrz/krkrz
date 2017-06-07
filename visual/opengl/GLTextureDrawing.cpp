

#include "tjsCommHead.h"
#include "GLShaderUtil.h"
#include "GLTextureDrawing.h"


tTVPGLTextureDrawing::tTVPGLTextureDrawing() : Program(0), PositionLoc(0), TexCoordLoc(0), SamplerLoc(0) {}

bool tTVPGLTextureDrawing::InitializeShader() {
	const std::string vs = SHADER_SOURCE
	(
		attribute vec4 a_position;
		attribute vec2 a_texCoord;
		varying vec2 v_texCoord;
		varying vec4 vPosition;
		void main()
		{
			gl_Position = a_position;
			v_texCoord = a_texCoord;
			vPosition = a_position;
		}
	);
	const std::string fs = SHADER_SOURCE
	(
		precision mediump float;
		varying vec2 v_texCoord;
		varying vec4 vPosition;
		uniform sampler2D s_texture;
		void main()
		{
			gl_FragColor = texture2D( s_texture, v_texCoord );
		}
	);
	Program = CompileProgram(vs, fs);
	if( !Program ) {
		return false;
	}
	PositionLoc = glGetAttribLocation(Program, "a_position");
	TexCoordLoc = glGetAttribLocation(Program, "a_texCoord");
	SamplerLoc = glGetUniformLocation(Program, "s_texture");


	const std::string vs2 = SHADER_SOURCE
	(
		attribute vec2 a_position;
		attribute vec4 a_color;
		varying vec4 v_color;
		void main()
		{
			gl_Position = vec4( a_position, 0.0, 1.0 );
			v_color = a_color;
		}
	);
	const std::string fs2 = SHADER_SOURCE
	(
		precision mediump float;
		varying vec4 v_color;
		void main()
		{
			gl_FragColor = v_color;
		}
	);
	ProgramColorPoly = CompileProgram( vs2, fs2 );
	if( !ProgramColorPoly ) {
		return false;
	}
	PositionColorPolyLoc = glGetAttribLocation( ProgramColorPoly, "a_position" );
	ColorColorPolyLoc = glGetAttribLocation( ProgramColorPoly, "a_color" );

	return true;
}
void tTVPGLTextureDrawing::DestroyShader() {
	if( Program ) {
		glDeleteProgram( Program );
		Program = 0;
	}
}
void tTVPGLTextureDrawing::SetProgram( GLuint program ) {
	glUseProgram(Program);
	PositionLoc = glGetAttribLocation(Program, "a_position");
	TexCoordLoc = glGetAttribLocation(Program, "a_texCoord");
	SamplerLoc = glGetUniformLocation(Program, "s_texture");
}
GLuint tTVPGLTextureDrawing::GetProgram() const {
	GLint program;
	glGetIntegerv( GL_CURRENT_PROGRAM, &program );
	return program;
}
void tTVPGLTextureDrawing::DrawTexture( GLuint tex, int x, int y,int w, int h, int sw, int sh ) {
	float left  =((float)x/(float)sw)*2.0f-1.0f;
	float top   =((float)y/(float)sh)*2.0f-1.0f;
	float right =((float)(x+w)/(float)sw)*2.0f-1.0f;
	float bottom=((float)(y+h)/(float)sh)*2.0f-1.0f;
	top   =-top;
	bottom=-bottom;
#if 1
	GLfloat vertices[] = {
        left,  top, 0.0f,  // Position 0
         0.0f,  0.0f,        // TexCoord 0
        left, bottom, 0.0f,  // Position 1
         0.0f,  1.0f,        // TexCoord 1
         right, bottom, 0.0f,  // Position 2
         1.0f,  1.0f,        // TexCoord 2
         right,  top, 0.0f,  // Position 3
         1.0f,  0.0f         // TexCoord 3
	};
#else
	GLfloat vertices[] = {
        -0.5f,  0.5f, 0.0f,  // Position 0
         0.0f,  0.0f,        // TexCoord 0
        -0.5f, -0.5f, 0.0f,  // Position 1
         0.0f,  1.0f,        // TexCoord 1
         0.5f, -0.5f, 0.0f,  // Position 2
         1.0f,  1.0f,        // TexCoord 2
         0.5f,  0.5f, 0.0f,  // Position 3
         1.0f,  0.0f         // TexCoord 3
	};
	/*
	GLfloat vertices[] = {
		-0.5f,  0.5f, 0.0f,  // Position 0
		0.0f,  1.0f,        // TexCoord 0
		-0.5f, -0.5f, 0.0f,  // Position 1
		0.0f,  0.0f,        // TexCoord 1
		0.5f, -0.5f, 0.0f,  // Position 2
		1.0f,  0.0f,        // TexCoord 2
		0.5f,  0.5f, 0.0f,  // Position 3
		1.0f,  1.0f         // TexCoord 3
	};
	*/
#endif
	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Set the viewport
	// glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

	// Clear the color buffer
	// glClear(GL_COLOR_BUFFER_BIT);
	// clipping rect
	//glScissor( 10, 10, 400, 400 );
	//glEnable( GL_SCISSOR_TEST );

	// Use the program object
	glUseProgram(Program);

	// Load the vertex position
	glVertexAttribPointer(PositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
	// Load the texture coordinate
	glVertexAttribPointer(TexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices + 3);

	glEnableVertexAttribArray(PositionLoc);
	glEnableVertexAttribArray(TexCoordLoc);

	// Bind the texture
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, tex );
	glUniform1i( SamplerLoc, 0 );

	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
}

void tTVPGLTextureDrawing::DrawColoredPolygon( tjs_uint32 color[4], int x, int y, int w, int h, int sw, int sh ) {
	float left = ( (float)x / (float)sw )*2.0f - 1.0f;
	float top = ( (float)y / (float)sh )*2.0f - 1.0f;
	float right = ( (float)( x + w ) / (float)sw )*2.0f - 1.0f;
	float bottom = ( (float)( y + h ) / (float)sh )*2.0f - 1.0f;
	top = -top;
	bottom = -bottom;
	GLfloat vertices[] = {
		left,  top,		// 左上
		left,  bottom,  // 左下
		right, top,		// 右上
		right, bottom,	// 右下
	};
	GLfloat colors[16];
	for( tjs_int i = 0; i < 4; i++ ) {
		float a = ( ( color[i] & 0xff000000 ) >> 24 ) / 255.0f;
		float r = ( ( color[i] & 0x00ff0000 ) >> 16 ) / 255.0f;
		float g = ( ( color[i] & 0x0000ff00 ) >> 8 ) / 255.0f;
		float b = ( ( color[i] & 0x000000ff ) >> 0 ) / 255.0f;
		colors[i * 4 + 0] = r;
		colors[i * 4 + 1] = g;
		colors[i * 4 + 2] = b;
		colors[i * 4 + 3] = a;
	}
	glUseProgram( ProgramColorPoly );
	glVertexAttribPointer( PositionColorPolyLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	glVertexAttribPointer( ColorColorPolyLoc, 4, GL_FLOAT, GL_FALSE, 4*sizeof( GLfloat ), colors );
	glEnableVertexAttribArray( PositionColorPolyLoc );
	glEnableVertexAttribArray( ColorColorPolyLoc );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
