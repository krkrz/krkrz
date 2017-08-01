package jp.kirikiri.krkrz;

import  android.opengl.GLES20;
import  android.os.Build;
import  android.graphics.SurfaceTexture;
import  javax.microedition.khronos.egl.EGL10;
import  javax.microedition.khronos.egl.EGLConfig;
import  javax.microedition.khronos.egl.EGLContext;
import  javax.microedition.khronos.egl.EGLDisplay;
import  javax.microedition.khronos.egl.EGLSurface;
import  android.util.Log;

// Android 4.2 (API Level 17) or later
import static android.opengl.EGL14.EGL_CONTEXT_CLIENT_VERSION;
import static android.opengl.EGL14.EGL_OPENGL_ES2_BIT;

// http://wlog.flatlib.jp/item/1669
public class GPUType {
    public static int       VersionCode= 0; // 200 or 300
    public static String    VersionString;
    public static String    RendererString;
    private static int      AdrenoID= 0;

    public static boolean isGLES3() {
        return  VersionCode == 300;
    }

	public static int getVersionCode() {
		return VersionCode;
	}

    public static boolean isAdreno300s() {
        return  AdrenoID >= 300 && AdrenoID < 400;
    }

    private static int decodeAdrenoID( String gpu_name ) {
        if( gpu_name.equals( "Adreno" ) || gpu_name.indexOf( "AMD Z430" ) >= 0 ){
            return  200;
        }else if( gpu_name.indexOf( "Adreno" ) >= 0 ){
            return  getAdrenoNumber( gpu_name );
        }
        return  0;
    }

    public static void  update() {
        EGL10   egl = (EGL10)EGLContext.getEGL();
        int[]   iparam = new int[8];
        if( Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB ){
            VersionCode = 200;
            return;
        }

        // Initialize
        EGLDisplay  display = egl.eglGetDisplay( EGL10.EGL_DEFAULT_DISPLAY );
        egl.eglInitialize( display, iparam );

        // Config
        int[] config_attr = { EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL10.EGL_NONE };
        final int CONFIG_SIZE = 1;
        EGLConfig[] config_array = new EGLConfig[CONFIG_SIZE];
        egl.eglChooseConfig( display, config_attr, config_array, CONFIG_SIZE, iparam );
        EGLConfig config = config_array[0];

        // Surface
        SurfaceTexture surfaceTexture = new SurfaceTexture( 0 );    // API Level 11
        EGLSurface surface = egl.eglCreateWindowSurface( display, config, surfaceTexture, null );

        // Context
        int[] context_attr = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
        EGLContext context= egl.eglCreateContext( display, config, EGL10.EGL_NO_CONTEXT, context_attr );

        // Make Current
        egl.eglMakeCurrent( display, surface, surface, context );

        // Query
        VersionString = GLES20.glGetString( GLES20.GL_VERSION );
        RendererString = GLES20.glGetString( GLES20.GL_RENDERER );
        VersionCode = getGLVersionString( VersionString );
        AdrenoID = decodeAdrenoID( RendererString );

        // Special
        if( isAdreno300s() ){
            if( Build.VERSION.SDK_INT <= Build.VERSION_CODES.JELLY_BEAN_MR2 ){ // .. Android 4.3
                VersionCode = 200;   // GLES 2.0
            }
        }

        // Release
        egl.eglMakeCurrent( display, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT );
        egl.eglDestroyContext( display, context );
        egl.eglDestroySurface( display, surface );
    }

    private static int getGLVersionString( String version_string ) {
        int length = version_string.length();
        int ci = 0;
        for( ; ci < length; ci++ ) {
            char ch = version_string.charAt( ci );
            if( ch >= '0' && ch <= '9' ){
                break;
            }
        }
        int version = 0;
        int num_shift = 100;
        for( ; num_shift > 0 && ci < length; ) {
            char ch = version_string.charAt( ci++ );
            if( ch >= '0' && ch <= '9' ) {
                if( num_shift == 100 ) {
                    if( version_string.charAt( ci ) == '.' ) {
                        ci++;
                    }
                }
                version += (ch - '0') * num_shift;
                num_shift /= 10;
            } else {
                break;
            }
        }
        return  version;
    }

    private static int getAdrenoNumber( String text ) {
        int length = text.length();
        int num = 0;
        boolean is_num = false;
        for( int i = 0 ; i < length; i++ ){
            char ch = text.charAt( i );
            if( is_num ){
                if( ch >= '0' && ch <= '9' ){
                    num *= 10;
                    num += ch - '0';
                } else {
                    break;
                }
            } else {
                if( ch >= '0' && ch <= '9' ){
                    is_num = true;
                    num = ch - '0';
                }
            }
        }
        return  num;
    }
}
