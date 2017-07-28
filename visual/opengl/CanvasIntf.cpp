
#include "tjsCommHead.h"

#include "CanvasIntf.h"
#include "Matrix32Intf.h"
#include "OffscreenIntf.h"
#include "TextureIntf.h"
#include "BitmapIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "tjsArray.h"
#include "LayerIntf.h"
#include "RectItf.h"
#include "OpenGLHeader.h"
#include "OpenGLScreen.h"
#include "WindowIntf.h"
#include "Application.h"
#include "DebugIntf.h"
#include "tvpgl.h"
#include "CharacterSet.h"
#include "GLShaderUtil.h"
#include "ShaderProgramIntf.h"
#include "VertexBinderIntf.h"
#include "VertexBufferIntf.h"

#include <memory>

tTVPCanvasState::tTVPCanvasState( class tTJSNI_Matrix32* mat, class tTJSNI_Rect* clip, bool enableClip ) {
	if( mat ) {
		memcpy( Matrix, mat->GetMatrixArray(), sizeof( float ) * 6 );
	} else {
		Matrix[0] = Matrix[3] = 1.0f;
		Matrix[1] = Matrix[2] = Matrix[4] = Matrix[5] = 0.0f;
	}
	if( clip ) {
		memcpy( ClipRect, clip->Get().array, sizeof( tjs_int ) * 4 );
	} else {
		memset( ClipRect, 0, sizeof( tjs_int ) * 4 );
	}
	Flag = enableClip ? FLAG_CLIP_RECT : 0;
}

//----------------------------------------------------------------------
const ttstr tTJSNI_Canvas::DefaultVertexShaderText(
TJS_W( "attribute vec2 a_pos;" )
TJS_W( "attribute vec2 a_texCoord;" )
TJS_W( "uniform mat4 a_modelMat4;" )
TJS_W( "uniform vec2 a_size;" )
TJS_W( "varying vec2 v_texCoord;" )
TJS_W( "void main()" )
TJS_W( "{" )
TJS_W( "  mat4 ortho = mat4(" )
TJS_W( "    vec4( 2.0 / a_size.x, 0.0, 0.0, 0.0 )," )
TJS_W( "    vec4( 0.0, -2.0 / a_size.y, 0.0, 0.0 )," )
TJS_W( "    vec4( 0.0, 0.0, -1.0, 0.0 )," )
TJS_W( "    vec4( -1.0, 1.0, 0.0, 1.0 ) );" )
TJS_W( "  gl_Position = ortho * a_modelMat4 * vec4( a_pos, 0.0, 1.0 );" )
TJS_W( "  v_texCoord = a_texCoord;" )
TJS_W( "}" )
	);
//----------------------------------------------------------------------
const ttstr tTJSNI_Canvas::DefaultFragmentShaderText(
TJS_W( "precision mediump float;" )
TJS_W( "varying vec2 v_texCoord;" )
TJS_W( "uniform sampler2D s_tex0;" )
TJS_W( "uniform float a_opacity;" )
TJS_W( "void main()" )
TJS_W( "{" )
TJS_W( "vec4 color = texture2D( s_tex0, v_texCoord );" )
TJS_W( "color.a *= a_opacity;" )
TJS_W( "gl_FragColor = color;" )
TJS_W( "}")
);
//----------------------------------------------------------------------
const ttstr tTJSNI_Canvas::DefaultFillVertexShaderText(
TJS_W( "attribute vec2 a_pos;" )
TJS_W( "attribute vec4 a_color;" )
TJS_W( "uniform mat4 a_modelMat4;" )
TJS_W( "uniform vec2 a_size;" )
TJS_W( "varying vec4 v_color;" )
TJS_W( "void main()" )
TJS_W( "{" )
TJS_W( "  mat4 ortho = mat4(" )
TJS_W( "    vec4( 2.0 / a_size.x, 0.0, 0.0, 0.0 )," )
TJS_W( "    vec4( 0.0, -2.0 / a_size.y, 0.0, 0.0 )," )
TJS_W( "    vec4( 0.0, 0.0, -1.0, 0.0 )," )
TJS_W( "    vec4( -1.0, 1.0, 0.0, 1.0 ) );" )
TJS_W( "  gl_Position = ortho * a_modelMat4 * vec4( a_pos, 0.0, 1.0 );" )
TJS_W( "  v_color = a_color;" )
TJS_W( "}" )
);
//----------------------------------------------------------------------
const ttstr tTJSNI_Canvas::DefaultFillFragmentShaderText(
	TJS_W( "precision mediump float;" )
	TJS_W( "varying vec4 v_color;" )
	TJS_W( "void main()" )
	TJS_W( "{" )
	TJS_W( "  gl_FragColor = v_color;" )
	TJS_W( "}" )
);
//----------------------------------------------------------------------
const float tTJSNI_Canvas::DefaultUVs[] = {
	0.0f,  0.0f,
	0.0f,  1.0f,
	1.0f,  0.0f,
	1.0f,  1.0f
};
//----------------------------------------------------------------------
tTJSNI_Canvas::tTJSNI_Canvas() : IsFirst(true), InDrawing(false), EnableClipRect(false), GLScreen(nullptr), ClearColor(0xff000000), BlendMode(tTVPBlendMode::bmAlpha),
PrevViewportWidth(0), PrevViewportHeight(0), CurrentScissorRect(-1,-1,-1,-1), 
RenderTargetInstance(nullptr), ClipRectInstance(nullptr), Matrix32Instance(nullptr), DefaultShaderInstance(nullptr), DefaultFillShaderInstance(nullptr) {
	TVPInitializeOpenGLPlatform();
}
//----------------------------------------------------------------------
tTJSNI_Canvas::~tTJSNI_Canvas() {
}
//----------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_Canvas::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	// get the window native instance
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	tTJSNI_Window *win = nullptr;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Window::ClassID, (iTJSNativeInstance**)&win)))
			TVPThrowExceptionMessage( TJS_W("Parameter require Window class instance.") );
#ifdef WIN32
		// GetNativeHandle() とか言う名前にするべきよね
		HWND hWnd = win->GetWindowHandle();
		if( GLScreen ) delete GLScreen;
		GLScreen = new tTVPOpenGLScreen( (void*)hWnd );
#elif defined( ANDROID )
		ANativeWindow* win = Application->getWindow();
		if( GLScreen ) delete GLScreen;
		GLScreen = new tTVPOpenGLScreen( (void*)win );
#endif
	}

	// set default matrix
	CreateDefaultMatrix();

	if( !GLScreen ) TVPThrowExceptionMessage( TJS_W("Cannot create low level graphics system(maybe low memory).") );
	if( !GLScreen->Initialize() ) TVPThrowExceptionMessage( TJS_W("Cannot initialize low level graphics system.") );

	// set default shader (after OpenGL init)
	CreateDefaultShader();

	TextureVertexBuffer.createStaticVertex( DefaultUVs, sizeof(DefaultUVs) );

	return TJS_S_OK;
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Canvas::Invalidate() {

	// release render target
	if( RenterTaretObject.Type() == tvtObject )
		RenterTaretObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, RenterTaretObject.AsObjectNoAddRef() );

	// release clip rect
	if( ClipRectObject.Type() == tvtObject )
		ClipRectObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, ClipRectObject.AsObjectNoAddRef() );

	// release matrix
	if( Matrix32Object.Type() == tvtObject )
		Matrix32Object.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, Matrix32Object.AsObjectNoAddRef() );

	// release shader
	SetDefaultShader( tTJSVariant() );
	if( DefaultShaderObject.Type() == tvtObject )
		DefaultShaderObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, DefaultShaderObject.AsObjectNoAddRef() );

	// release fill shader
	SetDefaultFillShader( tTJSVariant() );
	if( DefaultFillShaderObject.Type() == tvtObject )
		DefaultFillShaderObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, DefaultFillShaderObject.AsObjectNoAddRef() );
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Canvas::Destruct() {
	if( GLScreen ) {
		GLScreen->Destroy();
		delete GLScreen;
		GLScreen = nullptr;
	}
	tTJSNativeInstance::Destruct();
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::UpdateWindowSurface() {
#if defined( ANDROID )
	if( GLScreen ) {
		ANativeWindow* win = Application->getWindow();
		GLScreen->UpdateWindowSurface( (void*)win );
	}
#endif
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::ReleaseWindowSurface() {
#if defined( ANDROID )
	if( GLScreen ) {
		GLScreen->ReleaseSurface();
	}
#endif
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::CreateDefaultMatrix() {
	{	// set default matrix
		iTJSDispatch2 * cls = NULL;
		iTJSDispatch2 * newobj = NULL;
		try
		{
			cls = new tTJSNC_Matrix32();
			if( TJS_FAILED( cls->CreateNew( 0, NULL, NULL, &newobj, 0, NULL, cls ) ) )
				TVPThrowExceptionMessage( TVPInternalError, TJS_W( "tTJSNI_Matrix32::Construct" ) );
			SetMatrix32Object( tTJSVariant( newobj, newobj ) );
		} catch( ... ) {
			if( cls ) cls->Release();
			if( newobj ) newobj->Release();
			throw;
		}
		if( cls ) cls->Release();
		if( newobj ) newobj->Release();
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::CreateDefaultShader() {
	{	// set default shader
		iTJSDispatch2 * cls = NULL;
		iTJSDispatch2 * newobj = NULL;
		try
		{
			cls = new tTJSNC_ShaderProgram();
			tTJSVariant param[4] = { DefaultVertexShaderText, DefaultFragmentShaderText, 0, 0 };
			tTJSVariant *pparam[4] = { param, param + 1, param + 2, param + 3 };
			if( TJS_FAILED( cls->CreateNew( 0, NULL, NULL, &newobj, 4, pparam, cls ) ) )
				TVPThrowExceptionMessage( TVPInternalError, TJS_W( "tTJSNI_ShaderProgram::Construct" ) );
			EmbeddedDefaultShaderObject = tTJSVariant( newobj, newobj );
			SetDefaultShader( EmbeddedDefaultShaderObject );
			if( newobj ) newobj->Release();

			param[0] = DefaultFillVertexShaderText;
			param[1] = DefaultFillFragmentShaderText;
			if( TJS_FAILED( cls->CreateNew( 0, NULL, NULL, &newobj, 4, pparam, cls ) ) )
				TVPThrowExceptionMessage( TVPInternalError, TJS_W( "tTJSNI_ShaderProgram::Construct" ) );
			EmbeddedDefaultFillShaderObject = tTJSVariant( newobj, newobj );
			SetDefaultFillShader( EmbeddedDefaultFillShaderObject );
		} catch( ... ) {
			if( cls ) cls->Release();
			if( newobj ) newobj->Release();
			throw;
		}
		if( cls ) cls->Release();
		if( newobj ) newobj->Release();
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::BeginDrawing()
{
#ifdef WIN32
	if( IsFirst ) {
		IsFirst = false;
		glClear( GL_COLOR_BUFFER_BIT );
		if( GLScreen ) GLScreen->Swap();
		// swapするとサーフェイスサイズの変更が反映されるので、Windowsだと初回生成時はサイズが合っていないため
		// 最初に一度呼び出すことでサーフェイスサイズとクライアント領域のサイズを合わせる。
		// ダミー描画でonDraw処理してしまうと、一瞬モザイク状の画像が見えてしまったりするので、好ましくない。
	}
#endif
	tTVPARGB<tjs_uint32> c;
	c = ClearColor;
	glClearColor( c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	glViewport( 0, 0, sw, sh );
	PrevViewportWidth = sw;
	PrevViewportHeight = sh;

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );

	ApplyBlendMode();
	if( EnableClipRect ) {
		ApplyClipRect();
	} else {
		DisableClipRect();
	}
	InDrawing = true;

	if( RenderTargetInstance ) {
		RenderTargetInstance->BindFrameBuffer();
	}
	StateStack.clear();
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::EndDrawing()
{
	if( GLScreen ) GLScreen->Swap();
	StateStack.clear();
	InDrawing = false;
}
//----------------------------------------------------------------------
/**
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendEquation.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendEquationSeparate.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendFunc.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendFuncSeparate.xml
 */
void tTJSNI_Canvas::ApplyBlendMode() {
	if( BlendMode == tTVPBlendMode::bmDisable ) {
		glDisable( GL_BLEND );
	} else {
		glEnable( GL_BLEND );
		glBlendEquation( GL_FUNC_ADD );
		switch( BlendMode ) {
		case tTVPBlendMode::bmAlpha:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		case tTVPBlendMode::bmOpaque:
			glBlendFuncSeparate( GL_ONE, GL_ZERO, GL_ONE, GL_ZERO );
			break;
		case tTVPBlendMode::bmAdd:
			glBlendFunc( GL_ONE, GL_ONE );
			break;
		case tTVPBlendMode::bmAddWithAlpha:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
		default:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		}
	}
}
//----------------------------------------------------------------------
tjs_int tTJSNI_Canvas::GetCanvasWidth() const {
	if( RenderTargetInstance ) {
		return RenderTargetInstance->GetWidth();
	}
	return PrevViewportWidth;
}
//----------------------------------------------------------------------
tjs_int tTJSNI_Canvas::GetCanvasHeight() const {
	if( RenderTargetInstance ) {
		return RenderTargetInstance->GetHeight();
	}
	return PrevViewportHeight;
}
//----------------------------------------------------------------------
// method
void tTJSNI_Canvas::Capture( class tTJSNI_Bitmap* bmp, bool front ) {
	// Bitmap の内部表現が正順(上下反転されていない)ことを前提としているので注意
	tjs_int sw = GetCanvasWidth();
	tjs_int sh = GetCanvasHeight();
	bmp->SetSize( sw, sh, false );
	tTVPBaseBitmap* b = bmp->GetBitmap();
	tjs_uint32* dest = reinterpret_cast<tjs_uint32*>(b->GetScanLineForWrite(0));
	if( GLFrameBufferObject::readFrameBuffer( 0, 0, sw, sh, reinterpret_cast<tjs_uint8*>(dest), front ) ) {
		tjs_int pitch = b->GetPitchBytes();
		for( tjs_int y = 0; y < sh; y++ ) {
			TVPRedBlueSwap( dest, sw );
			dest = reinterpret_cast<tjs_uint32*>(reinterpret_cast<tjs_uint8*>(dest)+pitch);
		}
	}
	tTVPRect rect( 0, 0, sw, sh );
	b->UDFlip(rect);
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Capture( const class iTVPTextureInfoIntrface* texture, bool front ) {
	tjs_int sw = GetCanvasWidth();
	tjs_int sh = GetCanvasHeight();
	tjs_int tw = texture->GetWidth();
	tjs_int th = texture->GetHeight();
	if( tw < sw ) sw = tw;
	if( th < sh ) sh = th;
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
	glReadBuffer( front ? GL_FRONT : GL_BACK );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, sw, sh );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Clear( tjs_uint32 color ) {
	SetupEachDrawing();

	tTVPARGB<tjs_uint32> c;
	c = color;
	glClearColor( c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Fill( tjs_int width, tjs_int height, tjs_uint32 colors[4], tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	if( !shader ) shader = DefaultFillShaderInstance;

	const float w = (float)width;
	const float h = (float)height;
	const GLfloat vertices[] = {
		0.0f, 0.0f,	// 左上
		0.0f,    h,	// 左下
		w, 0.0f,	// 右上
		w,    h,	// 右下
	};
	GLfloat glcolors[16];
	for( tjs_int i = 0; i < 4; i++ ) {
		float a = ( ( colors[i] & 0xff000000 ) >> 24 ) / 255.0f;
		float r = ( ( colors[i] & 0x00ff0000 ) >> 16 ) / 255.0f;
		float g = ( ( colors[i] & 0x0000ff00 ) >> 8 ) / 255.0f;
		float b = ( ( colors[i] & 0x000000ff ) >> 0 ) / 255.0f;
		glcolors[i * 4 + 0] = r;
		glcolors[i * 4 + 1] = g;
		glcolors[i * 4 + 2] = b;
		glcolors[i * 4 + 3] = a;
	}

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();

	shader->SetupProgram();
	GLint posLoc = shader->FindLocation( std::string( "a_pos" ) );
	if( posLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_pos in shader." ) );
	glVertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	GLint colorLoc = shader->FindLocation( std::string( "a_color" ) );
	if( colorLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_texCoord in shader." ) );
	glVertexAttribPointer( colorLoc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( GLfloat ), glcolors );
	glEnableVertexAttribArray( posLoc );
	glEnableVertexAttribArray( colorLoc );

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_modelMat4 in shader." ) );
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, Matrix32Instance->GetMatrixArray16() );

	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_size in shader." ) );
	glUniform2f( vpLoc, (float)ssize.x, (float)ssize.y );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetupTextureDrawing( tTJSNI_ShaderProgram* shader, const iTVPTextureInfoIntrface* tex, tTJSNI_Matrix32* mat, const tTVPPoint& vpSize ) {
#if 1
	GLint posLoc = shader->FindLocation( std::string( "a_pos" ) );
	if( posLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_pos in shader.") );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	if( uvLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_texCoord in shader.") );
	GLuint vboid = (GLuint)tex->GetVBOHandle();
	if( vboid == 0 ) TVPThrowExceptionMessage( TJS_W("This method require VBO.") );
	glBindBuffer( GL_ARRAY_BUFFER, vboid );
	glVertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), 0 );
	TextureVertexBuffer.bindBuffer();
	glVertexAttribPointer( uvLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), 0 );
	TextureVertexBuffer.unbindBuffer();
	glEnableVertexAttribArray( posLoc );
	glEnableVertexAttribArray( uvLoc );

	GLint texLoc = shader->FindLocation( std::string( "s_tex0" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found s_tex0 in shader." ) );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)tex->GetNativeHandle() );
	glUniform1i( texLoc, 0 );

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_modelMat4 in shader.") );
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, mat->GetMatrixArray16() );

	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_size in shader.") );
	glUniform2f( vpLoc, (float)vpSize.x, (float)vpSize.y );
#else
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
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, mat->GetMatrixArray16() );

	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	glUniform2f( vpLoc, (float)vpSize.x, (float)vpSize.y );

	//glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
#endif
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTexture( const iTVPTextureInfoIntrface* texture, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	if( !shader ) shader = DefaultShaderInstance;

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgram();
	SetupTextureDrawing( shader, texture, Matrix32Instance, ssize );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTexture( const iTVPTextureInfoIntrface* texture0, const iTVPTextureInfoIntrface* texture1, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgram();
	SetupTextureDrawing( shader, texture0, Matrix32Instance, ssize );
	GLint texLoc = shader->FindLocation( std::string( "s_tex1" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found s_tex1 in shader.") );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture1->GetNativeHandle() );
	glUniform1i( texLoc, 1 );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTexture( const iTVPTextureInfoIntrface* texture0, const iTVPTextureInfoIntrface* texture1, const iTVPTextureInfoIntrface* texture2, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgram();
	SetupTextureDrawing( shader, texture0, Matrix32Instance, ssize );
	GLint texLoc = shader->FindLocation( std::string( "s_tex1" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found s_tex1 in shader.") );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture1->GetNativeHandle() );
	glUniform1i( texLoc, 1 );
	texLoc = shader->FindLocation( std::string( "s_tex2" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found s_tex2 in shader.") );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture2->GetNativeHandle() );
	glUniform1i( texLoc, 2 );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTextureAtlas( const tTJSNI_Rect* rect, const iTVPTextureInfoIntrface* texture, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	if( !shader ) shader = DefaultShaderInstance;

	shader->SetupProgram();

	GLint posLoc = shader->FindLocation( std::string( "a_pos" ) );
	if( posLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_pos in shader.") );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	if( uvLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_texCoord in shader.") );

	const float tw = (float)texture->GetWidth();
	const float th = (float)texture->GetHeight();
	tTVPRect r = rect->Get();
	if( r.top < 0 ) r.top = 0;
	if( r.left < 0 ) r.left = 0;
	if( r.right > (tjs_int)tw ) r.right = (tjs_int)tw;
	if( r.bottom > (tjs_int)th ) r.bottom = (tjs_int)th;

	const float width = (float)r.get_width();
	const float height = (float)r.get_height();
	const GLfloat vertices[] = {
		0.0f,  0.0f,	// 左上
		0.0f,  height,  // 左下
		width, 0.0f,	// 右上
		width, height,	// 右下
	};
	const GLfloat uvs[] = {
		r.left/ tw,  r.top/ th,
		r.left/ tw,  r.bottom/ th,
		r.right/ tw, r.top/ th,
		r.right/ tw, r.bottom/ th,
	};
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glVertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	glVertexAttribPointer( uvLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), uvs );
	glEnableVertexAttribArray( posLoc );
	glEnableVertexAttribArray( uvLoc );

	GLint texLoc = shader->FindLocation( std::string( "s_tex0" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found s_tex0 in shader." ) );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetNativeHandle() );
	glUniform1i( texLoc, 0 );

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_modelMat4 in shader.") );
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, Matrix32Instance->GetMatrixArray16() );

	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found a_size in shader.") );
	glUniform2f( vpLoc, (float)ssize.x, (float)ssize.y );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Draw9PatchTexture( class tTJSNI_Texture* tex, tjs_int width, tjs_int height, tTVPRect& margin, class tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	if( !shader ) shader = DefaultShaderInstance;

	GLint posLoc = shader->FindLocation( std::string( "a_pos" ) );
	if( posLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_pos in shader." ) );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	if( uvLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_texCoord in shader." ) );

	const float tw = (float)tex->GetWidth();
	const float th = (float)tex->GetHeight();
	tTVPRect scale = tex->GetScale9Patch();
	margin = tex->GetMargin9Patch();

	const float tx_left = 0.0f;
	const float tx_left_right = (scale.left-1) / tw;
	const float tx_center_left = scale.left / tw;
	const float tx_center_right = (scale.right-1) / tw;
	const float tx_right_left = scale.right / tw;
	const float tx_right = 1.0f;
	const float tx_top = 0.0f;
	const float tx_top_bottom = (scale.top-1) / th;
	const float tx_center_top = scale.top / th;
	const float tx_center_bottom = (scale.bottom-1) / th;
	const float tx_bottom_top = scale.bottom / th;
	const float tx_bottom = 1.0f;
	// 36頂点
	// 縦方向基準に並べる
	const GLfloat uvs[] = {
		tx_left, tx_top,
		tx_left, tx_top_bottom,
		tx_left, tx_center_top,
		tx_left, tx_center_bottom,
		tx_left, tx_bottom_top,
		tx_left, tx_bottom,

		tx_left_right, tx_top,
		tx_left_right, tx_top_bottom,
		tx_left_right, tx_center_top,
		tx_left_right, tx_center_bottom,
		tx_left_right, tx_bottom_top,
		tx_left_right, tx_bottom,

		tx_center_left, tx_top,
		tx_center_left, tx_top_bottom,
		tx_center_left, tx_center_top,
		tx_center_left, tx_center_bottom,
		tx_center_left, tx_bottom_top,
		tx_center_left, tx_bottom,

		tx_center_right, tx_top,
		tx_center_right, tx_top_bottom,
		tx_center_right, tx_center_top,
		tx_center_right, tx_center_bottom,
		tx_center_right, tx_bottom_top,
		tx_center_right, tx_bottom,

		tx_right_left, tx_top,
		tx_right_left, tx_top_bottom,
		tx_right_left, tx_center_top,
		tx_right_left, tx_center_bottom,
		tx_right_left, tx_bottom_top,
		tx_right_left, tx_bottom,

		tx_right, tx_top,
		tx_right, tx_top_bottom,
		tx_right, tx_center_top,
		tx_right, tx_center_bottom,
		tx_right, tx_bottom_top,
		tx_right, tx_bottom,
	};
	const float dst_left = 0.0f;
	const float dst_center_left = static_cast<const float>(scale.left);
	const float dst_center_right = width - (tw - scale.right);
	const float dst_right = static_cast<const float>(width);
	const float dst_top = 0.0f;
	const float dst_center_top = static_cast<const float>(scale.top);
	const float dst_center_bottom = height - ( th - scale.bottom );
	const float dst_bottom = static_cast<const float>(height);
	const GLfloat vertices[] = {
		dst_left, dst_top,
		dst_left, dst_center_top,
		dst_left, dst_center_top,
		dst_left, dst_center_bottom,
		dst_left, dst_center_bottom,
		dst_left, dst_bottom,
		dst_center_left, dst_top,
		dst_center_left, dst_center_top,
		dst_center_left, dst_center_top,
		dst_center_left, dst_center_bottom,
		dst_center_left, dst_center_bottom,
		dst_center_left, dst_bottom,
		dst_center_left, dst_top,
		dst_center_left, dst_center_top,
		dst_center_left, dst_center_top,
		dst_center_left, dst_center_bottom,
		dst_center_left, dst_center_bottom,
		dst_center_left, dst_bottom,
		dst_center_right, dst_top,
		dst_center_right, dst_center_top,
		dst_center_right, dst_center_top,
		dst_center_right, dst_center_bottom,
		dst_center_right, dst_center_bottom,
		dst_center_right, dst_bottom,
		dst_center_right, dst_top,
		dst_center_right, dst_center_top,
		dst_center_right, dst_center_top,
		dst_center_right, dst_center_bottom,
		dst_center_right, dst_center_bottom,
		dst_center_right, dst_bottom,
		dst_right, dst_top,
		dst_right, dst_center_top,
		dst_right, dst_center_top,
		dst_right, dst_center_bottom,
		dst_right, dst_center_bottom,
		dst_right, dst_bottom,
	};
	const GLubyte indexes[] = {
		0, 1, 6,
		1, 7, 6,
		2, 3, 8,
		3, 9, 8,
		4, 5, 10,
		5, 11, 10,

		0 + 12, 1 + 12, 6 + 12,
		1 + 12, 7 + 12, 6 + 12,
		2 + 12, 3 + 12, 8 + 12,
		3 + 12, 9 + 12, 8 + 12,
		4 + 12, 5 + 12, 10 + 12,
		5 + 12, 11 + 12, 10 + 12,

		0 + 24, 1 + 24, 6 + 24,
		1 + 24, 7 + 24, 6 + 24,
		2 + 24, 3 + 24, 8 + 24,
		3 + 24, 9 + 24, 8 + 24,
		4 + 24, 5 + 24, 10 + 24,
		5 + 24, 11 + 24, 10 + 24,
	};

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glVertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	glVertexAttribPointer( uvLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), uvs );
	glEnableVertexAttribArray( posLoc );
	glEnableVertexAttribArray( uvLoc );

	GLint texLoc = shader->FindLocation( std::string( "s_tex0" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found s_tex0 in shader." ) );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)tex->GetNativeHandle() );
	glUniform1i( texLoc, 0 );

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_modelMat4 in shader." ) );
	glUniformMatrix4fv( matLoc, 1, GL_FALSE, Matrix32Instance->GetMatrixArray16() );

	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc < 0 ) TVPThrowExceptionMessage( TJS_W( "Not found a_size in shader." ) );
	glUniform2f( vpLoc, (float)ssize.x, (float)ssize.y );

	const GLsizei count = sizeof( indexes ) / sizeof( GLubyte );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_BYTE, indexes );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color ) { /* TODO: 次期実装 */}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawMesh( tTJSNI_ShaderProgram* shader, tjs_int primitiveType, tjs_int offset, tjs_int count ) {
	if( count <= 0 ) return;

	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgramFull();

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc >= 0 ) {
		glUniformMatrix4fv( matLoc, 1, GL_FALSE, Matrix32Instance->GetMatrixArray16() );
	}
	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc >= 0 ) {
		glUniform2f( vpLoc, (float)ssize.x, (float)ssize.y );
	}
	glDrawArrays( primitiveType, offset, count );

	shader->UnbindParam();
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawMesh( tTJSNI_ShaderProgram* shader, tjs_int primitiveType, const tTJSNI_VertexBinder* index, tjs_int count ) {
	if( count <= 0 ) return;
	GLenum type = (GLenum)index->GetVertexBuffer()->GetDataType();
	if( type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT ) {
		TVPThrowExceptionMessage( TJS_W("Index type is invalid.") );
		return;
	}

	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgramFull();

	GLint matLoc = shader->FindLocation( std::string( "a_modelMat4" ) );
	if( matLoc >= 0 ) {
		glUniformMatrix4fv( matLoc, 1, GL_FALSE, Matrix32Instance->GetMatrixArray16() );
	}
	GLint vpLoc = shader->FindLocation( std::string( "a_size" ) );
	if( vpLoc >= 0 ) {
		glUniform2f( vpLoc, (float)ssize.x, (float)ssize.y );
	}
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index->GetVertexBuffer()->GetNativeHandle() );
	glDrawElements( primitiveType, count, type, (const GLvoid *)index->GetOffset() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	shader->UnbindParam();
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Save() {
	StateStack.push_back(std::unique_ptr<tTVPCanvasState>( new tTVPCanvasState( Matrix32Instance, ClipRectInstance, EnableClipRect ) ) );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::Restore() {
	if( StateStack.empty() == false ) {
		const std::unique_ptr<tTVPCanvasState>& state = StateStack.back();
		if( Matrix32Instance ) {
			Matrix32Instance->Set( state->Matrix );
		}
		if( ClipRectInstance ) {
			ClipRectInstance->Set( state->ClipRect );
		}
		EnableClipRect = ( state->Flag & tTVPCanvasState::FLAG_CLIP_RECT ) != 0;

		StateStack.pop_back();
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::ApplyClipRect() {
	if( ClipRectInstance && GLScreen ) {
		CurrentScissorRect = ClipRectInstance->Get();
		GLScreen->SetScissorRect( CurrentScissorRect );
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DisableClipRect() {
	if( GLScreen ) GLScreen->DisableScissorRect();
}
//----------------------------------------------------------------------
// 各描画前に呼び出して、状態の変化に応じた設定を行う。
void tTJSNI_Canvas::SetupEachDrawing() {
	if( EnableClipRect ) {
		if( CurrentScissorRect != ClipRectInstance->Get() ) {
			ApplyClipRect();
		}
	}
}
//----------------------------------------------------------------------
// prop
void tTJSNI_Canvas::SetRenterTargetObject( const tTJSVariant & val ) {
	// invalidate existing render terget
	// if( RenterTaretObject.Type() == tvtObject )
	// 	RenterTaretObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, RenterTaretObject.AsObjectNoAddRef() );

	// assign new rect
	RenterTaretObject = val;
	RenderTargetInstance = nullptr;

	// extract interface
	if( RenterTaretObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = RenterTaretObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&RenderTargetInstance ) ) ) {
				RenderTargetInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive rect instance." ) );
			}
		}
	}

	// 描画途中であれば、その場でターゲットに指定する
	if( InDrawing ) {
		glFlush();
		if( RenderTargetInstance ) {
			RenderTargetInstance->BindFrameBuffer();
		} else {
			if( GLScreen ) glBindFramebuffer( GL_FRAMEBUFFER, GLScreen->GetDefaultFrameBufferId() );
			glViewport( 0, 0, PrevViewportWidth, PrevViewportHeight );
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetClipRectObject( const tTJSVariant & val ) {
	// invalidate existing clip rect
	// if( ClipRectObject.Type() == tvtObject )
	// 	ClipRectObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, ClipRectObject.AsObjectNoAddRef() );

	// assign new rect
	ClipRectObject = val;
	ClipRectInstance = nullptr;

	// extract interface
	if( ClipRectObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = ClipRectObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Rect::ClassID, (iTJSNativeInstance**)&ClipRectInstance ) ) ) {
				ClipRectInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive rect instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetMatrix32Object( const tTJSVariant & val ) {
	// invalidate existing matrix
	// if( Matrix32Object.Type() == tvtObject )
	// 	Matrix32Object.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, Matrix32Object.AsObjectNoAddRef() );

	// assign new matrix
	Matrix32Object = val;
	Matrix32Instance = nullptr;

	// extract interface
	if( Matrix32Object.Type() == tvtObject ) {
		tTJSVariantClosure clo = Matrix32Object.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Matrix32::ClassID, (iTJSNativeInstance**)&Matrix32Instance ) ) ) {
				Matrix32Instance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive matrix instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetDefaultShader( const tTJSVariant & val ) {
	// invalidate existing shader
	// if( DefaultShaderObject.Type() == tvtObject )
	// 	DefaultShaderObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, DefaultShaderObject.AsObjectNoAddRef() );

	// assign new shader
	DefaultShaderObject = val;
	DefaultShaderInstance = nullptr;

	if( val.Type() == tvtVoid ) {
		DefaultShaderObject = EmbeddedDefaultShaderObject;
	}

	// extract interface
	if( DefaultShaderObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = DefaultShaderObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&DefaultShaderInstance ) ) ) {
				DefaultShaderInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive shader instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetDefaultFillShader( const tTJSVariant & val ) {
	// invalidate existing shader
	// if( DefaultFillShaderObject.Type() == tvtObject )
	// 	DefaultFillShaderObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, DefaultFillShaderObject.AsObjectNoAddRef() );

	// assign new shader
	DefaultFillShaderObject = val;
	DefaultFillShaderInstance = nullptr;

	if( val.Type() == tvtVoid ) {
		DefaultFillShaderObject = EmbeddedDefaultFillShaderObject;
	}

	// extract interface
	if( DefaultFillShaderObject.Type() == tvtObject ) {
		tTJSVariantClosure clo = DefaultFillShaderObject.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&DefaultFillShaderInstance ) ) ) {
				DefaultFillShaderInstance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive shader instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetBlendMode( tTVPBlendMode bm ) {
	BlendMode = bm;
	if( InDrawing ) {
		ApplyBlendMode();
	}
}
//----------------------------------------------------------------------
tjs_uint tTJSNI_Canvas::GetWidth() const {
	if( GLScreen ) return GLScreen->GetSurfaceWidth();
	return 0;
}
//----------------------------------------------------------------------
tjs_uint tTJSNI_Canvas::GetHeight() const {
	if( GLScreen ) return GLScreen->GetSurfaceHeight();
	return 0;
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetEnableClipRect( bool b ) {
	if( InDrawing ) {
		if( EnableClipRect != b ) {
			EnableClipRect = b;
			if( b ) {
				ApplyClipRect();
			} else {
				DisableClipRect();
			}
		}
	} else {
		EnableClipRect = b;
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetWaitVSync( bool b ) {
	if( GLScreen ) GLScreen->SetWaitVSync( b );
}
//----------------------------------------------------------------------
/**
 * テクスチャとして使用できるTJS2クラスインスタンス群から、テクスチャインターフェイスを取得する。
 * テクスチャとして使用できるクラスを問わず、テクスチャ描画するための関数
 */
const iTVPTextureInfoIntrface* TVPGetTextureInfo( const tTJSVariant *param ) {
	const tTJSNI_Texture* texture = (const tTJSNI_Texture*)TJSGetNativeInstance( tTJSNC_Texture::ClassID, param );
	if( texture ) return texture;

	const tTJSNI_Offscreen* offscreen = (const tTJSNI_Offscreen*)TJSGetNativeInstance( tTJSNC_Offscreen::ClassID, param );
	if( offscreen ) return offscreen;

	return nullptr;
}
//----------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTJSNC_Canvas : TJS Canvas class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Canvas::ClassID = -1;
tTJSNC_Canvas::tTJSNC_Canvas() : tTJSNativeClass(TJS_W("Canvas"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Canvas) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Canvas, /*TJS class name*/Canvas)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Canvas)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/clear)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	// パラメータ未指定時はproperty::clearColorが使われる
	tjs_uint32 color;
	if( numparams > 0 ) {
		color = static_cast<tjs_uint32>((tjs_int64)*param[0]);
	} else {
		color = _this->GetClearColor();
	}
	_this->Clear( color );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/clear)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/capture)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	bool front = true;
	if( numparams > 1 ) front = ( (tjs_int)param[1] ) ? true : false;

	const iTVPTextureInfoIntrface* texture = TVPGetTextureInfo( param[0] );
	if( texture ) {
		_this->Capture( texture, front );
	} else {
		tTJSNI_Bitmap* bmp = (tTJSNI_Bitmap*)TJSGetNativeInstance( tTJSNC_Bitmap::ClassID, param[0] );
		if( !bmp ) return TJS_E_INVALIDPARAM;
		_this->Capture( bmp, front );
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/capture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/fill)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tjs_int width = *param[0];
	tjs_int height = *param[1];

	tjs_uint32 color = 0xffffffff;

	tTJSNI_ShaderProgram* shader = nullptr;
	if( numparams >= 4 ) {
		shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[3] );
	}

	if( numparams >= 3 ) {
		tTJSVariantType vt = param[2]->Type();
		if( vt == tvtInteger ) {
			color = (tjs_uint32)(tjs_int64)*param[2];
		} else if( vt == tvtObject ) {
			tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
			tjs_int count = TJSGetArrayElementCount( clo.Object );
			tjs_uint32 colors[4];
			if( count >= 4 ) {
				tTJSVariant tmp;
				for( tjs_int i = 0; i < 4; i++ ) {
					if( TJS_FAILED( clo.Object->PropGetByNum( TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis ) ) )
						TVPThrowExceptionMessage( TJS_W( "Insufficient number of arrays." ) );
					colors[i] = (tjs_uint32)(tjs_int64)tmp;
				}
				_this->Fill( width, height, colors, shader );
				return TJS_S_OK;
			}
		}
	}
	tjs_uint32 colors[4] = { color,color,color,color };
	_this->Fill( width, height, colors, shader );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/fill)
//----------------------------------------------------------------------
#if 0
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawMesh2D)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNI_Texture::ClassID, (iTJSNativeInstance**)&texture)))
			return TJS_E_INVALIDPARAM;
	}
	if(!texture) TVPThrowExceptionMessage(TJS_W("Parameter require Texture class instance."));

	// Mesh にしてしまうと3Dやる時に被るよな……
	// TODO もうちょっと名前考える
	tTJSNI_Mesh* mesh = nullptr;
	tTJSVariantClosure clo = param[1]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNI_Mesh::ClassID, (iTJSNativeInstance**)&mesh)))
			return TJS_E_INVALIDPARAM;
	}
	if(!mesh) TVPThrowExceptionMessage(TJS_W("Parameter require Mesh class instance."));

	_this->DrawMesh( texture, mesh );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawMesh2D)
#endif
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawTexture )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	if( numparams == 1 ) {
		const iTVPTextureInfoIntrface* texture = TVPGetTextureInfo( param[0] );
		if( !texture ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture );
	} else if( numparams == 2 ) {
		const iTVPTextureInfoIntrface* texture = TVPGetTextureInfo( param[0] );
		if( !texture ) return TJS_E_INVALIDPARAM;
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[1] );
		if( !shader ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture, shader );
	} else if( numparams == 3 ) {
		const iTVPTextureInfoIntrface* texture0 = TVPGetTextureInfo( param[0] );
		if( !texture0 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture1 = TVPGetTextureInfo( param[1] );
		if( !texture1 ) return TJS_E_INVALIDPARAM;
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[2] );
		if( !shader ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture0, texture1, shader );
	} else if( numparams == 4 ) {
		const iTVPTextureInfoIntrface* texture0 = TVPGetTextureInfo( param[0] );
		if( !texture0 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture1 = TVPGetTextureInfo( param[1] );
		if( !texture1 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture2 = TVPGetTextureInfo( param[2] );
		if( !texture2 ) return TJS_E_INVALIDPARAM;
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[3] );
		if( !shader ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture0, texture1, texture2, shader );
	} else {
		return TJS_E_BADPARAMCOUNT;
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawTexture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawTextureAtlas )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	const tTJSNI_Rect* rect = (tTJSNI_Rect*)TJSGetNativeInstance( tTJSNC_Rect::ClassID, param[0] );
	if( !rect ) return TJS_E_INVALIDPARAM;
	const iTVPTextureInfoIntrface* texture = TVPGetTextureInfo( param[1] );
	if( !texture ) return TJS_E_INVALIDPARAM;
	tTJSNI_ShaderProgram* shader = nullptr;
	if( numparams >= 3 ) {
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[2] );
		if( !shader ) return TJS_E_INVALIDPARAM;
	}
	_this->DrawTextureAtlas( rect, texture, shader );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawTextureAtlas)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawText)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 5 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Font* font = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Font::ClassID, (iTJSNativeInstance**)&font)))
			return TJS_E_INVALIDPARAM;
	}
	if(!font) TVPThrowExceptionMessage(TJS_W("Parameter require Font class instance."));

	tjs_int x = *param[1];
	tjs_int y = *param[2];
	ttstr text = *param[3];
	tjs_uint32 color = (tjs_uint32)(tjs_int64)*param[4];
	_this->DrawText( font, x, y, text, color );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawText)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawMesh)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[0] );
	if( !shader ) return TJS_E_INVALIDPARAM;

	if( param[1]->Type() == tvtObject ) {
		if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

		tTJSNI_VertexBinder* index = (tTJSNI_VertexBinder*)TJSGetNativeInstance( tTJSNC_VertexBinder::ClassID, param[1] );
		if( !index ) return TJS_E_INVALIDPARAM;

		tjs_int count = *param[2];
		tjs_int type = (tjs_int)GL_TRIANGLES;
		if( numparams >= 4 ) type = *param[3];
		_this->DrawMesh( shader, type, index, count );
	} else {
		tjs_int count = *param[1];
		tjs_int type = (tjs_int)GL_TRIANGLES;
		if( numparams >= 3 ) type = *param[2];
		tjs_int offset = 0;
		if( numparams >= 4 ) offset = *param[3];
		_this->DrawMesh( shader, type, offset, count );
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawMesh)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/draw9Patch )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = (tTJSNI_Texture*)TJSGetNativeInstance( tTJSNC_Texture::ClassID, param[0] );
	if( !texture ) return TJS_E_INVALIDPARAM;

	tTJSNI_ShaderProgram* shader = nullptr;
	if( numparams >= 4 ) {
		shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[3] );
		if( !shader ) return TJS_E_INVALIDPARAM;
	}
	tjs_int width = *param[1];
	tjs_int height = *param[2];
	tTVPRect margin;
	_this->Draw9PatchTexture( texture, width, height, margin, shader );
	if( result ) {
		iTJSDispatch2 *ret = TVPCreateRectObject( margin.left, margin.top, margin.right, margin.bottom );
		*result = tTJSVariant( ret, ret );
		ret->Release();
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/draw9Patch )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/flush )
{
	// TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	// 実質staticメソッド
	glFlush();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/flush )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/save )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	_this->Save();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/save )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/restore )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	_this->Restore();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/restore )
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(clearColor)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int64)_this->GetClearColor();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetClearColor( static_cast<tjs_uint32>((tjs_int64)*param) );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(clearColor)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL( renderTarget )
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = _this->GetRenderTargetObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		if( param->Type() == tvtObject  ) {
			_this->SetRenterTargetObject( *param );
		} else {
			_this->SetRenterTargetObject( tTJSVariant() );
		}
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL( renderTarget )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(clipRect)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = _this->GetClipRectObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetClipRectObject( *param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(clipRect)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(blendMode)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int)_this->GetBlendMode();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetBlendMode( (tTVPBlendMode)(tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(blendMode)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(matrix)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = _this->GetMatrix32Object();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	_this->SetMatrix32Object( *param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(matrix)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL( defaultShader )
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
		*result = _this->GetDefaultShader();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
		_this->SetDefaultShader( *param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL( defaultShader )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL( defaultFillShader )
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	*result = _this->GetDefaultFillShader();
	return TJS_S_OK;
	}
		TJS_END_NATIVE_PROP_GETTER

		TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	_this->SetDefaultFillShader( *param );
	return TJS_S_OK;
	}
		TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL( defaultFillShader )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int64)_this->GetWidth();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(width)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(height)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int64)_this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL( enableClipRect )
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
		*result = _this->GetEnableClipRect() ? (tjs_int)1 : (tjs_int)0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
		_this->SetEnableClipRect( ((tjs_int)*param) == 0 ? false : true );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL( enableClipRect )
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Canvas()
{
	tTJSNativeClass *cls = new tTJSNC_Canvas();
	return cls;
}
//---------------------------------------------------------------------------
