
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

#include <memory>

tTJSNI_Canvas::tTJSNI_Canvas() : IsFirst(true), InDrawing(false), GLScreen(nullptr), ClearColor(0xff00ff00), BlendMode(tTVPBlendMode::bmAlpha),
StretchType(tTVPStretchType::stLinear), PrevViewportWidth(0), PrevViewportHeight(0),
RenderTargetInstance(nullptr), ClipRectInstance(nullptr) {
	TVPInitializeOpenGLPlatform();
}
tTJSNI_Canvas::~tTJSNI_Canvas() {
}
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
	if( !GLScreen ) TVPThrowExceptionMessage( TJS_W("Cannot create low level graphics system(maybe low memory).") );
	if( !GLScreen->Initialize() ) TVPThrowExceptionMessage( TJS_W("Cannot initialize low level graphics system.") );

	if( !GLDrawer.InitializeShader() ) TVPThrowExceptionMessage( TJS_W("Cannot initialize shader.") );

	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Canvas::Invalidate() {

	// release render target
	SetRenterTargetObject( tTJSVariant() );

	// release clip rect
	SetClipRectObject( tTJSVariant() );
}
void TJS_INTF_METHOD tTJSNI_Canvas::Destruct() {
	GLDrawer.DestroyShader();
	if( GLScreen ) {
		GLScreen->Destroy();
		delete GLScreen;
		GLScreen = nullptr;
	}
	tTJSNativeInstance::Destruct();
}
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
	glClear( GL_COLOR_BUFFER_BIT );

	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	glViewport( 0, 0, sw, sh );
	PrevViewportWidth = sw;
	PrevViewportHeight = sh;

	glEnable( GL_BLEND );
	ApplyBlendMode();
	InDrawing = true;

	if( RenderTargetInstance ) {
		RenderTargetInstance->BindFrameBuffer();
	}
}
void tTJSNI_Canvas::EndDrawing()
{
	if( GLScreen ) GLScreen->Swap();
	InDrawing = false;
}
/**
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendEquation.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendEquationSeparate.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendFunc.xml
 * https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glBlendFuncSeparate.xml
 */
void tTJSNI_Canvas::ApplyBlendMode() {
	glBlendEquation( GL_FUNC_ADD );
	if( BlendMode == tTVPBlendMode::bmAlpha ) {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	} else if( BlendMode == tTVPBlendMode::bmOpaque ) {
		glBlendFuncSeparate( GL_ONE, GL_ZERO, GL_ONE, GL_ZERO );
	} else if( BlendMode == tTVPBlendMode::bmAdd ) {
		glBlendFunc( GL_ONE, GL_ONE );
	} else if( BlendMode == tTVPBlendMode::bmAddWithAlpha ) {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	/* } else if( BlendMode == tTVPBlendMode::bmAdditive ) {
		glBlendEquation( GL_FUNC_ADD );
		glBlendFuncSeparate( GL_SRC_COLOR, GL_SRC_ALPHA, GL_DST_COLOR, GL_DST_ALPHA );
		*/
	} else {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
}
// method
void tTJSNI_Canvas::Capture( class tTJSNI_Bitmap* bmp, bool front ) {
	if( !bmp ) {
		TVPAddLog( TJS_W("Bitmap is null.") );
		return;
	}

	// Bitmap の内部表現が正順(上下反転されていない)ことを前提としているので注意
	if( GLScreen ) {
		EGLint sw = GLScreen->GetSurfaceWidth();
		EGLint sh = GLScreen->GetSurfaceHeight();
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
}

void tTJSNI_Canvas::Clear( tjs_uint32 color ) {
	tTVPARGB<tjs_uint32> c;
	c = color;
	glClearColor( c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}
iTJSDispatch2* tTJSNI_Canvas::CreateTexture( class tTJSNI_Bitmap* bmp, bool gray ) { return nullptr; }
iTJSDispatch2* tTJSNI_Canvas::CreateTexture( const ttstr &filename, bool gray ) { return nullptr; }
void tTJSNI_Canvas::DrawScreen( class tTJSNI_Offscreen* screen, tjs_real opacity ) {}
void tTJSNI_Canvas::DrawScreenUT( class tTJSNI_Offscreen* screen, class tTJSNI_Texture* texture, tjs_int vague, tjs_real opacity ) {}
void tTJSNI_Canvas::SetClipMask( class tTJSNI_Texture* texture, tjs_int left, tjs_int top ) { /* TODO: 次期実装  */}
void tTJSNI_Canvas::Fill( tjs_int left, tjs_int top, tjs_int width, tjs_int height, tjs_uint32 colors[4] ) {
	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	GLDrawer.DrawColoredPolygon( colors, left, top, width, height, sw, sh );
}
void tTJSNI_Canvas::DrawTexture( class tTJSNI_Texture* texture, tjs_int left, tjs_int top ) {
	GLuint texId = (GLuint)texture->GetNativeHandle();
	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	GLDrawer.DrawTexture( texId, left, top, texture->GetWidth(), texture->GetHeight(), sw, sh );
}
void tTJSNI_Canvas::DrawTexture( tTJSNI_Texture* texture, tTJSNI_ShaderProgram* shader ) {
	GLuint texId = (GLuint)texture->GetNativeHandle();
	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	GLint texLoc = shader->FindLocation( std::string( "s_texture" ) );
	GLint posLoc = shader->FindLocation( std::string( "a_position" ) );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	GLDrawer.DrawTexture( texId, texture->GetWidth(), texture->GetHeight(), sw, sh, posLoc, uvLoc, texLoc );
}
void tTJSNI_Canvas::DrawTexture2( class tTJSNI_Texture* texture0, class tTJSNI_Texture* texture1, class tTJSNI_ShaderProgram* shader ) {
	GLuint tex0Id = (GLuint)texture0->GetNativeHandle();
	GLuint tex1Id = (GLuint)texture1->GetNativeHandle();
	EGLint sw = GLScreen->GetSurfaceWidth();
	EGLint sh = GLScreen->GetSurfaceHeight();
	GLint tx0Loc = shader->FindLocation( std::string( "s_texture0" ) );
	GLint tx1Loc = shader->FindLocation( std::string( "s_texture1" ) );
	GLint posLoc = shader->FindLocation( std::string( "a_position" ) );
	GLint uvLoc = shader->FindLocation( std::string( "a_texCoord" ) );
	GLDrawer.DrawTexture2( tex0Id, tex1Id, texture0->GetWidth(), texture0->GetHeight(), sw, sh, posLoc, uvLoc, tx0Loc, tx1Loc );
}
void tTJSNI_Canvas::DrawText( class tTJSNI_Font* font, tjs_int x, tjs_int y, const ttstr& text, tjs_uint32 color ) { /* TODO: 次期実装 */}
void tTJSNI_Canvas::ApplyClipRect() {
	if( ClipRectInstance && GLScreen ) {
		GLScreen->SetScissorRect( ClipRectInstance->Get() );
	}
}
void tTJSNI_Canvas::DisableClipRect() {
	if( GLScreen ) GLScreen->DisableScissorRect();
}
void tTJSNI_Canvas::UseShader( tTJSNI_ShaderProgram* shader ) {
	shader->SetProgram();
}

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

void tTJSNI_Canvas::SetTargetScreen( class tTJSNI_Offscreen* screen ) {}
iTJSDispatch2* tTJSNI_Canvas::GetTargetScreenNoAddRef() { return nullptr; }
void tTJSNI_Canvas::SetBlendMode( tTVPBlendMode bm ) {
	BlendMode = bm;
	if( InDrawing ) {
		ApplyBlendMode();
	}
}
void tTJSNI_Canvas::SetStretchType( tTVPStretchType st ) {}
tTVPStretchType tTJSNI_Canvas::GetStretchType() const { return tTVPStretchType::stLinear; }
void tTJSNI_Canvas::SetMatrix( class tTJSNI_Matrix44* matrix ) {}
iTJSDispatch2* tTJSNI_Canvas::GetMatrixNoAddRef() { return nullptr; }
tjs_uint tTJSNI_Canvas::GetWidth() const { return 0;  }
tjs_uint tTJSNI_Canvas::GetHeight() const { return 0;  }

tjs_uint tTJSNI_Canvas::RegisterShader( const ttstr& name, const ttstr& vertex, const ttstr& fragment ) {
	std::string vs;
	std::string fs;
	if( TVPUtf16ToUtf8( vs, vertex.AsStdString() ) == false ) {
		TVPThrowExceptionMessage( TJS_W("Vertex shader character code error.") );
	}
	if( TVPUtf16ToUtf8( fs, fragment.AsStdString() ) == false ) {
		TVPThrowExceptionMessage( TJS_W("Fragment shader character code error.") );
	}
	GLuint program = CompileProgram( vs, fs );
	if( program == 0 ) {
		TVPThrowExceptionMessage( TJS_W("Shader compile error.") );
	}
	ShaderList.Add( name, program );
	return program;
}
tjs_uint tTJSNI_Canvas::FindShader( const ttstr& name ) const {
	tjs_uint *program = ShaderList.Find( name );
	if( program ) {
		return *program;
	} else {
		return 0;
	}
}
tjs_uint tTJSNI_Canvas::GetCurrentShader() const {
	return GLDrawer.GetProgram();
}
void tTJSNI_Canvas::SetCurrentShader( tjs_uint index ) {
	GLDrawer.SetProgram( index );
}

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

	tTJSNI_Bitmap* dstbmp = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&dstbmp)))
			return TJS_E_INVALIDPARAM;
		// TODO Offscreenへのキャプチャも実装する

		bool front = true;
		if( numparams > 1 ) front = ( (tjs_int)param[1] ) ? true : false;
		_this->Capture( dstbmp, front );
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/capture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Offscreen* screen = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&screen)))
			return TJS_E_INVALIDPARAM;
	}
	if(!screen) TVPThrowExceptionMessage(TJS_W("Parameter require Offscreen class instance."));
	tjs_real opacity = *param[1];
	_this->DrawScreen( screen, opacity );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawScreenUT )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Offscreen* screen = nullptr;
	{
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Offscreen::ClassID, (iTJSNativeInstance**)&screen ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !screen ) TVPThrowExceptionMessage( TJS_W( "Parameter require Offscreen class instance." ) );
	}

	tTJSNI_Texture* texture = nullptr;
	{
		tTJSVariantClosure clo = param[1]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !texture ) TVPThrowExceptionMessage( TJS_W( "Parameter require Texture class instance." ) );
	}

	tjs_int vague = *param[2];
	tjs_real opacity = *param[3];
	_this->DrawScreenUT( screen, texture, vague, opacity );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawScreen)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setClipMask)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture)))
			return TJS_E_INVALIDPARAM;
	}
	if(!texture) TVPThrowExceptionMessage(TJS_W("Parameter require Texture class instance."));

	tjs_int left = *param[1];
	tjs_int top = *param[2];
	_this->SetClipMask( texture, left, top );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setClipMask)
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
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture = nullptr;
	if( param[0]->Type() == tvtObject ) {
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !texture ) TVPThrowExceptionMessage( TJS_W( "Parameter require Texture class instance." ) );
	} else {
		return TJS_E_INVALIDPARAM;
	}

	if( param[1]->Type() == tvtObject ) {
		tTJSNI_ShaderProgram* shader = nullptr;
		tTJSVariantClosure clo = param[1]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&shader ) ) )
				return TJS_E_INVALIDPARAM;
		}
		if( !shader ) TVPThrowExceptionMessage( TJS_W( "Parameter require Shader class instance." ) );
		_this->DrawTexture( texture, shader );
	} else if( numparams > 2 ) {
		tjs_int left = *param[1];
		tjs_int top = *param[2];
		_this->DrawTexture( texture, left, top );
	} else {
		return TJS_E_INVALIDPARAM;
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawTexture)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawTexture2 )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_Texture* texture0 = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture0 ) ) )
			return TJS_E_INVALIDPARAM;
	}
	if( !texture0 ) TVPThrowExceptionMessage( TJS_W( "Parameter require Texture class instance." ) );

	tTJSNI_Texture* texture1 = nullptr;
	clo = param[1]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Texture::ClassID, (iTJSNativeInstance**)&texture1 ) ) )
			return TJS_E_INVALIDPARAM;
	}
	if( !texture1 ) TVPThrowExceptionMessage( TJS_W( "Parameter require Texture class instance." ) );

	tTJSNI_ShaderProgram* shader = nullptr;
	clo = param[2]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&shader ) ) )
			return TJS_E_INVALIDPARAM;
	}
	if( !shader ) TVPThrowExceptionMessage( TJS_W( "Parameter require Shader class instance." ) );


	_this->DrawTexture2( texture0, texture1, shader );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/drawTexture2 )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setShader )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	tTJSNI_ShaderProgram* shader = nullptr;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		if( TJS_FAILED( clo.Object->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_ShaderProgram::ClassID, (iTJSNativeInstance**)&shader ) ) )
			return TJS_E_INVALIDPARAM;
	}
	if( !shader ) TVPThrowExceptionMessage( TJS_W( "Parameter require Shader class instance." ) );

	_this->UseShader( shader  );

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setShader )
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
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/applyClipRect )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	_this->ApplyClipRect();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/applyClipRect )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/disableClipRect )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas );
	_this->DisableClipRect();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/disableClipRect )
//----------------------------------------------------------------------
#if 0
// Texture クラスのコンストラクタに移動する
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/createTexture)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	bool doGray = false;
	if( numparams > 1 ) {
		doGray = ((tjs_int)*(param[0])) != 0;
	}
	iTJSDispatch2* texture = nullptr;
	if( param[0]->Type() == tvtString ) {
		texture = _this->CreateTexture( *param[0], doGray );
	} else {
		tTJSNI_Bitmap* srcbmp = nullptr;
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
			tTJSNC_Bitmap::ClassID, (iTJSNativeInstance**)&srcbmp)))
			return TJS_E_INVALIDPARAM;
		texture = _this->CreateTexture( srcbmp, doGray );
	}
	if( !texture ) {
		return TJS_E_INVALIDPARAM;
	}

	try {
		if(result) *result = texture;
	} catch(...) {
		if(texture) texture->Release();
		throw;
	}
	if(texture) texture->Release();

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/createTexture)
#endif 
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
TJS_BEGIN_NATIVE_PROP_DECL(stretchType)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		*result = (tjs_int)_this->GetStretchType();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		_this->SetStretchType( (tTVPStretchType)(tjs_int)*param );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(stretchType)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(matrix)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);
		iTJSDispatch2 *dsp = _this->GetMatrixNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Canvas);

		tTJSNI_Matrix44* matrix = nullptr;
		tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Matrix44::ClassID, (iTJSNativeInstance**)&matrix)))
			return TJS_E_INVALIDPARAM;
		if( matrix != nullptr ) _this->SetMatrix( matrix );

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(matrix)
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

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Canvas()
{
	tTJSNativeClass *cls = new tTJSNC_Canvas();
	return cls;
}
//---------------------------------------------------------------------------
