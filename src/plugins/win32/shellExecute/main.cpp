#include <windows.h>
#include <tchar.h>
#include <process.h>
#include "ncbind/ncbind.hpp"

#define WM_SHELLEXECUTED   (WM_APP+1)
#define WM_SHELLCONSOLEOUT (WM_APP+2)

#define WSO_LOOPTIMEOUT 100

/**
 * コンソールコマンド用共通
 */

struct CommandExecute
{
	typedef void (*LineCallbackT)(void*, int, LPCWSTR);

	HANDLE hOR, hOW, hIR, hIW, hEW;
	PROCESS_INFORMATION pi;
	DWORD exitcode;
	bool timeouted;
	enum { ERR_NONE, ERR_PIPE, ERR_PROC, ERR_TOUT, ERR_WAIT } error;

	~CommandExecute() {
		if (hOR) ::CloseHandle(hOR);
		if (hOW) ::CloseHandle(hOW);
		if (hIR) ::CloseHandle(hIR);
		if (hIW) ::CloseHandle(hIW);
		if (hEW) ::CloseHandle(hEW);
		if (pi.hThread)  ::CloseHandle(pi.hThread);
		if (pi.hProcess) ::CloseHandle(pi.hProcess);
	}

	CommandExecute()
		: hOR(0), hOW(0), hIR(0), hIW(0), hEW(0), exitcode(~0L), timeouted(false), error(ERR_NONE)
	{
		::ZeroMemory(&pi, sizeof(pi));

		// セキュリティ属性
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;
		::ZeroMemory(&sa, sizeof(sa));
		sa.nLength= sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		// NT系の場合はセキュリティ記述子も
		OSVERSIONINFO osv;
		::ZeroMemory(&osv, sizeof(osv));
		osv.dwOSVersionInfoSize = sizeof(osv);
		::GetVersionEx(&osv);
		if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			::ZeroMemory(&sd, sizeof(sd));
			::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			::SetSecurityDescriptorDacl(&sd, true, NULL, false);
			sa.lpSecurityDescriptor = &sd;
		}

		// パイプを作成
		HANDLE hOT=0, hIT=0;
		HANDLE hPID = ::GetCurrentProcess();
		if (!(::CreatePipe(&hOT, &hOW, &sa,0) &&
			  ::CreatePipe(&hIR, &hIT, &sa,0) &&
			  ::DuplicateHandle(hPID, hOW, hPID, &hEW, 0,  TRUE, DUPLICATE_SAME_ACCESS) &&
			  ::DuplicateHandle(hPID, hOT, hPID, &hOR, 0, FALSE, DUPLICATE_SAME_ACCESS) &&
			  ::DuplicateHandle(hPID, hIT, hPID, &hIW, 0, FALSE, DUPLICATE_SAME_ACCESS)
			  )) {
			error = ERR_PIPE;
		}
		::CloseHandle(hOT);
		::CloseHandle(hIT);
	}

	bool start(ttstr const &target, ttstr const &param) {
		if (hasError()) return false;

		ttstr cmd(L"\"");
		// 吉里吉里サーチパス上にある場合はそちらを優先
		if (TVPIsExistentStorage(target)) {
			ttstr tmp = TVPGetPlacedPath(target);
			TVPGetLocalName(tmp);
			/**/cmd += tmp    + L"\"";
		} else  cmd += target + L"\"";

		if (param.length() > 0) cmd += L" " + param;
		LPWSTR cmdline = (LPWSTR)cmd.c_str();

		// 子プロセス作成
		STARTUPINFO si;
		::ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdOutput = hOW;
		si.hStdInput  = hIR;
		si.hStdError  = hEW;
		si.wShowWindow = SW_HIDE;
		if (!::CreateProcessW(0, cmdline, 0, 0, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi)) {
			error = ERR_PROC;
			return false;
		}
		return true;
	}

#define		BUF_SIZE	1024

	bool wait(LineCallbackT linecb, void *cbdata, int timeout = 0, DWORD cycle = WSO_LOOPTIMEOUT) {
		// パイプから出力を読み込み
		ttstr output;
		DWORD cnt, last=::GetTickCount();
		::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
		char buf[BUF_SIZE], crlf=0;
		char tmp[BUF_SIZE+1];			//文字の上位バイトだけ分断されてしまった場合に並べ直すエリア
		char kind[BUF_SIZE+1];			//文字構成種 0:半角 1:全角上位バイト 2:全角下位バイト
		char halfchar;					//分断された上位バイト
		bool ishalf = false;			//分断されているかどうか
		tjs_char wbuf[BUF_SIZE+1];
		bool rest = false;
		int line = 0;

		if (!hasError()) while (true) {
			if (cnt > 0) {
				last = GetTickCount();
				::ReadFile(hOR, buf, sizeof(buf)-1, &cnt, NULL);
				buf[cnt] = 0;
				if( ishalf )
				{
					//分断された上位バイトに連結
					ZeroMemory(tmp, sizeof(tmp));
					tmp[0] = halfchar;
					memcpy( &tmp[1], buf, cnt );
					cnt++;
					memcpy( buf, tmp, cnt );
				}
				halfchar = 0;
				ishalf = false;

				//パイプから読み込んだデータの終端マルチバイト判定および途中途中での
				//改行コード判定が随所で必要なので先に先頭から全部なめておく
				ZeroMemory(kind, sizeof(kind));
				for (DWORD pos = 0; pos < cnt; pos++) {
					unsigned char cl = buf[pos];
					if( pos )
					{
						//前のバイトが全角上位バイトだったら無条件で下位バイト扱い
						if ( kind[pos-1] == 1 )	{
							kind[pos] = 2;
							continue;
						}
					}
					//コマンドラインの標準入出力なのでSJIS前提での固定処理：上位バイト判定
					if ( cl > 0x80 && cl < 0xA0 || cl > 0xDF && cl < 0xFD )
						kind[pos] = 1;
				}

				if ( kind[cnt-1] == 1 ) {
					//最終バイトがマルチバイトの上位バイトかどうか判定
					ishalf = true;
					halfchar = buf[cnt-1];
					cnt--;
					buf[cnt] = 0;
				}
				DWORD start = 0;
				bool mb = false;
				for (DWORD pos = 0; pos < cnt; pos++) {
					char ch = buf[pos];
					//改行コード判定は半角(kind=0)であることが前提
					if ( (ch == '\r' || ch == '\n') && ! kind[pos] ) {
						if (crlf == 0 || crlf == ch) {
							buf[pos] = 0;

							//	マルチバイト文字列をワイド文字列に変換して ttstr に入れないと例外が出る
							ZeroMemory(wbuf, sizeof(wbuf));
							MultiByteToWideChar(0, 0, buf+start, pos-start, wbuf, sizeof(wbuf)-1);
							ttstr append(wbuf);

							output += append;
							linecb(cbdata, line++, output.c_str());
							output.Clear();
							buf[pos] = ch;
							crlf = 0;
						}
						if (crlf) crlf = 0;
						else crlf = ch;
						start = pos+1;
					} else {
						crlf = 0;
					}
				}
				if ((rest = (start < cnt))) {
					ZeroMemory(wbuf, sizeof(wbuf));
					MultiByteToWideChar(0, 0, buf+start, cnt-start, wbuf, sizeof(wbuf)-1);
					ttstr append(wbuf);

					output += append;
				}
				if ((int)cnt == sizeof(buf)-1) {
					::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
					if (cnt > 0) continue;
				}
			} else {
				if (timeout > 0 && ::GetTickCount() > last+timeout) {
					::TerminateProcess(pi.hProcess, -1);
					error = ERR_TOUT;
					timeouted = true;
				}
			}
			DWORD wait = ::WaitForSingleObject(pi.hProcess, cycle);
			if (wait == WAIT_FAILED) {
				error = ERR_WAIT;
				break;
			}
			::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
			if (cnt == 0 && wait == WAIT_OBJECT_0) {
				::GetExitCodeProcess(pi.hProcess, &exitcode);
				break;
			}
		}
		if (rest) linecb(cbdata, line++, output.c_str());

		return hasError();
	}

	bool hasError() const { return (error != ERR_NONE); }
	LPCWSTR getLastError() const {
		switch (error) {
		case ERR_PIPE: return L"can't create/duplicate pipe";
		case ERR_PROC: return L"can't create child process";
		case ERR_TOUT: return L"child process timeout";
		case ERR_WAIT: return L"child process wait failed";
		}
		return L"";
	}

	DWORD getExitCode(bool *err = 0, bool *tout = 0, LPCWSTR *errmes = 0) const {
		if (err )   *err    = hasError();
		if (tout)   *tout   = timeouted;
		if (errmes) *errmes = getLastError();
		return exitcode;
	}

	HANDLE getProcessHandle() const { return pi.hProcess;    }
	DWORD  getProcessId()     const { return pi.dwProcessId; }
};



/**
 * メソッド追加用クラス
 */
class WindowShell {

protected:
	iTJSDispatch2 *objthis; //< オブジェクト情報の参照
	HWND msgHWND;

	typedef WindowShell SelfClass;
#define MSGWND_CLASSNAME  L"Window ShellExecute Message Window Class"
#define MSGWND_WINDOWNAME L"Window ShellExecute Message"
	static ATOM MessageWindowClass;
	HWND  createMessageWindow() {
		HINSTANCE hinst = ::GetModuleHandle(NULL);
		if (!MessageWindowClass) {
			WNDCLASSEXW wcex = {
				/*size*/sizeof(WNDCLASSEX), /*style*/0, /*proc*/WndProc, /*extra*/0L,0L, /*hinst*/hinst,
				/*icon*/NULL, /*cursor*/NULL, /*brush*/NULL, /*menu*/NULL,
				/*class*/MSGWND_CLASSNAME, /*smicon*/NULL };
			if (!(MessageWindowClass = ::RegisterClassExW(&wcex)))
				TVPThrowExceptionMessage(TJS_W("register window class failed."));
		}
		HWND hwnd = ::CreateWindowExW(0, (LPCWSTR)MAKELONG(MessageWindowClass, 0), MSGWND_WINDOWNAME,
									  0, 0, 0, 1, 1, HWND_MESSAGE, NULL, hinst, NULL);
		if (!hwnd) TVPThrowExceptionMessage(TJS_W("create message window failed."));
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
		return hwnd;
	}
	HWND destroyMessageWindow(HWND hwnd) {
		if (hwnd) {
			::SetWindowLong(hwnd, GWL_USERDATA, 0);
			::DestroyWindow(hwnd);
		}
		return NULL;
	}
	static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		SelfClass *self = (SelfClass*)(::GetWindowLong(hwnd, GWL_USERDATA));
		if (self) switch (msg) {
		case WM_SHELLEXECUTED:
			self->onShellExecuted(wp, lp);
			return 0;
		case WM_SHELLCONSOLEOUT:
			self->onCommandLineOutput(wp, lp);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wp, lp);
	}
public:
	static void UnregisterMessageWindowClass() {
		if (MessageWindowClass != 0 && ::UnregisterClassW((LPCWSTR)MAKELONG(MessageWindowClass, 0), ::GetModuleHandle(NULL)))
			MessageWindowClass = 0;
	}

protected:
	// イベント処理
	void onShellExecuted(WPARAM wp, LPARAM lp) {
		tTJSVariant process = (tjs_int)wp;
		tTJSVariant endCode = (tjs_int)lp;
		removeProcessMap((HANDLE)wp);
		tTJSVariant *p[] = {&process, &endCode};
		objthis->FuncCall(0, L"onShellExecuted", NULL, NULL, 2, p, objthis);
	}
	void onCommandLineOutput(WPARAM wp, LPARAM lp) {
		tTJSVariant process = (tjs_int)wp;
		tTJSVariant text = ttstr((LPCWSTR)lp);
		tTJSVariant *p[] = { &process, &text };
		objthis->FuncCall(0, L"onCommandLineOutput", NULL, NULL, 2, p, objthis);
	}

public:
	// コンストラクタ
	WindowShell(iTJSDispatch2 *objthis) : objthis(objthis) {
		msgHWND = createMessageWindow();
	}

	// デストラクタ
	~WindowShell() {
		msgHWND = destroyMessageWindow(msgHWND);
	}

public:
	/**
	 * 実行情報
	 */
	struct ExecuteInfo {
		HWND   message; // メッセージ送信先
		HANDLE process; // 待ち対象プロセス
		CommandExecute *cmd;
		ExecuteInfo(HWND message, HANDLE process, CommandExecute *cmd = 0)
			: message(message), process(process), cmd(cmd) {}
	};
	
	/**
	 * 終了待ちスレッド処理
	 * @param data ユーザ(ExecuteInfo)
	 */
	static void waitProcess(void *data) {
		// パラメータ引き継ぎ
		HWND   message = ((ExecuteInfo*)data)->message;
		HANDLE process = ((ExecuteInfo*)data)->process;
		delete data;

		// プロセス待ち
		WaitForSingleObject(process, INFINITE);
		DWORD dt;
		GetExitCodeProcess(process, &dt); // 結果取得
		CloseHandle(process);

		// 送信
		PostMessage(message, WM_SHELLEXECUTED, (WPARAM)process, (LPARAM)dt);
	}
	
	/**
	 * プロセスの停止
	 * @param process プロセスID
	 * @param endCode 終了コード
	 */
	void terminateProcess(int process, int endCode) {
		TerminateProcess((HANDLE)process, endCode);
	}

	/**
	 * プロセスの実行
	 * @param target ターゲット
	 * @praam param パラメータ
	 */
	int shellExecute(LPCTSTR target, LPCTSTR param) {
		SHELLEXECUTEINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.lpVerb = _T("open");
		si.lpFile = target;
		si.lpParameters = param;
		si.nShow = SW_SHOWNORMAL;
		si.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		if (ShellExecuteEx(&si)) {
			_beginthread(waitProcess, 0, new ExecuteInfo(msgHWND, si.hProcess));
			return (int)si.hProcess;
		}
		return (int)INVALID_HANDLE_VALUE;
	}


	/**
	 * commandExec コンソール処理
	 */
	struct ConsoleOutputParam {
		HWND message;
		HANDLE process;
		ConsoleOutputParam(HWND message, HANDLE process) : message(message), process(process) {}
		static void consoleLineOutCallback(void *v, int line, LPCWSTR text) {
			ConsoleOutputParam *self = (ConsoleOutputParam*)v;
			if (self) SendMessage(self->message, WM_SHELLCONSOLEOUT, (WPARAM)self->process, (LPARAM)text);
		}
	};
	/**
	 * commandExec 終了待ちスレッド処理
	 * @param data ユーザ(ExecuteInfo)
	 */
	static void waitCommand(void *data) {
		// パラメータ引き継ぎ
		HWND        message = ((ExecuteInfo*)data)->message;
		HANDLE      process = ((ExecuteInfo*)data)->process;
		CommandExecute *cmd = ((ExecuteInfo*)data)->cmd;
		delete data;

		ConsoleOutputParam prm(message, process);
		cmd->wait(ConsoleOutputParam::consoleLineOutCallback, (void*)&prm, 0);
		DWORD exit = cmd->getExitCode();
		delete cmd;

		// 送信
		PostMessage(message, WM_SHELLEXECUTED, (WPARAM)process, (LPARAM)exit);
	}
	/**
	 * コマンドラインの実行
	 * @param target ターゲット
	 * @praam param パラメータ
	 */
	int commandExecute(ttstr target, ttstr param) {
		CommandExecute *cmd = new CommandExecute();
		if (cmd->start(target, param)) {
			HANDLE proc = cmd->getProcessHandle();
			setProcessMap(proc, cmd->getProcessId());
			_beginthread(waitCommand, 0, new ExecuteInfo(msgHWND, proc, cmd));
			return (int)proc;
		}
		delete cmd;
		return (int)INVALID_HANDLE_VALUE;
	}


	/**
	 * シグナル送信
	 */
	bool commandSendSignal(int process, bool type) {
		DWORD id = getProcessMap((HANDLE)process);
		DWORD ev = type ? CTRL_BREAK_EVENT : CTRL_C_EVENT;

		BOOL r = ::GenerateConsoleCtrlEvent(ev, id);
		if (!r) {
			if (::AttachConsole(id)) {
				r = ::GenerateConsoleCtrlEvent(ev, id);
				::FreeConsole();
			} else  {
				ttstr err;
				getLastError(err);
				TVPAddLog(err.c_str());
			}
		}
		return !!r;
	}
	void  setProcessMap(HANDLE proc, DWORD id) { pmap.SetValue((tjs_int32)proc, (tTVInteger)id); }
	DWORD getProcessMap(HANDLE proc)  { return (DWORD)(pmap.getIntValue((tjs_int32)proc, -1)); }
	void removeProcessMap(HANDLE proc) {
		iTJSDispatch2 *dsp = pmap.GetDispatch();
		dsp->DeleteMemberByNum(0, (tjs_int)proc, dsp);
	}
static void getLastError(ttstr &message) {
	LPVOID lpMessageBuffer;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   NULL, GetLastError(),
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPWSTR)&lpMessageBuffer, 0, NULL);
	message = ((tjs_char*)lpMessageBuffer);
	LocalFree(lpMessageBuffer);
}


private:
	ncbDictionaryAccessor pmap;
};

ATOM WindowShell::MessageWindowClass = 0;
static void PostUnregistCallback() { WindowShell::UnregisterMessageWindowClass(); }
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);


// インスタンスゲッタ
NCB_GET_INSTANCE_HOOK(WindowShell)
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

// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(WindowShell, Window) {
	Method(L"shellExecute", &WindowShell::shellExecute);
	Method(L"commandExecute", &WindowShell::commandExecute);
	Method(L"terminateProcess", &WindowShell::terminateProcess);
	Method(L"commandSendSignal", &WindowShell::commandSendSignal);
}


static void cmdExecLineCallback(void *va, int line, LPCWSTR text) {
	iTJSDispatch2 *array = (iTJSDispatch2*)va;
	array->PropSetByNum(TJS_MEMBERENSURE, line, &tTJSVariant(text), array);
}

/**
 * コマンドライン呼び出し
 */
tjs_error TJS_INTF_METHOD commandExecute(
	tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	tTJSVariant vStdOut;

	// パラメータチェック
	if (numparams == 0) return TJS_E_BADPARAMCOUNT;
	if (param[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;

	// コマンドライン/タイムアウト取得
	int timeout = 0;
	ttstr target(param[0]->GetString()), cmdprm;

	if (numparams > 1) cmdprm  = param[1]->GetString();
	if (numparams > 2) timeout = (tjs_int)*param[2];

	DWORD exit = ~0L;
	bool haserr = true, timeouted = false;
	LPCWSTR errmes = 0;

	iTJSDispatch2 *array = TJSCreateArrayObject();
	if (array != 0) {
		CommandExecute exec;
		if (exec.start(target, cmdprm)) exec.wait(cmdExecLineCallback, array, timeout);
		exit = exec.getExitCode(&haserr, &timeouted, &errmes);

		vStdOut.SetObject(array, array);
		array->Release();
	}

	ncbDictionaryAccessor ret;
	if (ret.IsValid()) {
		ret.SetValue(L"stdout", vStdOut);
		if (haserr && errmes != 0) {
			ret.SetValue(L"status", ttstr(timeouted ? L"timeout" : L"error"));
			ret.SetValue(L"message", ttstr(errmes));
		} else {
			ret.SetValue(L"status", ttstr(haserr ? L"failed" : L"ok"));
			ret.SetValue(L"exitcode", (tjs_int)exit);
		}
	}
	if (result != NULL) *result = ret;
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(commandExecute,    System, commandExecute);

