

#include "tjsCommHead.h"
#include "GLShaderUtil.h"
#include "GLTextureDrawing.h"
#include "ShaderProgramIntf.h"
#include "Matrix44Intf.h"
#include "TextureInfo.h"

tTVPGLTextureDrawing::tTVPGLTextureDrawing() : ProgramColorPoly(0), PositionColorPolyLoc(0), ColorColorPolyLoc(0) {}

bool tTVPGLTextureDrawing::InitializeShader() {
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

	/*
	両方のアルファ値を考慮する場合
	Ar = As + (1 - As) * Ad
	Cr = [(Cs * As) + (Cd * (1 - As) * Ad)] / Ar
	愚直な実装
	*/
#if 0
	const std::string fscross = SHADER_SOURCE
	(
		precision mediump float;
		varying vec2 v_texCoord;
		varying vec4 vPosition;
		uniform sampler2D s_texture0;
		uniform sampler2D s_texture1;
		uniform float s_opacity;
		void main()
		{
			vec4 cs = texture2D( s_texture0, v_texCoord );
			vec4 cd = texture2D( s_texture1, v_texCoord );
			float as = cs.a;
			float ad = cd.a;
			float ar = as + ( 1.0 - as ) * ad;
			gl_FragColor.rgb = ( ( cs.rgb * as ) + ( cd.rgb * ( 1 - as ) * ad ) ) / ar;
			gl_FragColor.a = ar;
		}
	);
#endif
	return true;
}
void tTVPGLTextureDrawing::DestroyShader() {
	if( ProgramColorPoly ) {
		glDeleteProgram( ProgramColorPoly );
		ProgramColorPoly = 0;
	}
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
void tTVPGLTextureDrawing::DrawTexture( tTJSNI_ShaderProgram* shader, const iTVPTextureInfoIntrface* tex, tTJSNI_Matrix44* mat, const tTVPPoint& sufaceSize ) {
	const float width = (float)tex->GetWidth();
	const float height = (float)tex->GetHeight();
	const GLfloat vertices[] = {
		0.0f,  0.0f,	// 左上
		0.0f,  height,  // 左下
		width, 0.0f,	// 右上
		width, height,	// 右下
	};
	const GLfloat uvs[] = {
		0.0f,  0.0f,
		0.0f,  1.0f,
		1.0f,  0.0f,
		1.0f,  1.0f,
	};
	GLint posLoc = shader->FindLocation( std::string( "a_pos" ) );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	glVertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	glVertexAttribPointer( uvLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), uvs );
	glEnableVertexAttribArray( posLoc );
	glEnableVertexAttribArray( uvLoc );

	GLint texLoc = shader->FindLocation( std::string( "s_tex0" ) );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)tex->GetNativeHandle() );
	glUniform1i( texLoc, 0 );

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, mat->GetMatrixArray() );
	
	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	glUniform2f( vpLoc, (float)sufaceSize.x, (float)sufaceSize.y );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
