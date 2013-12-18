#include "ncbind.hpp"
#include "serv.hpp"

// メッセージコード
#define	WM_HTTP_REQUEST	(WM_APP+8)	// リクエストされた

#define CP_SJIS 932
#define CP_EUC  20932 //51932
#define CP_JIS  50220

class SimpleHTTPServerResponse
{
public:
	typedef SimpleHTTPServerResponse SelfClass;
	enum TransferType { UNKNOWN, TEXT, FILE, BINARY };
	typedef PwRequestResponse::String PwStr;
	typedef PwRequestResponse::Size   PwSize;
private:
	iTJSDispatch2 *owner;
	PwRequestResponse *rr;

	int   status, transfer_type, codepage;
	PwSize           transfer_binsize;
	tjs_uint8 const *transfer_binptr;
	tjs_uint8       *transfer_allocated;
	ttstr content_type, transfer_text, transfer_file;

	ncbDictionaryAccessor arg;
	tTJSVariant result;

	PwStr toNarrowString(ttstr const &ref) {
		PwStr ret;
		tjs_int len = ref.GetNarrowStrLen();
		tjs_nchar *tmp = new tjs_nchar[len+1];
		ref.ToNarrowStr(tmp, len+1);
		ret.assign(  tmp, len);
		delete [] tmp;
		return ret;
	}

	static void TJS_USERENTRY TryRequestBlock(void *data) {
		SelfClass *self = (SelfClass*)data;
		if (self) self->callOnRequest();
	}
	static bool TJS_USERENTRY CatchRequestBlock(void *data, const tTVPExceptionDesc & desc) {
		SelfClass *self = (SelfClass*)data;
		if (self) self->onRequestError(desc);
		return false;
	}

	struct NameValueCBWork {
		int codepage;
		ncbPropAccessor *dic;
	};
	static void NameValueCallback(const PwStr &key, const PwStr &value, void *data) {
		NameValueCBWork *wk = (NameValueCBWork*)data;
		if (wk != NULL && wk->dic->IsValid()) {
			int cp = wk->codepage;
			ttstr vkey(ConvertInputText(key, cp)), vval(ConvertInputText(value, cp));
			if (vkey.length() > 0) wk->dic->SetValue(vkey.c_str(), vval);
		}
	}

	static PwSize ConvertOutputText(ttstr const &text, int cp, tjs_uint8* &mem) {
		int rlen = text.length();
		PwSize wlen = ::WideCharToMultiByte(cp, 0, text.c_str(), rlen, NULL,       0,    NULL, NULL);
		/**/   mem  = new tjs_uint8[wlen + 1];
		/**/   wlen = ::WideCharToMultiByte(cp, 0, text.c_str(), rlen, (LPSTR)mem, wlen, NULL, NULL);
		mem[wlen] = 0;
		return wlen;
	}
	static ttstr ConvertInputText(PwStr const &text, int cp) {
		int rlen = text.length();
		PwSize wlen = ::MultiByteToWideChar(cp, 0, text.c_str(), rlen, NULL,      0);
		tjs_char* p = new tjs_char[wlen + 1];
		/**/   wlen = ::MultiByteToWideChar(cp, 0, text.c_str(), rlen, (LPWSTR)p, wlen);
		p[wlen] = 0;
		ttstr ret(p);
		delete [] p;
		return ret;
	}

public:
	~SimpleHTTPServerResponse() {
		if (transfer_allocated) delete [] transfer_allocated;
	}
	SimpleHTTPServerResponse(iTJSDispatch2 *obj, PwRequestResponse *_rr, int cp) :
	owner(obj), rr(_rr), status(0), transfer_type(UNKNOWN), codepage(cp),
	transfer_binsize(0), transfer_binptr(0), transfer_allocated(0) {
		ncbDictionaryAccessor head;
		ncbDictionaryAccessor form;
		{
			NameValueCBWork nvcbwk = { codepage, &head };
			rr->getHeader  (NameValueCallback, (void*)&nvcbwk);
			nvcbwk.dic = &form;
			rr->getFormData(NameValueCallback, (void*)&nvcbwk);
		}
		arg.SetValue(TJS_W("method"),  ConvertInputText(rr->getMethod(), CP_ACP));
		arg.SetValue(TJS_W("request"), ConvertInputText(rr->getURI(),    CP_ACP));
		arg.SetValue(TJS_W("path"),    ConvertInputText(rr->getPath(),   codepage));
		arg.SetValue(TJS_W("host"),    ConvertInputText(rr->getHost(),   CP_ACP));
		arg.SetValue(TJS_W("client"),  ConvertInputText(rr->getClient(), CP_ACP));
		arg.SetValue(TJS_W("header"),  tTJSVariant(head, head));
		arg.SetValue(TJS_W("form"),    tTJSVariant(form, form));
	}

	void request() {
		TVPDoTryBlock(TryRequestBlock, CatchRequestBlock, NULL, (void*)this);
	}
	void response() {
		PwStr tempstr;
		// ファイル転送時は先に存在チェック（無ければ404）
		if (transfer_type == FILE) {
			ttstr fname = TVPGetPlacedPath(transfer_file);
			if (fname.length() > 0 && TVPIsExistentStorage(fname)) {
				if (fname.length() > 0 && !wcschr(fname.c_str(), '>')) {
					// 通常のファイルはローカルパスで直接渡す
					TVPGetLocalName(fname);
					transfer_file = fname;
				} else {
					// アーカイブ内の場合はオンメモリでバイナリ転送に変更する
					IStream *file = TVPCreateIStream(fname, TJS_BS_READ);
					if (!file) setError(404, TJS_W("file not found"), transfer_file.c_str());
					else {
						transfer_allocated = 0;
						try {
							STATSTG stat;
							file->Stat(&stat, STATFLAG_NONAME);
							PwSize size = (PwSize)stat.cbSize.QuadPart; // XXX 巨大ファイルは無理
							tjs_uint8 *p = new tjs_uint8[size];
							file->Read(p, size, &transfer_binsize);
							transfer_binptr = transfer_allocated = p;
							transfer_type = BINARY;
						} catch (...) {
							if (file) file->Release();
							if (transfer_allocated) delete [] transfer_allocated;
							transfer_allocated = 0;
							throw;
						}
						if (file) file->Release();
					}
				}
			} else {
				setError(404, TJS_W("file not found"), transfer_file.c_str());
			}
		}
		// ステータス設定
		if (status > 0) {
			PwStr tmp(toNarrowString(ttstr((tjs_int)status)));
			rr->setStatus(tmp.c_str());
		}

		// content-type設定
		if (content_type.length() > 0) {
			PwStr tmp(toNarrowString(content_type));
			rr->setContentType(tmp.c_str());
			tmp = rr->getCharset(tmp.c_str());
			ttstr cset(tmp.c_str());
			if (cset.length() > 0) {
				cset.ToLowerCase();
				// 投げやりな charset -> codepage 判定
				if      (cset.StartsWith(TJS_W("utf-8"      ))) codepage = CP_UTF8;
				else if (cset.StartsWith(TJS_W("shift"      ))) codepage = CP_SJIS;
				else if (cset.StartsWith(TJS_W("sjis"       ))) codepage = CP_SJIS;
				else if (cset.StartsWith(TJS_W("x-sjis"     ))) codepage = CP_SJIS;
				else if (cset.StartsWith(TJS_W("windows-31j"))) codepage = CP_SJIS;
				else if (cset.StartsWith(TJS_W("euc"        ))) codepage = CP_EUC;
				else if (cset.StartsWith(TJS_W("iso-2022-jp"))) codepage = CP_JIS;
				else if (cset.StartsWith(TJS_W("jis"        ))) codepage = CP_JIS;
			}
		}
		// 転送処理
		switch (transfer_type) {
		case UNKNOWN:
			rr->sendBuffer("", 0);
		case FILE:
			{
				PwStr tmp(toNarrowString(transfer_file));
				rr->sendFile(tmp.c_str());
			}
			break;
		case TEXT:
			transfer_binsize = ConvertOutputText(transfer_text, codepage, transfer_allocated);
			transfer_binptr = transfer_allocated;
			/* NOT BREAK */
		case BINARY:
			rr->sendBuffer(transfer_binptr, transfer_binsize);
			break;
		}
	}
	void callOnRequest() {
		tTJSVariantClosure closure(owner, owner);
		tTJSVariant param(arg, arg), *p[] = { &param };
		const tjs_char *method = TJS_W("onRequest");
		if (TJS_SUCCEEDED(closure.FuncCall(0, method, NULL, &result, 1, p, NULL))) {
			if (result.Type() == tvtObject) setResult(result.AsObjectNoAddRef(),   method);
			else
				setError(500, TJS_W("response"), TJS_W("no object returned from"), method);
		} else  setError(500, TJS_W("callback"), TJS_W("failed to call"),          method);
	}
	void onRequestError(const tTVPExceptionDesc & desc) {
		setError(500, desc.type.c_str(), desc.message.c_str());
	}

	// textデータ
	void setTransferText(ttstr const& text) {
		transfer_type = TEXT;
		transfer_text = text;
		if (status < 0) status = 200;
	}
	// file(IStream)データ
	void setTransferFile(ttstr const& file) {
		transfer_type = FILE;
		transfer_file = file;
		if (status < 0) status = 200;
	}
	// Arrayデータ
	void setTransferObject(iTJSDispatch2 *obj, const tjs_char* method) {
		if (!obj->IsInstanceOf(0, NULL, NULL, TJS_W("Array"), obj)) 
			setError(501, TJS_W("invalid data"), TJS_W("no array object returned from"), method);
		else {
			ncbPropAccessor arr(obj);
			transfer_binsize = arr.GetArrayCount();
			tjs_uint8 *p  = new tjs_uint8[transfer_binsize];
			transfer_binptr = transfer_allocated = p;
			for (PwSize i = 0; i < transfer_binsize; i++) {
				p[i] = arr.getIntValue(i) & 0xFF;
			}
		}
	}
	// Octetデータ
	void setTransferOctet(tTJSVariantOctet *oct, const tjs_char* method) {
		transfer_binsize = oct->GetLength();
		if (!transfer_binsize) 
			setError(500, TJS_W("no data"), TJS_W("zero-length octet transfer returned from"), method);
		transfer_binptr = oct->GetData();
	}

	// レスポンスデータを解析
	void setResult(iTJSDispatch2 *obj, const tjs_char* method) {
		ncbPropAccessor dic(obj);
		status         = dic.getIntValue(TJS_W("status"), -1);
		content_type   = dic.getStrValue(TJS_W("content_type"));

		// 転送タイプ別処理
		ncbTypedefs::Tag<tTJSVariant> tag;
		if      (dic.HasValue(TJS_W("text")))  setTransferText (dic.getStrValue(TJS_W("text")));
		else if (dic.HasValue(TJS_W("file")))  setTransferFile (dic.getStrValue(TJS_W("file")));
		else if (dic.HasValue(TJS_W("binary"))) {
			tTJSVariant v(dic.GetValue(TJS_W("binary"), tag));
			switch (v.Type()) {
			case tvtObject: setTransferObject(v.AsObjectNoAddRef(), method); break;
			case tvtOctet:  setTransferOctet (v.AsOctetNoAddRef(),  method); break;
			default:
				setError(500, TJS_W("invalid data"), TJS_W("invalid binary data returned from"), method);
				break;
			}
		}

		// リダイレクト処理
		ttstr redirect = dic.getStrValue(TJS_W("redirect"));
		if (redirect.length() > 0) {
			if (status != 301 && status != 302 && status != 307) status = 301;
			PwStr tmp(toNarrowString(redirect));
			rr->setRedirect(tmp.c_str());
			if (transfer_type == UNKNOWN) {
				content_type  =  TJS_W("text/html");
				ttstr refresh =  TJS_W("<html><head><meta http-equiv=refresh content=\"0; url=");
				refresh += redirect;
				refresh += TJS_W("\"></head><body><a href=\"");
				refresh += redirect;
				refresh += TJS_W("\">Redirect...</a></body></html>");
				setTransferText(refresh);
			}
		}

		// 何も転送するものが無い
		if (transfer_type == UNKNOWN || status < 0) {
			int st = (status < 0) ? 500 : status;
			ttstr type = dic.getStrValue(TJS_W("error_type"));
			ttstr desc = dic.getStrValue(TJS_W("error_desc"));
			if (type.length() > 0 && type.length() > 0) {
				setError(st, type, desc);
			} else {
				setError(st, TJS_W("no data"), TJS_W("no transfer data returned from"), method);
			}
		}
	}

	void setError(int st, ttstr type, ttstr desc) {
		status = st;
		ttstr numstr((tjs_int)status);
		PwStr reason(toNarrowString(numstr));
		reason = rr->getReason(reason.c_str());
		content_type = "text/html; charset=utf-8";
		ttstr error(TJS_W("<html><head><title>Error</title></head><body><h1>"));
		error += numstr;
		error += TJS_W(" ");
		error += ttstr(reason.c_str());
		error += TJS_W("</h1><p>");
		error += type;
		error += TJS_W(" : ");
		error += desc;
		error += TJS_W("</p></body></html>");
		setTransferText(error);
	}
	void setError(int st, const tjs_char *type, const tjs_char *desc, const tjs_char *method) {
		ttstr t(desc);
		t += TJS_W(" ");
		t += method;
		t += TJS_W("method.");
		setError(st, ttstr(type), t);
	}
};

class SimpleHTTPServer
{
public:
	typedef SimpleHTTPServer SelfClass;
private:
	iTJSDispatch2 *self;
	PwHTTPServer *instance;
	HWND message;
	int port, timeout, codepage;

	static ATOM WindowClass;
	HWND createMessageWindow() {
		HINSTANCE hinst = ::GetModuleHandle(NULL);
		if (!WindowClass) {
			WNDCLASSEXW wcex = {
				/*size*/sizeof(WNDCLASSEX), /*style*/0, /*proc*/WndProc, /*extra*/0L,0L, /*hinst*/hinst,
				/*icon*/NULL, /*cursor*/NULL, /*brush*/NULL, /*menu*/NULL,
				/*class*/TJS_W("SimpleHTTPServer Message Window Class"), /*smicon*/NULL };
			WindowClass = ::RegisterClassExW(&wcex);
			if (!WindowClass)
				TVPThrowExceptionMessage(TJS_W("register window class failed."));
		}
		HWND hwnd = ::CreateWindowExW(0, (LPCWSTR)MAKELONG(WindowClass, 0), TJS_W("SimpleHHTPServer Message"),
									  0, 0, 0, 1, 1, HWND_MESSAGE, NULL, hinst, NULL);
		if (!hwnd) TVPThrowExceptionMessage(TJS_W("create message window failed."));
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
		return hwnd;
	}
	static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		if (msg == WM_HTTP_REQUEST) {
			SelfClass *self = (SelfClass*)(::GetWindowLong(hwnd, GWL_USERDATA));
			PwRequestResponse *rr = (PwRequestResponse*)lp;
			if (self && rr) self->onRequest(rr);
			if (rr) rr->done();
			return 0;
		}
		return DefWindowProc(hwnd, msg, wp, lp);
	}

public:
	SimpleHTTPServer(iTJSDispatch2 *obj, tjs_int n, tTJSVariant **p) :
	self(obj), instance(0), message(0), port(0), timeout(10), codepage(CP_UTF8) {
		if (n > 0 && p[0]->Type() == tvtInteger) port     = (int)p[0]->AsInteger();
		if (n > 1 && p[1]->Type() == tvtInteger) timeout  = (int)p[1]->AsInteger();
		if (n > 2 && p[2]->Type() == tvtInteger) codepage = (int)p[2]->AsInteger();
		message = createMessageWindow();
		instance = PwHTTPServer::Factory(&RequestCallback, (void*)message, timeout);
	}
	~SimpleHTTPServer() {
		if (message) {
			::SetWindowLong(message, GWL_USERDATA, 0);
			::DestroyWindow(message);
		}
		message = NULL;
		stop();
		if (instance) delete instance;
	}
	static tjs_error TJS_INTF_METHOD Factory(SimpleHTTPServer **instance, tjs_int n, tTJSVariant **p, iTJSDispatch2 *self) {
		*instance = new SimpleHTTPServer(self, n, p);
		return TJS_S_OK;
	}

	// 別スレッドから呼ばれるのでメッセージウィンドウにメッセージを投げてメインスレッド側で実行する
	static void RequestCallback(PwRequestResponse *rr, void *param) {
		HWND hwnd = (HWND)param;
		if (hwnd) ::PostMessage(hwnd, WM_HTTP_REQUEST, 0, (LPARAM)rr);
	}

	// メッセージウィンドウからの呼び返しによってリクエストに対応
	void onRequest(PwRequestResponse *rr) {
		SimpleHTTPServerResponse res(self, rr, codepage);
		res.request();
		res.response();
	}

	int  getPort()    const { return port; }
	int  getTimeout() const { return timeout; }
	int  start() { if (instance) port = instance->start(port); return port; }
	void stop()  { if (instance)        instance->stop(); }

	int  getCodePage() const { return codepage;      }
	void setCodePage(int cp) {        codepage = cp; }
};

ATOM SimpleHTTPServer::WindowClass = 0;


NCB_REGISTER_CLASS(SimpleHTTPServer)
{
	Factory(&Class::Factory);
	Property(TJS_W("port"   ),  &Class::getPort,     0);
	Property(TJS_W("timeout"),  &Class::getTimeout,  0);
	Property(TJS_W("codepage"), &Class::getCodePage, &Class::setCodePage);
	Method(  TJS_W("start"), &Class::start);
	Method(  TJS_W("stop" ), &Class::stop);

	Variant(TJS_W("cpACP"),  (tjs_int)CP_ACP);
	Variant(TJS_W("cpOEM"),  (tjs_int)CP_OEMCP);
	Variant(TJS_W("cpUTF8"), (tjs_int)CP_UTF8);
	Variant(TJS_W("cpSJIS"), (tjs_int)CP_SJIS);
	Variant(TJS_W("cpEUC"),  (tjs_int)CP_EUC );
	Variant(TJS_W("cpJIS"),  (tjs_int)CP_JIS );
}

