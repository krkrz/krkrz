
#ifndef GLTextureDrawingH
#define GLTextureDrawingH

#include "OpenGLHeader.h"

class tTVPGLTextureDrawing {
	// Handle to a program object
	GLuint mProgram;
	// Attribute locations
	GLint mPositionLoc;
	GLint mTexCoordLoc;
	// Sampler location
	GLint mSamplerLoc;

public:
	tTVPGLTextureDrawing();

	// シンプルなシェーダーのみとりあえず
	bool InitializeShader();
	void DestroyShader();
	void DrawTexture( GLuint tex, int x, int y,int w, int h, int sw, int sh );
};



#endif
