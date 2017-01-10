package jp.kirikiri.krkrz;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelFileDescriptor;
import android.support.v4.provider.DocumentFile;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.io.FileNotFoundException;
import java.util.Locale;

public class MainActivity extends Activity  implements SurfaceHolder.Callback {
    private static String TAG = "KrkrZActivity";

    static final int READ_DOCUMENT_REQUEST_CODE = 1;
    static final int SELECT_TREE_REQUEST_CODE = 2;
    static final int CREATE_DOCUMENT_REQUEST_CODE = 3;

    private Handler mHandler;

    private boolean mSelectedStartFolder;
    private boolean mOpenStartFolder;

    class FinishEvent implements Runnable {
        @Override
        public void run() {
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
            Toast.makeText(MainActivity.this,mMessage,Toast.LENGTH_LONG).show();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSelectedStartFolder = false;
        mOpenStartFolder = false;
        mHandler = new Handler();

        initializeNative();

        Intent intent = getIntent();
        Bundle bundle = intent.getExtras();
        if(bundle != null) {
            String path = bundle.getString("startup_path");
            nativeSetStartupPath(path);
        }

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

		// Execute startup.tjs
		nativeInitialize();
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
        if( mSelectedStartFolder == false ) {
            mSelectedStartFolder = true;
            mOpenStartFolder = true;
            selectFolder();
        }
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
        nativeSetActivity( null );
    }
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyDown(keyCode, event);
    }
    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        return super.onKeyMultiple(keyCode, repeatCount, event);
    }
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
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
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        if (requestCode == READ_DOCUMENT_REQUEST_CODE ) {
            if( resultCode == Activity.RESULT_OK ) {
                Uri uri = null;
                if (resultData != null) {
                    uri = resultData.getData();
                    DocumentFile doc = DocumentFile.fromSingleUri(this, uri);
                    String disp = doc.getName();
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                        final int takeFlags = resultData.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                        getContentResolver().takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION );
                        try {
                            ParcelFileDescriptor desc = getContentResolver().openFileDescriptor(uri, "r");
                            // String r = disp + ":" + nativeReadFile(desc);
                        } catch( FileNotFoundException e ) {
                        }
                    }
                }
            } else {
                // cancel
            }
        } else if( requestCode == SELECT_TREE_REQUEST_CODE ) {
            if( resultCode == Activity.RESULT_OK ) {
                Uri treeUri = resultData.getData();
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                    getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                }
                if( mOpenStartFolder ) {
                    // nativeSelectPath( treeUri.toString() + "/", this );
                    nativeSetStartupPath( treeUri.toString() + "/" );
                }
            }
            mOpenStartFolder = false;
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
    //public String retrievePackageName() { return getPackageName(); }
    //public String retrievePackageCodePath() { return getPackageCodePath(); }
    public void postFinish() {
        mHandler.post( new FinishEvent() );
    }
    public void postChangeSurfaceSize( int w, int h ) { mHandler.post( new SurfaceSizeChangeEvent( w, h ) ); }
    public void postChangeCaption( String t ) { mHandler.post( new ActivityTitleChangeEvent(t) ); }
    public String getCaption() { return getTitle().toString(); }
    public void postShowToastMessage( String m ) { mHandler.post( new ShowToastEvent(m) ); }

	// *** native 関数列挙
	public static native void nativeToMessage(int mes, long wparam, long lparam );
    public static native void nativeSetSurface(Surface surface);
	public static native void nativeSetAssetManager(AssetManager am);
    public static native void nativeSetActivity( Activity activity );
	public static native void nativeInitialize();
	public static native void nativeSetStartupPath( String path );

    public static native void nativeOnTouch( int type, float x, float y, float c, int id, long tick );
}
