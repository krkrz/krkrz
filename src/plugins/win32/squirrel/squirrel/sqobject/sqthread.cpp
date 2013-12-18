/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#include "sqthread.h"
#include <string.h>
extern SQRESULT sqstd_loadmemory(HSQUIRRELVM v, const char *dataBuffer, int dataSize, const SQChar *filename, SQBool printerror);

namespace sqobject {

// パラメータが不正
static SQRESULT ERROR_INVALIDPARAM(HSQUIRRELVM v) {
	return sq_throwerror(v, _SC("invalid param"));
}

// スレッドが存在しない
static SQRESULT ERROR_NOTHREAD(HSQUIRRELVM v) {
	return sq_throwerror(v, _SC("no thread"));
}

// fork に失敗した
static SQRESULT ERROR_FORK(HSQUIRRELVM v) {
	return sq_throwerror(v,_SC("failed to fork"));
}

bool
Thread::isWait()
{
	return !_waitSystem.isNull() || _waitList.len() > 0 || _waitTimeout >= 0;
}

/**
 * @return 該当スレッドと現在管理中のスレッドが一致してれば true
 */
bool
Thread::isSameThread(HSQUIRRELVM v)
{
	return _thread.isSameThread(v);
}

/**
 * オブジェクトに対する待ち情報を完了させる
 * @param target 待ち対象
 * @return 該当オブジェクトを待ってた場合は true
 */
bool
Thread::notifyObject(ObjectInfo &target)
{
	bool find = false;
	if (!_waitSystem.isNull() && _waitSystem == target) { // systemコマンド専用の待ち
		find = true;
		Thread *th = _waitSystem;
		if (th) {
			_waitResult = th->_exitCode;
		}
		_waitSystem.clear();
	} else {
		SQInteger i = 0;
		SQInteger max = _waitList.len();
		while (i < max) {
			ObjectInfo obj = _waitList.get(i);
			if (obj == target) {
				find = true;
				_waitResult = obj;
				_waitList.remove(i);
				max--;
			} else {
				i++;
			}
		}
	}
	if (find) {
		_clearWait();
	}
	return find;
}

/**
 * トリガに対する待ち情報を完了させる
 * @param name トリガ名
 * @return 該当オブジェクトを待ってた場合は true
	 */
bool
Thread::notifyTrigger(const SQChar *name)
{
	bool find = false;
	int i = 0;
	SQInteger max = _waitList.len();
	while (i < max) {
		ObjectInfo obj = _waitList.get(i);
		if (obj == name) {
			find = true;
			_waitResult = obj;
			_waitList.remove(i);
			max--;
		} else {
			i++;
		}
	}
	if (find) {
		_clearWait();
	}
	return find;
}

// コンストラクタ
Thread::Thread() : _currentTick(0), _fileHandler(NULL), _waitTimeout(-1), _status(THREAD_NONE)
{
	_waitList.initArray();
}

// コンストラクタ
Thread::Thread(HSQUIRRELVM v) : Object(v), _currentTick(0), _fileHandler(NULL), _waitTimeout(-1), _status(THREAD_NONE)
{
	_waitList.initArray();
	// 実行
	if (sq_gettop(v) >= 3) {
		_exec(v,3);
		_entryThread(v);
	}
}

// デストラクタ
Thread::~Thread()
{
	_exit();
	_thread.clear();
	_waitList.clear();
}

/**
 * 情報破棄
 */
void
Thread::_init()
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_newthread(gv, 1024);
	_thread.getStack(gv, -1);
	sq_pop(gv, 1);
}

/**
 * 情報破棄
 */
void
Thread::_clear()
{
	_clearWait();
	if (_fileHandler) {
		sqobjCloseFile(_fileHandler);
		_fileHandler = NULL;
		_scriptName.clear();
	}
	_args.clear();
	_status = THREAD_NONE;
}

void
Thread::_exit()
{
	notifyAll();
	_clear();
}

/**
 * オブジェクトに対する待ち情報をクリアする
 * @param status キャンセルの場合は true
 */
void
Thread::_clearWait()
{
	// system用の waitの解除
	Object *obj = _waitSystem;
	if (obj) {
		obj->removeWait(self);
	}
	_waitSystem.clear();

	// その他の wait の解除
	SQInteger max = _waitList.len();
	for (SQInteger i=0;i<max;i++) {
		Object *obj = _waitList.get(i);
		if (obj) {
			obj->removeWait(self);
		}
	}
	_waitList.clearData();
	// タイムアウト指定の解除
	_waitTimeout = -1;
}

/**
 * 内部用:fork 処理。スレッドを１つ生成して VMにPUSHする
 * @param v squirrelVM
 * @return 成功したら true
 */
bool
Thread::_fork(HSQUIRRELVM v)
{
	// スレッドオブジェクトはグローバル上に生成する
	SQInteger max = sq_gettop(v);
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushroottable(gv); // root
	sq_pushstring(gv, SQTHREADNAME, -1);
	if (SQ_SUCCEEDED(sq_get(gv,-2))) { // class
		sq_pushroottable(gv); // 引数:self(root)
		sq_pushnull(gv);      // 引数:delegate
		// 引数をコピー
		int argc = 2;
		if (gv == v) {
			for (int i=2;i<=max;i++) {
				sq_push(gv,i);
				argc++;
			}
		} else {
			for (int i=2;i<=max;i++) {
				sq_move(gv, v, i);
				argc++;
			}
		}
		if (SQ_SUCCEEDED(sq_call(gv, argc, SQTrue, SQTrue))) { // コンストラクタ呼び出し
			if (gv == v) {
				sq_remove(gv, -2); // class
				sq_remove(gv, -2); // root
			} else {
				sq_move(v, gv, sq_gettop(gv)); // 元VMのほうに移す
				sq_pop(gv, 3); // thread,class,root
			}
			return true;
		}
		sq_pop(gv, 1); // class
	}
	sq_pop(gv,1); // root
	return false;
}

/**
 * 内部用: wait処理
 * @param v squirrelVM
 * @param idx 該当 idx 以降にあるものを待つ
 */
void
Thread::_wait(HSQUIRRELVM v, int idx)
{
	_clearWait();
	_waitResult.clear();
	SQInteger max = sq_gettop(v);
	for (int i=idx;i<=max;i++) {
		switch (sq_gettype(v, i)) {
		case OT_INTEGER:
		case OT_FLOAT:
			// 数値の場合はタイムアウト待ち
			{
				SQInteger timeout;
				sq_getinteger(v, i, &timeout);
				if (timeout >= 0) {
					if (_waitTimeout < 0  || _waitTimeout > timeout) {
						_waitResult.getStack(v, i);
						_waitTimeout = timeout;
					}
				}
			}
			break;
		case OT_STRING:
			// 待ちリストに登録
			_waitList.append(ObjectInfo(v,i));
			break;
		case OT_INSTANCE:
			// オブジェクトに待ち登録してから待ちリストに登録
			{
				ObjectInfo o;
				o.getStackWeak(v,i);
				Object *obj = o;
				if (obj) {
					obj->addWait(self);
				}
				_waitList.append(o);
			}
			break;
		default:
			break;
		}
	}
}

/**
 * 内部用: system処理の待ち登録。スタック先頭にあるスレッドを待つ
 * @param v squirrelVM
 */
void
Thread::_system(HSQUIRRELVM v)
{
	_clearWait();
	_waitResult.clear();
	_waitSystem.getStackWeak(v, -1);
	Object *obj = _waitSystem;
	if (obj) {
		obj->addWait(self);
	}
}

// ---------------------------------------------------------------

SQRESULT
Thread::wait(HSQUIRRELVM v)
{
	_wait(v);
	return 0;
}

/**
 * waitのキャンセル
 */
void
Thread::cancelWait()
{
	_clearWait();
	_waitResult.clear();
}

/**
 * 内部用: exec処理
 * @param v squirrelVM
 * @param idx このインデックスから先にあるものを実行開始する。文字列ならスクリプト、ファンクションなら直接
 */
void
Thread::_exec(HSQUIRRELVM v, int idx)
{
	_clear();
	_thread.clear();
	// スレッド先頭にスクリプトをロード
	if (sq_gettype(v, idx) == OT_STRING) {
		// スクリプト指定で遅延ロード
		_scriptName.getStack(v, idx);
		_fileHandler = sqobjOpenFile(getString(v, idx));
		_status = THREAD_LOADING_FILE;
	} else {
		// ファンクション指定
		_func.getStack(v, idx);
		_status = THREAD_LOADING_FUNC;
	}

	// 引数を記録
	SQInteger max = sq_gettop(v);
	if (max > idx) {
		_args.initArray();
		for (int i=idx+1;i<=max;i++) {
			_args.append(v, i);
		}
	}
}

/**
 * スレッドとして登録する
 */
void
Thread::_entryThread(HSQUIRRELVM v)
{
	// スレッド情報として登録
	ObjectInfo thinfo(v,1);
	SQInteger max = threadList->len();
	for (int i=0;i<max;i++) {
		if (threadList->get(i) == thinfo) {
			return;
		}
	}
	newThreadList->append(thinfo);
}


/**
 * 実行開始
 * @param func 実行対象ファンクション。文字列の場合該当スクリプトを読み込む
 */
SQRESULT
Thread::exec(HSQUIRRELVM v)
{
	if (sq_gettop(v) <= 1) {
		return ERROR_INVALIDPARAM(v);
	}
		
	_exec(v);
	_entryThread(v);

	return SQ_OK;
}

/**
 * 実行終了
 */
SQRESULT
Thread::exit(HSQUIRRELVM v)
{
	if (sq_gettop(v) >= 2) {
		_exitCode.getStack(v,2);
	} else {
		_exitCode.clear();
	}
	_exit();
	return SQ_OK;
}

/**
 * @return 実行ステータス
 */
SQRESULT
Thread::getExitCode(HSQUIRRELVM v)
{
	_exitCode.push(v);
	return 1;
}

/**
 * 実行停止
 */
void
Thread::stop()
{
	if (_status == THREAD_RUN) {
		_status = THREAD_STOP;
	}
}

/**
 * 実行再開
 */
void
Thread::run()
{
	if (_status == THREAD_STOP) {
		_status = THREAD_RUN;
	}
}

/**
 * @return 実行ステータス
 */
int
Thread::getStatus()
{
	return isWait() ? THREAD_WAIT : _status;
}

/**
 * スレッドのメイン処理
 * @param diff 経過時間
 * @return スレッド実行終了なら true
 */
bool
Thread::_main(long diff)
{
	// スレッドとして動作できてない場合は即終了
	if (_status == THREAD_NONE) {
		return true;
	}

	if (_status == THREAD_LOADING_FILE) {
		// ファイル読み込み処理
		const char *dataAddr;
		int dataSize;
		if (sqobjCheckFile(_fileHandler, &dataAddr, &dataSize)) {
			_init();
			SQRESULT ret = sqstd_loadmemory(_thread, dataAddr, dataSize, _scriptName.getString(), SQTrue);
			sqobjCloseFile(_fileHandler);
			_fileHandler = NULL;
			if (SQ_SUCCEEDED(ret)) {
				_status = THREAD_RUN;
			} else {
				// exit相当
				printError();
				_exit();
				return true;
			}
		} else {
			// 読み込み完了待ち
			return false;
		}
	} else if (_status == THREAD_LOADING_FUNC) {
		// スクリプト読み込み処理
		_init();
		_func.push(_thread);
		_func.clear();
		_status = THREAD_RUN;
	}

	_currentTick += diff;
	
	// タイムアウト処理
	if (_waitTimeout >= 0) {
		_waitTimeout -= diff;
		if (_waitTimeout < 0) {
			_clearWait();
		}
	}
		
	// スレッド実行
	if (!isWait() && _status == THREAD_RUN) {
		SQRESULT result;
		if (sq_getvmstate(_thread) == SQ_VMSTATE_SUSPENDED) {
			_waitResult.push(_thread);
			_waitResult.clear();
			result = sq_wakeupvm(_thread, SQTrue, SQTrue, SQTrue, SQFalse);
		} else {
			sq_pushroottable(_thread);
			SQInteger n = _args.pushArray(_thread) + 1;
			_args.clear();
			result = sq_call(_thread, n, SQTrue, SQTrue);
		}
		if (SQ_FAILED(result)) {
			// スレッドがエラー終了
			printError();
			_exit();
		} else {
			// 終了コード取得。return/suspend の値が格納される
			_exitCode.getStack(_thread, -1);
			sq_pop(_thread, 1);
			if (sq_getvmstate(_thread) == SQ_VMSTATE_IDLE) {
				// スレッドが終了
				_exit();
			}
		}
	}
	
	return _status == THREAD_NONE;
}

// -------------------------------------------------------------------------

/**
 * スレッドのエラー情報の表示
 */
void
Thread::printError()
{
	SQPRINTFUNCTION print = sq_getprintfunc(_thread);
	if (print) {
		sq_getlasterror(_thread);
		const SQChar *err;
		if (SQ_FAILED(sq_getstring(_thread, -1, &err))) {
			err = _SC("unknown");
		}
		print(_thread,_SC("error:%s:%s\n"), _scriptName.getString(), err);
		sq_pop(_thread, 1);
	}
}

/**
 * 動作スレッドの破棄
 */
void
Thread::init()
{
	threadList = new ObjectInfo();
	newThreadList = new ObjectInfo();
	threadList->initArray();
	newThreadList->initArray();
}

/*
 * 時間更新
 * @param diff 経過時間
 */
void
Thread::update(long diff)
{
	diffTick = diff;
	currentTick += diff;
}

/*
 * 実行処理メインループ
 * 現在存在するスレッドを総なめで１度だけ実行する。
 * システム本体のメインループ(イベント処理＋画像処理)
 * から1度だけ呼び出すことで機能する。それぞれのスレッドは、
 * 自分から明示的に suspend() または wait系のメソッドを呼び出して処理を
 * 次のスレッドに委譲する必要がある。
 * @return 動作中のスレッドの数
 */
int
Thread::main(ThreadCallback *onThreadDone, void *userData)
{
	threadList->appendArray(*newThreadList);
	newThreadList->clearData();
	SQInteger i=0;
	SQInteger max = threadList->len();
	while (i < max) {
		ObjectInfo thObj = threadList->get(i);
		Thread *th = thObj;
		if (!th || th->_main(diffTick)) {
			if (onThreadDone) {
				onThreadDone(thObj, userData);
			}
			threadList->remove(i);
			max--;
		} else {
			i++;
		}
	}
	return getThreadCount();
};

int
Thread::getThreadCount()
{
	return (int)threadList->len() + (int)newThreadList->len();
}

/**
 * スクリプト実行開始用
 * @param scriptName スクリプト名
 * @param argc 引数の数
 * @param argv 引数
 * @return 成功なら true
 */
bool
Thread::fork(const SQChar *scriptName, int argc, const SQChar **argv)
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushroottable(gv); // root
	sq_pushstring(gv, SQTHREADNAME, -1);
	if (SQ_SUCCEEDED(sq_get(gv,-2))) { // class
		sq_pushroottable(gv); // 引数:self(root)
		sq_pushnull(gv);      // 引数:delegate
		sq_pushstring(gv, scriptName, -1); // 引数:func
		int n = 3;
		for (int i=0;i<argc;i++) {
			sq_pushstring(gv, argv[i], -1);
			n++;
		}
		if (SQ_SUCCEEDED(sq_call(gv, n, SQTrue, SQTrue))) { // コンストラクタ呼び出し
			sq_pop(gv, 3); // thread,class,root
			return true;
		}
		sq_pop(gv, 1); // class
	}
	sq_pop(gv,1); // root
	return false;
}

/**
 * 全スレッドへのトリガ通知
 * @param name 処理待ちトリガ名
 */
void
Thread::trigger(const SQChar *name)
{
	SQInteger max = threadList->len();
	for (SQInteger i=0;i<max;i++) {
		Thread *th = threadList->get(i);
		if (th) {
			th->notifyTrigger(name);
		}
	}
}

/**
 * 動作スレッドの破棄
 */
void
Thread::done()
{
	// 全スレッドを強制中断
	threadList->appendArray(*newThreadList);
	newThreadList->clearData();
	SQInteger max = threadList->len();
	for (SQInteger i=0;i<max;i++) {
		Thread *th = threadList->get(i);
		if (th) { th->_exit();}
	}
	threadList->clearData();
	// リスト自体を破棄
	threadList->clear();
	newThreadList->clear();
	delete threadList;
	delete newThreadList;
}

// -------------------------------------------------------------
// グローバルスレッド用
// -------------------------------------------------------------

ObjectInfo *Thread::threadList; //< スレッド一覧
ObjectInfo *Thread::newThreadList; //< 新規スレッド一覧
long Thread::currentTick = 0;  //< 現在のシステムtick
long Thread::diffTick = 0;  //< 今回の呼び出し差分

// -------------------------------------------------------------
// グローバルメソッド用
// -------------------------------------------------------------

/**
 * 現在時刻の取得
 */
SQRESULT
Thread::global_getCurrentTick(HSQUIRRELVM v)
{
	sq_pushinteger(v, currentTick);
	return 1;
}

/**
 * 差分時刻の取得
 */
SQRESULT
Thread::global_getDiffTick(HSQUIRRELVM v)
{
	sq_pushinteger(v, diffTick);
	return 1;
}

/*
 * @return 現在のスレッドを返す
 */
SQRESULT
Thread::global_getCurrentThread(HSQUIRRELVM v)
{
	SQInteger max = threadList->len();
	for (SQInteger i=0;i<max;i++) {
		Thread *th = threadList->get(i);
		if (th && th->isSameThread(v)) {
			th->push(v);
			return 1;
		}
	}
	return ERROR_NOTHREAD(v);
}

/*
 * @return 現在のスレッド一覧を返す
 */
SQRESULT
Thread::global_getThreadList(HSQUIRRELVM v)
{
	threadList->pushClone(v);
	return 1;
}

/*
 * スクリプトを新しいスレッドとして実行する
 * ※ return Thread(func); 相当
 * @param func スレッドで実行するファンクション
 * @return 新スレッド
 */
SQRESULT
Thread::global_fork(HSQUIRRELVM v)
{
	if (!_fork(v)) {
		return ERROR_FORK(v);
	}
	return 1;
}

/**
 * @return 現在実行中のスレッド情報オブジェクト(Thread*)
 */
Thread *
Thread::getCurrentThread(HSQUIRRELVM v)
{
	SQInteger max = threadList->len();
	for (SQInteger i=0;i<max;i++) {
		Thread *th = threadList->get(i);
		if (th && th->isSameThread(v)) {
			return th;
		}
	}
	return NULL;
}

/**
 * スクリプトを切り替える
 * @param func スレッドで実行するファンクション
 */
SQRESULT
Thread::global_exec(HSQUIRRELVM v)
{
	Thread *th = getCurrentThread(v);
	if (!th) {
		return ERROR_NOTHREAD(v);
	}
	if (sq_gettop(v) <= 1) {
		return ERROR_INVALIDPARAM(v);
	}
	th->_exec(v);
	return sq_suspendvm(v);
}

/**
 * 実行中スレッドの終了
 */
SQRESULT
Thread::global_exit(HSQUIRRELVM v)
{
	Thread *th = getCurrentThread(v);
	if (!th) {
		return ERROR_NOTHREAD(v);
	}
	th->exit(v);
	return sq_suspendvm(v);
}

/**
 * コマンド実行
 * @param func スレッドで実行するファンクション
 * @return 終了コード
 */
SQRESULT
Thread::global_system(HSQUIRRELVM v)
{
	Thread *th = getCurrentThread(v);
	if (!th) {
		return ERROR_NOTHREAD(v);
	}
	if (!_fork(v)) {
		return ERROR_FORK(v);
	}
	th->_system(v);
	sq_pop(v,1);
	return sq_suspendvm(v);
}

/**
 * 実行中スレッドの処理待ち
 * @param target int:時間待ち(ms), string:トリガ待ち, obj:オブジェクト待ち
 * @param timeout タイムアウト(省略時は無限に待つ)
 * @return 待ちがキャンセルされたら true
 */
SQRESULT
Thread::global_wait(HSQUIRRELVM v)
{
	Thread *th = getCurrentThread(v);
	if (!th) {
		return ERROR_NOTHREAD(v);
	}
	th->_wait(v);
	return sq_suspendvm(v);
}

/**
 * 全スレッドへのトリガ通知
 * @param name 処理待ちトリガ名
 */
SQRESULT
Thread::global_trigger(HSQUIRRELVM v)
{
	trigger(getString(v,2));
	return SQ_OK;
}

/**
 * ベースVM上でスクリプトを実行する。
 * この呼び出しはスレッドによるものではないため、処理中に suspend() / wait() を
 * 呼ぶとエラーになるので注意してください。必ず1度で呼びきれるものを渡す必要があります。
 * @param func グローバル関数。※ファイルは指定できません
 * @param ... 引数
 */
SQRESULT
Thread::global_execOnBase(HSQUIRRELVM v)
{
	SQInteger max = sq_gettop(v);
	if (max <= 1) {
		return ERROR_INVALIDPARAM(v);
	}
	HSQUIRRELVM gv = getGlobalVM();
	SQRESULT result = SQ_OK;
	if (gv == v) {
		sq_push(gv, 2);
		sq_pushroottable(gv); // 引数:self(root)
		int argc = 1;
		for (int i=3;i<=max;i++) {
			sq_push(v, i);
			argc++;
		}
		if (SQ_SUCCEEDED(result = sq_call(gv, argc, SQTrue, SQTrue))) {
			sq_remove(gv, -2); // func
		} else {
			sq_pop(gv, 1); // func
		}
	} else {
		sq_move(gv, v, 2);    // func
		sq_pushroottable(gv); // 引数:self(root)
		int argc = 1;
		for (int i=3;i<=max;i++) {
			sq_move(gv, v, i);
			argc++;
		}
		if (SQ_SUCCEEDED(result = sq_call(gv, argc, SQTrue, SQTrue))) {
			sq_move(v, gv, sq_gettop(gv));
			sq_pop(gv, 2);
		} else {
			sq_pop(gv, 1); // func
		}
	}
	return result;
}

/**
 * グローバルメソッド登録
 */
void
Thread::registerGlobal()
{
	// メソッド登録（名前つき）
#define REGISTERMETHODNAME(name, method) \
	sq_pushstring(v, _SC(#name), -1);\
	sq_newclosure(v, method, 0);\
	sq_createslot(v, -3);

	// enum 登録（名前つき）
#define REGISTERENUM(name, value) \
	sq_pushstring(v, _SC(#name), -1); /* 名前を push */ \
	sq_pushinteger(v, value);          /* 値を push */ \
	sq_createslot(v, -3)              /* テーブルに登録 */
	
	HSQUIRRELVM v = getGlobalVM();

	// グローバルメソッドの登録
	sq_pushroottable(v); // root
	REGISTERMETHODNAME(getCurrentTick, global_getCurrentTick);
	REGISTERMETHODNAME(getDiffTick, global_getDiffTick);
	REGISTERMETHODNAME(getCurrentThread, global_getCurrentThread);
	REGISTERMETHODNAME(getThreadList, global_getThreadList);
	REGISTERMETHODNAME(fork, global_fork);
	REGISTERMETHODNAME(exec, global_exec);
	REGISTERMETHODNAME(exit, global_exit);
	REGISTERMETHODNAME(system, global_system);
	REGISTERMETHODNAME(wait, global_wait);
	REGISTERMETHODNAME(notify, global_trigger);
	REGISTERMETHODNAME(execOnBase, global_execOnBase);
	sq_pop(v, 1); // root
	
	// 定数の登録
	sq_pushconsttable(v); // consttable
	sq_pushstring(v, _SC("THREADSTATUS"), -1); // テーブル名を push
	sq_newtable(v);                  // 新しい enum テーブル
	REGISTERENUM(NONE,THREAD_NONE);
	REGISTERENUM(LOADING_FILE,THREAD_LOADING_FILE);
	REGISTERENUM(LOADING_FUNC,THREAD_LOADING_FUNC);
	REGISTERENUM(STOP,THREAD_STOP);
	REGISTERENUM(RUN,THREAD_RUN);
	REGISTERENUM(WAIT,THREAD_WAIT);
	sq_createslot(v, -3);              /* テーブルに登録 */
	sq_pop(v, 1); // consttable
}

};
