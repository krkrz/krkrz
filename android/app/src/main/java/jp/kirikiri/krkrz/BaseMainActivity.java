package jp.kirikiri.krkrz;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.ParcelFileDescriptor;
import android.support.annotation.RequiresApi;
import android.support.v4.provider.DocumentFile;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;

import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.extractor.DefaultExtractorsFactory;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.trackselection.AdaptiveTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;
import com.google.android.exoplayer2.ui.AspectRatioFrameLayout;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.util.Util;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Locale;

public class BaseMainActivity extends Activity  implements SurfaceHolder.Callback, ExoPlayer.EventListener, ExtractorMediaSource.EventListener,
        SimpleExoPlayer.VideoListener {
    private static String TAG = "KrkrZActivity";
    private static String LOGTAG = "krkrz";
    private static boolean LOGDMOV = BuildConfig.DEBUG;
    private static boolean LOGDSTAT = BuildConfig.DEBUG;

    static final int READ_DOCUMENT_REQUEST_CODE = 1;
    static final int SELECT_TREE_REQUEST_CODE = 2;
    static final int CREATE_DOCUMENT_REQUEST_CODE = 3;
    static int AudioOptimalSampleRate = 44100;
    static int AudioOptimalBufferSize = 256;

    private Handler mHandler;

    private SimpleExoPlayer mPlayer;
    private static final DefaultBandwidthMeter BANDWIDTH_METER = new DefaultBandwidthMeter();
    private DefaultTrackSelector trackSelector;
    private DataSource.Factory mMediaDataSourceFactory;


    class FinishEvent implements Runnable {
        @Override public void run() {
            finish();
        }
    }
    class SurfaceSizeChangeEvent implements Runnable {
        private int mWidth;
        private int mHeight;
        public SurfaceSizeChangeEvent( int w, int h ) { mWidth = w; mHeight = h; }
        @Override
        public void run() {
            SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
            surfaceView.getHolder().setFixedSize(mWidth, mHeight);
        }
    }
    class ActivityTitleChangeEvent implements Runnable {
        private String mTitle;
        public ActivityTitleChangeEvent( String title ) { mTitle = title; }
        @Override
        public void run() {
            setTitle( mTitle );
        }
    }
    class ShowToastEvent implements Runnable {
        private String mMessage;
        public ShowToastEvent( String title ) { mMessage = title; }
        @Override
        public void run() {
            Toast.makeText(BaseMainActivity.this,mMessage,Toast.LENGTH_LONG).show();
        }
    }
    class OpenMovieEvent implements Runnable {
        private String mPath;
        public OpenMovieEvent( String path ) { mPath = path; }
        @Override
        public void run() { if( openMovie(mPath) != null ) {nativeToMessage(EventCode.AM_MOVIE_LOAD_ERROR,0,0);} }
    }
    class PlayMovieEvent implements Runnable {
        @Override
        public void run() { playMovie();}
    }
    class PauseMovieEvent implements Runnable {
        @Override public void run() { pauseMovie(); }
    }
    class StopMovieEvent implements Runnable {
        @Override public void run() { stopMovie(); }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mHandler = new Handler();

        // フルスクリーン
        View decor = this.getWindow().getDecorView();
        decor.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION  | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_IMMERSIVE);

        // ハードウェアボタンでミュージック音量を変更可能に
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        // Audio Output latency
        // https://googlesamples.github.io/android-audio-high-performance/guides/audio-output-latency.html
        AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String frameRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        AudioOptimalSampleRate = Integer.parseInt(frameRate);
        if(AudioOptimalSampleRate == 0) AudioOptimalSampleRate = 44100; // Use a default value if property not found
        String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        AudioOptimalBufferSize = Integer.parseInt(framesPerBuffer);
        if(AudioOptimalBufferSize == 0) AudioOptimalBufferSize = 256; // Use default

    	GPUType.update();
        initializeNative();

        setContentView(R.layout.activity_main);

        mMediaDataSourceFactory = new DefaultDataSourceFactory(getApplication(), Util.getUserAgent(this, "AppName"));

        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(this);
        surfaceView.getHolder().setFormat(PixelFormat.RGBX_8888 );
        // surfaceView.setOnClickListener(new View.OnClickListener() { public void onClick(View view) {} });
    }

	private void initializeNative() {
        System.loadLibrary("krkrz");
        nativeSetSoundNativeParameter( AudioOptimalSampleRate, AudioOptimalBufferSize );
        nativeSetActivity( this );
        Resources res = getResources();
        nativeSetAssetManager( res.getAssets() );
        String[] mes = res.getStringArray(R.array.system_message_resource);
        nativeSetMessageResource( mes );

		// Execute startup.tjs
		nativeInitialize();
	}
	@Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if( LOGDSTAT ) Log.i(LOGTAG, "onSaveInstanceState()");
    }
	@Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        if( LOGDSTAT ) Log.i(LOGTAG, "onRestoreInstanceState()");
    }

    @Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
        if( LOGDSTAT ) Log.i(LOGTAG, "onConfigurationChanged()");
		
		// update configuration
        Resources res = getResources();
        nativeSetAssetManager( res.getAssets() );
	}
	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
        if( LOGDSTAT ) Log.i(LOGTAG, "onWindowFocusChanged()");
	}
    @Override
    protected void onStart() {
        super.onStart();
        if( LOGDSTAT ) Log.i(LOGTAG, "onStart()");
        nativeToMessage(EventCode.AM_START,0,0);
    }
    @Override
	protected void onRestart() {
        super.onRestart();
        if( LOGDSTAT ) Log.i(LOGTAG, "onRestart()");
        nativeToMessage(EventCode.AM_RESTART,0,0);
	}
    @Override
    protected void onResume() {
        super.onResume();
        if( LOGDSTAT ) Log.i(LOGTAG, "onResume()");
        nativeToMessage(EventCode.AM_RESUME,0,0);
    }
    @Override
    protected void onPause() {
        super.onPause();
        if( LOGDSTAT ) Log.i(LOGTAG, "onPause()");
        nativeToMessage(EventCode.AM_PAUSE,0,0);
        if( mPlayer != null ) {
            mPlayer.release();
            mPlayer = null;
        }
    }
    @Override
    protected void onStop() {
        super.onStop();
        if( LOGDSTAT ) Log.i(LOGTAG, "onStop()");
        nativeToMessage(EventCode.AM_STOP,0,0);
    }
    @Override
    protected void onDestroy() {
    	super.onDestroy();
        if( LOGDSTAT ) Log.i(LOGTAG, "onDestroy()");
        nativeToMessage(EventCode.AM_DESTROY,0,0);
        nativeSetActivity( null );
    }

    /**
     * ゲームパッドなどの十字キー入力のキーコードを変換する
     * @param : keyCode 入力キーコード
     * @param : event 入力キーイベント
     * @return 変換後のキーコード
     */
    /*
    private int keyCodeTranslate( int keyCode, KeyEvent event ) {
        if( event != null ) {
            InputDevice device = event.getDevice();
            if( device != null ) {
                int src = device.getSources();
                if( (src & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD ||
                        (src & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD ||
                        (src & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK ) {
                    switch(keyCode){
                        case KeyEvent.KEYCODE_DPAD_LEFT:
                            return VirtualKey.VK_PADLEFT;
                        case KeyEvent.KEYCODE_DPAD_RIGHT:
                            return VirtualKey.VK_PADRIGHT;
                        case KeyEvent.KEYCODE_DPAD_UP:
                            return VirtualKey.VK_PADUP;
                        case KeyEvent.KEYCODE_DPAD_DOWN:
                            return VirtualKey.VK_PADDOWN;
                    }
                    return keyCode;
                } else { // InputDevice.SOURCE_KEYBOARD
                    return keyCode;
                }
            }
        }
        return keyCode;
    }
    */
    private boolean isHandleKeyCode(int keyCode ) {
        return keyCode != KeyEvent.KEYCODE_HOME && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN &&
                keyCode != KeyEvent.KEYCODE_VOLUME_UP && keyCode != KeyEvent.KEYCODE_VOLUME_MUTE;
    }
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if( isHandleKeyCode( keyCode ) ) {
            int meta = getModifiersToInt(event.getMetaState(), false);
            //keyCode = keyCodeTranslate(keyCode, event);
            nativeToMessage(EventCode.AM_KEY_DOWN, keyCode, meta );
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }
    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        return super.onKeyMultiple(keyCode, repeatCount, event);
    }
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if( isHandleKeyCode( keyCode ) ) {
            int meta = getModifiersToInt(event.getMetaState(), false);
            nativeToMessage(EventCode.AM_KEY_UP, keyCode, meta);
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onGenericMotionEvent( MotionEvent event ) {
        // ジョイスティック、ゲームパッド、マウス、トラックボール イベント
        // 現在のところ実装していない
        int action = event.getActionMasked();
        int meta = getModifiersToInt( event.getMetaState(), event.getDownTime() > 0 );
        switch( action ) {
            case MotionEvent.ACTION_HOVER_MOVE: // 非押下移動も判定出来るようにする
                meta = getModifiersToInt( event.getMetaState(), false );
                break;
            case MotionEvent.ACTION_OUTSIDE:
                meta = getModifiersToInt( event.getMetaState(), false );
                break;
        }
        int type = getMouseToolType( event.getToolType(0) );
        switch( type ) {
            case MotionEvent.TOOL_TYPE_MOUSE: //	マウス
            case MotionEvent.TOOL_TYPE_STYLUS: //	スタイラス
                int btn = getMouseButtonNumber( event.getButtonState() );
                break;
            case MotionEvent.TOOL_TYPE_FINGER: //	指
            case MotionEvent.TOOL_TYPE_ERASER: //	消しゴム
            case MotionEvent.TOOL_TYPE_UNKNOWN: //	その他
            default:
                break;
        }
        return super.onGenericMotionEvent(event);
    }
    public static final int TOUCH_DOWN = 0;
    public static final int TOUCH_MOVE = 1;
    public static final int TOUCH_UP = 2;
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //int action = event.getAction();
        //int index = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
        int index = event.getActionIndex();
        int eventID = event.getPointerId(index);

        int action = event.getActionMasked();
        //int meta = getModifiersToInt( event.getMetaState(), event.getDownTime() > 0 );
        float x = event.getX();
        float y = event.getY();
        long tick = event.getEventTime();
        float c = event.getSize();
        int count = 0;
        switch( action ) {
            case MotionEvent.ACTION_DOWN:
                // meta = getModifiersToInt( event.getMetaState(), true );
                nativeOnTouch( TOUCH_DOWN, x, y, c, eventID, tick );
                break;
            case MotionEvent.ACTION_POINTER_DOWN: // マルチタッチ
                count = event.getPointerCount();
                for(int i = 0; i < count; i++ ) {
                    int pid = event.getPointerId(i);
                    x = event.getX(i);
                    y = event.getY(i);
                    c = event.getSize(i);
                    nativeOnTouch( TOUCH_DOWN, x, y, c, pid, tick );
                }
                break;
            case MotionEvent.ACTION_POINTER_UP: // マルチタッチ
                count = event.getPointerCount();
                for(int i = 0; i < count; i++ ) {
                    int pid = event.getPointerId(i);
                    x = event.getX(i);
                    y = event.getY(i);
                    c = event.getSize(i);
                    nativeOnTouch( TOUCH_UP, x, y, c, pid, tick );
                }
                break;
            case MotionEvent.ACTION_UP:
                nativeOnTouch( TOUCH_UP, x, y, c, eventID, tick );
                //meta = getModifiersToInt( event.getMetaState(), false );
                break;
            case MotionEvent.ACTION_MOVE: {
                //meta = getModifiersToInt(event.getMetaState(), true);
                count = event.getPointerCount();
                for(int i = 0; i < count; i++ ) {
                    int pid = event.getPointerId(i);
                    x = event.getX(i);
                    y = event.getY(i);
                    c = event.getSize(i);
                    nativeOnTouch( TOUCH_MOVE, x, y, c, pid, tick );
                    int hsize = event.getHistorySize();
                    for( int j = 0; j < hsize; j++ ) {
                        x = event.getHistoricalX(i, j);
                        y = event.getHistoricalY(i, j);
                        c = event.getHistoricalSize(i, j);
                        tick = event.getHistoricalEventTime(j);
                        nativeOnTouch( TOUCH_MOVE, x, y, c, pid, tick );
                    }
                }
                break;
            }
            case MotionEvent.ACTION_CANCEL:
                // 押されたが即座に離された場合は無視扱いしておく
                // meta = getModifiersToInt( event.getMetaState(), false );
                break;
        }
        return true;
    }
    public static final int SS_SHIFT   = 0x01;
    public static final int SS_ALT     = 0x02;
    public static final int SS_CTRL    = 0x04;
    public static final int SS_LEFT    = 0x08;
    public static final int SS_RIGHT   = 0x10;
    public static final int SS_MIDDLE  = 0x20;
    public static final int SS_DOUBLE  = 0x40;
    public static final int SS_REPEAT  = 0x80;
    public static final int SS_ALTGRAPH= 0x100; // AltGraph キー
    public static final int SS_META    = 0x200; // Meta キー
    private static final int META_ALT_MASK = KeyEvent.META_ALT_ON|KeyEvent.META_ALT_LEFT_ON|KeyEvent.META_ALT_RIGHT_ON; // API 11
    private static final int META_SHIFT_MASK = KeyEvent.META_SHIFT_ON|KeyEvent.META_SHIFT_LEFT_ON|KeyEvent.META_SHIFT_RIGHT_ON; // API 11
    private static final int META_CTRL_MASK = 0x00007000; // API 11
    private static final int META_META_MASK = 0x00070000; // API 11
    private static int getModifiersToInt( int meta, boolean ispressing ) {
        int f = 0;
        if( (meta & META_ALT_MASK) != 0 ) { f += SS_ALT; }
        if( (meta & META_SHIFT_MASK) != 0 ) { f += SS_SHIFT; }
        if( (meta & META_CTRL_MASK) != 0 ) { f += SS_CTRL; }
        if( (meta & META_META_MASK) != 0 ) { f += SS_META; }
        if( ispressing ) { f += SS_LEFT; }
        return f;
    }
    public static final int mbLeft   = 0;
    public static final int mbRight  = 1;
    public static final int mbMiddle = 2;
    public static final int mbX1      = 3;
    public static final int mbX2      = 4;
    public static final int mbForward   = 5;
    public static final int mbBack      = 6;
    public static final int mbStylusPrimary     = 7;
    public static final int mbStylusSecondary   = 8;
    private static int getMouseButtonNumber( int state ) {
        if( (state & MotionEvent.BUTTON_PRIMARY) != 0 ) { return mbLeft; /* 左クリック */ }
        if( (state & MotionEvent.BUTTON_SECONDARY) != 0 ) { return mbRight; /* 右クリック */ }
        if( (state & MotionEvent.BUTTON_TERTIARY) != 0 ) { return mbMiddle; /* 真ん中クリック */ }
        if( (state & MotionEvent.BUTTON_FORWARD) != 0 ) { return mbForward; /* マウスのホイールボタンを前に進める */ }
        if( (state & MotionEvent.BUTTON_BACK) != 0 ) { return mbBack; /* マウスのホイールボタンを後ろに進める */ }
        if( (state & MotionEvent.BUTTON_STYLUS_PRIMARY) != 0 ) { return mbStylusPrimary; /* スタイラス主 */ }
        if( (state & MotionEvent.BUTTON_STYLUS_SECONDARY) != 0 ) { return mbStylusSecondary; /* スタイラス副 */ }
        return -1;
    }
    private static int getMouseToolType( int type ) {
        switch( type ) {
            case MotionEvent.TOOL_TYPE_FINGER: //	指
            case MotionEvent.TOOL_TYPE_MOUSE: //	マウス
            case MotionEvent.TOOL_TYPE_STYLUS: //	スタイラス
            case MotionEvent.TOOL_TYPE_ERASER: //	消しゴム
            case MotionEvent.TOOL_TYPE_UNKNOWN: //	その他
        }
        return 0;
    }
    public void selectFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, SELECT_TREE_REQUEST_CODE);
    }
    // IntentでProviderを実装している呼びます。
    public void performFileSearch() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        startActivityForResult(intent, READ_DOCUMENT_REQUEST_CODE);
    }
    public void createFile( String filename ) {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_TITLE, filename );
        startActivityForResult(intent, CREATE_DOCUMENT_REQUEST_CODE );
    }
    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        if (requestCode == READ_DOCUMENT_REQUEST_CODE ) {
            if( resultCode == Activity.RESULT_OK ) {
                Uri uri = null;
                if (resultData != null) {
                    uri = resultData.getData();
                    DocumentFile doc = DocumentFile.fromSingleUri(this, uri);
                    String disp = doc.getName();
                    final int takeFlags = resultData.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    getContentResolver().takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION );
                    try {
                        ParcelFileDescriptor desc = getContentResolver().openFileDescriptor(uri, "r");
                        // String r = disp + ":" + nativeReadFile(desc);
                    } catch( FileNotFoundException e ) {
                    }
                }
            } else {
                // cancel
            }
        } else if( requestCode == SELECT_TREE_REQUEST_CODE ) {
            if( resultCode == Activity.RESULT_OK ) {
                Uri treeUri = resultData.getData();
                getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            }
        }
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        nativeSetSurface(holder.getSurface());
    }
    public void surfaceCreated(SurfaceHolder holder) {
    }
    public void surfaceDestroyed(SurfaceHolder holder) {
        nativeSetSurface(null);
    }
	// *** native から呼び出す用関数
    public String getMessageRes( int id ) {
        String[] mes  = getResources().getStringArray(R.array.system_message_resource);
        return mes[id];
    }
	public String getLanguage() {
        Locale locale = Locale.getDefault();
        return locale.getLanguage();
	}
    public String getCachePath() { return getCacheDir().toString(); }
    public String getExternalCachePath() { return getExternalCacheDir().toString(); }
    public String getObbPath() { return getObbDir().toString(); }
    public String getInternalDataPath() {
        return getFilesDir().toString();
    }
    public String getExternalDataPath() {
        return getExternalFilesDir(null).toString();
    }
    public String getSoPath() {
        ApplicationInfo appliInfo = null;
        try {
            appliInfo = getPackageManager().getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
        } catch (PackageManager.NameNotFoundException e) {}
        if( appliInfo != null ) {
            return appliInfo.nativeLibraryDir + "/";
        }
        return "/data/data/" + getPackageName() + "/lib/" ;
    }
    public String getDataDirectory() { return Environment.getDataDirectory().toString(); }
    public String getDownloadCacheDirectory() { return Environment.getDownloadCacheDirectory().toString(); }
    public String getExternalStorageDirectory() { return Environment.getExternalStorageDirectory().toString(); }
    public String getExternalStoragePublicDirectoryMusic() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC).toString();}
    public String getExternalStoragePublicDirectoryPodcasts() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PODCASTS).toString();}
    public String getExternalStoragePublicDirectoryRingtones() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_RINGTONES).toString();}
    public String getExternalStoragePublicDirectoryAlarms() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_ALARMS).toString();}
    public String getExternalStoragePublicDirectoryNotifications() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_NOTIFICATIONS).toString();}
    public String getExternalStoragePublicDirectoryPictures() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).toString();}
    public String getExternalStoragePublicDirectoryMovies() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES).toString();}
    public String getExternalStoragePublicDirectoryDownloads() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).toString();}
    public String getExternalStoragePublicDirectoryDCIM() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString();}
    public String getExternalStoragePublicDirectoryDocuments() { return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS).toString();}
    public String getRootDirectory() { return Environment.getRootDirectory().toString(); }
    public String getExternalStorageState() { return Environment.getExternalStorageState(); }
    public boolean isExternalStorageEmulated() { return Environment.isExternalStorageEmulated(); }
    public boolean isExternalStorageRemovable() { return Environment.isExternalStorageRemovable(); }
    public int getDisplayRotate() {
		int val = -1;
		Display d = getWindowManager().getDefaultDisplay();
		int rotation = d.getRotation();
		switch(rotation) {
		case Surface.ROTATION_0:
			val = 0;
			break;
		case Surface.ROTATION_90:
			val = 90;
			break;
		case Surface.ROTATION_180:
			val = 180;
			break;
		case Surface.ROTATION_270:
			val = 270;
			break;
		}
		return val;
	}
	public int getMainViewWidth() {
		View view = (View)findViewById(R.id.activity_main);
		return view.getWidth();
	}
	public int getMainViewHeight() {
		View view = (View)findViewById(R.id.activity_main);
		return view.getHeight();
	}
	public int getActivityWidth() {
		Display display = getWindowManager().getDefaultDisplay();
		Point point = new Point();
		display.getSize(point);
		return point.x;
	}
	public int getActivityHeight() {
		Display display = getWindowManager().getDefaultDisplay();
		Point point = new Point();
		display.getSize(point);
		return point.y;
	}
	public int getScreenWidth() {
        Display display = getWindowManager().getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        return point.x;
    }
    public int getScreenHeight() {
        Display display = getWindowManager().getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        return point.y;
    }
    public void postFinish() {
        mHandler.post( new FinishEvent() );
    }
    public void postChangeSurfaceSize( int w, int h ) { mHandler.post( new SurfaceSizeChangeEvent( w, h ) ); }
    public void postChangeCaption( String t ) { mHandler.post( new ActivityTitleChangeEvent(t) ); }
    public String getCaption() { return getTitle().toString(); }
    public void postShowToastMessage( String m ) { mHandler.post( new ShowToastEvent(m) ); }
    public void postOpenMovie( String path ) { mHandler.post( new OpenMovieEvent(path)); }
    public void postPlayMovie() { mHandler.post( new PlayMovieEvent()); }
    public void postStopMovie() { mHandler.post( new StopMovieEvent()); }
    public void postPauseMovie() { mHandler.post( new PauseMovieEvent()); }
    public int getOpenGLESVersionCode() { return GPUType.getVersionCode(); }

    /**  動画再生関係  **/
    private MediaSource buildMediaSource(Uri uri) {
        return new ExtractorMediaSource(uri, mMediaDataSourceFactory, new DefaultExtractorsFactory(), mHandler, this);
    }
    public String openMovie( String path ) {
        String result = null;
        stopMovie();
        try {
            TrackSelection.Factory videoTrackSelectionFactory = new AdaptiveTrackSelection.Factory(BANDWIDTH_METER);
            trackSelector = new DefaultTrackSelector(videoTrackSelectionFactory);
            mPlayer = ExoPlayerFactory.newSimpleInstance(this,trackSelector);
            mPlayer.addListener(this);
            SurfaceView surfaceView = (SurfaceView)findViewById(R.id.videosurfaceview);
            mPlayer.setVideoSurfaceView( surfaceView );
            mPlayer.setVideoListener(this);
            MediaSource mediaSource = buildMediaSource( Uri.parse(path) );
            mPlayer.prepare(mediaSource,true,true);
        } catch( Exception e) {
            finishMovie();
            result = e.getLocalizedMessage();
        }
        return result;
    }
    public void playMovie() {
        mPlayer.setPlayWhenReady(true);
    }
    public void pauseMovie() {
        if( mPlayer != null ) {
            mPlayer.setPlayWhenReady(false);
        }
    }
    public void stopMovie() {
        if( mPlayer != null ) {
            mPlayer.stop();
            finishMovie();
        }
    }
    public void finishMovie() {
        if( mPlayer != null ) {
            mPlayer.release();
            mPlayer = null;

            SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
            surfaceView.setVisibility(View.VISIBLE);
            View contentView = (View)findViewById(R.id.video_content_frame);
            contentView.setVisibility(View.INVISIBLE);
            nativeToMessage(EventCode.AM_SURFACE_PAINT_REQUEST,0,0);
        }
    }
    public long getMovieCurrentPosition() {
        if( mPlayer != null ) { return mPlayer.getCurrentPosition(); }
        return 0;
    }
    public void setMovieCurrentPosition( long ms ) {
        if( mPlayer != null ) mPlayer.seekTo(ms);
    }
    public long getMovieDuration() {
        if( mPlayer != null ) return mPlayer.getDuration();
        return 0;
    }
    public float getMovieVolume() {
        if( mPlayer != null ) return mPlayer.getVolume();
        return 0.0f;
    }
    public void setMovieVolume( float vol ) {
        if( mPlayer != null ) mPlayer.setVolume(vol);
    }
    public boolean getMovieVisible() {
        return findViewById(R.id.video_content_frame).getVisibility() == View.VISIBLE;
    }
    // Called when the timeline and/or manifest has been refreshed.
    @Override
    public void onTimelineChanged(Timeline timeline, Object manifest) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie timeline changed.");
    }
    // Called when the available or selected tracks change.
    @Override
    public void onTracksChanged(TrackGroupArray trackGroups, TrackSelectionArray trackSelections) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie trackes changed.");
    }
    // Called when the player starts or stops loading the source.
    @Override
    public void onLoadingChanged(boolean isLoading) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie loading changed.");
    }
    // Called when the value returned from either ExoPlayer.getPlayWhenReady() or ExoPlayer.getPlaybackState() changes.
    @Override
    public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie player state changed - " + Integer.toString(playbackState));
        switch( playbackState ) {
            case ExoPlayer.STATE_ENDED:
                nativeToMessage(EventCode.AM_MOVIE_ENDED,0,0);
                finishMovie();
                break;
            case ExoPlayer.STATE_BUFFERING:
                nativeToMessage(EventCode.AM_MOVIE_BUFFERING,0,0);
                break;
            case ExoPlayer.STATE_IDLE:
                nativeToMessage(EventCode.AM_MOVIE_IDLE,0,0);
                break;
            case ExoPlayer.STATE_READY:
                nativeToMessage(EventCode.AM_MOVIE_READY,0,0);
                break;
        }
    }
    // Called when an error occurs.
    @Override
    public void onPlayerError(ExoPlaybackException error) {
        Log.e(LOGTAG,"Movie player error : " + error.getMessage() );
        nativeToMessage(EventCode.AM_MOVIE_PLAYER_ERROR,0,0);
    }
    // Called when a position discontinuity occurs without a change to the timeline.
    @Override
    public void onPositionDiscontinuity() {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie position discontinuity.");
    }

    @Override
    public void onPlaybackParametersChanged(PlaybackParameters playbackParameters) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie playback parameter changed." + playbackParameters.toString());
    }
    // ExtractorMediaSource.EventListener
    @Override
    public void onLoadError(IOException error) {
        if( LOGDMOV ) Log.e(LOGTAG,"Movie player error : " + error.getMessage() );
        nativeToMessage(EventCode.AM_MOVIE_LOAD_ERROR,0,0);
    }
    @Override
    public void onVideoSizeChanged(int width, int height, int unappliedRotationDegrees, float pixelWidthHeightRatio) {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie video size changed.");
        AspectRatioFrameLayout contentView = (AspectRatioFrameLayout)findViewById(R.id.video_content_frame);
        float aspectRatio = height == 0 ? 1 : (width * pixelWidthHeightRatio) / height;
        contentView.setAspectRatio(aspectRatio);
    }
    @Override
    public void onRenderedFirstFrame() {
        if( LOGDMOV ) Log.i(LOGTAG,"Movie rendered first frame.");
        nativeToMessage(EventCode.AM_MOVIE_PLAY,0,0);
        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.setVisibility(View.INVISIBLE);
        View contentView = (View)findViewById(R.id.video_content_frame);
        contentView.setVisibility(View.VISIBLE);
    }

	// *** native 関数列挙
	public static native void nativeToMessage(int mes, long wparam, long lparam );
    public static native void nativeSetSurface(Surface surface);
	public static native void nativeSetAssetManager(AssetManager am);
    public static native void nativeSetActivity( Activity activity );
	public static native void nativeInitialize();
	public static native void nativeSetStartupPath( String path );
    public static native void nativeSetMessageResource(String[] mes);

    public static native void nativeOnTouch( int type, float x, float y, float c, int id, long tick );
    public static native void nativeSetSoundNativeParameter( int rate, int size );
    public static native void nativeStartScript();
}
