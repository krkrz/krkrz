#include <windows.h>
#include <activscp.h>
#include <stdio.h>
#include <map>

#include "ncbind/ncbind.hpp"

#define GLOBAL L"kirikiri"

// ログ出力用
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "../win32ole/IDispatchWrapper.hpp"

//---------------------------------------------------------------------------

/*
 * Windows Script Host 処理用ネイティブインスタンス
 */
class WindowsScriptHost : IActiveScriptSite
{
public:
	static WindowsScriptHost *singleton;

	static void initSingleton() {
		singleton = new WindowsScriptHost();
	}

	static void removeSingleton() {
		if (singleton) {
			delete singleton;
			singleton = NULL;
		}
	}
	
protected:
	/// tjs global 保持用
	IDispatchEx *global;

	// ------------------------------------------------------
	// IUnknown 実装
	// ------------------------------------------------------
protected:
	ULONG refCount;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) {
		*ppvObject = NULL;
		return E_NOTIMPL;
	}
	virtual ULONG _stdcall AddRef(void) {
		return ++refCount;
	}
	virtual ULONG _stdcall Release(void) {
		if(--refCount <= 0) return 0;
		return refCount;
	}

	// ------------------------------------------------------
	// IActiveScriptSite 実装
	// ------------------------------------------------------
public:
	virtual HRESULT __stdcall GetLCID(LCID *plcid) {
		return S_OK;
	}

	virtual HRESULT __stdcall GetItemInfo(LPCOLESTR pstrName,
										  DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppti) {
		if (ppti) {
			*ppti = NULL;
		}
		if (ppunkItem) {
			*ppunkItem = NULL;
			if (dwReturnMask & SCRIPTINFO_IUNKNOWN) {
				if (!_wcsicmp(GLOBAL, pstrName)) {
					global->AddRef();
					*ppunkItem = global;
				}
			}
		}
		return S_OK;
	}
	
	virtual HRESULT __stdcall GetDocVersionString(BSTR *pbstrVersion) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *ei) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnStateChange(SCRIPTSTATE ssScriptState) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnScriptError(IActiveScriptError *pscriptError) {
		log(TJS_W("OnScriptError"));
		ttstr errMsg;
		BSTR sourceLine;
		if (pscriptError->GetSourceLineText(&sourceLine) == S_OK) {
			log(TJS_W("source:%ls"), sourceLine);
			::SysFreeString(sourceLine);
		}
		DWORD sourceContext;
		ULONG lineNumber;
		LONG charPosition;
		if (pscriptError->GetSourcePosition(
			&sourceContext,
			&lineNumber,
			&charPosition) == S_OK) {
			log(TJS_W("context:%ld lineNo:%d pos:%d"), sourceContext, lineNumber, charPosition);
		}		
		EXCEPINFO ei;
		memset(&ei, 0, sizeof ei);
		if (pscriptError->GetExceptionInfo(&ei) == S_OK) {
			log(TJS_W("exception code:%x desc:%ls"), ei.wCode, ei.bstrDescription);
		}
		return S_OK;
	}

	virtual HRESULT __stdcall OnEnterScript(void) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnLeaveScript(void) {
		return S_OK;
	}

	// ------------------------------------------------------
	// 処理部
	// ------------------------------------------------------

protected:
	/// 拡張子とProgId のマッピング
	map<ttstr, ttstr> extMap;
	// CLSID 比較用
	struct CompareCLSID : public binary_function<CLSID,CLSID,bool> {
		bool operator() (const CLSID &key1, const CLSID &key2) const {
#define CHK(a) if (key1.a!=key2.a) { return key1.a<key2.a; }
			CHK(Data1);
			CHK(Data2);
			CHK(Data3);
			CHK(Data4[0]);
			CHK(Data4[1]);
			CHK(Data4[2]);
			CHK(Data4[3]);
			CHK(Data4[4]);
			CHK(Data4[5]);
			CHK(Data4[6]);
			CHK(Data4[7]);
			return false;
		}
	};
	map<CLSID, IActiveScript*, CompareCLSID> scriptMap;

	/**
	 * 指定された ActiveScript エンジンを取得する
	 * @param type 拡張子 または progId または CLSID
	 * @return エンジンインターフェース
	 */
	IActiveScript *getScript(const tjs_char *type) {
		HRESULT hr;
		CLSID   clsid;
		
		// ProgId または CLSID の文字列表現からエンジンの CLSID を決定する
		OLECHAR *oleType = ::SysAllocString(type);
		if (FAILED(hr = CLSIDFromProgID(oleType, &clsid))) {
			hr = CLSIDFromString(oleType, &clsid);
		}
		::SysFreeString(oleType);

		if (SUCCEEDED(hr)) {
			map<CLSID, IActiveScript*, CompareCLSID>::const_iterator n = scriptMap.find(clsid);
			if (n != scriptMap.end()) {
				// すでに取得済みのエンジンの場合はそれを返す
				return n->second;
			} else {
				// 新規取得
				IActiveScript *pScript;
				hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&pScript);
				if (SUCCEEDED(hr)) {
					IActiveScriptParse *pScriptParse;
					if (SUCCEEDED(pScript->QueryInterface(IID_IActiveScriptParse, (void **)&pScriptParse))) {
						// ActiveScriptSite を登録
						pScript->SetScriptSite(this);
						// グローバル変数の名前を登録
						pScript->AddNamedItem(GLOBAL, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
						// 初期化
						pScriptParse->InitNew();
						pScriptParse->Release();
						scriptMap[clsid] = pScript;
						return pScript;
					} else {
						log(TJS_W("QueryInterface IActipveScriptParse failed %s"), type);
						pScript->Release();
					}
				} else {
					log(TJS_W("CoCreateInstance failed %s"), type);
				}
			}
		} else {
			log(TJS_W("bad ProgId/CLSID %s"), type);
		}
		return NULL;
	}
	
public:
	/**
	 * コンストラクタ
	 */
	WindowsScriptHost() : global(NULL), refCount(1) {
		// global の取得
		iTJSDispatch2 * tjsGlobal = TVPGetScriptDispatch();
		global = new IDispatchWrapper(tjsGlobal);
		tjsGlobal->Release();
		// 拡張子に対する ProgId のマッピングの登録
		extMap["js"]  = "JScript";
		extMap["vbs"] = "VBScript";
		extMap["pl"]  = "PerlScript";
		extMap["pls"] = "PerlScript";
		extMap["rb"]  = "RubyScript";
		extMap["rbs"] = "RubyScript";
	}

	/**
	 * デストラクタ
	 */
	~WindowsScriptHost() {
		// エンジンの開放
		map<CLSID, IActiveScript*, CompareCLSID>::iterator i = scriptMap.begin();
		while (i != scriptMap.end()) {
			i->second->Close();
			i->second->Release();
			i = scriptMap.erase(i);
		}
		// global を開放
		global->Release();
	}

	/**
	 * 拡張子を ProgId に変換する
	 * @param exe 拡張子
	 * @return ProgId
	 */
	const tjs_char *getProgId(const tjs_char *ext) {
		ttstr extStr(ext);
		extStr.ToLowerCase();
		map<ttstr, ttstr>::const_iterator n = extMap.find(extStr);
		if (n != extMap.end()) {
			return n->second.c_str();
		}
		return ext;
	}

	/**
	 * スクリプトの実行
	 * @param script スクリプト文字列
	 * @param progId スクリプトの種別
	 * @param result 結果格納先
	 */
	tjs_error exec(const tjs_char *script, const tjs_char *progId, tTJSVariant *result) {
		IActiveScript *pScript = getScript(getProgId(progId));
		if (pScript) {
			IActiveScriptParse *pScriptParse;
			if (SUCCEEDED(pScript->QueryInterface(IID_IActiveScriptParse, (void **)&pScriptParse))) {
				
				// 結果格納用
				HRESULT hr;
				EXCEPINFO ei;
				VARIANT rs;
				memset(&ei, 0, sizeof ei);

				BSTR pParseText = ::SysAllocString(script);
				if (SUCCEEDED(hr = pScriptParse->ParseScriptText(pParseText, GLOBAL, NULL, NULL, 0, 0, 0L, &rs, &ei))) {
					hr = pScript->SetScriptState(SCRIPTSTATE_CONNECTED);
				}
				::SysFreeString(pParseText);
				
				switch (hr) {
				case S_OK:
					if (result) {
						IDispatchWrapper::storeVariant(*result, rs);
					}
					return TJS_S_OK;
				case DISP_E_EXCEPTION:
					log(TJS_W("exception code:%x desc:%ls"), ei.wCode, ei.bstrDescription);
					TVPThrowExceptionMessage(TJS_W("exception"));
					break;
				case E_POINTER:
					TVPThrowExceptionMessage(TJS_W("memory error"));
					break;
				case E_INVALIDARG:
					return TJS_E_INVALIDPARAM;
				case E_NOTIMPL:
					return TJS_E_NOTIMPL;
				case E_UNEXPECTED:
					return TJS_E_ACCESSDENYED;
				default:
					log(TJS_W("error:%x"), hr);
					return TJS_E_FAIL;
				}
			}
		}
		return TJS_E_FAIL;
	}

	/**
	 * スクリプトのファイルからの実行
	 * @param script スクリプトファイル名
	 * @param progId スクリプトの種別
	 * @param result 結果格納先
	 */
	tjs_error execStorage(const tjs_char *filename, const tjs_char *progId, tTJSVariant *result) {
		
		if (progId == NULL) {
			const tjs_char *ext = wcsrchr(filename, '.');
			if (ext) {
				progId = ext + 1;
			}
		}
		if (!progId) {
			return TJS_E_FAIL;
		}
		
		iTJSTextReadStream * stream = TVPCreateTextStreamForRead(filename, TJS_W(""));
		tjs_error ret;
		try {
			ttstr data;
			stream->Read(data, 0);
			ret = exec(data.c_str(), progId, result);
		}
		catch(...)
		{
			stream->Destruct();
			throw;
		}
		stream->Destruct();
		return ret;
	}

	/**
	 * 拡張子と ProgId の組を追加登録する
	 * @param exe 拡張子
	 * @param progId ProgId
	 */
	void addProgId(const tjs_char *ext, const tjs_char *progId) {
		ttstr extStr(ext);
		extStr.ToLowerCase();
		extMap[extStr] = progId;
	}

	static void addProgIdMethod(const tjs_char *ext, const tjs_char *progId) {
		if (singleton) {
			singleton->addProgId(ext, progId);
		}
	}

	/**
	 * スクリプトの実行
	 * @param script スクリプト文字列
	 * @param progId スクリプトの種別
	 * @param result 結果格納先
	 */
	static tjs_error execMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (singleton) {
			return singleton->exec(param[0]->GetString(), param[1]->GetString(), result);
		}
		return TJS_E_FAIL;
	}

	/**
	 * スクリプトのファイルからの実行
	 * @param script スクリプトファイル名
	 * @param progId スクリプトの種別
	 * @param result 結果格納先
	 */
	static tjs_error execStorageMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (singleton) {
			return singleton->execStorage(param[0]->GetString(), numparams > 1 ? param[1]->GetString() : NULL, result);
		}
		return TJS_E_FAIL;
	}
};

NCB_ATTACH_CLASS(WindowsScriptHost, Scripts) {
	Method("addProgId",           &WindowsScriptHost::addProgId);
	RawCallback("execWSH",        &WindowsScriptHost::execMethod,        TJS_STATICMEMBER);
	RawCallback("execStorageWSH", &WindowsScriptHost::execStorageMethod, TJS_STATICMEMBER);
};

WindowsScriptHost *WindowsScriptHost::singleton = NULL;

//---------------------------------------------------------------------------

static BOOL gOLEInitialized = false;

/**
 * 登録処理前
 */
static void PreRegistCallback()
{
	if (!gOLEInitialized) {
		if (SUCCEEDED(OleInitialize(NULL))) {
			gOLEInitialized = true;
		} else {
			log(L"OLE 初期化失敗");
		}
	}
}

/**
 * 登録処理後
 */
static void PostRegistCallback()
{
	WindowsScriptHost::initSingleton();
}

/**
 * 開放処理前
 */
static void PreUnregistCallback()
{
	WindowsScriptHost::removeSingleton();
}

/**
 * 開放処理後
 */
static void PostUnregistCallback()
{
	if (gOLEInitialized) {
		OleUninitialize();
		gOLEInitialized = false;
	}
}


NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);

