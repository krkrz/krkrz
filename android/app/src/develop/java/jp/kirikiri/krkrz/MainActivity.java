package jp.kirikiri.krkrz;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;

public class MainActivity extends BaseMainActivity {
    private boolean mSelectedStartFolder;
    private boolean mOpenStartFolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSelectedStartFolder = false;
        mOpenStartFolder = false;

        Intent intent = getIntent();
        Bundle bundle = intent.getExtras();
        if(bundle != null) {
            String path = bundle.getString("startup_path");
            if( path != null && !path.isEmpty()) {
                nativeSetStartupPath(path);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if( mSelectedStartFolder == false ) {
            mSelectedStartFolder = true;
            mOpenStartFolder = true;
            selectFolder();
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        if( mOpenStartFolder && requestCode == SELECT_TREE_REQUEST_CODE ) {
            if( resultCode == Activity.RESULT_OK ) {
                Uri treeUri = resultData.getData();
                getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                if( mOpenStartFolder ) {
                    nativeSetStartupPath( treeUri.toString() + "/" );
                }
            }
            mOpenStartFolder = false;
        } else {
            super.onActivityResult( requestCode, resultCode, resultData );
        }
    }
}

