#include "ncbind/ncbind.hpp"

#include <string>
#include <vector>
using namespace std;
#include <process.h>


#define WM_SIGCHECKPROGRESS (WM_APP+2)
#define WM_SIGCHECKDONE     (WM_APP+3)

//---------------------------------------------------------------------------

class WindowSigCheck;

class SigChecker {

protected:
	// 初期化変数
	WindowSigCheck *notify;
	ttstr filename;
	string publickey;

	/// ユーザ情報
	tTJSVariant info; 

	// キャンセル指示
	bool canceled;

	// 結果変数
	tTJSVariant handler;
	tTJSVariant progressPercent;
	tTJSVariant result;
	tTJSVariant errormsg;

protected:
	void progress(int percent);
	int CheckKrkrExecutable(const char *mark);
	int CheckSignatureOfFile(int ignorestart, int ignoreend, int ofs);
	int CheckSignature();

public:
	// 経過イベント送信
	void eventProgress(iTJSDispatch2 *objthis) {
		tTJSVariant *vars[] = {&handler, &info, &progressPercent};
		objthis->FuncCall(0, L"onCheckSignatureProgress", NULL, NULL, 3, vars, objthis);
	}

	// 終了イベント送信
	void eventDone(iTJSDispatch2 *objthis) {
		tTJSVariant *vars[] = {&handler, &info, &result, &errormsg};
		objthis->FuncCall(0, L"onCheckSignatureDone", NULL, NULL, 4, vars, objthis);
	}
	
public:
	// コンストラクタ
	SigChecker(int handler, WindowSigCheck *notify, const tjs_char *filename, const char *publickey, tTJSVariant &info)
		: handler(handler), notify(notify), filename(filename), publickey(publickey), info(info), canceled(false) {}
	
	// デストラクタ
	~SigChecker() {}

	// ハンドラ取得
	int getHandler() {
		return (int)handler;
	}
	
	// 処理開始
	void start();

	// 処理キャンセル
	void cancel() {
		canceled = true;
	}

	// 強制終了
	void stop() {
		canceled = true;
		notify = NULL;
	}
	
};

class WindowSigCheck {

protected:
	iTJSDispatch2 *objthis; //< オブジェクト情報の参照

	// ユーザメッセージレシーバの登録/解除
	void setReceiver(tTVPWindowMessageReceiver receiver, bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)objthis;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (objthis->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, objthis) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}

public:
	// メッセージ送信
	void postMessage(UINT msg, WPARAM wparam=NULL, LPARAM lparam=NULL) {
		// ウィンドウハンドルを取得して通知
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		HWND hwnd = reinterpret_cast<HWND>((tjs_int)(val));
		::PostMessage(hwnd, msg, wparam, lparam);
	}

protected:

	vector<SigChecker*> checkers;

	// 実行スレッド
	static void checkThread(void *data) {
		((SigChecker*)data)->start();
	}

	// 進捗通知
	void eventProgress(SigChecker *sender) {
		int handler = sender->getHandler();
		if (checkers[handler] == sender) {
			sender->eventProgress(objthis);
		}
	}

	// 終了通知
	void eventDone(SigChecker *sender) {
		int handler = sender->getHandler();
		if (checkers[handler] == sender) {
			checkers[handler] = NULL;
			sender->eventDone(objthis);
		}
		delete sender;
	}

	// ウインドウイベント処理
	static bool __stdcall sigcheckevent(void *userdata, tTVPWindowMessage *Message) {
		if (Message->Msg == WM_SIGCHECKPROGRESS) {
			iTJSDispatch2 *obj = (iTJSDispatch2*)userdata;
			WindowSigCheck *self = ncbInstanceAdaptor<WindowSigCheck>::GetNativeInstance(obj);
			if (self) {
				self->eventProgress((SigChecker*)Message->WParam);
			}
			return true;
		} else if (Message->Msg == WM_SIGCHECKDONE) {
			iTJSDispatch2 *obj = (iTJSDispatch2*)userdata;
			WindowSigCheck *self = ncbInstanceAdaptor<WindowSigCheck>::GetNativeInstance(obj);
			if (self) {
				self->eventDone((SigChecker*)Message->WParam);
			}
			return true;
		}
		return false;
	}
	
public:
	
	WindowSigCheck(iTJSDispatch2 *objthis) : objthis(objthis) {
		setReceiver(sigcheckevent, true);
	}

	~WindowSigCheck() {
		setReceiver(sigcheckevent, false);
		for (int i=0;i<checkers.size();i++) {
			SigChecker *checker = checkers[i];
			if (checker) {
				checker->stop();
				checkers[i] = NULL;
			}
		}
	}

	/**
	 * 署名チェックを行う
	 * @param filename 対象ファイル
	 * @param publickey 公開鍵
	 * @return ハンドラ
	 */
	int checkSignature(const tjs_char *filename, const char *publickey, tTJSVariant info) {
		int handler = checkers.size();
		for (int i=0;i<checkers.size();i++) {
			if (checkers[i] == NULL) {
				handler = i;
				break;
			}
		}
		if (handler >= checkers.size()) {
			checkers.resize(handler + 1);
		}
		SigChecker *checker = new SigChecker(handler, this, filename, publickey, info);
		checkers[handler] = checker;
		_beginthread(checkThread, 0, checker);
		return handler;
	}

	/**
	 * 実行のキャンセル
	 */
	void cancelCheckSignature(int handler) {
		if (handler < checkers.size() && checkers[handler] != NULL) {
			checkers[handler]->cancel();
		}
	}

	/**
	 * 実行の停止
	 */
	void stopCheckSignature(int handler) {
		if (handler < checkers.size() && checkers[handler] != NULL) {
			checkers[handler]->stop();
			checkers[handler] = NULL;
		}
	}


};

//---------------------------------------------------------------------------

// IStream の読み込み位置取得
static DWORD getPosition(IStream *is)
{
	if (is) {
		LARGE_INTEGER move = {0};
		ULARGE_INTEGER newposition;
		if (is->Seek(move, STREAM_SEEK_CUR, &newposition) == S_OK) {
			return (DWORD)newposition.QuadPart;
		}
	}
	return -1;

}

// IStream の読み込み位置指定
static void setPosition(IStream *is, DWORD offset)
{
	if (is) {
		LARGE_INTEGER move;
		move.QuadPart = offset;
		ULARGE_INTEGER newposition;
		is->Seek(move, STREAM_SEEK_SET, &newposition);
	}
}

// IStream のサイズ取得
static DWORD getSize(IStream *is)
{
	if (is) {
		STATSTG stat;
		is->Stat(&stat, STATFLAG_NONAME);
		return (DWORD)stat.cbSize.QuadPart;
	}
	return 0;
}

// copy from 
// kirikiri\kirikiri2\src\tools\win32\sigchecker\krkrsiglib

//---------------------------------------------------------------------------
#define MIN_KRKR_MARK_SEARCH 512*1024
#define MAX_KRKR_MARK_SEARCH 4*1024*1024
//---------------------------------------------------------------------------
char XOPT_EMBED_AREA_[] = " OPT_EMBED_AREA_";
char XCORE_SIG_______[] = " CORE_SIG_______";
char XRELEASE_SIG____[] = " RELEASE_SIG____";
char XP3_SIG[] = " P3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01";   // mark_size = 11
//---------------------------------------------------------------------------

#include <tomcrypt.h>
#undef HASH_PROCESS
#define HASH_INIT    sha256_init
#define HASH_PROCESS sha256_process
#define HASH_DONE    sha256_done
#define HASH_DESC    sha256_desc
#define HASH_METHOD_STRING "SHA256"
#define HASH_METHOD_INTERNAL_STRING "sha256"
#define HASH_SIZE     32

#define EXCEPTION -2
#define CANCELED  -1

int
SigChecker::CheckKrkrExecutable(const char *mark)
{
	// Find the mark in the krkr executable.
	// All krkr executable have
	// "XOPT_EMBED_AREA_" mark at the end of the executable image.
	// Additionally
	// "XCORE_SIG_______" and "XRELEASE_SIG____" are reserved area for
	// signatures.
	// "XP3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01" are optional XP3 archive attached
	// to the executable.
	
	// This function returns the mark offset
	// (area size bofore specified mark)
	int mark_size = strlen(mark);
	
	IStream *st = TVPCreateIStream(filename, TJS_BS_READ);
	if (st == NULL) {
		errormsg = ttstr(L"can't open file:") + filename;
		return EXCEPTION;
	}
	
	int imagesize = 0;
	try {
//		int ofs = MIN_KRKR_MARK_SEARCH;
		int ofs = 0;
		char buf[4096];
		DWORD read;
		bool found = false;
		setPosition(st, ofs);
		while( st->Read(buf, sizeof(buf), &read) == S_OK && read != 0){
			for(int i = 0; i < read; i += 16) {
				// the mark is aligned to paragraph (16bytes)
				if(buf[i] == 'X') {
					if(!memcmp(buf + i + 1, mark + 1, mark_size - 1)) {
						// mark found
						imagesize = i + ofs;
						found = true;
						break;
						}
				}
			}
			if(found) break;
			ofs += read;
//			if(ofs >= MAX_KRKR_MARK_SEARCH) break;
		}
	} catch(...) {
		st->Release();
		errormsg = L"exception";
		return EXCEPTION;
	}
	st->Release();
	return imagesize;
}

//---------------------------------------------------------------------------

int
SigChecker::CheckSignatureOfFile(int ignorestart, int ignoreend, int ofs)
{
	// read publickey
	unsigned char pubbuf[10240];
	unsigned long pubbuf_len;
	{
		const char *inkey = publickey.c_str();
		const char *startline = "-----BEGIN PUBLIC KEY----";
		const char *endline   = "-----END PUBLIC KEY-----";
		
		// read pubkey
		const char *start = strstr(inkey, startline);
		if (!start) {
			errormsg = ttstr(L"Cannot find \"") + startline + "\" in the key string";
			return EXCEPTION;
		}
		const char *end = strstr(inkey, endline);
		if (!end) {
			errormsg = ttstr(L"Cannot find \"") + endline + "\" in the key string";
			return EXCEPTION;
		}
		start += strlen(startline);
		
		int errnum;
		pubbuf_len = sizeof(pubbuf) - 1;
		errnum = base64_decode((const unsigned char*)(start), end - start, pubbuf, &pubbuf_len);
		if(errnum != CRYPT_OK) {
			errormsg = error_to_string(errnum);
			return EXCEPTION;
		}
	}
	
	// read signature file
	unsigned char buf[10240];
	unsigned long buf_len;
	{
		char buf_asc[sizeof(buf)*3/2+2];
		unsigned long buf_asc_len;
		
		IStream *st = NULL;
		if(ofs == -1) {
			// separated
			ttstr signame = filename + L".sig";
			if (TVPGetPlacedPath(signame) == "") {
				errormsg = ttstr(L"not exist:") + signame;
				return EXCEPTION;
			}
			st = TVPCreateIStream(filename + L".sig", TJS_BS_READ);
		} else {
			// embedded
			st = TVPCreateIStream(filename, TJS_BS_READ);
			setPosition(st, ofs);
		}
		if (st == NULL) {
			// 署名ファイルが開けないので失敗
			errormsg = L"can't open signature file";
			return EXCEPTION;
		}
		try {
			st->Read(buf_asc, sizeof(buf_asc) - 1, &buf_asc_len);
		} catch(...) {
			st->Release();
			errormsg = L"exception";
			return EXCEPTION;
		}
		st->Release();
		
		buf_asc[buf_asc_len] = 0;
		buf_asc_len = strlen(buf_asc);
		
		string signmark("-- SIGNATURE - " HASH_METHOD_STRING "/PSS/RSA --");
		if(strncmp(buf_asc, signmark.data(), signmark.size())) {
			errormsg = L"Invalid signature file format";
			return EXCEPTION;
		}
		
		buf_len = sizeof(buf) - 1;
		int errnum = base64_decode((const unsigned char*)(buf_asc + signmark.size()),
								   buf_asc_len - signmark.size(), buf, &buf_len);
		if(errnum != CRYPT_OK) {
			errormsg = error_to_string(errnum);
			return EXCEPTION;
		}
	}

	int pts = 0;
	progress(pts);
	
	// make target hash
	unsigned char hash[HASH_SIZE];
	{
		if (find_hash(HASH_METHOD_INTERNAL_STRING) == -1) {
			int errnum = register_hash(&HASH_DESC);
			if(errnum != CRYPT_OK) {
				errormsg = error_to_string(errnum);
				return EXCEPTION;
			}
		}
		if (canceled) {
			return CANCELED;
		}

		IStream *st = TVPCreateIStream(filename, TJS_BS_READ);
		if (st == NULL) {
			errormsg = ttstr(L"can't open file:") + filename;
			return EXCEPTION;
		}
		
		int size = getSize(st);
		if(ignorestart != -1 && ignoreend == -1) ignoreend = size;
		int signfnsize = size;

		try {
			hash_state state;
			HASH_INIT(&state);
			
			DWORD read;
			unsigned char buf[4096];
			int ofs = 0;
			while(st->Read(buf, sizeof(buf), &read) == S_OK && read != 0) {
				if(ignorestart != -1 && read + ofs > ignorestart) {
					read = ignorestart - ofs;
					if(read) HASH_PROCESS(&state, buf, read);
					break;
				} else {
					HASH_PROCESS(&state, buf, read);
				}
				ofs += read;
				
				if (canceled) {
					st->Release();
					return CANCELED;
				}
				// callback notify
				int npts = (int)( (__int64)ofs * (__int64)100 / (__int64)signfnsize );
				if(pts < npts) {
					pts = npts;
					progress(pts);
				}
			}
			
			if (ignorestart != -1 && getPosition(st) != ignoreend) {
				setPosition(st, (ofs = ignoreend));
				while (st->Read(buf, sizeof(buf), &read) == S_OK && read != 0) {
					HASH_PROCESS(&state, buf, read);
					ofs += read;

					if (canceled) {
						st->Release();
						return CANCELED;
					}
					// callback notify
					int npts = (int)( (__int64)ofs * (__int64)100 / (__int64)signfnsize );
					if(pts < npts) {
						pts = npts;
						progress(pts);
					}
				}
			}
			HASH_DONE(&state, hash);
		} catch(...) {
			st->Release();
			errormsg = L"exception";
			return EXCEPTION;
		}
		st->Release();
	}

	if (canceled) {
		return CANCELED;
	}
	
	// 結果判定
	int stat = 0;
	{
		rsa_key key;
		int errnum;
		if ((errnum = rsa_import(pubbuf, pubbuf_len, &key)) != CRYPT_OK) {
			errormsg = error_to_string(errnum);
			return EXCEPTION;
		}
		if ((errnum = rsa_verify_hash(buf, buf_len, hash, HASH_SIZE,
							find_hash(HASH_METHOD_INTERNAL_STRING), HASH_SIZE,
							&stat, &key)) != CRYPT_OK) {
			errormsg = error_to_string(errnum);
			rsa_free(&key);
			return EXCEPTION;
		}
		rsa_free(&key);
	}

	if (pts < 100) {
		pts = 100;
		progress(100);
	}

	return stat;
}

//---------------------------------------------------------------------------

/**
 * @return 結果 -2:失敗 -1:中断 0:失敗 1:成功
 */
int
SigChecker::CheckSignature() {
	if (filename == "") {
		errormsg = L"Specify target file";
		return EXCEPTION;
	}
	if (publickey == "") {
		errormsg = L"Specify public key";
		return EXCEPTION;
	}

	if (TVPGetPlacedPath(filename) == "") {
		errormsg = ttstr(L"not exist:") + filename;
		return EXCEPTION;
	}
	
	int ignorestart = CheckKrkrExecutable(XOPT_EMBED_AREA_);
	if (ignorestart < 0) {
		return EXCEPTION;
	}
	int signofs     = CheckKrkrExecutable(XRELEASE_SIG____);
	if (signofs < 0) {
		return EXCEPTION;
	}
	int xp3ofs      = CheckKrkrExecutable(XP3_SIG);
	if (signofs < 0) {
		return EXCEPTION;
	}

	if (ignorestart != 0 && signofs != 0) {
		// krkr executable
		return CheckSignatureOfFile(ignorestart, xp3ofs?xp3ofs:-1, signofs+ 16+4);
	} else {
		// normal file
		return CheckSignatureOfFile(-1, -1, -1);
	}
};

void
SigChecker::progress(int percent)
{
	progressPercent = percent;
	if (notify) {
		notify->postMessage(WM_SIGCHECKPROGRESS, (WPARAM)this);
		Sleep(0);
	}
}

void
SigChecker::start()
{
	result = CheckSignature();
	if (notify) {
		notify->postMessage(WM_SIGCHECKDONE, (WPARAM)this);
		Sleep(0);
	} else {
		delete this;
	}
}

//---------------------------------------------------------------------------

// インスタンスゲッタ
NCB_GET_INSTANCE_HOOK(WindowSigCheck)
{
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		return obj;
	}
};

NCB_ATTACH_CLASS_WITH_HOOK(WindowSigCheck, Window) {
	NCB_METHOD(checkSignature);
	NCB_METHOD(cancelCheckSignature);
	NCB_METHOD(stopCheckSignature);
};

/**
 * 登録処理後
 */
static void PostRegistCallback()
{
	ltc_mp = ltm_desc;
}

NCB_POST_REGIST_CALLBACK(PostRegistCallback);
