/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#include "sqthread.h"
#include <string.h>
#include <ctype.h>

namespace sqobject {

/**
 * クロージャかどうか
 */
static bool
isClosure(SQObjectType type)
{
	return type == OT_CLOSURE || type == OT_NATIVECLOSURE;
}


/**
 * 文字列取得用
 * @param v VM
 * @param idx インデックス
 * @return 文字列
 */
const SQChar *getString(HSQUIRRELVM v, SQInteger idx) {
	const SQChar *x = NULL;
	sq_getstring(v, idx, &x);
	return x;
};

// setter名前決定
static void pushSetterName(HSQUIRRELVM v, const SQChar *name)
{
	int len = sizeof(SQChar) * (scstrlen(name) + 4);
	SQChar *buf = (SQChar*)sq_malloc(len);
	SQChar *p = buf;
	*p++ = 's';
	*p++ = 'e';
	*p++ = 't';
	*p++ = toupper(*name++);
	while (*name) { *p++ = *name++; }
	*p++ = '\0';
	sq_pushstring(v, buf, -1);
	sq_free(buf, len);
}

// getter名前決定
static void pushGetterName(HSQUIRRELVM v, const SQChar *name)
{
	int len = sizeof(SQChar) * (scstrlen(name) + 4);
	SQChar *buf = (SQChar*)sq_malloc(len);
	SQChar *p = buf;
	*p++ = 'g';
	*p++ = 'e';
	*p++ = 't';
	*p++ = toupper(*name++);
	while (*name) { *p++ = *name++; };
	*p++ = '\0';
	sq_pushstring(v, buf, -1);
	sq_free(buf, len);
}

// ---------------------------------------------------------
// Object
// ---------------------------------------------------------

/**
 * オブジェクト待ちの登録
 * @param thread スレッド
 */
void
Object::addWait(ObjectInfo &thread)
{
	_waitThreadList.append(thread);
}

/**
 * オブジェクト待ちの解除
 * @param thread スレッド
 */
void
Object::removeWait(ObjectInfo &thread)
{
	_waitThreadList.removeValue(thread, true);
}

/**
 * コンストラクタ
 */
Object::Object()
{
	_waitThreadList.initArray();
}

/**
 * コンストラクタ
 */
Object::Object(HSQUIRRELVM v, int delegateIdx)
{
	self.getStackWeak(v,1);
	_waitThreadList.initArray();
	if (sq_gettop(v) >= delegateIdx) {
		delegate.getStackWeak(v, delegateIdx);
	}
}

/**
 * デストラクタ
 */
Object::~Object()
{
	notifyAll();
	delegate.clear();
	_waitThreadList.clear();
	self.clear();
}

/**
 * 自己参照初期化用
 * オブジェクト生成後必ずこの処理をよぶこと
 * @param v SQUIRREL vm
 * @param idx 自分のオブジェクトがあるインデックス
 */
void
Object::initSelf(HSQUIRRELVM v, int idx)
{
	if (sq_gettop(v) >= idx) {
		self.getStackWeak(v,idx);
	}
}

/**
 * 終了時処理
 */
void
Object::destructor()
{
	callEvent(_SC("destructor"));
}

/**
 * このオブジェクトを待っている１スレッドの待ちを解除
 */
void
Object::notify()
{
	SQInteger max = _waitThreadList.len();
	for (int i=0;i<max;i++) {
		Thread *th = _waitThreadList.get(i);
		if (th && th->notifyObject(self)) {
			_waitThreadList.remove(i);
			return;
		}
	}
}
	
/**
 * このオブジェクトを待っている全スレッドの待ちを解除
 */
void
Object::notifyAll()
{
	SQInteger max = _waitThreadList.len();
	for (int i=0;i<max;i++) {
		Thread *th = _waitThreadList.get(i);
		if (th) {
			th->notifyObject(self);
		}
	}
	_waitThreadList.clearData();
}

/**
 * プロパティから値を取得
 * @param name プロパティ名
 * @return プロパティ値
 */
SQRESULT
Object::_get(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	const SQChar *name = getString(v, 2);
	if (name && *name) {
		// delegateの参照
		if (delegate.isDelegate()) {
			delegate.push(v);
			sq_pushstring(v, name, -1);
			if (SQ_SUCCEEDED(result = sq_get(v,-2))) {
				// メソッドの場合は束縛処理
				if (isClosure(sq_gettype(v,-1)) && delegate.isBindDelegate()) {
					delegate.push(v);
					if (SQ_SUCCEEDED(sq_bindenv(v, -2))) {
						sq_remove(v, -2); // 元のクロージャ
					}
				}
				sq_remove(v, -2);
				return 1;
			} else {
				sq_pop(v,1); // delegate
			}
		}
		
		// getter を探してアクセス
		sq_push(v, 1); // self
		pushGetterName(v,name); // getter名
		if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
			sq_push(v, 1); //  self;
			if (SQ_SUCCEEDED(result = sq_call(v,1,SQTrue,SQTrue))) {
				//sqprintf("呼び出し成功:%s\n", name);
				sq_remove(v, -2); // func
				sq_remove(v, -2); // self
				return 1;
			} else {
				sq_pop(v,2); // func, self
			}
		} else {
			sq_pop(v, 1); // self
#if 0
			// グローバル変数を参照
			sq_pushroottable(v);
			sq_pushstring(v, name, -1);
			if (SQ_SUCCEEDED(sq_rawget(v,-2))) {
				sq_remove(v, -2); // root
				return 1;
			} else {
				sq_pop(v,1);
			}
#endif
		}
	}
	return SQ_ERROR;
}

/**
 * プロパティに値を設定
 * @param name プロパティ名
 * @param value プロパティ値
 */
SQRESULT
Object::_set(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	const SQChar *name = getString(v, 2);
	if (name && *name) {
		// delegateの参照
		if (delegate.isDelegate()) {
			delegate.push(v);
			sq_push(v, 2); // name
			sq_push(v, 3); // value
			if (SQ_SUCCEEDED(result = sq_set(v,-3))) {
				sq_pop(v,1); // delegate
				return SQ_OK;
			} else {
				sq_pop(v,1); // delegate
			}
		}
		
		// setter を探してアクセス
		sq_push(v, 1); // self
		pushSetterName(v, name);
		if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
			sq_push(v, 1); // self
			sq_push(v, 3); // value
			if (SQ_SUCCEEDED(result = sq_call(v,2,SQFalse,SQTrue))) {
				//sqprintf("呼び出し成功:%s\n", name);
				sq_pop(v,2); // func, self
				return SQ_OK;
			} else {
				sq_pop(v,2); // func, self
			}
		}
		
	}
	//return result;
	return SQ_ERROR;
}

/**
 * setプロパティの存在確認
 * @param name プロパティ名
 * @return setプロパティが存在したら true
 */
SQRESULT
Object::hasSetProp(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	SQBool ret = SQFalse;
	if (sq_gettop(v) > 1) {
		const SQChar *name = getString(v, 2);
		if (name && *name) {
			sq_push(v, 1); // object
			pushSetterName(v, name); // setter名
			if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
				sq_pop(v,1);
				ret = SQTrue;
			} else {
				sq_pushstring(v, name, -1);
				if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
					sq_pop(v,1);
					ret = SQTrue;
				}
			}
			sq_pop(v,1); // object
		}
	}
	if (SQ_SUCCEEDED(result)) {
		sq_pushbool(v, ret);
		return 1;
	} else {
		return result;
	}
}

/**
 * 委譲の設定
 */
SQRESULT
Object::setDelegate(HSQUIRRELVM v)
{
	if (sq_gettop(v) > 1) {
		delegate.getStackWeak(v,2);
	} else {
		delegate.clear();
	}
	return SQ_OK;
}

/**
 * 委譲の設定
 */
SQRESULT
Object::getDelegate(HSQUIRRELVM v)
{
	delegate.push(v);
	return 1;
}

bool
pushObject(HSQUIRRELVM v, Object *obj)
{
	if (obj->isInit()) {
		obj->push(v);
		return true;
	}
	return false;
}

void pushValue(HSQUIRRELVM v, const StackValue &value) { value.push(v); }

};

