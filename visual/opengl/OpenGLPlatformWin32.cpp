
#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "Application.h"
#include "FilePathUtil.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "SysInitIntf.h"

//include "EGL/egl.h"
//#include "GLES2/gl2.h"
//#include "GLES3/gl3.h"
//#include "platform/Platform.h"

#include "OpenGLHeaderWin32.h"

static HMODULE TVPhModuleLibEGL = nullptr;
static HMODULE TVPhModuleLibGLESv2 = nullptr;

///libEGL
#ifdef EGLAPI
#undef EGLAPI
#endif
#define EGLAPI

extern "C" {
EGLAPI EGLContext (EGLAPIENTRY* eglGetCurrentContext)( void );
EGLAPI EGLBoolean (EGLAPIENTRY* eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLAPI EGLBoolean (EGLAPIENTRY* eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
EGLAPI EGLContext (EGLAPIENTRY* eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
EGLAPI EGLBoolean (EGLAPIENTRY* eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);
EGLAPI EGLBoolean (EGLAPIENTRY* eglDestroySurface)(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean (EGLAPIENTRY* eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean (EGLAPIENTRY* eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLAPI EGLDisplay (EGLAPIENTRY* eglGetCurrentDisplay)(void);
EGLAPI EGLSurface (EGLAPIENTRY* eglGetCurrentSurface)(EGLint readdraw);
EGLAPI EGLDisplay (EGLAPIENTRY* eglGetDisplay)(EGLNativeDisplayType display_id);
EGLAPI EGLint (EGLAPIENTRY* eglGetError)(void);
EGLAPI __eglMustCastToProperFunctionPointerType (EGLAPIENTRY* eglGetProcAddress)(const char *procname);
EGLAPI EGLBoolean (EGLAPIENTRY* eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLAPI EGLBoolean (EGLAPIENTRY* eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLAPI EGLBoolean (EGLAPIENTRY* eglQueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
EGLAPI const char *(EGLAPIENTRY* eglQueryString)(EGLDisplay dpy, EGLint name);
EGLAPI EGLBoolean (EGLAPIENTRY* eglQuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean (EGLAPIENTRY* eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean (EGLAPIENTRY* eglTerminate)(EGLDisplay dpy);
EGLAPI EGLBoolean (EGLAPIENTRY* eglWaitGL)(void);
EGLAPI EGLBoolean (EGLAPIENTRY* eglWaitNative)(EGLint engine);

EGLAPI EGLBoolean (EGLAPIENTRY* eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLAPI EGLBoolean (EGLAPIENTRY* eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLAPI EGLBoolean (EGLAPIENTRY* eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
EGLAPI EGLBoolean (EGLAPIENTRY* eglSwapInterval)(EGLDisplay dpy, EGLint interval);

EGLAPI EGLBoolean (EGLAPIENTRY* eglBindAPI)(EGLenum api);
EGLAPI EGLenum (EGLAPIENTRY* eglQueryAPI)(void);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
EGLAPI EGLBoolean (EGLAPIENTRY* eglReleaseThread)(void);
EGLAPI EGLBoolean (EGLAPIENTRY* eglWaitClient)(void);

EGLAPI EGLDisplay (EGLAPIENTRY* eglGetPlatformDisplayEXT)(EGLenum platform, void *native_display, const EGLint *attrib_list);
//EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePlatformWindowSurfaceEXT)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
//EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePlatformPixmapSurfaceEXT)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
/* 1.5
EGLAPI EGLSync (EGLAPIENTRY* eglCreateSync)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
EGLAPI EGLBoolean (EGLAPIENTRY* eglDestroySync)(EGLDisplay dpy, EGLSync sync);
EGLAPI EGLint (EGLAPIENTRY* eglClientWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
EGLAPI EGLBoolean (EGLAPIENTRY* eglGetSyncAttrib)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value);
EGLAPI EGLImage (EGLAPIENTRY* eglCreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
EGLAPI EGLBoolean (EGLAPIENTRY* eglDestroyImage)(EGLDisplay dpy, EGLImage image);
EGLAPI EGLDisplay (EGLAPIENTRY* eglGetPlatformDisplay)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePlatformWindowSurface)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list);
EGLAPI EGLSurface (EGLAPIENTRY* eglCreatePlatformPixmapSurface)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list);
EGLAPI EGLBoolean (EGLAPIENTRY* eglWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags);
*/
}

#define FIND_PROC(s,type) (s = (type)::GetProcAddress( hModule, #s ))

bool LoadLibEGL( const tjs_string& dllpath ) {
	if( TVPhModuleLibEGL ) return true;

	tjs_string path = dllpath + tjs_string(TJS_W("libEGL.dll"));
	HMODULE hModule = ::LoadLibrary( path.c_str() );
	if( !hModule ) {
		LPVOID lpMsgBuf;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, ::GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
		TVPAddLog( (tjs_char*)lpMsgBuf );
		::LocalFree( lpMsgBuf );
		return false;
	}
	TVPhModuleLibEGL = hModule;

	FIND_PROC( eglGetCurrentContext, EGLContext( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglChooseConfig, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config ) );
	FIND_PROC( eglCopyBuffers, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target ) );
	FIND_PROC( eglCreateContext, EGLContext( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list ) );
	FIND_PROC( eglCreatePbufferSurface, EGLSurface( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list ) );
	FIND_PROC( eglCreatePixmapSurface, EGLSurface( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list ) );
	FIND_PROC( eglCreateWindowSurface, EGLSurface( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list ) );
	FIND_PROC( eglDestroyContext, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLContext ctx ) );
	FIND_PROC( eglDestroySurface, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface ) );
	FIND_PROC( eglGetConfigAttrib, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value ) );
	FIND_PROC( eglGetConfigs, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config ) );
	FIND_PROC( eglGetCurrentDisplay, EGLDisplay( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglGetCurrentSurface, EGLSurface( EGLAPIENTRY* )( EGLint readdraw ) );
	FIND_PROC( eglGetDisplay, EGLDisplay( EGLAPIENTRY* )( EGLNativeDisplayType display_id ) );
	FIND_PROC( eglGetError, EGLint( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglGetProcAddress, __eglMustCastToProperFunctionPointerType( EGLAPIENTRY* )( const char *procname ) );
	FIND_PROC( eglInitialize, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLint *major, EGLint *minor ) );
	FIND_PROC( eglMakeCurrent, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx ) );
	FIND_PROC( eglQueryContext, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value ) );
	FIND_PROC( eglQueryString, const char * (EGLAPIENTRY*)( EGLDisplay dpy, EGLint name ) );
	FIND_PROC( eglQuerySurface, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value ) );
	FIND_PROC( eglSwapBuffers, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface ) );
	FIND_PROC( eglTerminate, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy ) );
	FIND_PROC( eglWaitGL, EGLBoolean( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglWaitNative, EGLBoolean( EGLAPIENTRY* )( EGLint engine ) );

	FIND_PROC( eglBindTexImage, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface, EGLint buffer ) );
	FIND_PROC( eglReleaseTexImage, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface, EGLint buffer ) );
	FIND_PROC( eglSurfaceAttrib, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value ) );
	FIND_PROC( eglSwapInterval, EGLBoolean( EGLAPIENTRY* )( EGLDisplay dpy, EGLint interval ) );

	FIND_PROC( eglBindAPI, EGLBoolean( EGLAPIENTRY* )( EGLenum api ) );
	FIND_PROC( eglQueryAPI, EGLenum( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglCreatePbufferFromClientBuffer, EGLSurface( EGLAPIENTRY* )( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list ) );
	FIND_PROC( eglReleaseThread, EGLBoolean( EGLAPIENTRY* )( void ) );
	FIND_PROC( eglWaitClient, EGLBoolean( EGLAPIENTRY* )( void ) );
#if 0
; Extensions
	FIND_PROC(eglQuerySurfacePointerANGLE);
	FIND_PROC(eglPostSubBufferNV);
	FIND_PROC(eglQueryDisplayAttribEXT);
	FIND_PROC(eglQueryDeviceAttribEXT);
	FIND_PROC(eglQueryDeviceStringEXT);
	FIND_PROC(eglCreateImageKHR);
	FIND_PROC(eglDestroyImageKHR);
	FIND_PROC(eglCreateDeviceANGLE);
	FIND_PROC(eglReleaseDeviceANGLE);
	FIND_PROC(eglCreateStreamKHR);
	FIND_PROC(eglDestroyStreamKHR);
	FIND_PROC(eglStreamAttribKHR);
	FIND_PROC(eglQueryStreamKHR);
	FIND_PROC(eglQueryStreamu64KHR);
	FIND_PROC(eglStreamConsumerGLTextureExternalKHR);
	FIND_PROC(eglStreamConsumerAcquireKHR);
	FIND_PROC(eglStreamConsumerReleaseKHR);
	FIND_PROC(eglStreamConsumerGLTextureExternalAttribsNV);
	FIND_PROC(eglCreateStreamProducerD3DTextureNV12ANGLE);
	FIND_PROC(eglStreamPostD3DTextureNV12ANGLE);
	FIND_PROC(eglSwapBuffersWithDamageEXT);
#endif
	FIND_PROC(eglGetPlatformDisplayEXT, EGLDisplay (EGLAPIENTRY*)(EGLenum platform, void *native_display, const EGLint *attrib_list) );
//	FIND_PROC(eglCreatePlatformWindowSurfaceEXT, EGLSurface (EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list) );
//	FIND_PROC(eglCreatePlatformPixmapSurfaceEXT, EGLSurface (EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list) );
#if 0
; 1.5 entry points
eglCreateSync
	FIND_PROC(eglDestroySync);
	FIND_PROC(eglClientWaitSync);
	FIND_PROC(eglGetSyncAttrib);
	FIND_PROC(eglCreateImage);
	FIND_PROC(eglDestroyImage);
	FIND_PROC(eglGetPlatformDisplay);
	FIND_PROC(eglCreatePlatformWindowSurface);
	FIND_PROC(eglCreatePlatformPixmapSurface);
	FIND_PROC(eglWaitSync);
#endif
	return (
		(eglBindAPI != nullptr ) &&
		(eglBindTexImage != nullptr ) &&
		(eglChooseConfig != nullptr ) &&
		(eglCopyBuffers != nullptr ) &&
		(eglCreateContext != nullptr ) &&
		(eglCreatePbufferFromClientBuffer != nullptr ) &&
		(eglCreatePbufferSurface != nullptr ) &&
		(eglCreatePixmapSurface != nullptr ) &&
		(eglCreateWindowSurface != nullptr ) &&
		(eglDestroyContext != nullptr ) &&
		(eglDestroySurface != nullptr ) &&
		(eglGetConfigAttrib != nullptr ) &&
		(eglGetConfigs != nullptr ) &&
		(eglGetCurrentContext != nullptr ) &&
		(eglGetCurrentDisplay != nullptr ) &&
		(eglGetCurrentSurface != nullptr ) &&
		(eglGetDisplay != nullptr ) &&
		(eglGetError != nullptr ) &&
		(eglGetProcAddress != nullptr ) &&
		(eglInitialize != nullptr ) &&
		(eglMakeCurrent != nullptr ) &&
		(eglQueryAPI != nullptr ) &&
		(eglQueryContext != nullptr ) &&
		(eglQueryString != nullptr ) &&
		(eglQuerySurface != nullptr ) &&
		(eglReleaseTexImage != nullptr ) &&
		(eglReleaseThread != nullptr ) &&
		(eglSurfaceAttrib != nullptr ) &&
		(eglSwapBuffers != nullptr ) &&
		(eglSwapInterval != nullptr ) &&
		(eglTerminate != nullptr ) &&
		(eglWaitClient != nullptr ) &&
		(eglWaitGL != nullptr ) &&
		(eglWaitNative != nullptr ) &&
		(eglGetPlatformDisplayEXT != nullptr ) );
}
#ifdef GL_APICALL
#undef GL_APICALL
#endif
#define GL_APICALL
extern "C" {
// 2.0
GL_APICALL void (GL_APIENTRY* glActiveTexture)(GLenum texture);
GL_APICALL void (GL_APIENTRY* glAttachShader)(GLuint program, GLuint shader);
GL_APICALL void (GL_APIENTRY* glBindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
GL_APICALL void (GL_APIENTRY* glBindBuffer)(GLenum target, GLuint buffer);
GL_APICALL void (GL_APIENTRY* glBindFramebuffer)(GLenum target, GLuint framebuffer);
GL_APICALL void (GL_APIENTRY* glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
GL_APICALL void (GL_APIENTRY* glBindTexture)(GLenum target, GLuint texture);
GL_APICALL void (GL_APIENTRY* glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void (GL_APIENTRY* glBlendEquation)(GLenum mode);
GL_APICALL void (GL_APIENTRY* glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void (GL_APIENTRY* glBlendFunc)(GLenum sfactor, GLenum dfactor);
GL_APICALL void (GL_APIENTRY* glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GL_APICALL void (GL_APIENTRY* glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_APICALL void (GL_APIENTRY* glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_APICALL GLenum (GL_APIENTRY* glCheckFramebufferStatus)(GLenum target);
GL_APICALL void (GL_APIENTRY* glClear)(GLbitfield mask);
GL_APICALL void (GL_APIENTRY* glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void (GL_APIENTRY* glClearDepthf)(GLfloat d);
GL_APICALL void (GL_APIENTRY* glClearStencil)(GLint s);
GL_APICALL void (GL_APIENTRY* glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GL_APICALL void (GL_APIENTRY* glCompileShader)(GLuint shader);
GL_APICALL void (GL_APIENTRY* glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void (GL_APIENTRY* glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void (GL_APIENTRY* glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void (GL_APIENTRY* glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLuint (GL_APIENTRY* glCreateProgram)(void);
GL_APICALL GLuint (GL_APIENTRY* glCreateShader)(GLenum type);
GL_APICALL void (GL_APIENTRY* glCullFace)(GLenum mode);
GL_APICALL void (GL_APIENTRY* glDeleteBuffers)(GLsizei n, const GLuint *buffers);
GL_APICALL void (GL_APIENTRY* glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
GL_APICALL void (GL_APIENTRY* glDeleteProgram)(GLuint program);
GL_APICALL void (GL_APIENTRY* glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
GL_APICALL void (GL_APIENTRY* glDeleteShader)(GLuint shader);
GL_APICALL void (GL_APIENTRY* glDeleteTextures)(GLsizei n, const GLuint *textures);
GL_APICALL void (GL_APIENTRY* glDepthFunc)(GLenum func);
GL_APICALL void (GL_APIENTRY* glDepthMask)(GLboolean flag);
GL_APICALL void (GL_APIENTRY* glDepthRangef)(GLfloat n, GLfloat f);
GL_APICALL void (GL_APIENTRY* glDetachShader)(GLuint program, GLuint shader);
GL_APICALL void (GL_APIENTRY* glDisable)(GLenum cap);
GL_APICALL void (GL_APIENTRY* glDisableVertexAttribArray)(GLuint index);
GL_APICALL void (GL_APIENTRY* glDrawArrays)(GLenum mode, GLint first, GLsizei count);
GL_APICALL void (GL_APIENTRY* glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_APICALL void (GL_APIENTRY* glEnable)(GLenum cap);
GL_APICALL void (GL_APIENTRY* glEnableVertexAttribArray)(GLuint index);
GL_APICALL void (GL_APIENTRY* glFinish)(void);
GL_APICALL void (GL_APIENTRY* glFlush)(void);
GL_APICALL void (GL_APIENTRY* glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void (GL_APIENTRY* glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void (GL_APIENTRY* glFrontFace)(GLenum mode);
GL_APICALL void (GL_APIENTRY* glGenBuffers)(GLsizei n, GLuint *buffers);
GL_APICALL void (GL_APIENTRY* glGenerateMipmap)(GLenum target);
GL_APICALL void (GL_APIENTRY* glGenFramebuffers)(GLsizei n, GLuint *framebuffers);
GL_APICALL void (GL_APIENTRY* glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
GL_APICALL void (GL_APIENTRY* glGenTextures)(GLsizei n, GLuint *textures);
GL_APICALL void (GL_APIENTRY* glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void (GL_APIENTRY* glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void (GL_APIENTRY* glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GL_APICALL GLint (GL_APIENTRY* glGetAttribLocation)(GLuint program, const GLchar *name);
GL_APICALL void (GL_APIENTRY* glGetBooleanv)(GLenum pname, GLboolean *data);
GL_APICALL void (GL_APIENTRY* glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
GL_APICALL GLenum (GL_APIENTRY* glGetError)(void);
GL_APICALL void (GL_APIENTRY* glGetFloatv)(GLenum pname, GLfloat *data);
GL_APICALL void (GL_APIENTRY* glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetIntegerv)(GLenum pname, GLint *data);
GL_APICALL void (GL_APIENTRY* glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void (GL_APIENTRY* glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void (GL_APIENTRY* glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GL_APICALL void (GL_APIENTRY* glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GL_APICALL const GLubyte *(GL_APIENTRY* glGetString)(GLenum name);
GL_APICALL void (GL_APIENTRY* glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void (GL_APIENTRY* glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetUniformfv)(GLuint program, GLint location, GLfloat *params);
GL_APICALL void (GL_APIENTRY* glGetUniformiv)(GLuint program, GLint location, GLint *params);
GL_APICALL GLint (GL_APIENTRY* glGetUniformLocation)(GLuint program, const GLchar *name);
GL_APICALL void (GL_APIENTRY* glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
GL_APICALL void (GL_APIENTRY* glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer);
GL_APICALL void (GL_APIENTRY* glHint)(GLenum target, GLenum mode);
GL_APICALL GLboolean (GL_APIENTRY* glIsBuffer)(GLuint buffer);
GL_APICALL GLboolean (GL_APIENTRY* glIsEnabled)(GLenum cap);
GL_APICALL GLboolean (GL_APIENTRY* glIsFramebuffer)(GLuint framebuffer);
GL_APICALL GLboolean (GL_APIENTRY* glIsProgram)(GLuint program);
GL_APICALL GLboolean (GL_APIENTRY* glIsRenderbuffer)(GLuint renderbuffer);
GL_APICALL GLboolean (GL_APIENTRY* glIsShader)(GLuint shader);
GL_APICALL GLboolean (GL_APIENTRY* glIsTexture)(GLuint texture);
GL_APICALL void (GL_APIENTRY* glLineWidth)(GLfloat width);
GL_APICALL void (GL_APIENTRY* glLinkProgram)(GLuint program);
GL_APICALL void (GL_APIENTRY* glPixelStorei)(GLenum pname, GLint param);
GL_APICALL void (GL_APIENTRY* glPolygonOffset)(GLfloat factor, GLfloat units);
GL_APICALL void (GL_APIENTRY* glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
GL_APICALL void (GL_APIENTRY* glReleaseShaderCompiler)(void);
GL_APICALL void (GL_APIENTRY* glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glSampleCoverage)(GLfloat value, GLboolean invert);
GL_APICALL void (GL_APIENTRY* glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
GL_APICALL void (GL_APIENTRY* glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
GL_APICALL void (GL_APIENTRY* glStencilFunc)(GLenum func, GLint ref, GLuint mask);
GL_APICALL void (GL_APIENTRY* glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
GL_APICALL void (GL_APIENTRY* glStencilMask)(GLuint mask);
GL_APICALL void (GL_APIENTRY* glStencilMaskSeparate)(GLenum face, GLuint mask);
GL_APICALL void (GL_APIENTRY* glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
GL_APICALL void (GL_APIENTRY* glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GL_APICALL void (GL_APIENTRY* glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void (GL_APIENTRY* glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
GL_APICALL void (GL_APIENTRY* glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void (GL_APIENTRY* glTexParameteri)(GLenum target, GLenum pname, GLint param);
GL_APICALL void (GL_APIENTRY* glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
GL_APICALL void (GL_APIENTRY* glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_APICALL void (GL_APIENTRY* glUniform1f)(GLint location, GLfloat v0);
GL_APICALL void (GL_APIENTRY* glUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniform1i)(GLint location, GLint v0);
GL_APICALL void (GL_APIENTRY* glUniform1iv)(GLint location, GLsizei count, const GLint *value);
GL_APICALL void (GL_APIENTRY* glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void (GL_APIENTRY* glUniform2fv)(GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniform2i)(GLint location, GLint v0, GLint v1);
GL_APICALL void (GL_APIENTRY* glUniform2iv)(GLint location, GLsizei count, const GLint *value);
GL_APICALL void (GL_APIENTRY* glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void (GL_APIENTRY* glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void (GL_APIENTRY* glUniform3iv)(GLint location, GLsizei count, const GLint *value);
GL_APICALL void (GL_APIENTRY* glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void (GL_APIENTRY* glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void (GL_APIENTRY* glUniform4iv)(GLint location, GLsizei count, const GLint *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUseProgram)(GLuint program);
GL_APICALL void (GL_APIENTRY* glValidateProgram)(GLuint program);
GL_APICALL void (GL_APIENTRY* glVertexAttrib1f)(GLuint index, GLfloat x);
GL_APICALL void (GL_APIENTRY* glVertexAttrib1fv)(GLuint index, const GLfloat *v);
GL_APICALL void (GL_APIENTRY* glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
GL_APICALL void (GL_APIENTRY* glVertexAttrib2fv)(GLuint index, const GLfloat *v);
GL_APICALL void (GL_APIENTRY* glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void (GL_APIENTRY* glVertexAttrib3fv)(GLuint index, const GLfloat *v);
GL_APICALL void (GL_APIENTRY* glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_APICALL void (GL_APIENTRY* glVertexAttrib4fv)(GLuint index, const GLfloat *v);
GL_APICALL void (GL_APIENTRY* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_APICALL void (GL_APIENTRY* glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

// 3.0
GL_APICALL void (GL_APIENTRY* glReadBuffer)(GLenum src);
GL_APICALL void (GL_APIENTRY* glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
GL_APICALL void (GL_APIENTRY* glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void (GL_APIENTRY* glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
GL_APICALL void (GL_APIENTRY* glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void (GL_APIENTRY* glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void (GL_APIENTRY* glGenQueries)(GLsizei n, GLuint *ids);
GL_APICALL void (GL_APIENTRY* glDeleteQueries)(GLsizei n, const GLuint *ids);
GL_APICALL GLboolean (GL_APIENTRY* glIsQuery)(GLuint id);
GL_APICALL void (GL_APIENTRY* glBeginQuery)(GLenum target, GLuint id);
GL_APICALL void (GL_APIENTRY* glEndQuery)(GLenum target);
GL_APICALL void (GL_APIENTRY* glGetQueryiv)(GLenum target, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
GL_APICALL GLboolean (GL_APIENTRY* glUnmapBuffer)(GLenum target);
GL_APICALL void (GL_APIENTRY* glGetBufferPointerv)(GLenum target, GLenum pname, void **params);
GL_APICALL void (GL_APIENTRY* glDrawBuffers)(GLsizei n, const GLenum *bufs);
GL_APICALL void (GL_APIENTRY* glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GL_APICALL void (GL_APIENTRY* glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
GL_APICALL void *(GL_APIENTRY* glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GL_APICALL void (GL_APIENTRY* glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
GL_APICALL void (GL_APIENTRY* glBindVertexArray)(GLuint array);
GL_APICALL void (GL_APIENTRY* glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
GL_APICALL void (GL_APIENTRY* glGenVertexArrays)(GLsizei n, GLuint *arrays);
GL_APICALL GLboolean (GL_APIENTRY* glIsVertexArray)(GLuint array);
GL_APICALL void (GL_APIENTRY* glGetIntegeri_v)(GLenum target, GLuint index, GLint *data);
GL_APICALL void (GL_APIENTRY* glBeginTransformFeedback)(GLenum primitiveMode);
GL_APICALL void (GL_APIENTRY* glEndTransformFeedback)(void);
GL_APICALL void (GL_APIENTRY* glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GL_APICALL void (GL_APIENTRY* glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
GL_APICALL void (GL_APIENTRY* glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
GL_APICALL void (GL_APIENTRY* glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GL_APICALL void (GL_APIENTRY* glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void (GL_APIENTRY* glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
GL_APICALL void (GL_APIENTRY* glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
GL_APICALL void (GL_APIENTRY* glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GL_APICALL void (GL_APIENTRY* glVertexAttribI4iv)(GLuint index, const GLint *v);
GL_APICALL void (GL_APIENTRY* glVertexAttribI4uiv)(GLuint index, const GLuint *v);
GL_APICALL void (GL_APIENTRY* glGetUniformuiv)(GLuint program, GLint location, GLuint *params);
GL_APICALL GLint (GL_APIENTRY* glGetFragDataLocation)(GLuint program, const GLchar *name);
GL_APICALL void (GL_APIENTRY* glUniform1ui)(GLint location, GLuint v0);
GL_APICALL void (GL_APIENTRY* glUniform2ui)(GLint location, GLuint v0, GLuint v1);
GL_APICALL void (GL_APIENTRY* glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void (GL_APIENTRY* glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void (GL_APIENTRY* glUniform1uiv)(GLint location, GLsizei count, const GLuint *value);
GL_APICALL void (GL_APIENTRY* glUniform2uiv)(GLint location, GLsizei count, const GLuint *value);
GL_APICALL void (GL_APIENTRY* glUniform3uiv)(GLint location, GLsizei count, const GLuint *value);
GL_APICALL void (GL_APIENTRY* glUniform4uiv)(GLint location, GLsizei count, const GLuint *value);
GL_APICALL void (GL_APIENTRY* glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint *value);
GL_APICALL void (GL_APIENTRY* glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value);
GL_APICALL void (GL_APIENTRY* glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
GL_APICALL void (GL_APIENTRY* glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
GL_APICALL const GLubyte *(GL_APIENTRY* glGetStringi)(GLenum name, GLuint index);
GL_APICALL void (GL_APIENTRY* glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
GL_APICALL void (GL_APIENTRY* glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
GL_APICALL void (GL_APIENTRY* glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
GL_APICALL GLuint (GL_APIENTRY* glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
GL_APICALL void (GL_APIENTRY* glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
GL_APICALL void (GL_APIENTRY* glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
GL_APICALL void (GL_APIENTRY* glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
GL_APICALL void (GL_APIENTRY* glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
GL_APICALL GLsync (GL_APIENTRY* glFenceSync)(GLenum condition, GLbitfield flags);
GL_APICALL GLboolean (GL_APIENTRY* glIsSync)(GLsync sync);
GL_APICALL void (GL_APIENTRY* glDeleteSync)(GLsync sync);
GL_APICALL GLenum (GL_APIENTRY* glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void (GL_APIENTRY* glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void (GL_APIENTRY* glGetInteger64v)(GLenum pname, GLint64 *data);
GL_APICALL void (GL_APIENTRY* glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
GL_APICALL void (GL_APIENTRY* glGetInteger64i_v)(GLenum target, GLuint index, GLint64 *data);
GL_APICALL void (GL_APIENTRY* glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params);
GL_APICALL void (GL_APIENTRY* glGenSamplers)(GLsizei count, GLuint *samplers);
GL_APICALL void (GL_APIENTRY* glDeleteSamplers)(GLsizei count, const GLuint *samplers);
GL_APICALL GLboolean (GL_APIENTRY* glIsSampler)(GLuint sampler);
GL_APICALL void (GL_APIENTRY* glBindSampler)(GLuint unit, GLuint sampler);
GL_APICALL void (GL_APIENTRY* glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
GL_APICALL void (GL_APIENTRY* glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *param);
GL_APICALL void (GL_APIENTRY* glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
GL_APICALL void (GL_APIENTRY* glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *param);
GL_APICALL void (GL_APIENTRY* glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params);
GL_APICALL void (GL_APIENTRY* glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params);
GL_APICALL void (GL_APIENTRY* glVertexAttribDivisor)(GLuint index, GLuint divisor);
GL_APICALL void (GL_APIENTRY* glBindTransformFeedback)(GLenum target, GLuint id);
GL_APICALL void (GL_APIENTRY* glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
GL_APICALL void (GL_APIENTRY* glGenTransformFeedbacks)(GLsizei n, GLuint *ids);
GL_APICALL GLboolean (GL_APIENTRY* glIsTransformFeedback)(GLuint id);
GL_APICALL void (GL_APIENTRY* glPauseTransformFeedback)(void);
GL_APICALL void (GL_APIENTRY* glResumeTransformFeedback)(void);
GL_APICALL void (GL_APIENTRY* glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
GL_APICALL void (GL_APIENTRY* glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
GL_APICALL void (GL_APIENTRY* glProgramParameteri)(GLuint program, GLenum pname, GLint value);
GL_APICALL void (GL_APIENTRY* glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
GL_APICALL void (GL_APIENTRY* glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void (GL_APIENTRY* glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GL_APICALL void (GL_APIENTRY* glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
// Platform
//void (*ANGLE_APIENTRY ANGLEPlatformInitialize)(angle::Platform*);
//void (*ANGLE_APIENTRY ANGLEPlatformShutdown)();
//angle::Platform *(*ANGLE_APIENTRY ANGLEPlatformCurrent)();
}

// libGLESv2
bool LoadLibGLESv2( const tjs_string& dllpath ) {
	if( TVPhModuleLibGLESv2 ) return true;
	tjs_string path = dllpath + tjs_string(TJS_W("libGLESv2.dll"));
	HMODULE hModule = ::LoadLibrary( path.c_str() );
	if( !hModule ) {
		LPVOID lpMsgBuf;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, ::GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
		TVPAddLog( (tjs_char*)lpMsgBuf );
		::LocalFree( lpMsgBuf );
		return false;
	}
	TVPhModuleLibGLESv2 = hModule;

	FIND_PROC( glActiveTexture, void  ( GL_APIENTRY* )( GLenum texture ) );
	FIND_PROC( glAttachShader, void  ( GL_APIENTRY* )( GLuint program, GLuint shader ) );
	FIND_PROC( glBindAttribLocation, void  ( GL_APIENTRY* )( GLuint program, GLuint index, const GLchar *name ) );
	FIND_PROC( glBindBuffer, void  ( GL_APIENTRY* )( GLenum target, GLuint buffer ) );
	FIND_PROC( glBindFramebuffer, void  ( GL_APIENTRY* )( GLenum target, GLuint framebuffer ) );
	FIND_PROC( glBindRenderbuffer, void  ( GL_APIENTRY* )( GLenum target, GLuint renderbuffer ) );
	FIND_PROC( glBindTexture, void  ( GL_APIENTRY* )( GLenum target, GLuint texture ) );
	FIND_PROC( glBlendColor, void  ( GL_APIENTRY* )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) );
	FIND_PROC( glBlendEquation, void  ( GL_APIENTRY* )( GLenum mode ) );
	FIND_PROC( glBlendEquationSeparate, void  ( GL_APIENTRY* )( GLenum modeRGB, GLenum modeAlpha ) );
	FIND_PROC( glBlendFunc, void  ( GL_APIENTRY* )( GLenum sfactor, GLenum dfactor ) );
	FIND_PROC( glBlendFuncSeparate, void  ( GL_APIENTRY* )( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha ) );
	FIND_PROC( glBufferData, void  ( GL_APIENTRY* )( GLenum target, GLsizeiptr size, const void *data, GLenum usage ) );
	FIND_PROC( glBufferSubData, void  ( GL_APIENTRY* )( GLenum target, GLintptr offset, GLsizeiptr size, const void *data ) );
	FIND_PROC( glCheckFramebufferStatus, GLenum( GL_APIENTRY* )( GLenum target ) );
	FIND_PROC( glClear, void  ( GL_APIENTRY* )( GLbitfield mask ) );
	FIND_PROC( glClearColor, void  ( GL_APIENTRY* )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) );
	FIND_PROC( glClearDepthf, void  ( GL_APIENTRY* )( GLfloat d ) );
	FIND_PROC( glClearStencil, void  ( GL_APIENTRY* )( GLint s ) );
	FIND_PROC( glColorMask, void  ( GL_APIENTRY* )( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) );
	FIND_PROC( glCompileShader, void  ( GL_APIENTRY* )( GLuint shader ) );
	FIND_PROC( glCompressedTexImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data ) );
	FIND_PROC( glCompressedTexSubImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data ) );
	FIND_PROC( glCopyTexImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) );
	FIND_PROC( glCopyTexSubImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) );
	FIND_PROC( glCreateProgram, GLuint( GL_APIENTRY* )( void ) );
	FIND_PROC( glCreateShader, GLuint( GL_APIENTRY* )( GLenum type ) );
	FIND_PROC( glCullFace, void  ( GL_APIENTRY* )( GLenum mode ) );
	FIND_PROC( glDeleteBuffers, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *buffers ) );
	FIND_PROC( glDeleteFramebuffers, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *framebuffers ) );
	FIND_PROC( glDeleteProgram, void  ( GL_APIENTRY* )( GLuint program ) );
	FIND_PROC( glDeleteRenderbuffers, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *renderbuffers ) );
	FIND_PROC( glDeleteShader, void  ( GL_APIENTRY* )( GLuint shader ) );
	FIND_PROC( glDeleteTextures, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *textures ) );
	FIND_PROC( glDepthFunc, void  ( GL_APIENTRY* )( GLenum func ) );
	FIND_PROC( glDepthMask, void  ( GL_APIENTRY* )( GLboolean flag ) );
	FIND_PROC( glDepthRangef, void  ( GL_APIENTRY* )( GLfloat n, GLfloat f ) );
	FIND_PROC( glDetachShader, void  ( GL_APIENTRY* )( GLuint program, GLuint shader ) );
	FIND_PROC( glDisable, void  ( GL_APIENTRY* )( GLenum cap ) );
	FIND_PROC( glDisableVertexAttribArray, void  ( GL_APIENTRY* )( GLuint index ) );
	FIND_PROC( glDrawArrays, void  ( GL_APIENTRY* )( GLenum mode, GLint first, GLsizei count ) );
	FIND_PROC( glDrawElements, void  ( GL_APIENTRY* )( GLenum mode, GLsizei count, GLenum type, const void *indices ) );
	FIND_PROC( glEnable, void  ( GL_APIENTRY* )( GLenum cap ) );
	FIND_PROC( glEnableVertexAttribArray, void  ( GL_APIENTRY* )( GLuint index ) );
	FIND_PROC( glFinish, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glFlush, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glFramebufferRenderbuffer, void  ( GL_APIENTRY* )( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ) );
	FIND_PROC( glFramebufferTexture2D, void  ( GL_APIENTRY* )( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) );
	FIND_PROC( glFrontFace, void  ( GL_APIENTRY* )( GLenum mode ) );
	FIND_PROC( glGenBuffers, void  ( GL_APIENTRY* )( GLsizei n, GLuint *buffers ) );
	FIND_PROC( glGenerateMipmap, void  ( GL_APIENTRY* )( GLenum target ) );
	FIND_PROC( glGenFramebuffers, void  ( GL_APIENTRY* )( GLsizei n, GLuint *framebuffers ) );
	FIND_PROC( glGenRenderbuffers, void  ( GL_APIENTRY* )( GLsizei n, GLuint *renderbuffers ) );
	FIND_PROC( glGenTextures, void  ( GL_APIENTRY* )( GLsizei n, GLuint *textures ) );
	FIND_PROC( glGetActiveAttrib, void  ( GL_APIENTRY* )( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) );
	FIND_PROC( glGetActiveUniform, void  ( GL_APIENTRY* )( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) );
	FIND_PROC( glGetAttachedShaders, void  ( GL_APIENTRY* )( GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders ) );
	FIND_PROC( glGetAttribLocation, GLint( GL_APIENTRY* )( GLuint program, const GLchar *name ) );
	FIND_PROC( glGetBooleanv, void  ( GL_APIENTRY* )( GLenum pname, GLboolean *data ) );
	FIND_PROC( glGetBufferParameteriv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint *params ) );
	FIND_PROC( glGetError, GLenum( GL_APIENTRY* )( void ) );
	FIND_PROC( glGetFloatv, void  ( GL_APIENTRY* )( GLenum pname, GLfloat *data ) );
	FIND_PROC( glGetFramebufferAttachmentParameteriv, void  ( GL_APIENTRY* )( GLenum target, GLenum attachment, GLenum pname, GLint *params ) );
	FIND_PROC( glGetIntegerv, void  ( GL_APIENTRY* )( GLenum pname, GLint *data ) );
	FIND_PROC( glGetProgramiv, void  ( GL_APIENTRY* )( GLuint program, GLenum pname, GLint *params ) );
	FIND_PROC( glGetProgramInfoLog, void  ( GL_APIENTRY* )( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) );
	FIND_PROC( glGetRenderbufferParameteriv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint *params ) );
	FIND_PROC( glGetShaderiv, void  ( GL_APIENTRY* )( GLuint shader, GLenum pname, GLint *params ) );
	FIND_PROC( glGetShaderInfoLog, void  ( GL_APIENTRY* )( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) );
	FIND_PROC( glGetShaderPrecisionFormat, void  ( GL_APIENTRY* )( GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision ) );
	FIND_PROC( glGetShaderSource, void  ( GL_APIENTRY* )( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source ) );
	FIND_PROC( glGetString, const GLubyte * (GL_APIENTRY*)( GLenum name ) );
	FIND_PROC( glGetTexParameterfv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLfloat *params ) );
	FIND_PROC( glGetTexParameteriv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint *params ) );
	FIND_PROC( glGetUniformfv, void  ( GL_APIENTRY* )( GLuint program, GLint location, GLfloat *params ) );
	FIND_PROC( glGetUniformiv, void  ( GL_APIENTRY* )( GLuint program, GLint location, GLint *params ) );
	FIND_PROC( glGetUniformLocation, GLint( GL_APIENTRY* )( GLuint program, const GLchar *name ) );
	FIND_PROC( glGetVertexAttribfv, void  ( GL_APIENTRY* )( GLuint index, GLenum pname, GLfloat *params ) );
	FIND_PROC( glGetVertexAttribiv, void  ( GL_APIENTRY* )( GLuint index, GLenum pname, GLint *params ) );
	FIND_PROC( glGetVertexAttribPointerv, void  ( GL_APIENTRY* )( GLuint index, GLenum pname, void **pointer ) );
	FIND_PROC( glHint, void  ( GL_APIENTRY* )( GLenum target, GLenum mode ) );
	FIND_PROC( glIsBuffer, GLboolean( GL_APIENTRY* )( GLuint buffer ) );
	FIND_PROC( glIsEnabled, GLboolean( GL_APIENTRY* )( GLenum cap ) );
	FIND_PROC( glIsFramebuffer, GLboolean( GL_APIENTRY* )( GLuint framebuffer ) );
	FIND_PROC( glIsProgram, GLboolean( GL_APIENTRY* )( GLuint program ) );
	FIND_PROC( glIsRenderbuffer, GLboolean( GL_APIENTRY* )( GLuint renderbuffer ) );
	FIND_PROC( glIsShader, GLboolean( GL_APIENTRY* )( GLuint shader ) );
	FIND_PROC( glIsTexture, GLboolean( GL_APIENTRY* )( GLuint texture ) );
	FIND_PROC( glLineWidth, void  ( GL_APIENTRY* )( GLfloat width ) );
	FIND_PROC( glLinkProgram, void  ( GL_APIENTRY* )( GLuint program ) );
	FIND_PROC( glPixelStorei, void  ( GL_APIENTRY* )( GLenum pname, GLint param ) );
	FIND_PROC( glPolygonOffset, void  ( GL_APIENTRY* )( GLfloat factor, GLfloat units ) );
	FIND_PROC( glReadPixels, void  ( GL_APIENTRY* )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels ) );
	FIND_PROC( glReleaseShaderCompiler, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glRenderbufferStorage, void  ( GL_APIENTRY* )( GLenum target, GLenum internalformat, GLsizei width, GLsizei height ) );
	FIND_PROC( glSampleCoverage, void  ( GL_APIENTRY* )( GLfloat value, GLboolean invert ) );
	FIND_PROC( glScissor, void  ( GL_APIENTRY* )( GLint x, GLint y, GLsizei width, GLsizei height ) );
	FIND_PROC( glShaderBinary, void  ( GL_APIENTRY* )( GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length ) );
	FIND_PROC( glShaderSource, void  ( GL_APIENTRY* )( GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length ) );
	FIND_PROC( glStencilFunc, void  ( GL_APIENTRY* )( GLenum func, GLint ref, GLuint mask ) );
	FIND_PROC( glStencilFuncSeparate, void  ( GL_APIENTRY* )( GLenum face, GLenum func, GLint ref, GLuint mask ) );
	FIND_PROC( glStencilMask, void  ( GL_APIENTRY* )( GLuint mask ) );
	FIND_PROC( glStencilMaskSeparate, void  ( GL_APIENTRY* )( GLenum face, GLuint mask ) );
	FIND_PROC( glStencilOp, void  ( GL_APIENTRY* )( GLenum fail, GLenum zfail, GLenum zpass ) );
	FIND_PROC( glStencilOpSeparate, void  ( GL_APIENTRY* )( GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass ) );
	FIND_PROC( glTexImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels ) );
	FIND_PROC( glTexParameterf, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLfloat param ) );
	FIND_PROC( glTexParameterfv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, const GLfloat *params ) );
	FIND_PROC( glTexParameteri, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint param ) );
	FIND_PROC( glTexParameteriv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, const GLint *params ) );
	FIND_PROC( glTexSubImage2D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels ) );
	FIND_PROC( glUniform1f, void  ( GL_APIENTRY* )( GLint location, GLfloat v0 ) );
	FIND_PROC( glUniform1fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLfloat *value ) );
	FIND_PROC( glUniform1i, void  ( GL_APIENTRY* )( GLint location, GLint v0 ) );
	FIND_PROC( glUniform1iv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLint *value ) );
	FIND_PROC( glUniform2f, void  ( GL_APIENTRY* )( GLint location, GLfloat v0, GLfloat v1 ) );
	FIND_PROC( glUniform2fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLfloat *value ) );
	FIND_PROC( glUniform2i, void  ( GL_APIENTRY* )( GLint location, GLint v0, GLint v1 ) );
	FIND_PROC( glUniform2iv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLint *value ) );
	FIND_PROC( glUniform3f, void  ( GL_APIENTRY* )( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ) );
	FIND_PROC( glUniform3fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLfloat *value ) );
	FIND_PROC( glUniform3i, void  ( GL_APIENTRY* )( GLint location, GLint v0, GLint v1, GLint v2 ) );
	FIND_PROC( glUniform3iv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLint *value ) );
	FIND_PROC( glUniform4f, void  ( GL_APIENTRY* )( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) );
	FIND_PROC( glUniform4fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLfloat *value ) );
	FIND_PROC( glUniform4i, void  ( GL_APIENTRY* )( GLint location, GLint v0, GLint v1, GLint v2, GLint v3 ) );
	FIND_PROC( glUniform4iv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLint *value ) );
	FIND_PROC( glUniformMatrix2fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix3fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix4fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUseProgram, void  ( GL_APIENTRY* )( GLuint program ) );
	FIND_PROC( glValidateProgram, void  ( GL_APIENTRY* )( GLuint program ) );
	FIND_PROC( glVertexAttrib1f, void  ( GL_APIENTRY* )( GLuint index, GLfloat x ) );
	FIND_PROC( glVertexAttrib1fv, void  ( GL_APIENTRY* )( GLuint index, const GLfloat *v ) );
	FIND_PROC( glVertexAttrib2f, void  ( GL_APIENTRY* )( GLuint index, GLfloat x, GLfloat y ) );
	FIND_PROC( glVertexAttrib2fv, void  ( GL_APIENTRY* )( GLuint index, const GLfloat *v ) );
	FIND_PROC( glVertexAttrib3f, void  ( GL_APIENTRY* )( GLuint index, GLfloat x, GLfloat y, GLfloat z ) );
	FIND_PROC( glVertexAttrib3fv, void  ( GL_APIENTRY* )( GLuint index, const GLfloat *v ) );
	FIND_PROC( glVertexAttrib4f, void  ( GL_APIENTRY* )( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w ) );
	FIND_PROC( glVertexAttrib4fv, void  ( GL_APIENTRY* )( GLuint index, const GLfloat *v ) );
	FIND_PROC( glVertexAttribPointer, void  ( GL_APIENTRY* )( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer ) );
	FIND_PROC( glViewport, void  ( GL_APIENTRY* )( GLint x, GLint y, GLsizei width, GLsizei height ) );

	// 3.0
	FIND_PROC( glReadBuffer, void  ( GL_APIENTRY* )( GLenum src ) );
	FIND_PROC( glDrawRangeElements, void  ( GL_APIENTRY* )( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices ) );
	FIND_PROC( glTexImage3D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels ) );
	FIND_PROC( glTexSubImage3D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels ) );
	FIND_PROC( glCopyTexSubImage3D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) );
	FIND_PROC( glCompressedTexImage3D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data ) );
	FIND_PROC( glCompressedTexSubImage3D, void  ( GL_APIENTRY* )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data ) );
	FIND_PROC( glGenQueries, void  ( GL_APIENTRY* )( GLsizei n, GLuint *ids ) );
	FIND_PROC( glDeleteQueries, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *ids ) );
	FIND_PROC( glIsQuery, GLboolean( GL_APIENTRY* )( GLuint id ) );
	FIND_PROC( glBeginQuery, void  ( GL_APIENTRY* )( GLenum target, GLuint id ) );
	FIND_PROC( glEndQuery, void  ( GL_APIENTRY* )( GLenum target ) );
	FIND_PROC( glGetQueryiv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint *params ) );
	FIND_PROC( glGetQueryObjectuiv, void  ( GL_APIENTRY* )( GLuint id, GLenum pname, GLuint *params ) );
	FIND_PROC( glUnmapBuffer, GLboolean( GL_APIENTRY* )( GLenum target ) );
	FIND_PROC( glGetBufferPointerv, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, void **params ) );
	FIND_PROC( glDrawBuffers, void  ( GL_APIENTRY* )( GLsizei n, const GLenum *bufs ) );
	FIND_PROC( glUniformMatrix2x3fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix3x2fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix2x4fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix4x2fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix3x4fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glUniformMatrix4x3fv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) );
	FIND_PROC( glBlitFramebuffer, void  ( GL_APIENTRY* )( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) );
	FIND_PROC( glRenderbufferStorageMultisample, void  ( GL_APIENTRY* )( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) );
	FIND_PROC( glFramebufferTextureLayer, void  ( GL_APIENTRY* )( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) );
	FIND_PROC( glMapBufferRange, void * (GL_APIENTRY*)( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) );
	FIND_PROC( glFlushMappedBufferRange, void  ( GL_APIENTRY* )( GLenum target, GLintptr offset, GLsizeiptr length ) );
	FIND_PROC( glBindVertexArray, void  ( GL_APIENTRY* )( GLuint array ) );
	FIND_PROC( glDeleteVertexArrays, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *arrays ) );
	FIND_PROC( glGenVertexArrays, void  ( GL_APIENTRY* )( GLsizei n, GLuint *arrays ) );
	FIND_PROC( glIsVertexArray, GLboolean( GL_APIENTRY* )( GLuint array ) );
	FIND_PROC( glGetIntegeri_v, void  ( GL_APIENTRY* )( GLenum target, GLuint index, GLint *data ) );
	FIND_PROC( glBeginTransformFeedback, void  ( GL_APIENTRY* )( GLenum primitiveMode ) );
	FIND_PROC( glEndTransformFeedback, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glBindBufferRange, void  ( GL_APIENTRY* )( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size ) );
	FIND_PROC( glBindBufferBase, void  ( GL_APIENTRY* )( GLenum target, GLuint index, GLuint buffer ) );
	FIND_PROC( glTransformFeedbackVaryings, void  ( GL_APIENTRY* )( GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode ) );
	FIND_PROC( glGetTransformFeedbackVarying, void  ( GL_APIENTRY* )( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name ) );
	FIND_PROC( glVertexAttribIPointer, void  ( GL_APIENTRY* )( GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer ) );
	FIND_PROC( glGetVertexAttribIiv, void  ( GL_APIENTRY* )( GLuint index, GLenum pname, GLint *params ) );
	FIND_PROC( glGetVertexAttribIuiv, void  ( GL_APIENTRY* )( GLuint index, GLenum pname, GLuint *params ) );
	FIND_PROC( glVertexAttribI4i, void  ( GL_APIENTRY* )( GLuint index, GLint x, GLint y, GLint z, GLint w ) );
	FIND_PROC( glVertexAttribI4ui, void  ( GL_APIENTRY* )( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w ) );
	FIND_PROC( glVertexAttribI4iv, void  ( GL_APIENTRY* )( GLuint index, const GLint *v ) );
	FIND_PROC( glVertexAttribI4uiv, void  ( GL_APIENTRY* )( GLuint index, const GLuint *v ) );
	FIND_PROC( glGetUniformuiv, void  ( GL_APIENTRY* )( GLuint program, GLint location, GLuint *params ) );
	FIND_PROC( glGetFragDataLocation, GLint( GL_APIENTRY* )( GLuint program, const GLchar *name ) );
	FIND_PROC( glUniform1ui, void  ( GL_APIENTRY* )( GLint location, GLuint v0 ) );
	FIND_PROC( glUniform2ui, void  ( GL_APIENTRY* )( GLint location, GLuint v0, GLuint v1 ) );
	FIND_PROC( glUniform3ui, void  ( GL_APIENTRY* )( GLint location, GLuint v0, GLuint v1, GLuint v2 ) );
	FIND_PROC( glUniform4ui, void  ( GL_APIENTRY* )( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) );
	FIND_PROC( glUniform1uiv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLuint *value ) );
	FIND_PROC( glUniform2uiv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLuint *value ) );
	FIND_PROC( glUniform3uiv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLuint *value ) );
	FIND_PROC( glUniform4uiv, void  ( GL_APIENTRY* )( GLint location, GLsizei count, const GLuint *value ) );
	FIND_PROC( glClearBufferiv, void  ( GL_APIENTRY* )( GLenum buffer, GLint drawbuffer, const GLint *value ) );
	FIND_PROC( glClearBufferuiv, void  ( GL_APIENTRY* )( GLenum buffer, GLint drawbuffer, const GLuint *value ) );
	FIND_PROC( glClearBufferfv, void  ( GL_APIENTRY* )( GLenum buffer, GLint drawbuffer, const GLfloat *value ) );
	FIND_PROC( glClearBufferfi, void  ( GL_APIENTRY* )( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil ) );
	FIND_PROC( glGetStringi, const GLubyte * (GL_APIENTRY*)( GLenum name, GLuint index ) );
	FIND_PROC( glCopyBufferSubData, void  ( GL_APIENTRY* )( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size ) );
	FIND_PROC( glGetUniformIndices, void  ( GL_APIENTRY* )( GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices ) );
	FIND_PROC( glGetActiveUniformsiv, void  ( GL_APIENTRY* )( GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params ) );
	FIND_PROC( glGetUniformBlockIndex, GLuint( GL_APIENTRY* )( GLuint program, const GLchar *uniformBlockName ) );
	FIND_PROC( glGetActiveUniformBlockiv, void  ( GL_APIENTRY* )( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params ) );
	FIND_PROC( glGetActiveUniformBlockName, void  ( GL_APIENTRY* )( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName ) );
	FIND_PROC( glUniformBlockBinding, void  ( GL_APIENTRY* )( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) );
	FIND_PROC( glDrawArraysInstanced, void  ( GL_APIENTRY* )( GLenum mode, GLint first, GLsizei count, GLsizei instancecount ) );
	FIND_PROC( glDrawElementsInstanced, void  ( GL_APIENTRY* )( GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount ) );
	FIND_PROC( glFenceSync, GLsync( GL_APIENTRY* )( GLenum condition, GLbitfield flags ) );
	FIND_PROC( glIsSync, GLboolean( GL_APIENTRY* )( GLsync sync ) );
	FIND_PROC( glDeleteSync, void  ( GL_APIENTRY* )( GLsync sync ) );
	FIND_PROC( glClientWaitSync, GLenum( GL_APIENTRY* )( GLsync sync, GLbitfield flags, GLuint64 timeout ) );
	FIND_PROC( glWaitSync, void  ( GL_APIENTRY* )( GLsync sync, GLbitfield flags, GLuint64 timeout ) );
	FIND_PROC( glGetInteger64v, void  ( GL_APIENTRY* )( GLenum pname, GLint64 *data ) );
	FIND_PROC( glGetSynciv, void  ( GL_APIENTRY* )( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values ) );
	FIND_PROC( glGetInteger64i_v, void  ( GL_APIENTRY* )( GLenum target, GLuint index, GLint64 *data ) );
	FIND_PROC( glGetBufferParameteri64v, void  ( GL_APIENTRY* )( GLenum target, GLenum pname, GLint64 *params ) );
	FIND_PROC( glGenSamplers, void  ( GL_APIENTRY* )( GLsizei count, GLuint *samplers ) );
	FIND_PROC( glDeleteSamplers, void  ( GL_APIENTRY* )( GLsizei count, const GLuint *samplers ) );
	FIND_PROC( glIsSampler, GLboolean( GL_APIENTRY* )( GLuint sampler ) );
	FIND_PROC( glBindSampler, void  ( GL_APIENTRY* )( GLuint unit, GLuint sampler ) );
	FIND_PROC( glSamplerParameteri, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, GLint param ) );
	FIND_PROC( glSamplerParameteriv, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, const GLint *param ) );
	FIND_PROC( glSamplerParameterf, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, GLfloat param ) );
	FIND_PROC( glSamplerParameterfv, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, const GLfloat *param ) );
	FIND_PROC( glGetSamplerParameteriv, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, GLint *params ) );
	FIND_PROC( glGetSamplerParameterfv, void  ( GL_APIENTRY* )( GLuint sampler, GLenum pname, GLfloat *params ) );
	FIND_PROC( glVertexAttribDivisor, void  ( GL_APIENTRY* )( GLuint index, GLuint divisor ) );
	FIND_PROC( glBindTransformFeedback, void  ( GL_APIENTRY* )( GLenum target, GLuint id ) );
	FIND_PROC( glDeleteTransformFeedbacks, void  ( GL_APIENTRY* )( GLsizei n, const GLuint *ids ) );
	FIND_PROC( glGenTransformFeedbacks, void  ( GL_APIENTRY* )( GLsizei n, GLuint *ids ) );
	FIND_PROC( glIsTransformFeedback, GLboolean( GL_APIENTRY* )( GLuint id ) );
	FIND_PROC( glPauseTransformFeedback, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glResumeTransformFeedback, void  ( GL_APIENTRY* )( void ) );
	FIND_PROC( glGetProgramBinary, void  ( GL_APIENTRY* )( GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary ) );
	FIND_PROC( glProgramBinary, void  ( GL_APIENTRY* )( GLuint program, GLenum binaryFormat, const void *binary, GLsizei length ) );
	FIND_PROC( glProgramParameteri, void  ( GL_APIENTRY* )( GLuint program, GLenum pname, GLint value ) );
	FIND_PROC( glInvalidateFramebuffer, void  ( GL_APIENTRY* )( GLenum target, GLsizei numAttachments, const GLenum *attachments ) );
	FIND_PROC( glInvalidateSubFramebuffer, void  ( GL_APIENTRY* )( GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height ) );
	FIND_PROC( glTexStorage2D, void  ( GL_APIENTRY* )( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height ) );
	FIND_PROC( glTexStorage3D, void  ( GL_APIENTRY* )( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth ) );
	FIND_PROC( glGetInternalformativ, void  ( GL_APIENTRY* )( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params ) );
/*
 ; ANGLE Platform Implementation
	FIND_PROC(ANGLEPlatformCurrent);
	FIND_PROC(ANGLEPlatformInitialize);
	FIND_PROC(ANGLEPlatformShutdown);
*/
	return (
		(glActiveTexture != nullptr) &&
		(glAttachShader != nullptr) &&
		(glBindAttribLocation != nullptr) &&
		(glBindBuffer != nullptr) &&
		(glBindFramebuffer != nullptr) &&
		(glBindRenderbuffer != nullptr) &&
		(glBindTexture != nullptr) &&
		(glBlendColor != nullptr) &&
		(glBlendEquation != nullptr) &&
		(glBlendEquationSeparate != nullptr) &&
		(glBlendFunc != nullptr) &&
		(glBlendFuncSeparate != nullptr) &&
		(glBufferData != nullptr) &&
		(glBufferSubData != nullptr) &&
		(glCheckFramebufferStatus != nullptr) &&
		(glClear != nullptr) &&
		(glClearColor != nullptr) &&
		(glClearDepthf != nullptr) &&
		(glClearStencil != nullptr) &&
		(glColorMask != nullptr) &&
		(glCompileShader != nullptr) &&
		(glCompressedTexImage2D != nullptr) &&
		(glCompressedTexSubImage2D != nullptr) &&
		(glCopyTexImage2D != nullptr) &&
		(glCopyTexSubImage2D != nullptr) &&
		(glCreateProgram != nullptr) &&
		(glCreateShader != nullptr) &&
		(glCullFace != nullptr) &&
		(glDeleteBuffers != nullptr) &&
		(glDeleteFramebuffers != nullptr) &&
		(glDeleteProgram != nullptr) &&
		(glDeleteRenderbuffers != nullptr) &&
		(glDeleteShader != nullptr) &&
		(glDeleteTextures != nullptr) &&
		(glDepthFunc != nullptr) &&
		(glDepthMask != nullptr) &&
		(glDepthRangef != nullptr) &&
		(glDetachShader != nullptr) &&
		(glDisable != nullptr) &&
		(glDisableVertexAttribArray != nullptr) &&
		(glDrawArrays != nullptr) &&
		(glDrawElements != nullptr) &&
		(glEnable != nullptr) &&
		(glEnableVertexAttribArray != nullptr) &&
		(glFinish != nullptr) &&
		(glFlush != nullptr) &&
		(glFramebufferRenderbuffer != nullptr) &&
		(glFramebufferTexture2D != nullptr) &&
		(glFrontFace != nullptr) &&
		(glGenBuffers != nullptr) &&
		(glGenFramebuffers != nullptr) &&
		(glGenRenderbuffers != nullptr) &&
		(glGenTextures != nullptr) &&
		(glGenerateMipmap != nullptr) &&
		(glGetActiveAttrib != nullptr) &&
		(glGetActiveUniform != nullptr) &&
		(glGetAttachedShaders != nullptr) &&
		(glGetAttribLocation != nullptr) &&
		(glGetBooleanv != nullptr) &&
		(glGetBufferParameteriv != nullptr) &&
		(glGetError != nullptr) &&
		(glGetFloatv != nullptr) &&
		(glGetFramebufferAttachmentParameteriv != nullptr) &&
		(glGetIntegerv != nullptr) &&
		(glGetProgramInfoLog != nullptr) &&
		(glGetProgramiv != nullptr) &&
		(glGetRenderbufferParameteriv != nullptr) &&
		(glGetShaderInfoLog != nullptr) &&
		(glGetShaderPrecisionFormat != nullptr) &&
		(glGetShaderSource != nullptr) &&
		(glGetShaderiv != nullptr) &&
		(glGetString != nullptr) &&
		(glGetTexParameterfv != nullptr) &&
		(glGetTexParameteriv != nullptr) &&
		(glGetUniformLocation != nullptr) &&
		(glGetUniformfv != nullptr) &&
		(glGetUniformiv != nullptr) &&
		(glGetVertexAttribPointerv != nullptr) &&
		(glGetVertexAttribfv != nullptr) &&
		(glGetVertexAttribiv != nullptr) &&
		(glHint != nullptr) &&
		(glIsBuffer != nullptr) &&
		(glIsEnabled != nullptr) &&
		(glIsFramebuffer != nullptr) &&
		(glIsProgram != nullptr) &&
		(glIsRenderbuffer != nullptr) &&
		(glIsShader != nullptr) &&
		(glIsTexture != nullptr) &&
		(glLineWidth != nullptr) &&
		(glLinkProgram != nullptr) &&
		(glPixelStorei != nullptr) &&
		(glPolygonOffset != nullptr) &&
		(glReadPixels != nullptr) &&
		(glReleaseShaderCompiler != nullptr) &&
		(glRenderbufferStorage != nullptr) &&
		(glSampleCoverage != nullptr) &&
		(glScissor != nullptr) &&
		(glShaderBinary != nullptr) &&
		(glShaderSource != nullptr) &&
		(glStencilFunc != nullptr) &&
		(glStencilFuncSeparate != nullptr) &&
		(glStencilMask != nullptr) &&
		(glStencilMaskSeparate != nullptr) &&
		(glStencilOp != nullptr) &&
		(glStencilOpSeparate != nullptr) &&
		(glTexImage2D != nullptr) &&
		(glTexParameterf != nullptr) &&
		(glTexParameterfv != nullptr) &&
		(glTexParameteri != nullptr) &&
		(glTexParameteriv != nullptr) &&
		(glTexSubImage2D != nullptr) &&
		(glUniform1f != nullptr) &&
		(glUniform1fv != nullptr) &&
		(glUniform1i != nullptr) &&
		(glUniform1iv != nullptr) &&
		(glUniform2f != nullptr) &&
		(glUniform2fv != nullptr) &&
		(glUniform2i != nullptr) &&
		(glUniform2iv != nullptr) &&
		(glUniform3f != nullptr) &&
		(glUniform3fv != nullptr) &&
		(glUniform3i != nullptr) &&
		(glUniform3iv != nullptr) &&
		(glUniform4f != nullptr) &&
		(glUniform4fv != nullptr) &&
		(glUniform4i != nullptr) &&
		(glUniform4iv != nullptr) &&
		(glUniformMatrix2fv != nullptr) &&
		(glUniformMatrix3fv != nullptr) &&
		(glUniformMatrix4fv != nullptr) &&
		(glUseProgram != nullptr) &&
		(glValidateProgram != nullptr) &&
		(glVertexAttrib1f != nullptr) &&
		(glVertexAttrib1fv != nullptr) &&
		(glVertexAttrib2f != nullptr) &&
		(glVertexAttrib2fv != nullptr) &&
		(glVertexAttrib3f != nullptr) &&
		(glVertexAttrib3fv != nullptr) &&
		(glVertexAttrib4f != nullptr) &&
		(glVertexAttrib4fv != nullptr) &&
		(glVertexAttribPointer != nullptr) &&
		(glViewport != nullptr) &&
		(glReadBuffer != nullptr) &&
		(glDrawRangeElements != nullptr) &&
		(glTexImage3D != nullptr) &&
		(glTexSubImage3D != nullptr) &&
		(glCopyTexSubImage3D != nullptr) &&
		(glCompressedTexImage3D != nullptr) &&
		(glCompressedTexSubImage3D != nullptr) &&
		(glGenQueries != nullptr) &&
		(glDeleteQueries != nullptr) &&
		(glIsQuery != nullptr) &&
		(glBeginQuery != nullptr) &&
		(glEndQuery != nullptr) &&
		(glGetQueryiv != nullptr) &&
		(glGetQueryObjectuiv != nullptr) &&
		(glUnmapBuffer != nullptr) &&
		(glGetBufferPointerv != nullptr) &&
		(glDrawBuffers != nullptr) &&
		(glUniformMatrix2x3fv != nullptr) &&
		(glUniformMatrix3x2fv != nullptr) &&
		(glUniformMatrix2x4fv != nullptr) &&
		(glUniformMatrix4x2fv != nullptr) &&
		(glUniformMatrix3x4fv != nullptr) &&
		(glUniformMatrix4x3fv != nullptr) &&
		(glBlitFramebuffer != nullptr) &&
		(glRenderbufferStorageMultisample != nullptr) &&
		(glFramebufferTextureLayer != nullptr) &&
		(glMapBufferRange != nullptr) &&
		(glFlushMappedBufferRange != nullptr) &&
		(glBindVertexArray != nullptr) &&
		(glDeleteVertexArrays != nullptr) &&
		(glGenVertexArrays != nullptr) &&
		(glIsVertexArray != nullptr) &&
		(glGetIntegeri_v != nullptr) &&
		(glBeginTransformFeedback != nullptr) &&
		(glEndTransformFeedback != nullptr) &&
		(glBindBufferRange != nullptr) &&
		(glBindBufferBase != nullptr) &&
		(glTransformFeedbackVaryings != nullptr) &&
		(glGetTransformFeedbackVarying != nullptr) &&
		(glVertexAttribIPointer != nullptr) &&
		(glGetVertexAttribIiv != nullptr) &&
		(glGetVertexAttribIuiv != nullptr) &&
		(glVertexAttribI4i != nullptr) &&
		(glVertexAttribI4ui != nullptr) &&
		(glVertexAttribI4iv != nullptr) &&
		(glVertexAttribI4uiv != nullptr) &&
		(glGetUniformuiv != nullptr) &&
		(glGetFragDataLocation != nullptr) &&
		(glUniform1ui != nullptr) &&
		(glUniform2ui != nullptr) &&
		(glUniform3ui != nullptr) &&
		(glUniform4ui != nullptr) &&
		(glUniform1uiv != nullptr) &&
		(glUniform2uiv != nullptr) &&
		(glUniform3uiv != nullptr) &&
		(glUniform4uiv != nullptr) &&
		(glClearBufferiv != nullptr) &&
		(glClearBufferuiv != nullptr) &&
		(glClearBufferfv != nullptr) &&
		(glClearBufferfi != nullptr) &&
		(glGetStringi != nullptr) &&
		(glCopyBufferSubData != nullptr) &&
		(glGetUniformIndices != nullptr) &&
		(glGetActiveUniformsiv != nullptr) &&
		(glGetUniformBlockIndex != nullptr) &&
		(glGetActiveUniformBlockiv != nullptr) &&
		(glGetActiveUniformBlockName != nullptr) &&
		(glUniformBlockBinding != nullptr) &&
		(glDrawArraysInstanced != nullptr) &&
		(glDrawElementsInstanced != nullptr) &&
		(glFenceSync != nullptr) &&
		(glIsSync != nullptr) &&
		(glDeleteSync != nullptr) &&
		(glClientWaitSync != nullptr) &&
		(glWaitSync != nullptr) &&
		(glGetInteger64v != nullptr) &&
		(glGetSynciv != nullptr) &&
		(glGetInteger64i_v != nullptr) &&
		(glGetBufferParameteri64v != nullptr) &&
		(glGenSamplers != nullptr) &&
		(glDeleteSamplers != nullptr) &&
		(glIsSampler != nullptr) &&
		(glBindSampler != nullptr) &&
		(glSamplerParameteri != nullptr) &&
		(glSamplerParameteriv != nullptr) &&
		(glSamplerParameterf != nullptr) &&
		(glSamplerParameterfv != nullptr) &&
		(glGetSamplerParameteriv != nullptr) &&
		(glGetSamplerParameterfv != nullptr) &&
		(glVertexAttribDivisor != nullptr) &&
		(glBindTransformFeedback != nullptr) &&
		(glDeleteTransformFeedbacks != nullptr) &&
		(glGenTransformFeedbacks != nullptr) &&
		(glIsTransformFeedback != nullptr) &&
		(glPauseTransformFeedback != nullptr) &&
		(glResumeTransformFeedback != nullptr) &&
		(glGetProgramBinary != nullptr) &&
		(glProgramBinary != nullptr) &&
		(glProgramParameteri != nullptr) &&
		(glInvalidateFramebuffer != nullptr) &&
		(glInvalidateSubFramebuffer != nullptr) &&
		(glTexStorage2D != nullptr) &&
		(glTexStorage3D != nullptr) &&
		( glGetInternalformativ != nullptr ) );
//		(ANGLEPlatformCurrent != nullptr) &&
//		(ANGLEPlatformInitialize != nullptr) &&
//		(ANGLEPlatformShutdown != nullptr) );
}
//---------------------------------------------------------------------------
static bool TVPANGLEInit = false;
int TVPOpenGLESVersion = 200;
//---------------------------------------------------------------------------
void TVPInitializeOpenGLPlatform() {
	if( TVPANGLEInit == false ) {
		tjs_string path = ExePath();
#ifdef TJS_64BIT_OS
		path = ExtractFilePath( path ) + TJS_W("plugin64\\");
#else
		path = ExtractFilePath( path ) + TJS_W("plugin\\");
#endif
		TCHAR oldCurDir[MAX_PATH];
		::GetCurrentDirectory( sizeof( oldCurDir ) / sizeof( oldCurDir[0] ), oldCurDir );
		::SetCurrentDirectory( path.c_str() );
		bool gles = LoadLibGLESv2( path );
		bool egl = LoadLibEGL( path );
		if( gles == false || egl == false ) {
			::SetCurrentDirectory( oldCurDir );
			TVPThrowExceptionMessage(TJS_W("Failed to load ANGLE."));
		} else {
			TVPANGLEInit = true;
			TVPOpenGLESVersion = 300;
		}
		::SetCurrentDirectory( oldCurDir );
	}
}
//---------------------------------------------------------------------------
static void TVPUninitializeANGLE() {
	if( TVPhModuleLibEGL ) {
		::FreeLibrary( TVPhModuleLibEGL );
		TVPhModuleLibEGL = nullptr;
	}
	if( TVPhModuleLibGLESv2 ) {
		TVPhModuleLibGLESv2 = nullptr;
		::FreeLibrary( TVPhModuleLibGLESv2 );
	}
}
//---------------------------------------------------------------------------
int TVPGetOpenGLESVersion() { return TVPOpenGLESVersion; }
//---------------------------------------------------------------------------
static tTVPAtExit TVPUninitANGLEAtExit
	(TVP_ATEXIT_PRI_SHUTDOWN, TVPUninitializeANGLE);
//---------------------------------------------------------------------------
void* TVPeglGetProcAddress(const char * procname) {
	return eglGetProcAddress(procname);
}
//---------------------------------------------------------------------------
