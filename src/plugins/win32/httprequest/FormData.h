#ifndef __FORMDATA_H__
#define __FORMDATA_H__

#include <vector>

/**
 * application/x-www-form-urlencoded 用のデータを生成する
 * ※登録されたデータは UTF-8 でエンコーディング
 */
class FormData {

public:
	/**
	 * コンストラクタ
	 */
	FormData() : hasParam(false) {}

	/**
	 * key と value の組を URL-encode して追加
	 * @param key キー
	 * @param value 登録する値
	 */
	void addParam(const TCHAR *key, const TCHAR *value) {
		if (hasParam) {
			data.push_back('&');
		} 
		addEncodedString(key);
		data.push_back('=');
		addEncodedString(value);
		hasParam = true;
	}

	/**
	 * key と value の組を URL-encode して追加
	 * @param key キー
	 * @param num 登録する値（数値）
	 */
	void addParam(const TCHAR *key, int num) {
		TCHAR value[100];
		_sntprintf_s(value, sizeof value, _T("%d"), num);
		addParam(key, value);
	}

	/**
	 * key と value の組を URL-encode して追加
	 * @param key キー
	 * @param value 登録する値（BOOL)
	 */
	void addParam(const TCHAR *key, bool value) {
		addParam(key, value ? _T("1") : _T("0"));
	}

	// HTTP で送るデータを取得
	const BYTE *getData() const {
		if (data.size() <= 0) {
			return NULL;
		} else {
			return &data[0];
		}
	}

	// URL エンコードした文字列をデータとして追記する
	void _addEncodedString(const char *p) {
		int ch;
		while ((ch = *p++)) {
			if (ch >= '0' && ch <= '9' ||
				ch >= 'a' && ch <= 'z' ||
				ch >= 'A' && ch <= 'Z') {
				data.push_back(ch);
			} else {
				data.push_back('%');
				data.push_back("0123456789ABCDEF"[ch / 16]);
				data.push_back("0123456789ABCDEF"[ch % 16]);
			}
		}
	}
	
protected:
	// URL エンコードした文字列をデータとして追記する
	void addEncodedString(const TCHAR *str) {
#ifdef _UNICODE
		// UTF-8 文字列に戻す
		int len = _tcslen(str);
		int mblen = ::WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
		char *buf = new char[mblen + 1];
		::WideCharToMultiByte(CP_UTF8, 0, str, len, buf, mblen, NULL, NULL);
		buf[mblen] = '\0';
		_addEncodedString(str);
		delete[] buf;
#else
		// そのまま処理
		_addEncodedString(str);
#endif
	}
	
private:
	std::vector<BYTE>data;	    ///< put/post のデータ(GETのパラメータも兼ねる)
	bool hasParam;	        ///< パラメータ(データ)がセットされた状態か
};

#endif
