package jp.kirikiri.krkrz;

import android.app.Application;
import android.content.Context;

import java.lang.ref.WeakReference;

/**
 */

public class KrkrzApplication extends Application {
    static WeakReference<Context> mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = new WeakReference<Context>( getApplicationContext() );
    }
    @Override
    public void onTerminate() {
        super.onTerminate();
    }
    public static Context getContext() { return mContext.get(); }
}
