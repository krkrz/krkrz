/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQTHREAD_H__
#define __SQTHREAD_H__

#ifndef SQTHREAD
#define SQTHREAD Thread
#define SQTHREADNAME _SC("Thread")
#endif

#include <stdio.h>
#include "sqobjectclass.h"

/**
 * ファイルを非同期に開く
 * @param filename スクリプトファイル名
 * @return ファイルハンドラ
 */
extern void *sqobjOpenFile(const SQChar *filename, bool binary=false);

/**
 * ファイルが開かれたかどうかのチェック
 * @param handler ファイルハンドラ
 * @param dataPtr データ格納先アドレス(出力)
 * @param dataSize データサイズ(出力)
 * @return ロード完了していたら true
 */
extern bool sqobjCheckFile(void *handler, const char **dataAddr, int *dataSize);

/**
 * ファイルを閉じる
 * @param handler ファイルハンドラ
 */
extern void sqobjCloseFile(void *handler);

namespace sqobject {

class Thread : public Object {

protected:
	long _currentTick; ///< このスレッドの実行時間

	ObjectInfo _scriptName; ///< スクリプト名
	
	void *_fileHandler; ///< このスレッドが開こうとしているファイル

	// スレッドデータ
	ObjectInfo _thread;

	// 実行スクリプト
	ObjectInfo _func;

	// 引数リスト
	ObjectInfo _args;

	// system用待ち
	ObjectInfo _waitSystem;
	// 待ち対象
	ObjectInfo _waitList;
	// 待ち時間
	SQInteger _waitTimeout;

	// 待ちの結果
	ObjectInfo _waitResult;
	
	/// 終了コード
	ObjectInfo _exitCode;
	
	/**
	 * スレッド状態
	 */
	enum ThreadStatus {
		THREAD_NONE,           // 未初期化
		THREAD_LOADING_FILE,   // ファイルロード中
		THREAD_LOADING_FUNC,   // 関数ロード中
		THREAD_STOP,   // 停止
		THREAD_RUN,    // 動作中
		THREAD_WAIT,   // 待ち中
	} _status;

	/**
	 * @return 処理待ち中か
	 */
	bool isWait();

	/**
	 * @return 該当スレッドと現在管理中のスレッドが一致してれば true
	 */
	bool isSameThread(HSQUIRRELVM v);

public:
	// コンストラクタ
	Thread();

	// コンストラクタ
	Thread(HSQUIRRELVM v);
	
	// デストラクタ
	~Thread();

protected:
	/**
	 * スレッド情報初期化
	 */
	void _init();

	/**
	 * オブジェクトに対する待ち情報をクリアする
	 * @param status キャンセルの場合は true
	 */
	void _clearWait();

	/**
	 * 情報破棄
	 */
	void _clear();

	/**
	 * exit内部処理
	 */
	void _exit();
	
	// ------------------------------------------------------------------
	//
	// Object からの制御用
	//
	// ------------------------------------------------------------------
	
public:

	/**
	 * トリガに対する待ち情報を完了させる
	 * @param name トリガ名
	 * @return 該当オブジェクトを待ってた場合は true
	 */
	bool notifyTrigger(const SQChar *name);

	/**
	 * オブジェクトに対する待ち情報を完了させる
	 * @param target 待ち対象
	 * @return 該当オブジェクトを待ってた場合は true
	 */
	bool notifyObject(ObjectInfo &target);
	
	// ------------------------------------------------------------------
	//
	// メソッド
	//
	// ------------------------------------------------------------------

protected:

	/**
	 * 内部用:fork 処理。スレッドを１つ生成して VMにPUSHする
	 * @param v squirrelVM
	 * @return 成功したら true
	 */
	static bool _fork(HSQUIRRELVM v);

	/**
	 * 内部用: wait処理
	 * @param v squirrelVM
	 * @param idx 該当 idx 以降にあるものを待つ
	 */
	void _wait(HSQUIRRELVM v, int idx=2);

	/**
	 * 内部用: system処理の待ち。スタック先頭にあるスレッドを待つ
	 * @param v squirrelVM
	 */
	void _system(HSQUIRRELVM v);

	
	/**
	 * 内部用: exec処理
	 * @param v squirrelVM
	 * @param idx このインデックスから先にあるものを実行開始する。文字列ならスクリプト、ファンクションなら直接
	 */
	void _exec(HSQUIRRELVM v, int idx=2);

	/**
	 * 現在のオブジェクトを実行スレッドとして登録
	 */
	void _entryThread(HSQUIRRELVM v);
	
public:
	/**
	 * 待ち登録
	 */
	SQRESULT wait(HSQUIRRELVM v);

	/**
	 * waitのキャンセル
	 */
	void cancelWait();

	/**
	 * 実行開始
	 * @param func 実行対象ファンクション。文字列の場合該当スクリプトを読み込む
	 */
	SQRESULT exec(HSQUIRRELVM v);

	/**
	 * 実行終了
	 * @param exitCode 終了コード
	 */
	SQRESULT exit(HSQUIRRELVM v);

	/**
	 * exitCode取得
	 */
	SQRESULT getExitCode(HSQUIRRELVM v);
	
	/**
	 * 実行停止
	 */
	void stop();

	/**
	 * 実行再開
	 */
	void run();

	/**
	 * @return 実行ステータス
	 */
	int getStatus();

	/**
	 * @return 現在時刻
	 */
	int getCurrentTick() {
		return _currentTick;
	}

	// ------------------------------------------------------------------
	//
	// 実行処理
	//
	// ------------------------------------------------------------------
	
protected:

	/**
	 * スレッドのエラー情報の表示
	 */
	void printError();

	/**
	 * スレッドのメイン処理
	 * @param diff 経過時間
	 * @return スレッド実行終了なら true
	 */
	bool _main(long diff);

public:

	/**
	 * 動作スレッド初期化
	 */
	static void init();

	/*
	 * 時間更新
	 * @param diff 経過時間
	 */
	static void update(long diff);

	/**
	 * スレッド処理用コールバック
	 * @param th スレッドオブジェクト
	 * @param userData ユーザデータ
	 */
	typedef void ThreadCallback(ObjectInfo th, void *userData);
	
	/*
	 * 実行処理メインループ
	 * 現在存在するスレッドを総なめで１度だけ実行する。
	 * システム本体のメインループ(イベント処理＋画像処理)
	 * から1度だけ呼び出すことで機能する。それぞれのスレッドは、
	 * 自分から明示的に suspend() または wait系のメソッドを呼び出して処理を
	 * 次のスレッドに委譲する必要がある。
	 * @param onThreadDone スレッド終了時に呼び出されるコールバック
	 * @param userData コールバックに渡すユーザデータ引数
	 * @return 動作中のスレッドの数
	 */
	static int main(ThreadCallback *onThreadDone=NULL, void *userData=NULL);

	/**
	 * スクリプト実行開始用
	 * @param scriptName スクリプト名
	 * @param argc 引数の数
	 * @param argv 引数
	 * @return 成功なら true
	 */
	static bool fork(const SQChar *scriptName, int argc=0, const SQChar **argv=NULL);

	/**
	 * 全スレッドへのトリガ通知
	 * @param name 処理待ちトリガ名
	 */
	static void trigger(const SQChar *name);
	
	/**
	 * 動作スレッドの破棄
	 */
	static void done();

	/**
	 * 動作スレッド数
	 */
	static int getThreadCount();

	// -------------------------------------------------------------
	// スレッド処理用
	// -------------------------------------------------------------

public:
	static long currentTick;  ///< 今回の呼び出し時間
	static long diffTick;     ///< 差分呼び出し時間
	
protected:
	static ObjectInfo *threadList; ///< スレッド一覧
	static ObjectInfo *newThreadList; ///< スレッド一覧

	// -------------------------------------------------------------
	// グローバルメソッド用
	// -------------------------------------------------------------

	/**
	 * 現在時刻の取得
	 */
	static SQRESULT global_getCurrentTick(HSQUIRRELVM v);

	/**
	 * 差分時刻の取得
	 */
	static SQRESULT global_getDiffTick(HSQUIRRELVM v);
	
	/*
	 * @return 現在のスレッドを返す
	 */
	static SQRESULT global_getCurrentThread(HSQUIRRELVM v);
	
	/*
	 * @return 現在のスレッド一覧を返す
	 */
	static SQRESULT global_getThreadList(HSQUIRRELVM v);

	/*
	 * スクリプトを新しいスレッドとして実行する
	 * ※ return Thread(func); 相当
	 * @param func スレッドで実行するファンクション
	 * @return 新スレッド
	 */
	static SQRESULT global_fork(HSQUIRRELVM v);

	/**
	 * @return 現在実行中のスレッド情報オブジェクト(Thread*)
	 */
	static Thread *getCurrentThread(HSQUIRRELVM v);

	/**
	 * スクリプトを切り替える
	 * @param func スレッドで実行するファンクション
	 */
	static SQRESULT global_exec(HSQUIRRELVM v);

	/**
	 * 実行中スレッドの終了
	 * @param exitCode 終了コード
	 */
	static SQRESULT global_exit(HSQUIRRELVM v);

	/**
	 * スクリプトを実行してその終了を待つ
	 * @param func スレッドで実行するファンクション
	 * @return スクリプトの終了コード
	 */
	static SQRESULT global_system(HSQUIRRELVM v);
	
	/**
	 * 実行中スレッドの処理待ち
	 * @param target int:時間待ち(ms), string:トリガ待ち, obj:オブジェクト待ち
	 * @param timeout タイムアウト(省略時は無限に待つ)
	 * @return 待ちがキャンセルされたら true
	 */
	static SQRESULT global_wait(HSQUIRRELVM v);

	/**
	 * 全スレッドへのトリガ通知
	 * @param name 処理待ちトリガ名
	 */
	static SQRESULT global_trigger(HSQUIRRELVM v);

	/**
	 * ベースVM上でスクリプトを実行する。
	 * この呼び出しはスレッドによるものではないため、処理中に suspend() / wait() を
	 * 呼ぶとエラーになるので注意してください。必ず1度で呼びきれるものを渡す必要があります。
	 * @param func グローバル関数。※ファイルは指定できません
	 */
	static SQRESULT global_execOnBase(HSQUIRRELVM v);
	
public:
	/**
	 * グローバルメソッドの登録
	 */
	static void registerGlobal();

	/**
	 * クラスの登録
	 */
	static void registerClass();

};

};

#endif
