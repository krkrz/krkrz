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


import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.AsyncTask;

// TODO use DownloadManager
public class FileDownloadTask extends AsyncTask<String, Integer, Boolean> implements DialogInterface.OnCancelListener {

	private Activity mActivity = null;
	private ProgressDialog mProgressDialog = null;
	private String mPath;

	public FileDownloadTask( Activity activity ) {
		mActivity = activity;
	}

	@Override
	protected void onPreExecute() {
		mProgressDialog = new ProgressDialog(mActivity);
		mProgressDialog.setProgressStyle( ProgressDialog.STYLE_HORIZONTAL );
		mProgressDialog.setMessage( "ダウンロード中..." );
		mProgressDialog.setCancelable(true);
		mProgressDialog.setOnCancelListener( this );
		mProgressDialog.setButton( DialogInterface.BUTTON_NEGATIVE, "Cancel",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						dialog.cancel();
					}
				}
		);
		mProgressDialog.show();
	}

	@Override
	protected Boolean doInBackground(String... params) {
		if( params != null ) {
			if( params.length >= 3 ) {
				final String url = params[0];
				final String savepath = params[1];
				mPath = savepath;
				final int filesize = Integer.valueOf(params[2]);
				boolean isclean = false;
				if( params.length >= 4 ) {
					if( "true".equals(params[3] ) ) {
						isclean = true;
					}
				}
				mProgressDialog.setMax(filesize);
				mProgressDialog.incrementProgressBy(0);
				FileDownload dl = new FileDownload( url, savepath, filesize );
				int ret = dl.startDownload(isclean);
				if( ret != FileDownload.SUCCESS ) {
					return Boolean.FALSE;
				}
				int cursize = dl.progressDownload(false);
				while( cursize >= 0 && cursize < filesize ) {
					mProgressDialog.setProgress(cursize);
					cursize = dl.progressDownload(isCancelled());
					if( isCancelled() ) {
						dl.finishDownload(); // delete tmp file
						return Boolean.FALSE;
					}
				}
				if( cursize == filesize ) {
					dl.finishDownload();
					return Boolean.TRUE;
				}
			}//
		}
		return Boolean.FALSE;
	}

	@Override
	protected void onPostExecute(Boolean result) {
		mProgressDialog.dismiss();
		// TODO 後で実装すること
		//mActivity.onFinishDownload(result.booleanValue(),mPath,isCancelled());
	}
	@Override
	protected void onCancelled() {
		mProgressDialog.dismiss();
		// TODO 後で実装すること
		//mActivity.onFinishDownload(false,mPath,true);
	}

	@Override
	public void onCancel(DialogInterface dialog) {
		cancel(true);
	}
}
