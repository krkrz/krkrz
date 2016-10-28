package jp.kirikiri.krkrz;

import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import java.util.Locale;

public class MainActivity extends Activity  implements SurfaceHolder.Callback {
    private static String TAG = "KrkrZActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initializeNative();

        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(this);
        surfaceView.getHolder().setFormat(PixelFormat.RGBX_8888 );
        // surfaceView.setOnClickListener(new View.OnClickListener() { public void onClick(View view) {} });
    }

	private void initializeNative() {
        System.loadLibrary("krkrz");
        nativeSetActivity( this );
        nativeSetAssetManager( getResources().getAssets() );
	}
	@Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.i(TAG, "onSaveInstanceState()");
    }
	@Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
        Log.i(TAG, "onConfigurationChanged()");
	}
    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "onStart()");
        nativeToMessage(EventCode.AM_START,0,0);
    }

    @Override
	protected void onRestart() {
        super.onRestart();
        nativeToMessage(EventCode.AM_RESTART,0,0);
	}

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume()");
        nativeToMessage(EventCode.AM_RESUME,0,0);
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "onPause()");
        nativeToMessage(EventCode.AM_PAUSE,0,0);
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG, "onStop()");
        nativeToMessage(EventCode.AM_STOP,0,0);
    }
    @Override
    protected void onDestroy() {
    	super.onDestroy();
        nativeToMessage(EventCode.AM_DESTROY,0,0);
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        nativeSetSurface(holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder) {
    }
    public void surfaceDestroyed(SurfaceHolder holder) {
        nativeSetSurface(null);
    }
	// *** native から呼び出すよう関数
    public String getMessageRes( int id ) {
        String[] mes  = getResources().getStringArray(R.array.system_message_resource);
        return mes[id];
    }
	public String getLanguage() {
		Locale locale = Locale.getDefault();
		return locale.getLanguage();
	}
    public String getCachePath() {
        // getCacheDir() の場合は内部、どちらでも取得できるほうがいいか？
        return getExternalCacheDir().toString();
    }
    public String getInternalDataPath() {
        return getFilesDir().toString();
    }
    public String getExternalDataPath() {
        return getExternalFilesDir(null).toString();
    }

	// *** native 関数列挙
	public static native void nativeToMessage(int mes, long wparam, long lparam );
    public static native void nativeSetSurface(Surface surface);
	public static native void nativeSetAssetManager(AssetManager am);
    public static native void nativeSetActivity( Activity activity );
}
