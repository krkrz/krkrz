#ifndef __HTTPCONNECTION_H_
#define __HTTPCONNECTION_H_

#include <windows.h>
#include <tchar.h>
#include <WinInet.h>
#include <vector>
#include <string>
#include <map>

#include "Base64.h"

using namespace std;
typedef basic_string<TCHAR> tstring;

/**
 * HTTP接続を実現するクラス
 */
class HttpConnection
{

public:
	// エラー状態
	enum Error {
		ERROR_NONE,  // エラーなし
		ERROR_INET,  // ネットワークライブラリのエラー
		ERROR_CANCEL // キャンセルされた
	};

	/**
	 * リクエスト用コールバック処理
	 * @param context コンテキスト
	 * @param buffer 書き込み先データバッファ
	 * @param size 書き込み先データバッファのサイズ。実際に書き込んだサイズを格納して返す
	 * @return 中断する場合は true を返す
	 */
	typedef bool (*RequestCallback)(void *context, void *buffer, DWORD &size);


	/**
	 * リトライ用コールバック処理
	 * @param context コンテキスト
	 * @return 中断する場合は true を返す
	 */
	typedef void (*RetryCallback)(void *context);
	
	/**
	 * レスポンス用コールバック処理
	 * @param context コンテキスト
	 * @param buffer 読み込み元データバッファ。最後は NULL
	 * @param size データバッファのサイズ。最後は0
	 * @return 中断する場合は true を返す
	 */
	typedef bool (*ResponseCallback)(void *context, const void *buffer, DWORD size);
	
	/**
	 * コンストラクタ
	 * @param agentName エージェント名
	 * @param checkCert 認証確認するかどうか
	 */
	HttpConnection(tstring agentName, bool checkCert=false) : agentName(agentName), checkCert(checkCert), contentLength(0), secure(false){
		::InitializeCriticalSection(&cs);
		hInet = NULL;
		hConn = NULL;
		hReq  = NULL;
	}

	// デストラクタ
	~HttpConnection(void) {
		clearParam();
		::DeleteCriticalSection(&cs);
	}

	// 送信ヘッダをクリア
	void clearHeader() {
		header.clear();
		requestContentLength = 0;
		requestContentType.erase();
		requestEncoding.erase();
	}

	// ハンドルをクリア
	void closeHandle();

	// 送信パラメータをクリア(名前を変えただけで、実体は送信データクリア)
	void clearParam() {
		closeHandle();
		clearHeader();
	}

	// ----------------------------------------------------------------------------------------
	
	// HTTP ヘッダを追加する
	void addHeader(const TCHAR *name, const TCHAR *value);

	// 認証ヘッダをセットする(addHeader のユーティリティ)
	void addBasicAuthHeader(const tstring &user, const tstring &passwd) {
		tstring sendStr = user + _T(":") + passwd;
		tstring value = _T("Basic") + base64encode(sendStr.c_str(), sendStr.length());
		addHeader(_T("Authorization"), value.c_str());
	}
	
	// ----------------------------------------------------------------------------------------------------
	
	/**
	 * リクエスト開始
	 * @param method アクセスメソッド
	 * @param url URL
	 * @param user アクセスユーザ
	 * @param passwd アクセスパスワード
	 * @return 成功したら true
	 */
	bool open(const TCHAR *method,
			  const TCHAR *url,
			  const TCHAR *user = NULL,
			  const TCHAR *passwd = NULL);

	/**
	 * リクエスト送信
	 * @param callback 送信用コールバック
	 * @param context コールバック用コンテキスト
	 * @return エラー
	 */
	int request(RequestCallback requestCallback=NULL, RetryCallback retryCalblack = NULL, void *context=NULL);


	/**
	 * レスポンス取得前情報収集
	 */
	void queryInfo();
	
	/**
	 * レスポンス受信
	 * @param callback 保存用コールバック
	 * @param context コールバック用コンテキスト
	 * @return エラー
	 */
	int response(ResponseCallback callback=NULL, void *context=NULL);
	
	// ----------------------------------------------------------------------------------------------------
	
	// エラーメッセージの取得
	const TCHAR *getErrorMessage() const {
		return errorMessage.c_str();
	}
	
	// 最後のリクエストが成功しているかどうか
	bool isValid() const {
		return hReq != NULL;
	}

	// HTTPステータスコードを取得
	int getStatusCode() const {
		return statusCode;
	}

	// HTTPステータステキストを取得
	const TCHAR *getStatusText() const {
		return statusText.c_str();
	}
	
	// 取得されたコンテンツの長さ
	int getContentLength() const {
		return contentLength;
	}

	// コンテンツの MIME-TYPE
	const TCHAR *getContentType() const {
		return contentType.c_str();
	}

	// コンテンツのエンコーディング情報
	const TCHAR *getEncoding() const {
		return encoding.c_str();
	}

	// コンテンツのエンコーディング情報
	const TCHAR *getRequestEncoding() const {
		return requestEncoding.c_str();
	}
	
	/**
	 * レスポンスのヘッダ情報を取得
	 */
	const TCHAR *getResponseHeader(const TCHAR *name) {
		map<tstring,tstring>::const_iterator it = responseHeaders.find(tstring(name));
		if (it != responseHeaders.end()) {
			return it->second.c_str();
		}
		return NULL;
	}

	// レスポンスヘッダ全取得用:初期化
	void initRH() {
		rhit = responseHeaders.begin();
	}

	// レスポンスヘッダ全取得用:取得
	bool getNextRH(tstring &name, tstring &value) {
		if (rhit != responseHeaders.end()) {
			name  = rhit->first;
			value = rhit->second;
			rhit++;
			return true;
		}
		return false;
	}

	bool getCheckCert() const { return checkCert;	}
	void setCheckCert(bool check)	{ checkCert = check;}

private:
	CRITICAL_SECTION cs;

	// 基礎情報
	tstring agentName; ///< ユーザエージェント名
	bool checkCert;	   ///< 証明書確認ダイアログを出すか
	bool secure;       ///< https 通信かどうか

	HINTERNET hInet; ///< インターネット接続
	HINTERNET hConn; ///< コネクション
	HINTERNET hReq;  ///< HTTPリクエスト
	
	// 送信用データ
	vector<tstring> header;	///< HTTP ヘッダ
	DWORD requestContentLength; ///< リクエストの Content-Length:
	tstring requestContentType; ///< リクエストの Content-Type:
	tstring requestEncoding;    ///< リクエストのエンコード指定

	// 受信用データ
	bool validContentLength;
	DWORD contentLength;     ///< Content-Length:
	tstring contentType;     ///< Content-Type: のtype部
	tstring encoding;        ///< Content-TYpe: のエンコーディング部

	DWORD statusCode;        ///< HTTP status code
	tstring statusText;      ///< HTTP status text
	map<tstring,tstring> responseHeaders; ///< レスポンスヘッダ
	map<tstring,tstring>::const_iterator rhit; //< レスポンスヘッダ参照用イテレータ

	// エラーコード
	tstring errorMessage; ///< エラーメッセージ
};

#endif
