#define INITGUID // XP 有効にするとdxguid.libが使用できないため
#include "tjsCommHead.h"

#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>

#include <eh.h>

#define DIRECTINPUT_VERSION 0x0500
#include <dinput.h>

#include <DbgHelp.h>
#include <Strsafe.h>

#include "tjsError.h"
#include "tjsDebug.h"

#include "Application.h"
#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "ScriptMgnIntf.h"
#include "tjsError.h"
#include "PluginImpl.h"
#include "SystemIntf.h"

#include "Exception.h"
#include "WindowFormUnit.h"
#include "Resource.h"
#include "SystemControl.h"
#include "MouseCursor.h"
#include "SystemImpl.h"
#include "WaveImpl.h"
#include "GraphicsLoadThread.h"

#include "resource.h"

#pragma comment(lib,"dbghelp.lib")
/*
kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;winmm.lib;dsound.lib;version.lib;mpr.lib;shlwapi.lib;vfw32.lib;imm32.lib;zlib_d.lib;jpeg-6bx_d.lib;libpng_d.lib;onig_s_d.lib;freetype250MT_D.lib;tvpgl_ia32.lib;tvpsnd_ia32.lib;%(AdditionalDependencies)
kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;winmm.lib;dsound.lib;version.lib;mpr.lib;shlwapi.lib;vfw32.lib;imm32.lib;zlib.lib;jpeg-6bx.lib;libpng.lib;onig_s.lib;freetype250MT.lib;tvpgl_ia32.lib;tvpsnd_ia32.lib;%(AdditionalDependencies)
*/

tTVPApplication* Application;
extern void TVPHandleSEHException( int ErrorCode, EXCEPTION_RECORD *P, unsigned long osEsp, PCONTEXT ctx);

// アプリケーションの開始時に呼ぶ
inline void CheckMemoryLeaksStart()
{
#ifdef  _DEBUG
   _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif  // _DEBUG
}

inline void DumpMemoryLeaks()
{
#ifdef  _DEBUG
	int is_leak = _CrtDumpMemoryLeaks();
	assert( !is_leak );
#endif  // _DEBUG
}

#if 0
tstring ParamStr( int index ) {
	if( index < (int)Application->CommandLines.size() ) {
		return tstring(Application->CommandLines[index]);
	} else {
		return tstring();
	}
}
#endif
std::wstring ExePath() {
	wchar_t szFull[_MAX_PATH];
	::GetModuleFileName(NULL, szFull, sizeof(szFull) / sizeof(wchar_t));
	return std::wstring(szFull);
}

bool TVPCheckCmdDescription();
bool TVPCheckAbout();
bool TVPCheckPrintDataPath();
void TVPOnError();

int _argc;
char ** _argv;
extern void TVPInitCompatibleNativeFunctions();
extern void TVPUninitializeFontRasterizers();
extern void TVPLoadMessage();

AcceleratorKeyTable::AcceleratorKeyTable() {
	// デフォルトを読み込む
	hAccel_ = ::LoadAccelerators( (HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDC_TVPWIN32));
}
AcceleratorKeyTable::~AcceleratorKeyTable() {
	std::map<HWND,AcceleratorKey*>::iterator i = keys_.begin();
	for( ; i != keys_.end(); i++ ) {
		delete (i->second);
	}
}
void AcceleratorKeyTable::AddKey( HWND hWnd, WORD id, WORD key, BYTE virt ) {
	std::map<HWND,AcceleratorKey*>::iterator i = keys_.find(hWnd);
	if( i != keys_.end() ) {
		i->second->AddKey(id,key,virt);
	} else {
		AcceleratorKey* acc = new AcceleratorKey();
		acc->AddKey( id, key, virt );
		keys_.insert( std::map<HWND, AcceleratorKey*>::value_type( hWnd, acc ) );
	}
}
void AcceleratorKeyTable::DelKey( HWND hWnd, WORD id ) {
	std::map<HWND,AcceleratorKey*>::iterator i = keys_.find(hWnd);
	if( i != keys_.end() ) {
		i->second->DelKey(id);
	}
}

void AcceleratorKeyTable::DelTable( HWND hWnd ) {
	std::map<HWND,AcceleratorKey*>::iterator i = keys_.find(hWnd);
	if( i != keys_.end() ) {
		delete (i->second);
		keys_.erase(i);
	}
}
AcceleratorKey::AcceleratorKey() : hAccel_(NULL), keys_(NULL), key_count_(0) {
}
AcceleratorKey::~AcceleratorKey() {
	if( hAccel_ != NULL ) ::DestroyAcceleratorTable( hAccel_ );
	delete[] keys_;
}
void AcceleratorKey::AddKey( WORD id, WORD key, BYTE virt ) {
	// まずは存在するかチェックする
	bool found = false;
	int index = 0;
	for( int i = 0; i < key_count_; i++ ) {
		if( keys_[i].cmd == id ) {
			index = i;
			found = true;
			break;
		}
	}
	if( found ) {
		// 既に登録されているコマンドなのでキー情報の更新を行う
		if( keys_[index].key == key && keys_[index].fVirt == virt ) {
			// 変更されていない
			return;
		}
		keys_[index].key = key;
		keys_[index].fVirt = virt;
		HACCEL hAccel = ::CreateAcceleratorTable( keys_, key_count_ );
		if( hAccel_ != NULL ) ::DestroyAcceleratorTable( hAccel_ );
		hAccel_ = hAccel;
	} else {
		ACCEL* table = new ACCEL[key_count_+1];
		for( int i = 0; i < key_count_; i++ ) {
			table[i] = keys_[i];
		}
		table[key_count_].cmd = id;
		table[key_count_].key = key;
		table[key_count_].fVirt = virt;
		key_count_++;
		HACCEL hAccel = ::CreateAcceleratorTable( table, key_count_ );
		if( hAccel_ != NULL ) ::DestroyAcceleratorTable( hAccel_ );
		hAccel_ = hAccel;
		delete[] keys_;
		keys_ = table;
	}

}
void AcceleratorKey::DelKey( WORD id ) {
	// まずは存在するかチェックする
	bool found = false;
	for( int i = 0; i < key_count_; i++ ) {
		if( keys_[i].cmd == id ) {
			found = true;
			break;
		}
	}
	if( found == false ) return;

	// 存在した場合作り直し
	ACCEL* table = new ACCEL[key_count_-1];
	int dest = 0;
	for( int i = 0; i < key_count_; i++ ) {
		if( keys_[i].cmd != id ) {
			table[dest] = keys_[i];
			dest++;
		}
	}
	key_count_--;
	HACCEL hAccel = ::CreateAcceleratorTable( table, key_count_ );
	if( hAccel_ != NULL ) ::DestroyAcceleratorTable( hAccel_ );
	hAccel_ = hAccel;
	delete[] keys_;
	keys_ = table;
}

int APIENTRY WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow ) {
	try {
		CheckMemoryLeaksStart();
		// ウォッチで _crtBreakAlloc にセットする

		// XP より後で使えるAPIを動的に読み込んで互換性を取る
		TVPInitCompatibleNativeFunctions();

		// メッセージ文字列をリソースから読込み
		TVPLoadMessage();

		_argc = __argc;
		_argv = __argv;

		MouseCursor::Initialize();
		Application = new tTVPApplication();
		Application->StartApplication( __argc, __argv );
	
		// delete application and exit forcely
		// this prevents ugly exception message on exit
		// アプリケーションを削除し強制終了させる。
		// これは終了時の醜い例外メッセージを抑止する
		delete Application;

#ifndef _DEBUG
//		::ExitProcess(TVPTerminateCode); // ここで終了させるとメモリリーク表示が行われない
#endif
	} catch (...) {
		return 2;
	}
	return TVPTerminateCode;
}
tTVPApplication::tTVPApplication() : is_attach_console_(false), tarminate_(false), ApplicationActivating(true)
	 , ImageLoadThread(NULL), HasMapReportProcess(false)
{
}
tTVPApplication::~tTVPApplication() {
	while( windows_list_.size() ) {
		std::vector<class TTVPWindowForm*>::iterator i = windows_list_.begin();
		delete (*i);
		// TTVPWindowForm のデストラクタ内でリストから削除されるはず
	}
	windows_list_.clear();
}
struct SEHException {
	unsigned int Code;
	_EXCEPTION_POINTERS* ExceptionPointers;
	SEHException( unsigned int code, _EXCEPTION_POINTERS* ep )
		: Code(code), ExceptionPointers(ep)
	{}
};

int TVPWriteHWEDumpFile( EXCEPTION_POINTERS* pExceptionPointers ) {
	BOOL bMiniDumpSuccessful;
	WCHAR szPath[MAX_PATH]; 
	WCHAR szFileName[MAX_PATH]; 
	const wchar_t* szAppName = TVPKirikiri;
	const wchar_t* szVersion = TVPGetVersionString().c_str();

	TVPEnsureDataPathDirectory();
	TJS_strcpy(szPath, TVPNativeDataPath.c_str());

	SYSTEMTIME stLocalTime;
	::GetLocalTime( &stLocalTime );
	StringCchPrintf( szFileName, MAX_PATH, L"%s%s%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
				szPath, szAppName, szVersion,
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
				GetCurrentProcessId(), GetCurrentThreadId());
	HANDLE hDumpFile = ::CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	MINIDUMP_EXCEPTION_INFORMATION ExpParam;
	ExpParam.ThreadId = ::GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;
	bMiniDumpSuccessful = MiniDumpWriteDump( ::GetCurrentProcess(), ::GetCurrentProcessId(), hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);
	return EXCEPTION_EXECUTE_HANDLER;
}
static bool TVPIsHandledHWException = false;
void se_translator_function(unsigned int code, struct _EXCEPTION_POINTERS* ep) {
	if( !TVPIsHandledHWException ) {
		//ShowStackTrace( ep->ContextRecord );
		TVPWriteHWEDumpFile( ep );
		TVPHandleSEHException( code, ep->ExceptionRecord, ep->ContextRecord->Esp, ep->ContextRecord );
		TVPIsHandledHWException = true;
	}
	throw SEHException(code,ep);
}
const wchar_t* SECodeToMessage( unsigned int code ) {
	switch(code){
	case EXCEPTION_ACCESS_VIOLATION: return L"Access Violation: スレッドが適切なアクセス権を持たない仮想アドレスに対して、読み取りまたは書き込みを試みました。";
	// case EXCEPTION_ACCESS_VIOLATION: return L"The thread attempts to read from or write to a virtual address for which it does not have access.";
	case EXCEPTION_BREAKPOINT: return L"Break Point: ブレークポイントに到達しました。";
	// case EXCEPTION_BREAKPOINT: return L"A breakpoint is encountered.";
	case EXCEPTION_DATATYPE_MISALIGNMENT: return L"Data Type Misalignment: 境界整列をサポートしないハードウェア上で、スレッドが境界に整列していないデータの読み取りまたは書き込みを試みました。たとえば、16 ビットの値は 2 バイトの境界線上に、32 ビットの値は 4 バイトの境界線上に整列していなければなりません。";
	// case EXCEPTION_DATATYPE_MISALIGNMENT: return L"The thread attempts to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries, 32-bit values on 4-byte boundaries, and so on.";
	case EXCEPTION_SINGLE_STEP: return L"Single Step: トレーストラップなどの単一命令メカニズムから 1 つの命令の実行が通知されました。";
	// case EXCEPTION_SINGLE_STEP: return L"A trace trap or other single instruction mechanism signals that one instruction is executed.";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return L"Array Bounds Exceede: スレッドが範囲外の配列要素にアクセスしようとしました。使用中のハードウェアは境界チェックをサポートしています。";
	// case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return L"The thread attempts to access an array element that is out of bounds, and the underlying hardware supports bounds checking.";
	case EXCEPTION_FLT_DENORMAL_OPERAND: return L"Denormal Operand: 浮動小数点演算のオペランドの 1 つが不正規化値です。不正規化値とは、小さすぎて標準の浮動小数点値として表現できない値です。";
	// case EXCEPTION_FLT_DENORMAL_OPERAND: return L"One of the operands in a floating point operation is denormal. A denormal value is one that is too small to represent as a standard floating point value.";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: return L"Divide By Zero: スレッドが浮動小数点値を 0 の浮動小数点除数で除算しようとしました。";
	// case EXCEPTION_FLT_DIVIDE_BY_ZERO return L"The thread attempts to divide a floating point value by a floating point divisor of 0 (zero).";
	case EXCEPTION_FLT_INEXACT_RESULT: return L"Inexact Result: 浮動小数点演算の結果を 10 進小数で正確に表現できません。";
	// case EXCEPTION_FLT_INEXACT_RESULT: return L"The result of a floating point operation cannot be represented exactly as a decimal fraction.";
	case EXCEPTION_FLT_INVALID_OPERATION: return L"Invalid Operation: このリストに含まれていない例外が発生しました。";
	// case EXCEPTION_FLT_INVALID_OPERATION: return L"A floating point exception that is not included in this list.";
	case EXCEPTION_FLT_OVERFLOW: return L"Overflow: 浮動小数点演算の指数が大きく、対応する型によって表現できません。";
	// case EXCEPTION_FLT_OVERFLOW: return L"The exponent of a floating point operation is greater than the magnitude allowed by the corresponding type.";
	case EXCEPTION_FLT_STACK_CHECK: return L"Stack Check: 浮動小数点演算の結果、スタックのオーバーフローまたはアンダーフローが発生しました。";
	// case EXCEPTION_FLT_STACK_CHECK: return L"The stack has overflowed or underflowed, because of a floating point operation.";
	case EXCEPTION_FLT_UNDERFLOW: return L"Underflow: 浮動小数点演算の指数が小さく、対応する型によって表現できません。";
	// case EXCEPTION_FLT_UNDERFLOW: return L"The exponent of a floating point operation is less than the magnitude allowed by the corresponding type.";
	case EXCEPTION_INT_DIVIDE_BY_ZERO: return L"Divide By Zero: スレッドが整数値を 0 の整数除数で除算しようとしました。";
	// case EXCEPTION_INT_DIVIDE_BY_ZERO: return L"The thread attempts to divide an integer value by an integer divisor of 0 (zero).";
	case EXCEPTION_INT_OVERFLOW: return L"Overflow: 整数演算結果の最上位ビットが繰り上がりました。";
	// case EXCEPTION_INT_OVERFLOW: return L"The result of an integer operation creates a value that is too large to be held by the destination register. In some cases, this will result in a carry out of the most significant bit of the result. Some operations do not set the carry flag.";
	case EXCEPTION_PRIV_INSTRUCTION: return L"Priv Instruction: 現在のマシンモードで許可されていない演算を行う命令をスレッドが実行しようとしました。";
	// case EXCEPTION_PRIV_INSTRUCTION: return L"The thread attempts to execute an instruction with an operation that is not allowed in the current computer mode.";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"Noncontinuable Exception: 継続不可能な例外が発生した後、スレッドが実行を継続しようとしました。";
	// case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"The thread attempts to continue execution after a non-continuable exception occurs.";

	case EXCEPTION_GUARD_PAGE: return L"Guard Page: The thread accessed memory allocated with the PAGE_GUARD modifier.";
	case EXCEPTION_ILLEGAL_INSTRUCTION: return L"Illegal Instruction: The thread tries to execute an invalid instruction.";
	case EXCEPTION_IN_PAGE_ERROR: return L"In Page Error: The thread tries to access a page that is not present, and the system is unable to load the page. For example, this exception might occur if a network connection is lost while running a program over a network.";
	case EXCEPTION_INVALID_DISPOSITION: return L"Invalid Disposition: An exception handler returns an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.";
	case EXCEPTION_INVALID_HANDLE: return L"Invalid Handle: The thread used a handle to a kernel object that was invalid (probably because it had been closed.)";
	case EXCEPTION_STACK_OVERFLOW: return L"Stack Overflow: The thread uses up its stack.";
	case STATUS_UNWIND_CONSOLIDATE: return L"Unwind Consolidate: A frame consolidation has been executed.";
	}
	return L"Unknown";
}

bool tTVPApplication::StartApplication( int argc, char* argv[] ) {
	_set_se_translator(se_translator_function);

	ArgC = argc;
	ArgV = argv;
	for( int i = 0; i < argc; i++ ) {
		CommandLines.push_back( std::string(argv[i]) );
		if(!strcmp(argv[i], "-@processohmlog")) {
			HasMapReportProcess = true;
		}
	}
	TVPTerminateCode = 0;

	CheckConsole();

	// try starting the program!
	bool engine_init = false;
	try {
		if(TVPCheckProcessLog()) return true; // sub-process for processing object hash map log

		ImageLoadThread = new tTVPAsyncImageLoader();

		TVPInitScriptEngine();
		engine_init = true;

		// banner
		TVPAddImportantLog( TVPFormatMessage(TVPProgramStartedOn, TVPGetOSName(), TVPGetPlatformName()) );

		// TVPInitializeBaseSystems
		TVPInitializeBaseSystems();

		Initialize();

		if(TVPCheckPrintDataPath()) return true;
		if(TVPCheckCmdDescription()) return true;
		//if(TVPExecuteUserConfig()) return true; // userconf エンジン設定起動はしない TODO

		TVPSystemInit();

		if(TVPCheckAbout()) return true; // version information dialog box;

		SetTitle( std::wstring(TVPKirikiri) );
		TVPSystemControl = new tTVPSystemControl();
#ifndef TVP_IGNORE_LOAD_TPM_PLUGIN
		TVPLoadPluigins(); // load plugin module *.tpm
#endif
		// Check digitizer
		CheckDigitizer();

		// start image load thread
		ImageLoadThread->Resume();

		if(TVPProjectDirSelected) TVPInitializeStartupScript();

		Run();

		try {
			// ImageLoadThread->ExitRequest();
			delete ImageLoadThread;
			ImageLoadThread = NULL;
		} catch(...) {
			// ignore errors
		}
		try {
			TVPSystemUninit();
		} catch(...) {
			// ignore errors
		}
	} catch( const EAbort & ) {
		// nothing to do
	} catch( const Exception &exception ) {
		TVPOnError();
		if(!TVPSystemUninitCalled)
			ShowException(exception.what());
	} catch( const TJS::eTJSScriptError &e ) {
		TVPOnError();
		if(!TVPSystemUninitCalled)
			ShowException( e.GetMessage().c_str() );
	} catch( const TJS::eTJS &e) {
		TVPOnError();
		if(!TVPSystemUninitCalled)
			ShowException( e.GetMessage().c_str() );
	} catch( const std::exception &e ) {
		ShowException( ttstr(e.what()).c_str() );
	} catch( const char* e ) {
		ShowException( ttstr(e).c_str() );
	} catch( const wchar_t* e ) {
		ShowException( e );
	} catch( const SEHException& e ) {
		PEXCEPTION_RECORD rec = e.ExceptionPointers->ExceptionRecord;
		std::wstring text(SECodeToMessage(e.Code));
		ttstr result = TJSGetStackTraceString( 10 );
		PrintConsole( result.c_str(), result.length(), true );

		TVPDumpHWException();
		ShowException( text.c_str() );
	} catch(...) {
		ShowException( (const tjs_char*)TVPUnknownError );
	}

	if(engine_init) TVPUninitScriptEngine();

	if(TVPSystemControl) delete TVPSystemControl;
	TVPSystemControl = NULL;
	TVPUninitializeFontRasterizers();

	CloseConsole();

	return false;
}
/**
 * コンソールからの起動か確認し、コンソールからの起動の場合は、標準出力を割り当てる
 */
void tTVPApplication::CheckConsole() {
#ifdef TVP_LOG_TO_COMMANDLINE_CONSOLE
	if( HasMapReportProcess ) return; // 書き出し用子プロセスして起動されていた時はコンソール接続しない
	HANDLE hin  = ::GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE herr = ::GetStdHandle(STD_ERROR_HANDLE);

	DWORD curProcId = ::GetCurrentProcessId();
	DWORD processList[256];
	DWORD count = ::GetConsoleProcessList( processList, 256 );
	bool thisProcHasConsole = false;
	for( DWORD i = 0; i < count; i++ ) {
		if( processList[i] == curProcId ) {
			thisProcHasConsole = true;
			break;
		}
	}
	bool attachedConsole = true;
	if( thisProcHasConsole == false ) {
		attachedConsole = ::AttachConsole(ATTACH_PARENT_PROCESS) != 0;
	}

	if( (hin==0||hout==0||herr==0) && attachedConsole ) {
		wchar_t console[256];
		::GetConsoleTitle( console, 256 );
		console_title_ = std::wstring( console );
		// 元のハンドルを再割り当て
		if (hin)  ::SetStdHandle(STD_INPUT_HANDLE, hin);
		if (hout) ::SetStdHandle(STD_OUTPUT_HANDLE, hout);
		if (herr) ::SetStdHandle(STD_ERROR_HANDLE, herr);
	}
	is_attach_console_ = attachedConsole;
#endif
}

void tTVPApplication::CloseConsole() {
	wchar_t buf[100];
	DWORD len = TJS_snprintf(buf, 100, TVPExitCode, TVPTerminateCode);
	PrintConsole(buf, len);
	if( is_attach_console_ ) {
		::SetConsoleTitle( console_title_.c_str() );
		::FreeConsole();
		is_attach_console_ = false;
	}
}

void tTVPApplication::PrintConsole( const wchar_t* mes, unsigned long len, bool iserror ) {
	HANDLE hStdOutput = ::GetStdHandle(iserror ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	if (hStdOutput > 0) {
		DWORD mode;
		if (GetConsoleMode(hStdOutput, &mode)) {
			// 実コンソール
			DWORD wlen;
			::WriteConsoleW( hStdOutput, mes, len, &wlen, NULL );
			::WriteConsoleW( hStdOutput, L"\n", 1, &wlen, NULL );
		} else {
			// その他のハンドル
			ttstr str = mes;
			tjs_int len = str.GetNarrowStrLen();
			tjs_nchar *dat = new tjs_nchar[len+1];
			try {
				str.ToNarrowStr(dat, len+1);
			}
			catch(...)	{
				delete [] dat;
				throw;
			} 
			DWORD wlen;
			::WriteFile( hStdOutput, dat, len, &wlen, NULL );
			::WriteFile( hStdOutput, "\n", 1, &wlen, NULL );
			//fprintf(stderr, "%s\n", dat);
			delete [] dat;
		}
	}
#ifdef _DEBUG
	::OutputDebugString( mes );
	::OutputDebugString( L"\n" );
#endif
}
HWND tTVPApplication::GetHandle() {
	if( windows_list_.size() > 0 ) {
		return windows_list_[0]->GetHandle();
	} else {
		return INVALID_HANDLE_VALUE;
	}
}
void tTVPApplication::Minimize() {
	size_t size = windows_list_.size();
	for( size_t i = 0; i < size; i++ ) {
		::ShowWindow( windows_list_[i]->GetHandle(), SW_MINIMIZE );
	}
}
void tTVPApplication::Restore() {
	size_t size = windows_list_.size();
	for( size_t i = 0; i < size; i++ ) {
		::ShowWindow( windows_list_[i]->GetHandle(), SW_RESTORE );
	}
}

void tTVPApplication::BringToFront() {
	size_t size = windows_list_.size();
	for( size_t i = 0; i < size; i++ ) {
		windows_list_[i]->BringToFront();
	}
}
void tTVPApplication::ShowException( const wchar_t* e ) {
	::MessageBox( NULL, e, TVPFatalError, MB_OK );
}
void tTVPApplication::Run() {
	MSG msg;
	HACCEL hAccelTable;

	// メイン メッセージ ループ:
	HWND mainWnd = INVALID_HANDLE_VALUE;
	if( ( windows_list_.size() > 0 ) ) {
		mainWnd = windows_list_[0]->GetHandle();
	}
	while( windows_list_.size() > 0 && tarminate_ == false ) {
		BOOL ret = TRUE;
		while( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) && tarminate_ == false ) {
			ret = ::GetMessage( &msg, NULL, 0, 0);
			hAccelTable = accel_key_.GetHandle(msg.hwnd);
			if( ret && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if( ret == 0 )  break;
		}
		if( ret == 0 )  break;

		bool done = true;
		if( TVPSystemControl ) {
			done = TVPSystemControl->ApplicationIdel();
		}
		if( done ) { // idle 処理が終わったら、メッセージ待ちへ
			BOOL dret = ::GetMessage( &msg, NULL, 0, 0 );
			hAccelTable = accel_key_.GetHandle(msg.hwnd);
			if( dret && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if( dret == 0 )  break;
		}
	}
	tarminate_ = true;
	TVPTerminateCode = 0;
	if( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) ) {
		if( msg.message == WM_QUIT ) {
			TVPTerminateCode = msg.wParam;
		}
	}
}
bool tTVPApplication::ProcessMessage( MSG &msg ) {
	bool result = false;
	if( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) ) {
		BOOL msgExists = ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE);
		if( msgExists == 0 ) {
			return result;
		}
		result = true;
		if( msg.message != WM_QUIT ) {
			HACCEL hAccelTable = accel_key_.GetHandle(msg.hwnd);
			if( !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			tarminate_ = true;
		}
	}
	return result;
}
void tTVPApplication::ProcessMessages() {
	MSG msg = {0};
	while(ProcessMessage(msg));
}
void tTVPApplication::HandleMessage() {
	MSG msg = {0};
	if( !ProcessMessage(msg) ) {
		HandleIdle(msg);
	}
}
void tTVPApplication::HandleIdle(MSG &) {
	::WaitMessage();
}
void tTVPApplication::SetTitle( const std::wstring& caption ) {
	title_ = caption;
	if( windows_list_.size() > 0 ) {
		windows_list_[0]->SetCaption( caption );
	}
	if( is_attach_console_ ) {
		::SetConsoleTitle( caption.c_str() );
	}
}
HWND tTVPApplication::GetMainWindowHandle() const {
	if( windows_list_.size() > 0 ) {
		return windows_list_[0]->GetHandle();
	}
	return INVALID_HANDLE_VALUE;
}

void tTVPApplication::RemoveWindow( TTVPWindowForm* win ) {
	std::vector<class TTVPWindowForm*>::iterator it = std::remove( windows_list_.begin(), windows_list_.end(), win );
	windows_list_.erase( it, windows_list_.end() );
}

void tTVPApplication::PostMessageToMainWindow(UINT message, WPARAM wParam, LPARAM lParam) {
	if( windows_list_.size() > 0 ) {
		::PostMessage( windows_list_[0]->GetHandle(), message, wParam, lParam );
	}
}
void tTVPApplication::GetDisableWindowList( std::vector<class TTVPWindowForm*>& win ) {
	size_t count = windows_list_.size();
	for( size_t i = 0; i < count; i++ ) {
		if( windows_list_[i]->GetEnable() == false ) {
			win.push_back( windows_list_[i] );
		}
	}
}
void tTVPApplication::GetEnableWindowList( std::vector<class TTVPWindowForm*>& win, class TTVPWindowForm* activeWindow ) {
	size_t count = windows_list_.size();
	for( size_t i = 0; i < count; i++ ) {
		if( activeWindow != windows_list_[i] && windows_list_[i]->GetEnable() ) {
			win.push_back( windows_list_[i] );
		}
	}
}

void tTVPApplication::DisableWindows() {
	size_t count = windows_list_.size();
	for( size_t i = 0; i < count; i++ ) {
		windows_list_[i]->SetEnable( false );
	}
}
void tTVPApplication::EnableWindows( const std::vector<TTVPWindowForm*>& win ) {
	size_t count = win.size();
	for( size_t i = 0; i < count; i++ ) {
		win[i]->SetEnable( true );
	}
	/*
	size_t count = windows_list_.size();
	for( size_t i = 0; i < count; i++ ) {
		TTVPWindowForm* win = windows_list_[i];
		std::vector<TTVPWindowForm*>::const_iterator f = std::find( ignores.begin(), ignores.end(), win );
		if( f == ignores.end() ) {
			windows_list_[i]->SetEnable( true );
		}
	}
	*/
}
void tTVPApplication::FreeDirectInputDeviceForWindows() {
	size_t count = windows_list_.size();
	for( size_t i = 0; i < count; i++ ) {
		windows_list_[i]->FreeDirectInputDevice();
	}
}


void tTVPApplication::RegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd) {
	accel_key_.AddKey( hWnd, cmd, key, virt );
}
void tTVPApplication::UnregisterAcceleratorKey(HWND hWnd, short cmd) {
	accel_key_.DelKey( hWnd, cmd );
}
void tTVPApplication::DeleteAcceleratorKeyTable( HWND hWnd ) {
	accel_key_.DelTable( hWnd );
}
void tTVPApplication::CheckDigitizer() {
	// Windows 7 以降でのみ有効
	OSVERSIONINFOEX ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	::GetVersionEx((OSVERSIONINFO*)&ovi);
	if( ovi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		ovi.dwMajorVersion >= 6 && ovi.dwMinorVersion >= 1 ) {

		int value = ::GetSystemMetrics(SM_DIGITIZER);
		if( value == 0 ) return;

		TVPAddLog( (const tjs_char*)TVPEnableDigitizer );
		if( value & NID_INTEGRATED_TOUCH ) {
			TVPAddLog( (const tjs_char*)TVPTouchIntegratedTouch );
		}
		if( value & NID_EXTERNAL_TOUCH ) {
			TVPAddLog( (const tjs_char*)TVPTouchExternalTouch );
		}
		if( value & NID_INTEGRATED_PEN ) {
			TVPAddLog( (const tjs_char*)TVPTouchIntegratedPen );
		}
		if( value & NID_EXTERNAL_PEN ) {
			TVPAddLog( (const tjs_char*)TVPTouchExternalPen );
		}
		if( value & NID_MULTI_INPUT ) {
			TVPAddLog( (const tjs_char*)TVPTouchMultiInput );
		}
		if( value & NID_READY ) {
			TVPAddLog( (const tjs_char*)TVPTouchReady );
		}
	}
}
void tTVPApplication::OnActivate( HWND hWnd )
{
	if( hWnd != GetMainWindowHandle() ) return;

	ApplicationActivating = true;
	
	TVPRestoreFullScreenWindowAtActivation();
	TVPResetVolumeToAllSoundBuffer();

	// trigger System.onActivate event
	TVPPostApplicationActivateEvent();
}
void tTVPApplication::OnDeactivate( HWND hWnd )
{
	if( hWnd != GetMainWindowHandle() ) return;

	ApplicationActivating = false;
	
	TVPMinimizeFullScreenWindowAtInactivation();
	
	// fire compact event
	TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_DEACTIVATE);

	// set sound volume
	TVPResetVolumeToAllSoundBuffer();

	// trigger System.onDeactivate event
	TVPPostApplicationDeactivateEvent();
}
bool tTVPApplication::GetNotMinimizing() const
{
	HWND hWnd = GetMainWindowHandle();
	if( hWnd != INVALID_HANDLE_VALUE && hWnd != NULL ) {
		return ::IsIconic( hWnd ) == 0;
	}
	return true; // メインがない時は最小化されているとみなす
}

void tTVPApplication::LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name ) {
	if( ImageLoadThread ) {
		ImageLoadThread->LoadRequest( owner, bmp, name );
	}
}
/**
 仮実装 TODO
*/
std::vector<std::string>* LoadLinesFromFile( const std::wstring& path ) {
	FILE *fp = NULL;
	_wfopen_s( &fp, path.c_str(), L"r");
    if( fp == NULL ) {
		return NULL;
    }
	char buff[1024];
	std::vector<std::string>* ret = new std::vector<std::string>();
    while( fgets(buff, 1024, fp) != NULL ) {
		ret->push_back( std::string(buff) );
    }
    fclose(fp);
	return ret;
}

void TVPRegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd) {
	if( Application ) Application->RegisterAcceleratorKey( hWnd, virt, key, cmd );
}
void TVPUnregisterAcceleratorKey(HWND hWnd, short cmd) {
	if( Application ) Application->UnregisterAcceleratorKey( hWnd, cmd );
}
void TVPDeleteAcceleratorKeyTable( HWND hWnd ) {
	if( Application ) Application->DeleteAcceleratorKeyTable( hWnd );
}
