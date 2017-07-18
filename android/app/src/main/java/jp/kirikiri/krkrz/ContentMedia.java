package jp.kirikiri.krkrz;

import android.content.Context;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.support.v4.provider.DocumentFile;

import java.io.FileNotFoundException;

/**
 */

public class ContentMedia {
    public static boolean isExist( String uri ) {
        Context ctx = KrkrzApplication.getContext();
        if( ctx != null ) {
            boolean result = DocumentFile.isDocumentUri(ctx, Uri.parse(uri));
            if( result == false ) {
                // 見付からない場合、ツリー名とディスプレイ名で分離して検索する。遅そうなので、isExist はキャッシュした方が良い
                int index = uri.lastIndexOf('/');
                if( index >= 0 ) {
                    String tree = uri.substring(0,index);
                    String file = uri.substring(index+1);
                    DocumentFile treeDoc = DocumentFile.fromTreeUri(ctx, Uri.parse(tree));
                    if( treeDoc != null ) {
                        return treeDoc.findFile(file) != null;
                    }
                }
            }
            return result;
        }
        // return DocumentFile.isDocumentUri(context,Uri.parse(uri));
        return false;
    }
    private static DocumentFile findDirInList( DocumentFile tree, String dir, boolean isDir ) {
        try {
            DocumentFile[] list = tree.listFiles();
            if( isDir ) {
                for (DocumentFile file : list) {
                    if (file.isDirectory()) {
                        String displayName = file.getName();
                        if (displayName.equals(dir)) {
                            return file;
                        }
                    }
                }
            } else {
                for (DocumentFile file : list) {
                    String displayName = file.getName();
                    if (displayName.equals(dir)) {
                        return file;
                    }
                }
            }
        } catch( UnsupportedOperationException e) {
        }
        return null;
    }
    public static String getFullUri( String path, String filename ) {
        try {
            Context ctx = KrkrzApplication.getContext();
            if (ctx != null) {
                DocumentFile treeDoc = DocumentFile.fromTreeUri(ctx, Uri.parse(path));
                if (treeDoc != null) {
                    DocumentFile doc = treeDoc.findFile(filename);
                    if (doc != null ) {
                        return doc.getUri().toString();
                    } else {
                        String[] dirs = filename.split("/");
                        DocumentFile tree = treeDoc;
                        for( String dir : dirs ) {
                            if( dir.isEmpty() == false ) {
                                DocumentFile d = findDirInList(tree, dir, false);
                                if (d == null) return null;
                                tree = d;
                            }
                        }
                        return tree.getUri().toString();
                    }
                }
            }
        } catch( Exception e) {
        }
        return null;
    }
    public static ParcelFileDescriptor open(String uri, String mode ) {
        try {
            Context ctx = KrkrzApplication.getContext();
            if( ctx != null ) {
                return ctx.getContentResolver().openFileDescriptor(Uri.parse(uri), mode);
            }
            // return context.getContentResolver().openFileDescriptor(Uri.parse(uri), mode);
        } catch( FileNotFoundException e ) {
        }
        return null;
    }
    public static String[] getFileList( String uri ) {
        Context ctx = KrkrzApplication.getContext();
        if( ctx != null ) {
            DocumentFile tree = DocumentFile.fromTreeUri(ctx, Uri.parse(uri));
            DocumentFile[] list = tree.listFiles();
            int count = 0;
            for (DocumentFile file : list) {
                if (file.isFile()) count++;
            }
            if (count > 0) {
                String[] ret = new String[count];
                int i = 0;
                for (DocumentFile file : list) {
                    if (file.isFile()) {
                        String dispname = file.getName();
                        ret[i] = dispname;
                        i++;
                    }
                }
                return ret;
            }
        }
        return new String[0];
    }
    public static String[] getFileListInDir( String uri, String path ) {
        Context ctx = KrkrzApplication.getContext();
        if( ctx != null ) {
            DocumentFile tree = DocumentFile.fromTreeUri(ctx, Uri.parse(uri));
            String[] dirs = path.split("/");
            for( String dir : dirs ) {
                if( dir.isEmpty() == false ) {
                    DocumentFile d = findDirInList(tree, dir, true);
                    if (d == null) return new String[0];
                    tree = d;
                }
            }
            DocumentFile[] list = tree.listFiles();
            int count = 0;
            for (DocumentFile file : list) {
                if (file.isFile()) count++;
            }
            if (count > 0) {
                String[] ret = new String[count];
                int i = 0;
                for (DocumentFile file : list) {
                    if (file.isFile()) {
                        String dispname = file.getName();
                        ret[i] = dispname;
                        i++;
                    }
                }
                return ret;
            }
        }
        return new String[0];
    }
}
