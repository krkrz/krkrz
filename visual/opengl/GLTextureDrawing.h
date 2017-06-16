
#ifndef GLTextureDrawingH
#define GLTextureDrawingH

#include "OpenGLHeader.h"
#include "ComplexRect.h"

class tTVPGLTextureDrawing {
	GLuint ProgramColorPoly;
	GLint PositionColorPolyLoc;
	GLint ColorColorPolyLoc;

public:
	tTVPGLTextureDrawing();

	// シンプルなシェーダーのみとりあえず
	bool InitializeShader();
	void DestroyShader();
	void DrawColoredPolygon( tjs_uint32 color[4], int x, int y, int w, int h, int sw, int sh );
	static void DrawTexture( class tTJSNI_ShaderProgram* shader, const class iTVPTextureInfoIntrface* tex, class tTJSNI_Matrix44* mat, const tTVPPoint& sufaceSize );
};
#endif
