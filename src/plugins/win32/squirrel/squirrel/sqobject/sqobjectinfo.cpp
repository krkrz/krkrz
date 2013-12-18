/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#include "sqobjectinfo.h"
#include <string.h>
#include <sqstdstring.h>

namespace sqobject {

// roottable の取得
ObjectInfo
ObjectInfo::getRoot()
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushroottable(gv); // root
	ObjectInfo ret(gv, -1);
	sq_pop(gv, 1);
	return ret;
}

ObjectInfo
ObjectInfo::createArray(SQInteger size)
{
	ObjectInfo ret;
	ret.initArray(size);
	return ret;
}

ObjectInfo
ObjectInfo::createTable()
{
	ObjectInfo ret;
	ret.initTable();
	return ret;
}

// 内容消去
void
ObjectInfo::clear()
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_release(gv,&obj);
	sq_resetobject(&obj);
}

// スタックから取得
void
ObjectInfo::getStack(HSQUIRRELVM v, SQInteger idx)
{
	clear();
	HSQUIRRELVM gv = getGlobalVM();
	sq_move(gv, v, idx);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 1);
}

// スタックから弱参照として取得
void
ObjectInfo::getStackWeak(HSQUIRRELVM v, SQInteger idx)
{
	clear();
	HSQUIRRELVM gv = getGlobalVM();
	sq_weakref(v, idx);
	sq_move(gv, v, -1);
	sq_pop(v, 1);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 1);
}

// オブジェクトを弱参照として取得
void
ObjectInfo::getWeak(const ObjectInfo &src)
{
	clear();
	HSQUIRRELVM gv = getGlobalVM();
	src.push(gv);
	sq_weakref(gv, -1);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 2);
}

// コンストラクタ
ObjectInfo::ObjectInfo() {
	sq_resetobject(&obj);
}

// コンストラクタ
ObjectInfo::ObjectInfo(HSQOBJECT obj) : obj(obj)
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_addref(gv, &obj);
}

// コンストラクタ
ObjectInfo::ObjectInfo(HSQUIRRELVM v, SQInteger idx)
{
	sq_resetobject(&obj);
	HSQUIRRELVM gv = getGlobalVM();
	sq_move(gv, v, idx);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 1);
}

// コピーコンストラクタ
ObjectInfo::ObjectInfo(const ObjectInfo &orig)
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_resetobject(&obj);
	obj = orig.obj;
	sq_addref(gv, &obj);
}

// 代入
ObjectInfo& ObjectInfo::operator =(const ObjectInfo &orig)
{
	HSQUIRRELVM gv = getGlobalVM();
	clear();
	obj = orig.obj;
	sq_addref(gv, &obj);
	return *this;
}

// デストラクタ
ObjectInfo::~ObjectInfo()
{
	clear();
}

// 同じスレッドか？
bool
ObjectInfo::isSameThread(const HSQUIRRELVM v) const
{
	return sq_isthread(obj) && obj._unVal.pThread == v;
}

// スレッドを取得
ObjectInfo::operator HSQUIRRELVM() const
{
	HSQUIRRELVM vm = sq_isthread(obj) ? obj._unVal.pThread : NULL;
	return vm;
}
	
// オブジェクトをPUSH
void
ObjectInfo::push(HSQUIRRELVM v) const
{
    if (sq_isweakref(obj)) {
		sq_pushobject(v, obj);
		sq_getweakrefval(v, -1);
		sq_remove(v, -2);
	} else {
		sq_pushobject(v, obj);
	}
}

/// 複製を登録(失敗したらnullを登録
void
ObjectInfo::pushClone(HSQUIRRELVM v) const
{
	push(v);
	if (!SQ_SUCCEEDED(sq_clone(v, -1))) {
		sq_pushnull(v);
	}
	sq_remove(v, -2);
}

// ---------------------------------------------------
// delegate 処理用
// ---------------------------------------------------

// delegate として機能するかどうか
bool
ObjectInfo::isDelegate() const
{
	SQObjectType t = type();
	return t == OT_INSTANCE || t == OT_TABLE;
}

// bindenv させるかどうか
bool
ObjectInfo::isBindDelegate() const
{
	return type() == OT_INSTANCE;
}

// ---------------------------------------------------
// データ取得
// ---------------------------------------------------

const SQChar *
ObjectInfo::getString()
{
	if (sq_isstring(obj)) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		const SQChar *mystr;
		sq_getstring(gv, -1, &mystr);
		sq_pop(gv, 1);
		return mystr;
	}
	return NULL;
}

// ---------------------------------------------------
// 配列処理用メソッド
// ---------------------------------------------------


/// 配列として初期化
void
ObjectInfo::initArray(SQInteger size)
{
	clear();
	HSQUIRRELVM gv = getGlobalVM();
	sq_newarray(gv, size);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 1);
}

/// 配列として初期化
void
ObjectInfo::initTable()
{
	clear();
	HSQUIRRELVM gv = getGlobalVM();
	sq_newtable(gv);
	sq_getstackobj(gv, -1, &obj);
	sq_addref(gv, &obj);
	sq_pop(gv, 1);
}

/// 配列に値を追加
SQRESULT ObjectInfo::append(HSQUIRRELVM v, SQInteger idx)
{
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	sq_move(gv, v, idx);
	SQRESULT ret = sq_arrayappend(gv, -2);
	sq_pop(gv,1);
	return ret;
}

/// 配列に配列を追加
SQRESULT ObjectInfo::appendArray(ObjectInfo &array)
{
	SQRESULT result = SQ_OK;
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	SQInteger max = array.len();
	for (SQInteger i=0;i<max;i++) {
		array.pushData(gv, i);
		if (SQ_FAILED(result = sq_arrayappend(gv, -2))) {
			break;
		}
	}
	sq_pop(gv,1);
	return result;
}

/// 配列の長さ
SQInteger
ObjectInfo::len() const
{
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	SQInteger ret = sq_getsize(gv,-1);
	sq_pop(gv,1);
	return ret;
}

/**
 * 配列の内容を全部PUSH
 * @param v squirrelVM
 * @return push した数
 */
SQInteger
ObjectInfo::pushArray(HSQUIRRELVM v) const
{
	if (!isArray()) {
		return 0;
	}
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	SQInteger len = sq_getsize(gv,-1);
	for (SQInteger i=0;i<len;i++) {
		sq_pushinteger(gv, i);
		if (SQ_SUCCEEDED(sq_get(gv, -2))) {
			sq_move(v, gv, -1);
			sq_pop(gv, 1);
		}
	}
	sq_pop(gv,1);
	return len;
}

// ---------------------------------------------------
// 関数処理用メソッド
// ---------------------------------------------------

SQRESULT ObjectInfo::call(ObjectInfo *self)
{
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	if (self) {
		self->push(gv);
	} else {
		sq_pushroottable(gv); // root
	}
	SQRESULT ret = sq_call(gv, 1, SQFalse, SQTrue);
	sq_pop(gv, 1);
	return ret;
}

// -------------------------------------------------------------
// クラス処理用
// -------------------------------------------------------------

bool
ObjectInfo::isClass() const
{
	return type() == OT_CLASS;
}

// ---------------------------------------------------
// 文字列処理他
// ---------------------------------------------------

sqstring
ObjectInfo::toString() const
{
	sqstring ret;
	HSQUIRRELVM gv = getGlobalVM();
	push(gv);
	sq_tostring(gv, -1);
	const SQChar *str;
	if (SQ_SUCCEEDED(sq_getstring(gv, -1, &str))) {
		ret = str;
	}
	sq_pop(gv, 2);
	return ret;
}

// 値の push
void pushValue(HSQUIRRELVM v, bool value) { sq_pushbool(v,value ? SQTrue : SQFalse); }
void pushValue(HSQUIRRELVM v, SQInteger value) { sq_pushinteger(v,value); }
void pushValue(HSQUIRRELVM v, SQFloat value) { sq_pushfloat(v,value); }
void pushValue(HSQUIRRELVM v, const SQChar *value) { if (value) {sq_pushstring(v,value,-1);} else { sq_pushnull(v);} }
void pushValue(HSQUIRRELVM v, SQUserPointer value) { if (value) {sq_pushuserpointer(v,value);} else { sq_pushnull(v);} }
void pushValue(HSQUIRRELVM v, const ObjectInfo &obj) { obj.push(v); }
void pushValue(HSQUIRRELVM v, const sqstring &value) { sq_pushstring(v,value.c_str(),value.length()); }
void pushValue(HSQUIRRELVM v, SQFUNCTION func) { sq_newclosure(v, func, 0); }
void pushValue(HSQUIRRELVM v, HSQOBJECT obj) { sq_pushobject(v, obj); }
void pushValue(HSQUIRRELVM v, const ObjectInfo::ObjectInfoReference &obj) { obj.pushData(v); }

// 値の取得
SQRESULT getValue(HSQUIRRELVM v, bool *value, int idx) { SQBool b;SQRESULT ret = sq_getbool(v, idx, &b); *value = b != SQFalse; return ret; }
SQRESULT getValue(HSQUIRRELVM v, SQInteger *value, int idx) { return sq_getinteger(v, idx, value); }
SQRESULT getValue(HSQUIRRELVM v, SQFloat *value, int idx) { return sq_getfloat(v, idx, value); }
SQRESULT getValue(HSQUIRRELVM v, const SQChar **value, int idx) { return sq_getstring(v, idx, value); }
SQRESULT getValue(HSQUIRRELVM v, SQUserPointer *value, int idx) { return sq_getuserpointer(v, idx, value); }
SQRESULT getValue(HSQUIRRELVM v, ObjectInfo *value, int idx) { value->getStack(v,idx); return SQ_OK; }
SQRESULT getValue(HSQUIRRELVM v, sqstring *value, int idx) {const SQChar *str; SQRESULT ret; if (SQ_SUCCEEDED((ret = sq_getstring(v, idx, &str)))) { *value = str;} return ret;}

// 値の強制初期化
void clearValue(bool *value) { *value = false; }
void clearValue(SQInteger *value) { *value = 0; }
void clearValue(SQFloat *value) { *value = 0.0f; }
void clearValue(const SQChar **value) { *value = 0; }
void clearValue(SQUserPointer *value) { *value = 0; }
void clearValue(ObjectInfo *value) { value->clear(); }
void clearValue(sqstring *value) { *value = _SC(""); }

// 値の取得：基本↑のコピペ。文字列は安全でない場合があるので排除する必要あり
SQRESULT getResultValue(HSQUIRRELVM v, bool *value) { SQBool b;SQRESULT ret = sq_getbool(v, -1, &b); *value = b != SQFalse; return ret; }
SQRESULT getResultValue(HSQUIRRELVM v, SQInteger *value) { return sq_getinteger(v, -1, value); }
SQRESULT getResultValue(HSQUIRRELVM v, SQFloat *value) { return sq_getfloat(v, -1, value); }
SQRESULT getResultValue(HSQUIRRELVM v, SQUserPointer *value) { return sq_getuserpointer(v, -1, value); }
SQRESULT getResultValue(HSQUIRRELVM v, ObjectInfo *value) { value->getStack(v,-1); return SQ_OK; }
SQRESULT getResultValue(HSQUIRRELVM v, sqstring *value) {const SQChar *str; SQRESULT ret; if (SQ_SUCCEEDED((ret = sq_getstring(v, -1, &str)))) { *value = str;} return ret;}

};
