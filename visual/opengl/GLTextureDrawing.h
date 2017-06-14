
#ifndef GLTextureDrawingH
#define GLTextureDrawingH

#include "OpenGLHeader.h"

class tTVPGLTextureDrawing {
	// Handle to a program object
	GLuint Program;
	// Attribute locations
	GLint PositionLoc;
	GLint TexCoordLoc;
	// Sampler location
	GLint SamplerLoc;


	GLuint ProgramColorPoly;
	GLint PositionColorPolyLoc;
	GLint ColorColorPolyLoc;
public:
	tTVPGLTextureDrawing();

	// シンプルなシェーダーのみとりあえず
	bool InitializeShader();
	void DestroyShader();
	void DrawTexture( GLuint tex, int x, int y,int w, int h, int sw, int sh );
	void DrawColoredPolygon( tjs_uint32 color[4], int x, int y, int w, int h, int sw, int sh );
	void DrawTexture( GLuint tex, int w, int h, int sw, int sh, GLint posLoc, GLint uvLoc, GLint texLoc );
	void DrawTexture2( GLuint tex0, GLuint tex1, int w, int h, int sw, int sh, GLint posLoc, GLint uvLoc, GLint tx0Loc, GLint tx1Loc );
};



#endif
