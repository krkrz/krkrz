#ifndef _serv_hpp_
#define _serv_hpp_

/*
 *  Poco::Net ラッパー（includeが混在するとちょっと困ったことになるので分離する）
 */

#include <string>

// レスポンス処理用
struct PwRequestResponse
{
	typedef std::string String;
	typedef unsigned long Size;
	typedef void (*NameValueCallback)(const String&, const String&, void *param);

	// スレッド対応用
	virtual void done() = 0;

	// 各種データを取得
	virtual int  getHeader  (NameValueCallback, void *param) const = 0;
	virtual int  getFormData(NameValueCallback, void *param) const = 0;
	virtual const String& getMethod() const = 0;
	virtual const String& getURI()    const = 0;
	virtual const String& getPath()   const = 0;
	virtual const String& getHost()   const = 0;
	virtual const String& getClient() const = 0;

	// 変換ユーティリティ
	virtual const String getCharset(char const *mediatype) = 0;
	virtual const String getReason(char const *status) = 0;

	// レスポンスを返す
	virtual void setStatus(char const *status) = 0;
	virtual void setContentType(char const *type) = 0;
	virtual void setRedirect(char const *type) = 0;
	// 以下は最後にどちらか１回のみしか呼べない（Content-lengthを送信してしまうため分割送信は不可）
	virtual void sendBuffer(void const*, Size length) = 0;
	virtual void sendFile(char const *path) = 0;
};

// サーバ用
struct PwHTTPServer
{
	typedef void (*RequestCallback)(PwRequestResponse *rr, void *param);

	virtual ~PwHTTPServer() {}
	virtual int  start(int port) = 0;
	virtual void stop()          = 0;

	static PwHTTPServer* Factory(RequestCallback, void *param, int timeoutsec);
};

#endif
