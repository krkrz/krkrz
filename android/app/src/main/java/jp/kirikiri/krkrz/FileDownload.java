/**
 ******************************************************************************
 * Copyright (c), Takenori Imoto
 * 楓 software http://www.kaede-software.com/
 * All rights reserved.
 ******************************************************************************
 * ソースコード形式かバイナリ形式か、変更するかしないかを問わず、以下の条件を満
 * たす場合に限り、再頒布および使用が許可されます。
 *
 * ・ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、および下記免責
 *   条項を含めること。
 * ・バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、上記の
 *   著作権表示、本条件一覧、および下記免責条項を含めること。
 * ・書面による特別の許可なしに、本ソフトウェアから派生した製品の宣伝または販売
 *   促進に、組織の名前またはコントリビューターの名前を使用してはならない。
 *
 * 本ソフトウェアは、著作権者およびコントリビューターによって「現状のまま」提供
 * されており、明示黙示を問わず、商業的な使用可能性、および特定の目的に対する適
 * 合性に関する暗黙の保証も含め、またそれに限定されない、いかなる保証もありませ
 * ん。著作権者もコントリビューターも、事由のいかんを問わず、損害発生の原因いか
 * んを問わず、かつ責任の根拠が契約であるか厳格責任であるか（過失その他の）不法
 * 行為であるかを問わず、仮にそのような損害が発生する可能性を知らされていたとし
 * ても、本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
 * 使用の喪失、データの喪失、利益の喪失、業務の中断も含め、またそれに限定されな
 * い）直接損害、間接損害、偶発的な損害、特別損害、懲罰的損害、または結果損害に
 * ついて、一切責任を負わないものとします。
 ******************************************************************************
 */
package jp.kirikiri.krkrz;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;

// TODO use DownloadManager
/**
 * 指定ファイルをダウンロードするためのクラス
 * サイズが大きいものを想定しているのでサーバーにはレジューム機能必須
 */
public class FileDownload {
	public static final int SUCCESS = 0;
	public static final int CANNOT_CREATE_FILE = -1;
	public static final int INVALIDATE_URL = -2;
	public static final int PROTOCOL_ERROR = -3;
	public static final int IO_ERROR = -4;
	public static final int UNMATCH_FILE_SIZE = -5;
	public static final int NOT_SUPPORT_RESUME = -6;

	private String mFileURL;
	private String mLocalFilePath;
	private int mRequestFileSize;
	private File mTmpFile;

	private int mFileSize;
	private int mFileSizeCount;
	private InputStream mDownloadStream;
	private FileOutputStream mSaveStream;
	private byte[] mWorkBufer;

	public FileDownload( String fileUrl, String savePath, int fileSize ) {
		mFileURL = fileUrl;
		mLocalFilePath = savePath;
		mRequestFileSize = fileSize;
	}

	/**
	 * ダウンロードを開始する
	 * @param clean リジュームせずに新規にダウンロードするかどうか、サイズや MD5 が一致しなかった時使用する
	 * @return エラーコード
	 */
	public int startDownload( boolean clean ) {
		// リジュームのチェック
		mTmpFile = new File( mLocalFilePath + ".tmp" );
		if( mTmpFile.exists() ) {
			mFileSize = 0;
			if( clean == false ) {
				mFileSizeCount = (int)mTmpFile.length();
			} else {
				mFileSizeCount = 0;
				mTmpFile.delete();
				try {
					mTmpFile.createNewFile();
				} catch (IOException e) {
					return CANNOT_CREATE_FILE;
				}
			}
		} else {
			mFileSize = 0;
			mFileSizeCount = 0;
			try {
				mTmpFile.createNewFile();
			} catch (IOException e) {
				return CANNOT_CREATE_FILE;
			}
		}
		// ファイルのサイズを得る
		URL url;
		try {
			mFileSize = -1;
			url = new URL(mFileURL);
			HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
			httpURLConnection.setRequestMethod("HEAD");
			httpURLConnection.connect();
			if( httpURLConnection.getResponseCode() == 200 ) {
				mFileSize = httpURLConnection.getContentLength();
				if( mFileSize != mRequestFileSize ) {
					return UNMATCH_FILE_SIZE; // 要求したファイルとサイズが一致しない
				}
			} else {
				mFileSize = mRequestFileSize; // サイズが取得できないときは、指定サイズをそのまま使う
			}
		} catch (MalformedURLException e) {
			return INVALIDATE_URL;
		} catch (ProtocolException e) {
			return PROTOCOL_ERROR;
		} catch (IOException e) {
			return IO_ERROR;
		}
		// ダウンロードを開始
		try {
			HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();

			httpURLConnection.setRequestMethod ("GET");
			httpURLConnection.setRequestProperty( "Range", String.format( "bytes=%d-%d", mFileSizeCount, mFileSize ) );
			httpURLConnection.connect ();

			if( /*(httpURLConnection.getResponseCode () == 200) ||*/ (httpURLConnection.getResponseCode () == 206) ) {
				mDownloadStream = httpURLConnection.getInputStream ();
				mSaveStream = new FileOutputStream(mTmpFile, true);
				mWorkBufer = new byte[4096];
			} else {
				return NOT_SUPPORT_RESUME;
			}
		} catch (IOException e) {
			return IO_ERROR;
		}
		return SUCCESS;
	}
	/**
	 *
	 * @param cancel ダウンロードをキャンセルするかどうか
	 * @return 負の値の時はエラーコード、正の値の時は現在のダウンロード済みデータサイズ
	 */
	public int progressDownload( boolean cancel ) {
		if( cancel ) {
			return mFileSizeCount;
		} else {
			int readBytes;
			try {
				readBytes = mDownloadStream.read(mWorkBufer);
				if( readBytes != -1 ) {
					mSaveStream.write( mWorkBufer, 0, readBytes );
					mFileSizeCount += readBytes;
				}
			} catch (IOException e) {
				return IO_ERROR;
			}
			return mFileSizeCount;
		}
	}
	/**
	 * ダウンロードが正常終了したときに呼び出す
	 * @return 正常終了しているかどうか。false の時は startDownload でclean指定してやり直す
	 */
	public boolean finishDownload() {
		if( mTmpFile != null ) {
			if( mFileSizeCount == mFileSize ) {
				mTmpFile.renameTo( new File(mLocalFilePath) );
				return true;
			} else {
				mTmpFile.delete();
				return false;
			}
		}
		return false;
	}
}
