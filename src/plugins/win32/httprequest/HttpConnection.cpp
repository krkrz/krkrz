#include "HttpConnection.h"
#pragma comment(lib, "Wininet.lib") 

class CriticalSectionHelper
{
public:
	CriticalSectionHelper(LPCRITICAL_SECTION p) : m_pCS( p ) {
		::EnterCriticalSection(m_pCS);
	}
	~CriticalSectionHelper() {
		::LeaveCriticalSection(m_pCS);
	}
private:
	LPCRITICAL_SECTION m_pCS;
};
#define	LOCK  CriticalSectionHelper csh(&cs)

#define BUFSIZE (1024*16)

// データ中から METAタグで Content-Type を取得する
// ※正規表現を想定してるのに注意
extern bool matchContentType(tstring &text, tstring &result);

/**
 * エラーメッセージを格納する
 */
static void
storeErrorMessage(DWORD error, tstring &errorMessage)
{
	TCHAR msg[1024];
	if (FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
					  GetModuleHandle(_T("wininet.dll")),
					  error,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
					  msg,
					  sizeof msg,
					  NULL
					  ) ||
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL,
					  error,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
					  msg,
					  sizeof msg,
					  NULL
					  )) {
		errorMessage = msg;
	} else {
		errorMessage = _T("unknown error");
	}
}

/**
 * Content-Type をパースして ContentType と encoding 指定を取得
 * @param buf バッファ
 * @param length バッファサイズ
 * @param contentType 取得した Content-Type を格納
 * @param encoding 取得したエンコード指定を格納
 */
static void
parseContentType(const TCHAR *buf, size_t length, tstring &contentType, tstring &encoding)
{
	// 頭のスペースを読み飛ばす
	while (_istspace(*buf)) {
		buf++;
		length--;
	}
	size_t n = 0;
	const TCHAR *p;
	if ((p = _tcschr(buf, ';'))) {
		size_t l = p - buf;
		while (n < l && !_istspace(buf[n])) {
			n++;
		}
		contentType = tstring(buf, n);
		n = l+1;
		while (_istspace(buf[n])) n++;
		if (_tcsnicmp(buf+n, _T("charset"), 7) == 0) {
			n += 7;
			while (_istspace(buf[n])) n++;
			if (buf[n] == '=') {
				n++;
				while (_istspace(buf[n])) n++;
				int l = 0;
				while (n+l < length && buf[n+l] && !_istspace(buf[n+l])) l++;
				encoding = tstring(buf+n, l);
			}
		}
	} else {
		while (n < length && buf[n] && !_istspace(buf[n])) {
			n++;
		}
		contentType = tstring(buf, n);
	}
}

// ハンドルをクリア
void
HttpConnection::closeHandle()
{
	LOCK;
	if (hReq) { InternetCloseHandle(hReq);hReq=NULL; }
	if (hConn) { InternetCloseHandle(hConn);hConn=NULL; }
	if (hInet) { InternetCloseHandle(hInet);hInet=NULL; }
}

void
HttpConnection::addHeader(const TCHAR *name, const TCHAR *value)
{
	if (_tcsicmp(name, _T("Content-Type")) == 0) {
		parseContentType(value, _tcslen(value), requestContentType, requestEncoding);
	} else if (_tcsicmp(name, _T("Content-Length")) == 0) {
		requestContentLength = _tcstol(value, NULL, 10);
	}
	tstring n = name;
	n += _T(":");
	n += value;
	header.push_back(n);
}

/**
 * コネクション接続開始
 */
bool
HttpConnection::open(const TCHAR *method,
					 const TCHAR *url,
					 const TCHAR *_user,
					 const TCHAR *_passwd) {
	LOCK;
	clearParam();
	errorMessage.erase();

	URL_COMPONENTS uc;
	ZeroMemory(&uc, sizeof uc);
	uc.dwStructSize = sizeof uc;
	uc.dwSchemeLength   = 1;
	uc.dwHostNameLength = 1;
	uc.dwUserNameLength = 1;
	uc.dwPasswordLength = 1;
	uc.dwUrlPathLength  = 1;

	if (!InternetCrackUrl(url, 0, 0, &uc)) {
		storeErrorMessage(GetLastError(), errorMessage);
		return false;
	}

	if (uc.nScheme != INTERNET_SCHEME_HTTP && uc.nScheme != INTERNET_SCHEME_HTTPS) {
		errorMessage = _T("invalid protocol");
		return false;
	}
	
	secure = uc.nScheme == INTERNET_SCHEME_HTTPS;
	tstring host;
	if (uc.lpszHostName) {
		host = tstring(uc.lpszHostName, uc.dwHostNameLength);
	}
	int port = uc.nPort;
	tstring user;
	if (_user) {
		user = _user;
	}
	if (uc.lpszUserName) {
		user = tstring(uc.lpszUserName, uc.dwUserNameLength);
	} else if (_user) {
		user = _user;
	}
	tstring passwd;
	if (uc.lpszPassword) {
		passwd = tstring(uc.lpszPassword, uc.dwPasswordLength);
	} else if (_passwd) {
		passwd = _passwd;
	}
	tstring path;
	if (uc.lpszUrlPath) {
		path = tstring(uc.lpszUrlPath);
	}
	
	bool errorProxyFirst = true;
retry:
	// Internetに接続する
	if ((hInet = InternetOpen(agentName.c_str(),
			errorProxyFirst ? INTERNET_OPEN_TYPE_PRECONFIG : INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, 
			NULL, NULL, 0)) == NULL) {
		storeErrorMessage(GetLastError(), errorMessage);
		return false;
	}

	// HTTPサーバーに接続
	if ((hConn = InternetConnect(hInet, 
								 host.c_str(),
								 port,									// ポート
								 user.size() ? user.c_str() : NULL,		// username
								 passwd.size() ? passwd.c_str() : NULL,	// password
								 INTERNET_SERVICE_HTTP,
								 0, NULL)) == NULL) {
		DWORD error = GetLastError();
		closeHandle();

		if (errorProxyFirst) {
			errorProxyFirst = false;
			goto retry;
		}

		storeErrorMessage(error, errorMessage);
		return false;
	}

	// サーバー上で欲しいURLを指定する
	if ((hReq = HttpOpenRequest(hConn,
								method,
								path.c_str(),
								NULL, // デフォルトのHTTPバージョン
								NULL, // 履歴を追加しない
								NULL, // AcceptType
								INTERNET_FLAG_NO_CACHE_WRITE | (secure ? INTERNET_FLAG_SECURE : 0), NULL)) == NULL) {

		storeErrorMessage(GetLastError(), errorMessage);
		closeHandle();
		return false;
	}

	// 認証ヘッダ追加
	if (user.size() > 0 && passwd.size() > 0) {
		addBasicAuthHeader(user, passwd);
	}

	return true;
}

/**
 * リクエスト送信
 */
int
HttpConnection::request(RequestCallback requestCallback, RetryCallback retryCallback, void *context)
{
	LOCK;

	if (!isValid()) {
		return ERROR_INET;
	}
	
	// HTTP ヘッダがある場合はそれを追加する
	if (header.size()) {
		vector<tstring>::iterator it = header.begin();
		while (it != header.end()) {
			tstring h = *it;
			HttpAddRequestHeaders(hReq, h.c_str(), (DWORD)h.size(), 
								  HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
			it++;
		}
	}

	// 証明書系を無視する場合の設定を行う
	if (secure && !checkCert) {
		DWORD dwError = 0;
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);
		BOOL ret;

		if ((ret = InternetQueryOption(hReq, INTERNET_OPTION_SECURITY_FLAGS,
									   (LPVOID)&dwFlags, &dwBuffLen)) == FALSE) {
			storeErrorMessage(GetLastError(), errorMessage);
			closeHandle();
			return ERROR_INET;
		}

		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		dwFlags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
		dwFlags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
		dwFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
		// dwFlags |= SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP;
		// dwFlags |= SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS;

		if ((ret = InternetSetOption(hReq, INTERNET_OPTION_SECURITY_FLAGS,
								(LPVOID)&dwFlags, sizeof(dwFlags))) == FALSE) {
			storeErrorMessage(GetLastError(), errorMessage);
			closeHandle();
			return ERROR_INET;
		}
	}

	INTERNET_BUFFERS BufferIn = {0};
	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS );
	BufferIn.dwBufferTotal = requestContentLength;

	// リクエスト送信開始
again:
	if (!HttpSendRequestEx(hReq, &BufferIn, NULL, 0, 0)) {
		DWORD dwError = GetLastError();
		
		// 証明書関連エラーの復帰処理
		if (dwError == ERROR_INTERNET_INVALID_CA ||
			dwError == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
			dwError == ERROR_INTERNET_SEC_CERT_CN_INVALID ||
			dwError == ERROR_INTERNET_SEC_CERT_REV_FAILED) {
			if (InternetErrorDlg (GetDesktopWindow(),
								  hReq,
								  dwError,
								  FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
								  FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
								  FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
								  NULL) == ERROR_SUCCESS) {
				if (dwError == ERROR_INTERNET_SEC_CERT_REV_FAILED){
					// なぜかこのエラーは正しくオプションが変更されないので手動で変更
					DWORD dwFlags;
					DWORD dwBuffLen = sizeof(dwFlags);
					if (InternetQueryOption(hReq, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen)) {
						dwFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
						InternetSetOption(hReq, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, sizeof(dwFlags));
					}
				}
				goto again;
			}
		}
		storeErrorMessage(dwError, errorMessage);
		closeHandle();
		return ERROR_INET;
	}

	// ファイル書き出し
	if (requestCallback) {
		DWORD len;
		do {
			BYTE work[BUFSIZE];
			len = sizeof work;
			if (!requestCallback(context, work, len)) {
				closeHandle();
				return ERROR_CANCEL;
			}
			if (len > 0) {
				DWORD n = 0;
				DWORD size = len;
				while (size > 0) {
					DWORD l;
					if (InternetWriteFile(hReq, work+n, size, &l)) {
						size -= l;
						n += l;
					} else {
						storeErrorMessage(GetLastError(), errorMessage);
						closeHandle();
						return ERROR_INET;
					}
				}
			}
		} while (len > 0);
	}

	// リクエスト完了
	if (!HttpEndRequest(hReq, NULL, 0, NULL)) {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INTERNET_FORCE_RETRY) {
		  if (retryCallback) 
		    retryCallback(context);
		  goto again;
		}

		storeErrorMessage(GetLastError(), errorMessage);
		closeHandle();
		return ERROR_INET;
	}
	
	return ERROR_NONE;
}

/**
 * レスポンス受信
 */
void
HttpConnection::queryInfo()
{
	LOCK;

	statusCode = 0;
	DWORD length = sizeof statusCode;
	HttpQueryInfo(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&statusCode, &length, NULL);

	statusText.erase();
	length = 0;
	if (!HttpQueryInfo(hReq, HTTP_QUERY_STATUS_TEXT, NULL, &length, NULL) &&
		GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		TCHAR *buf = (TCHAR*)malloc(sizeof(*buf) * length);
		if (buf && HttpQueryInfo(hReq, HTTP_QUERY_STATUS_TEXT, buf, &length, NULL)) {
			statusText = tstring(buf, length);
		}
		free(buf);
	}

	contentLength = 0;
	length = sizeof contentLength;
	validContentLength = HttpQueryInfo(hReq, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
									   (LPVOID)&contentLength,  &length, 0) != FALSE;
	
	contentType.erase();
	encoding.erase();
	length = 0;
	if (!HttpQueryInfo(hReq, HTTP_QUERY_CONTENT_TYPE, NULL, &length, NULL) &&
		GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		TCHAR *buf = (TCHAR*)malloc(sizeof(*buf) * length);
		if (buf && HttpQueryInfo(hReq, HTTP_QUERY_CONTENT_TYPE, buf, &length, NULL)) {
			parseContentType(buf, length, contentType, encoding);
		}
		free(buf);
	}
	
	// 全ヘッダを解析して取得
	responseHeaders.clear();
	length = 0;
	if (!HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS, NULL, &length, NULL) &&
		GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		TCHAR *buf = (TCHAR*)malloc(sizeof(*buf) * length);
		if (buf && HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS, buf, &length, NULL) && length > 0) {
			size_t n = 0;
			while (n<length) {
				TCHAR *p = buf + n;
				int len = 0;
				while (n+len<length && p[len]) {
					len++;
				}
				if (len > 0) {
					int n2 = 0;
					while (n2 < len) {
						if (p[n2] == ':') {
							tstring name(p,n2++);
							tstring value(p+n2,len-n2);
							responseHeaders[name] = value;
							break;
						}
						n2++;
					}
				}
				n += (len+1);
			}
		}
		free(buf);
	}
}


/**
 * レスポンス受信
 */
int 
HttpConnection::response(ResponseCallback callback, void *context)
{
	LOCK;

	if (!isValid()) {
		return ERROR_INET;
	}
	
	// HTTP が OK を返した場合のみファイルセーブを enable にする
	if (statusCode == HTTP_STATUS_OK && callback) {
		
		// HTMLをパースして Content-Type からエンコーディングを取得する必要がある
		bool needParseHtml = _tcsicmp(contentType.c_str(), _T("text/html")) == 0 && encoding.empty();

		DWORD size = 0;
		DWORD len;
		BYTE work[BUFSIZE];
		while (InternetReadFile(hReq, (void*)work, sizeof work, &len) && len > 0) {
			size += len;
			if (needParseHtml) {
				// 取得したファイル中から METAタグを参照して Content-Type を再取得
				tstring ctype;
#ifdef _UNICODE
				TCHAR *buf = new TCHAR[len];
				for (DWORD i=0;i<len;i++) {
					buf[i] = work[i];
				}
				tstring text(buf,len);
				delete[] buf;
#else
				tstring text(work,len);
#endif
				if (matchContentType(text, ctype)) {
					parseContentType(ctype.c_str(), ctype.size(), contentType, encoding);
				}
				needParseHtml = false;
			}
			if (!callback(context, work, len)) {
				closeHandle();
				return ERROR_CANCEL;
			}
		}
		if (!validContentLength) {
			contentLength = size;
		}
	}
	closeHandle();
	return ERROR_NONE;
}
