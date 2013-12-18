#include "HttpConnection.h"
#include "ncbind/ncbind.hpp"
#include <vector>
using namespace std;
#include <process.h>

// メッセージコード
#define	WM_HTTP_READYSTATE	(WM_APP+6)	// ステート変更
#define	WM_HTTP_PROGRESS	(WM_APP+7)	// プログレス状態

// エージェント名
#define AGENT_NAME _T("KIRIKIRI")
#define DEFAULT_ENCODING _T("UTF-8")
#define CTYPE_URLENCODED _T("application/x-www-form-urlencoded")

// エンコーディング名からコードページを取得
extern void initEncoding();
extern void doneEncoding();
extern int getEncoding(const wchar_t *encoding);
extern UINT getWCToMBLen(int enc, const wchar_t *wc, UINT wclen);
extern void convWCToMB(int enc, const wchar_t *wc, UINT *wclen, char *mb, UINT *mblen);
extern UINT getMBToWCLen(int enc, const char *mb, UINT mblen);
extern void convMBToWC(int enc, const char *mb, UINT *mblen, wchar_t *wc, UINT *wclen);

/**
 * HttpRequest クラス
 */
class HttpRequest {

public:

	enum ReadyState {
		READYSTATE_UNINITIALIZED,
		READYSTATE_OPEN,
		READYSTATE_SENT,
		READYSTATE_RECEIVING,
		READYSTATE_LOADED
	};

	/**
	 * コンストラクタ
	 * @param objthis 自己オブジェクト
	 * @param window 親ウインドウ
	 * @param cert HTTP通信時に証明書チェックを行う
	 */
	HttpRequest(iTJSDispatch2 *objthis, iTJSDispatch2 *window, bool cert, const tjs_char *agentName)
		 : objthis(objthis), window(window), http(agentName, cert),
		   threadHandle(NULL), canceled(false),
		   outputStream(NULL), outputLength(0), inputStream(NULL), inputLength(0),
		   readyState(READYSTATE_UNINITIALIZED), statusCode(0)
	{
		window->AddRef();
		setReceiver(true);
	}
	
	/**
	 * デストラクタ
	 */
	~HttpRequest() {
		abort();
		setReceiver(false);
		window->Release();
	}

	/**
	 * 指定したメソッドで指定URLにリクエストする
	 * ※常に非同期での呼び出しになります
	 * @param method GET|PUT|POST のいずれか
	 * @param url リクエスト先のURL
	 * @param userName ユーザ名。指定すると認証ヘッダをつけます
	 * @param password パスワード
	 */
	void _open(const tjs_char *method, const tjs_char *url, const tjs_char *userName, const tjs_char *password) {
		abort();
		if (http.open(method, url, userName, password)) {
			onReadyStateChange(READYSTATE_OPEN);
		} else {
			TVPThrowExceptionMessage(http.getErrorMessage());
		}
	}

	static tjs_error open(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->_open(params[0]->GetString(), params[1]->GetString(), numparams > 2 ? params[2]->GetString() : NULL, numparams > 3 ? params[3]->GetString() : NULL);
		return TJS_S_OK;
	}
	
	/**
	 * 送信時に送られるヘッダーを追加する
	 * @param name ヘッダ名
	 * @param value 値
	 */
	void setRequestHeader(const tjs_char *name, const tjs_char *value) {
		checkRunning();
		http.addHeader(name, value);
	}

	/**
	 * 送信処理
	 * @param 送信データ
	 * @param sendStorage 送信ファイル
	 * @param saveStorage 保存先ファイル
	 */
	void _send(tTJSVariant *data, const tjs_char *sendStorage, const tjs_char *saveStorage) {
		checkRunning();
		checkOpen();
		if (saveStorage) {
			outputStream = TVPCreateIStream(ttstr(saveStorage), TJS_BS_WRITE);
			if (outputStream == NULL) {
				TVPThrowExceptionMessage(L"saveStorage open failed");
			}
		}
		if (data) {
			switch (data->Type()) {
			case tvtString:
				{
					tTJSVariantString *str = data->AsStringNoAddRef();
					int enc = getEncoding(http.getRequestEncoding());
					inputLength = ::getWCToMBLen(enc, *str, str->GetLength());
					inputData.resize(inputLength);
					if (inputLength) {
					  UINT wlen = str->GetLength();
					  UINT blen = inputData.size();
					  ::convWCToMB(enc, *str, &wlen, (char*)&inputData[0], &blen);
					}
				}
				break;
			case tvtOctet:
				{
					tTJSVariantOctet *octet = data->AsOctetNoAddRef();
					if (octet) {
						inputLength = octet->GetLength();
						inputData.resize(inputLength);
						memcpy(&inputData[0], octet->GetData(), inputLength);
					}
				}
				break;
			}
			inputLength = inputData.size();
		} else if (sendStorage) {
			inputStream = TVPCreateIStream(ttstr(sendStorage), TJS_BS_READ);
			if (inputStream == NULL) {
				TVPThrowExceptionMessage(L"sendStorage open failed");
			}
			STATSTG stat;
			inputStream->Stat(&stat, STATFLAG_NONAME);
			inputLength = (DWORD)stat.cbSize.QuadPart;
		}
		if (inputLength > 0) {
			ttstr len(inputLength);
			http.addHeader(_T("Content-Length"), len.c_str());
		}
		startThread();
	}
	
	/**
	 * リクエストの送信
	 */
	static tjs_error send(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		self->_send(numparams > 0 ? params[0] : NULL, NULL, numparams > 1 ? params[1]->GetString() : NULL);
		return TJS_S_OK;
	}

	/**
	 * リクエストの送信
	 */
	static tjs_error sendStorage(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->_send(NULL, params[0]->GetString(), numparams > 1 ? params[1]->GetString() : NULL);
		return TJS_S_OK;
	}
	
	void clearInput() {
		if (inputStream) {
			inputStream->Release();
			inputStream = NULL;
		}
		inputData.clear();
		inputSize = inputLength = 0;
	}

	void closeOutput() {
		if (outputStream) {
			outputStream->Release();
			outputStream = NULL;
		}
	}
	
	void clearOutput() {
		closeOutput();
		outputData.clear();
		outputSize = outputLength = 0;
	}
	

	/**
	 * 現在実行中の送受信のキャンセル
	 */
	void abort() {
		stopThread();
		clearInput();
		clearOutput();
	}
	
	/**
	 * すべての HTTPヘッダを取得する
	 * @return HTTPヘッダが格納された辞書
	 */
	tTJSVariant getAllResponseHeaders() {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		tstring name;
		tstring value;
		http.initRH();
		while (http.getNextRH(name, value)) {
			tTJSVariant v(value.c_str());
			dict->PropSet(TJS_MEMBERENSURE, name.c_str(), NULL, &v, dict);
		}
		tTJSVariant ret(dict,dict);
		dict->Release();
		return ret;
	}

	/**
	 * 指定したHTTPヘッダを取得する
	 * @param name ヘッダラベル名
	 * @return ヘッダの値
	 */
	const tjs_char *getResponseHeader(const tjs_char *name) {
		return http.getResponseHeader(name);
	}

	/**
	 * 通信状態。読み込み専用
	 * @return 現在の通信状態
	 * 0: 初期状態
	 * 1: 読み込み中
	 * 2: 読み込んだ
	 * 3: 解析中
	 * 4: 完了
	 */
	int getReadyState() const {
		return readyState;
	}

	/**
	 * レスポンスをテキストの形で返す
	 * @param encoding エンコーディング指定
	 */
	tTJSString _getResponseText(const tjs_char *encoding) {
		tTJSString ret;
		if (encoding == NULL) {
			encoding = http.getEncoding();
		}
		if (outputData.size() > 0) {
			DWORD size = outputData.size();
			const char *data = (const char*)&outputData[0];
			UINT dlen = outputData.size();
			int enc = getEncoding(encoding);
			UINT l = ::getMBToWCLen(enc, data, dlen);
			if (l > 0) {
				tjs_char *str = ret.AllocBuffer(l);
				::convMBToWC(enc, data, &dlen, str, &l);
			}
		}
		return ret;
	}

	static tjs_error getResponseText(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (result) {
			*result = self->_getResponseText(numparams > 0 ? params[0]->GetString() : NULL);
		}
		return TJS_S_OK;
	}
	
	/**
	 * レスポンスデータ。読み込み専用
	 * @return レスポンスデータ
	 */
	tTJSVariant getResponse() {
		const TCHAR *contentType = http.getContentType();
		if (_tcsncmp(http.getContentType(), _T("text/"), 5) == 0) {
			return _getResponseText(http.getEncoding());
//		} else if (_tcscmp(contentType, CTYPE_URLENCODED) == 0) {
//			// URLENCODEDなデータを解析して辞書を構築
		} else if (outputData.size() > 0) {
			return tTJSVariant((const tjs_uint8 *)&outputData[0], outputData.size());
		}
		return tTJSVariant();
	}
	
	/**
	 * レスポンスデータ。読み込み専用
	 * @return レスポンスデータ
	 */
	tTJSVariant getResponseData() {
		if (outputData.size() > 0) {
			return tTJSVariant((const tjs_uint8 *)&outputData[0], outputData.size());
		}
		return tTJSVariant();
	}

	/**
	 * レスポンスの HTTPステータスコード。読み込み専用
	 * @return ステータスコード
	 */
	int getStatus() {
		return statusCode;
	}
	
	/**
	 * レスポンスの HTTPステータスの文字列
	 * @return レスポンス文字列
	 */
	const tjs_char *getStatusText() {
		return statusText.c_str();
	}

	const tjs_char *getContentType() {
		return http.getContentType();
	}

	const tjs_char *getContentTypeEncoding() {
		return http.getEncoding();
	}

	int getContentLength() {
		return http.getContentLength();
	}
	
	/**
	 * インスタンス生成ファクトリ
	 */
	static tjs_error factory(HttpRequest **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		iTJSDispatch2 *window = params[0]->AsObjectNoAddRef();
		if (window->IsInstanceOf(0, NULL, NULL, L"Window", window) != TJS_S_TRUE) {
			TVPThrowExceptionMessage(L"InvalidObject");
		}
		*result = new HttpRequest(objthis, window, numparams >= 2 ? params[1]->AsInteger() != 0 : true, AGENT_NAME);
		return S_OK;
	}
	

protected:

	void checkRunning() {
		if (threadHandle) {
			TVPThrowExceptionMessage(TJS_W("already running"));
		}
	}

	void checkOpen() {
		if (!http.isValid()) {
			TVPThrowExceptionMessage(TJS_W("not open"));
		}
	}

	/**
	 * readyState が変化した場合のイベント処理
	 * @param readyState 新しいステート
	 */
	void onReadyStateChange(int readyState) {
		this->readyState = readyState;
		if (readyState == READYSTATE_LOADED) {
			stopThread();
		}
		tTJSVariant param(readyState);
		static ttstr eventName(TJS_W("onReadyStateChange"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
	}
	
	/**
	 * データ読み込み中のイベント処理
	 * @param upload 送信中
	 * @param percent 進捗
	 */
	void onProgress(bool upload, int percent) {
		tTJSVariant params[2];
		params[0] = upload;
		params[1] = percent;
		static ttstr eventName(TJS_W("onProgress"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 2, params);
	}
	
	// ユーザメッセージレシーバの登録/解除
	void setReceiver(bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)this;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, objthis) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}

	/**
	 * イベント受信処理
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
		HttpRequest *self = (HttpRequest*)userdata;
		switch (Message->Msg) {
		case WM_HTTP_READYSTATE:
			if (self == (HttpRequest*)Message->WParam) {
				self->onReadyStateChange((ReadyState)Message->LParam);
				return true;
			}
			break;
		case WM_HTTP_PROGRESS:
			if (self == (HttpRequest*)Message->WParam) {
				int lparam = (int)Message->LParam;
				self->onProgress((lparam & 0xff00)!=0, (lparam & 0xff));
				return true;
			}
			break;
		}
		return false;
	}

	// -----------------------------------------------
	// スレッド処理
	// -----------------------------------------------

	/**
	 * ファイル送信処理を巻き戻し
	 */
        void rewindUpload(void) {
	  if (inputStream) {
	    LARGE_INTEGER pos;
	    pos.QuadPart = 0;
	    inputStream->Seek(pos, STREAM_SEEK_SET, NULL);
	  }
	  inputSize = 0;
	}
  
        /**
	 * 送信巻き戻しのコールバック処理
	 */
        static void rewindUploadCallback(void *context) {
	  HttpRequest *self = (HttpRequest*)context;
	  if (self)
	    self->rewindUpload();
	}
    
	/**
	 * ファイル送信処理
	 * @param buffer 読み取りバッファ
	 * @param size 読み出したサイズ
	 */
	bool upload(void *buffer, DWORD &size) {
		if (inputStream) {
			// ファイルから読み込む
			inputStream->Read(buffer, size, &size);
		} else {
			// メモリから読み込む
			DWORD s = inputData.size() - inputSize;
			if (s < size) {
				size = s;
			}
			if (size > 0) {
				memcpy(buffer, &inputData[inputSize], size);
			}
		}
		if (size > 0) {
			inputSize += size;
			int percent = (inputLength > 0) ? inputSize * 100 / inputLength : 0;
			::PostMessage(hwnd, WM_HTTP_PROGRESS, (WPARAM)this, 0x0100 | percent);
		}
		return !canceled;
	}

	/**
	 * 通信時のコールバック処理
	 * @return キャンセルなら false
	 */
	static bool uploadCallback(void *context, void *buffer, DWORD &size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->upload(buffer, size) : false;
	}
	
	/**
	 * ファイル読み取り処理
	 * @param buffer 読み取りバッファ
	 * @param size 読み出したサイズ
	 */
	bool download(const void *buffer, DWORD size) {
		if (outputStream) {
			if (buffer) {
				DWORD n = 0;
				DWORD s = size;
				while (s > 0) {
					DWORD l;
					if (outputStream->Write((BYTE*)buffer+n, s, &l) == S_OK) {
						s -= l;
						n += l;
					} else {
						break;
					}
				}
			} else {
				outputStream->Release();
				outputStream = NULL;
			}
		} else {
			outputData.resize(outputSize + size);
			memcpy(&outputData[outputSize], buffer, size);
		}
		outputSize += size;
		int percent = (outputLength > 0) ? outputSize * 100 / outputLength : 0;
		::PostMessage(hwnd, WM_HTTP_PROGRESS, (WPARAM)this, percent);
		return !canceled;
	}
	
	/**
	 * 通信時のコールバック処理
	 * @return キャンセルなら false
	 */
	static bool downloadCallback(void *context, const void *buffer, DWORD size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->download(buffer, size) : false;
	}

	/**
	 * バックグラウンドで実行する処理
	 */
	void threadMain() {

		{
			tTJSVariant val;
			window->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
			hwnd = reinterpret_cast<HWND>((tjs_int)(val));
		}

		::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_SENT);
		inputSize = 0;
		int errorCode;
		if (canceled) {
			errorCode = HttpConnection::ERROR_CANCEL;
			clearInput();
		} else {
			if ((errorCode = http.request(uploadCallback, rewindUploadCallback, (void*)this)) == HttpConnection::ERROR_NONE) {
				clearInput();
				if (canceled) {
					errorCode = HttpConnection::ERROR_CANCEL;
					clearOutput();
				} else {
					http.queryInfo();
					outputSize = 0;
					outputLength = http.getContentLength();
					::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_RECEIVING);
					if ((errorCode = http.response(downloadCallback, (void*)this)) == HttpConnection::ERROR_NONE) {
						closeOutput();
					} else {
						clearOutput();
					}
				}
			} else {
				clearInput();
			}
		}
		switch (errorCode) {
		case HttpConnection::ERROR_NONE:
			statusCode = http.getStatusCode();
			statusText = http.getStatusText();
			break;
		case HttpConnection::ERROR_CANCEL:
			statusCode = -1;
			statusText = L"aborted";
			break;
		default:
			statusCode = 0;
			statusText = http.getErrorMessage();
			break;
		}
		::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_LOADED);
	}

	// 実行スレッド
	static unsigned __stdcall threadFunc(void *data) {
		((HttpRequest*)data)->threadMain();
		_endthreadex(0);
		return 0;
	}

	// スレッド処理開始
	void startThread() {
		stopThread();
		canceled = false;
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, threadFunc, this, 0, NULL);
	}

	// スレッド処理終了
	void stopThread() {
		if (threadHandle) {
			canceled = true;
			WaitForSingleObject(threadHandle, INFINITE);
			CloseHandle(threadHandle);
			threadHandle = 0;
		}
	}
	
private:
	iTJSDispatch2 *objthis; ///< 自己オブジェクト情報の参照
	iTJSDispatch2 *window; ///< ウインドウオブジェクト情報の参照(イベント取得に必要)
	HWND hwnd; ///< ウインドウハンドラ。メインスレッド停止中に Window にアクセスすると固まるので処理前にとっておく
	
	// HTTP通信処理用
	HttpConnection http;

	// スレッド処理用
	HANDLE threadHandle; ///< スレッドのハンドル
	bool canceled; ///< キャンセルされた

	// リクエスト
	IStream *inputStream;   ///< 送信用ストリーム
	vector<BYTE>inputData;  ///< 送信用データ
	int inputLength; ///< 送信データサイズ
	int inputSize;   ///< 送信済みデータサイズ

	// レスポンス
	IStream *outputStream;  ///< 受信用ストリーム
	vector<BYTE>outputData; ///< 受信用データ
	int outputLength; ///< 受信データサイズ
	int outputSize;   ///< 受信済みデータサイズ

	int readyState;
	int statusCode; ///< HTTPステータスコード
	ttstr statusText; ///< HTTPステータステキスト
};

#define ENUM(n) Variant(#n, (int)HttpRequest::READYSTATE_ ## n)

NCB_REGISTER_CLASS(HttpRequest) {
	Factory(&ClassT::factory);
	ENUM(UNINITIALIZED);
	ENUM(OPEN);
	ENUM(SENT);
	ENUM(RECEIVING);
	ENUM(LOADED);
	RawCallback(TJS_W("open"), &Class::open, 0);
	NCB_METHOD(setRequestHeader);
	RawCallback(TJS_W("send"), &Class::send, 0);
	RawCallback(TJS_W("sendStorage"), &Class::sendStorage, 0);
	NCB_METHOD(abort);
	NCB_METHOD(getAllResponseHeaders);
	NCB_METHOD(getResponseHeader);
	RawCallback(TJS_W("getResponseText"), &Class::getResponseText, 0);
	NCB_PROPERTY_RO(readyState, getReadyState);
	NCB_PROPERTY_RO(response, getResponse);
	NCB_PROPERTY_RO(responseData, getResponseData);
	NCB_PROPERTY_RO(status, getStatus);
	NCB_PROPERTY_RO(statusText, getStatusText);
	NCB_PROPERTY_RO(contentType, getContentType);
	NCB_PROPERTY_RO(contentTypeEncoding, getContentTypeEncoding);
	NCB_PROPERTY_RO(contentLength, getContentLength);
}

NCB_PRE_REGIST_CALLBACK(initEncoding);
NCB_POST_UNREGIST_CALLBACK(doneEncoding);
