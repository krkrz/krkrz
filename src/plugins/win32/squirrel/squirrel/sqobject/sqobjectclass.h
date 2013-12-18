/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQOBJECT_H__
#define __SQOBJECT_H__

// 型名
#ifndef SQOBJECT
#define SQOBJECT Object
#define SQOBJECTNAME _SC("Object")
#endif

// オブジェクトバインド用処理
#ifndef NOUSESQRAT
#include <sqrat.h>
#else
#include "sqfunc.h"
#endif

#include "sqobjectinfo.h"

namespace sqobject {

const SQChar *getString(HSQUIRRELVM v, SQInteger idx);

/**
 * オブジェクト用
 */
class Object {

protected:
	// squirrelオブジェクトの自己参照
	ObjectInfo self;
	// このオブジェクトを待ってるスレッドの一覧
	ObjectInfo _waitThreadList;
	// delegate
	ObjectInfo delegate;

public:
#ifdef SQOBJHEAP
	SQHEAPDEFINE;
#endif
	bool isInit() {
		return !self.isNull();
	}
	void push(HSQUIRRELVM v) {
		self.push(v);
	}
	
	/**
	 * オブジェクト待ちの登録
	 * @param thread スレッド
	 */
	void addWait(ObjectInfo &thread);
	
	/**
	 * オブジェクト待ちの解除
	 * @param thread スレッド
	 */
	void removeWait(ObjectInfo &thread);
	
	/**
	 * コンストラクタ
	 */
	Object();
	
	/**
	 * コンストラクタ
	 * @param v squirrelVM
	 * @param delegateIdx デルゲートが格納されてる引数番号
	 */
	Object(HSQUIRRELVM v, int delegateIdx=2);

	/**
	 * デストラクタ
	 */
	virtual ~Object();

	/**
	 * 自己参照初期化用
	 * デフォルトコンストラクタで処理した場合は必ずこの処理をよぶこと
	 * @param v SQUIRREL vm
	 * @param idx 自分のオブジェクトがあるインデックス
	 */
	void initSelf(HSQUIRRELVM v, int idx=1);

	/**
	 * 破棄処理用
	 */
	void destructor();

public:
	
	// ------------------------------------------------------------------

	/**
	 * このオブジェクトを待っている１スレッドの待ちを解除
	 */
	void notify();
	
	/**
	 * このオブジェクトを待っている全スレッドの待ちを解除
	 */
	void notifyAll();
	
	/**
	 * プロパティから値を取得
	 * @param name プロパティ名
	 * @return プロパティ値
	 */
	SQRESULT _get(HSQUIRRELVM v);

	/**
	 * プロパティに値を設定
	 * @param name プロパティ名
	 * @param value プロパティ値
	 */
	SQRESULT _set(HSQUIRRELVM v);

	/**
	 * setプロパティの存在確認
	 * @param name プロパティ名
	 * @return setプロパティが存在したら true
	 */
	SQRESULT hasSetProp(HSQUIRRELVM v);
	
	/**
	 * 委譲の設定
	 */
	SQRESULT setDelegate(HSQUIRRELVM v);

	/**
	 * 委譲の取得
	 */
	SQRESULT getDelegate(HSQUIRRELVM v);

public:
	/**
	 * squirrel クラス登録
	 */
	static void registerClass();


protected:

	/**
	 * 自己オブジェクトイベント呼び出し（引数無し)
	 * C++から squirrel の指定メソッドをイベントとしてコールバックできます。
	 * @param eventName イベント名
	 */
	SQRESULT callEvent(const SQChar *eventName) {
		return self.callMethod(eventName);
	}

	/**
	 * 自己オブジェクトイベント呼び出し（引数1つ)
	 * @param eventName イベント名
	 * @param p1 引数
	 */
	template<typename T1> SQRESULT callEvent(const SQChar *eventName, T1 p1) {
		return self.callMethod(eventName, p1);
	}
	
	/**
	 * 自己オブジェクトイベント呼び出し（引数2つ)
	 * @param eventName イベント名
	 * @param p1 引数
	 * @param p2 引数2
	 */
	template<typename T1, typename T2> SQRESULT callEvent(const SQChar *eventName, T1 p1, T2 p2) {
		return self.callMethod(eventName, p1, p2);
	}
	
	/**
	 * 返値有り自己オブジェクトイベント呼び出し（引数無し)
	 * @param r 帰り値ポインタ
	 * @param eventName イベント名
	 */
	template<typename R> SQRESULT callEventResult(R* r, const SQChar *eventName) {
		return self.callMethodResult(r, eventName);
	}

	/**
	 * 返値あり自己オブジェクトイベント呼び出し（引数1つ)
	 * @param r 帰り値ポインタ
	 * @param eventName イベント名
	 * @param p1 引数
	 */
	template<typename R, typename T1> SQRESULT callEventResult(R* r, const SQChar *eventName, T1 p1) {
		return self.callMethodResult(r, eventName, p1);
	}
	
	/**
	 * 返値有り自己オブジェクトイベント呼び出し（引数2つ)
	 * @param r 帰り値ポインタ
	 * @param eventName イベント名
	 * @param p1 引数
	 * @param p2 引数2
	 */
	template<typename R, typename T1, typename T2> SQRESULT callEventResult(R* r, const SQChar *eventName, T1 p1, T2 p2) {
		return self.callMethodResult(r, eventName, p1, p2);
	}

};

// ---------------------------------------------------
// オブジェクトバインド処理用
// ---------------------------------------------------

// Objectを pushする
// @return すでに squirrel 用に初期化済みでインスタンスをもっていて push できたら true
bool pushObject(HSQUIRRELVM v, Object *obj);


#ifndef NOUSESQRAT
// 値の格納
// 格納失敗したときはオブジェクトが削除されるので delete の必要はない
// ※元が squirrel 側で生成されたオブジェクトだった場合は必ず成功する
template<typename T>
void pushValue(HSQUIRRELVM v, T *value) {
	if (value) {
		if (pushObject(v, value)) {
			return;
		}
		Sqrat::Var<T*>::push(v, value);
		return;
	}
	sq_pushnull(v);
}

// 値の格納その他用
template<typename T>
void pushOtherValue(HSQUIRRELVM v, T *value) {
	if (value) {
		Sqrat::Var<T*>::push(v, value);
		return;
	}
	sq_pushnull(v);
}

// 値の取得
template<typename T>
SQRESULT getValue(HSQUIRRELVM v, T **value, int idx=-1) {
	*value = Sqrat::Var<T*>(v, idx).value;
	return SQ_OK;
}
#else

// 値の格納
// 格納失敗したときはオブジェクトが削除されるので delete の必要はない
// ※元が squirrel 側で生成されたオブジェクトだった場合は必ず成功する
template<typename T>
void pushValue(HSQUIRRELVM v, T *value) {
	if (value) {
		if (pushObject(v, value)) {
			SQClassType<T>::pushInstance(v, value);
			return;
		}
	}
	sq_pushnull(v);
}

// 値の取得
template<typename T>
SQRESULT getValue(HSQUIRRELVM v, T **value, int idx=-1) {
	*value = SQClassType<T>::getInstance(v, idx);
	return SQ_OK;
}
#endif

// 値の強制初期化
template<typename T>
void clearValue(T **value) {
	*value = 0;
}

// 値の取得
template<typename T>
SQRESULT getResultValue(HSQUIRRELVM v, T **value) {
	return getValue(value);
}


// ---------------------------------------------------------
// StackValue
// ---------------------------------------------------------

/**
 * スタック参照用
 */
class StackValue {
	
public:
  // コンストラクタ
  StackValue(HSQUIRRELVM v, int idx) : v(v), idx(idx) {};
  
  // 任意型へのキャスト
  // 取得できなかった場合はクリア値になる
  template<typename T>
  operator T() const
  {
	T value;
	if (SQ_FAILED(getValue(v, &value, idx))) {
	  clearValue(&value);
	}
	return value;
  }

  // オブジェクトをPUSH
  void push(HSQUIRRELVM v) const {
	  sq_move(v, this->v, idx);
  }

  // 型を返す
  SQObjectType type() const {
	return sq_gettype(v, idx);
  }

  // int値
  SQInteger intValue() const {
	return (SQInteger)*this;
  }

  // flaot値
  SQFloat floatValue() const {
	return (SQFloat)*this;
  }

private:
  HSQUIRRELVM v;
  int idx;
};

void pushValue(HSQUIRRELVM v, const StackValue &sv);

// --------------------------------------------------------------------------------------

/**
 * 引数処理用情報
 */
class StackInfo {

public:
	/**
	 * コンストラクタ
	 * @param vm VM
	 */
	StackInfo(HSQUIRRELVM vm) : vm(vm) {
		argc = sq_gettop(vm) - 1;
	}
	
	/**
	 * @return 引数の数
	 */
	int len() const {
		return argc;
	}

	/**
	 * @return self参照
	 */
	ObjectInfo getSelf() const {
		return ObjectInfo(vm, 1);
	}

	/**
	 * @param n 引数番号 0～
	 * @return 引数の型
	 */
	SQObjectType getType(int n) const {
		if (n < argc) {
			return sq_gettype(vm, n+2);
		}
		return OT_NULL;
	}
	
	/**
	 * @param n 引数番号 0～
	 * @return 引数の型
	 */
	ObjectInfo getArg(int n) const {
		ObjectInfo ret;
		if (n < argc) {
			ret.getStack(vm, n+2);
		}
		return ret;
	}

	/**
	 * 引数取得
	 * @param n 引数番号 0～
	 * @return 引数オブジェクト
	 */
	ObjectInfo operator[](int n) const {
		return getArg(n);
	}

	// 結果登録
	SQRESULT setReturn() const { return 0; }
	template<typename T>
	SQRESULT setReturn(T value) const {
		pushValue(vm, value);
		return 1;
	}

protected:
	HSQUIRRELVM vm; //< VM
	SQInteger argc; //< 引数の数
};

};// namespace

#endif
