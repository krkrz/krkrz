
#include "tjsCommHead.h"

#include "CanvasIntf.h"
#include "Matrix44Intf.h"
#include "Mesh2DIntf.h"
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
#include "Matrix44Intf.h"

#include <memory>

//----------------------------------------------------------------------
ttstr tTJSNI_Canvas::DefaultVertexShaderText(
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
ttstr tTJSNI_Canvas::DefaultFragmentShaderText(
TJS_W( "precision mediump float;" )
TJS_W( "varying vec2 v_texCoord;" )
TJS_W( "uniform sampler2D s_tex0;" )
TJS_W( "void main()" )
TJS_W( "{" )
TJS_W( "  gl_FragColor = texture2D( s_tex0, v_texCoord );" )
TJS_W( "}")
);
//----------------------------------------------------------------------
const float tTJSNI_Canvas::DefaultUVs[] = {
	0.0f,  0.0f,
	0.0f,  1.0f,
	1.0f,  0.0f,
	1.0f,  1.0f
};
//----------------------------------------------------------------------
tTJSNI_Canvas::tTJSNI_Canvas() : IsFirst(true), InDrawing(false), EnableClipRect(false), GLScreen(nullptr), ClearColor(0xff00ff00), BlendMode(tTVPBlendMode::bmAlpha),
PrevViewportWidth(0), PrevViewportHeight(0), CurrentScissorRect(-1,-1,-1,-1), 
RenderTargetInstance(nullptr), ClipRectInstance(nullptr), Matrix44Instance(nullptr) {
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

	if( !GLDrawer.InitializeShader() ) TVPThrowExceptionMessage( TJS_W("Cannot initialize shader.") );

	TextureVertexBuffer.createStaticVertex( DefaultUVs, sizeof(DefaultUVs) );

	return TJS_S_OK;
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Canvas::Invalidate() {

	// release render target
	SetRenterTargetObject( tTJSVariant() );

	// release clip rect
	SetClipRectObject( tTJSVariant() );

	// release matrix
	SetMatrix44Object( tTJSVariant() );

	// release shader
	SetDefaultShader( tTJSVariant() );
}
//----------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Canvas::Destruct() {
	GLDrawer.DestroyShader();
	if( GLScreen ) {
		GLScreen->Destroy();
		delete GLScreen;
		GLScreen = nullptr;
	}
	tTJSNativeInstance::Destruct();
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::CreateDefaultMatrix() {
	{	// set default matrix
		iTJSDispatch2 * cls = NULL;
		iTJSDispatch2 * newobj = NULL;
		try
		{
			cls = new tTJSNC_Matrix44();
			if( TJS_FAILED( cls->CreateNew( 0, NULL, NULL, &newobj, 0, NULL, cls ) ) )
				TVPThrowExceptionMessage( TVPInternalError, TJS_W( "tTJSNI_Matrix44::Construct" ) );
			SetMatrix44Object( tTJSVariant( newobj, newobj ) );
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
			SetDefaultShader( tTJSVariant( newobj, newobj ) );
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
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::EndDrawing()
{
	if( GLScreen ) GLScreen->Swap();
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
void tTJSNI_Canvas::Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 colors[4] ) {
	SetupEachDrawing();

	tjs_int sw = GetCanvasWidth();
	tjs_int sh = GetCanvasHeight();
	GLDrawer.DrawColoredPolygon( colors, left, top, width, height, sw, sh );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetupTextureDrawing( tTJSNI_ShaderProgram* shader, const iTVPTextureInfoIntrface* tex, tTJSNI_Matrix44* mat, const tTVPPoint& vpSize ) {
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
	glUniformMatrix4fv( matLoc, 1, GL_TRUE, mat->GetMatrixArray() );

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
	glUniformMatrix4fv( matLoc, 1, GL_TRUE, mat->GetMatrixArray() );

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
	//GLDrawer.DrawTexture( shader, texture, Matrix44Instance, ssize );
	SetupTextureDrawing( shader, texture, Matrix44Instance, ssize );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTexture( const iTVPTextureInfoIntrface* texture0, const iTVPTextureInfoIntrface* texture1, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgram();
	SetupTextureDrawing( shader, texture0, Matrix44Instance, ssize );
	GLint texLoc = shader->FindLocation( std::string( "s_tex1" ) );
	if( texLoc < 0 ) TVPThrowExceptionMessage( TJS_W("Not found s_tex1 in shader.") );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture1->GetNativeHandle() );
	glUniform1i( texLoc, 1 );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawTexture( const iTVPTextureInfoIntrface* texture0, const iTVPTextureInfoIntrface* texture1, const iTVPTextureInfoIntrface* texture2, tTJSNI_ShaderProgram* shader ) {
	SetupEachDrawing();

	tTVPPoint ssize;
	ssize.x = GetCanvasWidth();
	ssize.y = GetCanvasHeight();
	shader->SetupProgram();
	SetupTextureDrawing( shader, texture0, Matrix44Instance, ssize );
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
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color ) { /* TODO: 次期実装 */}
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
	if( RenterTaretObject.Type() == tvtObject )
		RenterTaretObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, RenterTaretObject.AsObjectNoAddRef() );

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
	if( ClipRectObject.Type() == tvtObject )
		ClipRectObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, ClipRectObject.AsObjectNoAddRef() );

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
void tTJSNI_Canvas::SetMatrix44Object( const tTJSVariant & val ) {
	// invalidate existing matrix
	if( Matrix44Object.Type() == tvtObject )
		Matrix44Object.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, Matrix44Object.AsObjectNoAddRef() );

	// assign new matrix
	Matrix44Object = val;
	Matrix44Instance = nullptr;

	// extract interface
	if( Matrix44Object.Type() == tvtObject ) {
		tTJSVariantClosure clo = Matrix44Object.AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Matrix44::ClassID, (iTJSNativeInstance**)&Matrix44Instance ) ) ) {
				Matrix44Instance = nullptr;
				TVPThrowExceptionMessage( TJS_W( "Cannot retrive matrix instance." ) );
			}
		}
	}
}
//----------------------------------------------------------------------
void tTJSNI_Canvas::SetDefaultShader( const tTJSVariant & val ) {
	// invalidate existing matrix
	if( DefaultShaderObject.Type() == tvtObject )
		DefaultShaderObject.AsObjectClosureNoAddRef().Invalidate( 0, NULL, NULL, DefaultShaderObject.AsObjectNoAddRef() );

	// assign new matrix
	DefaultShaderObject = val;
	DefaultShaderInstance = nullptr;

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
/**
 * テクスチャとして使用できるTJS2クラスインスタンス群から、テクスチャインターフェイスを取得する。
 * テクスチャとして使用できるクラスを問わず、テクスチャ描画するための関数
 */
static const iTVPTextureInfoIntrface* GetTextureInfo( tTJSVariant *param ) {
	tTJSNI_Texture* texture = (tTJSNI_Texture*)TJSGetNativeInstance( tTJSNC_Texture::ClassID, param );
	if( texture ) return texture;


	tTJSNI_Offscreen* offscreen = (tTJSNI_Offscreen*)TJSGetNativeInstance( tTJSNC_Offscreen::ClassID, param );
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

	const iTVPTextureInfoIntrface* texture = GetTextureInfo( param[0] );
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
	if( numparams < 5 ) return TJS_E_BADPARAMCOUNT;

	tjs_int left = *param[0];
	tjs_int top = *param[1];
	tjs_int width = *param[2];
	tjs_int height = *param[3];

	tTJSVariantType vt = param[4]->Type();
	tjs_uint32 color = 0xffffffff;
	if( vt == tvtInteger ) {
		color = (tjs_uint32)(tjs_int64)*param[4];
	} else if( vt == tvtObject ) {
		tTJSVariantClosure clo = param[4]->AsObjectClosureNoAddRef();
		tjs_int count = TJSGetArrayElementCount(clo.Object);
		tjs_uint32 colors[4];
		if( count >= 4 ) {
			tTJSVariant tmp;
			for( tjs_int i = 0; i < 4; i++ ) {
				if(TJS_FAILED(clo.Object->PropGetByNum(TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis)))
					TVPThrowExceptionMessage( TJS_W("Insufficient number of arrays.") );
				colors[i] = (tjs_uint32)(tjs_int64)tmp;
			}
			_this->Fill( left, top, width, height, colors );
			return TJS_S_OK;
		}
	}
	tjs_uint32 colors[4] = { color,color,color,color };
	_this->Fill( left, top, width, height, colors );

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
		const iTVPTextureInfoIntrface* texture = GetTextureInfo( param[0] );
		if( !texture ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture );
	} else if( numparams == 2 ) {
		const iTVPTextureInfoIntrface* texture = GetTextureInfo( param[0] );
		if( !texture ) return TJS_E_INVALIDPARAM;
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[1] );
		if( !shader ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture, shader );
	} else if( numparams == 3 ) {
		const iTVPTextureInfoIntrface* texture0 = GetTextureInfo( param[0] );
		if( !texture0 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture1 = GetTextureInfo( param[1] );
		if( !texture1 ) return TJS_E_INVALIDPARAM;
		tTJSNI_ShaderProgram* shader = (tTJSNI_ShaderProgram*)TJSGetNativeInstance( tTJSNC_ShaderProgram::ClassID, param[2] );
		if( !shader ) return TJS_E_INVALIDPARAM;
		_this->DrawTexture( texture0, texture1, shader );
	} else if( numparams == 4 ) {
		const iTVPTextureInfoIntrface* texture0 = GetTextureInfo( param[0] );
		if( !texture0 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture1 = GetTextureInfo( param[1] );
		if( !texture1 ) return TJS_E_INVALIDPARAM;
		const iTVPTextureInfoIntrface* texture2 = GetTextureInfo( param[2] );
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
		} else if( param->Type() != tvtVoid ) {
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
		*result = _this->GetMatrix44Object();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	_this->SetMatrix44Object( *param );
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
