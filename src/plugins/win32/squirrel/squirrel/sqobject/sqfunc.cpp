/**
 * 一般的な sqobject 実装
 * 
 * Object, Thread の登録処理の実装例です。
 * 継承情報は単純リスト管理してます
 */
#include "sqobjectinfo.h"
#include "sqobjectclass.h"
#include "sqthread.h"

#include <string.h>
#include <sqstdstring.h>
#include <sqstdmath.h>
#include <sqstdaux.h>

SQRESULT ERROR_CREATE(HSQUIRRELVM v) {
	return sq_throwerror(v, _SC("can't create native instance"));
}

SQRESULT ERROR_BADINSTANCE(HSQUIRRELVM v) {
	return sq_throwerror(v, _SC("bad instance"));
}

SQRESULT ERROR_BADMETHOD(HSQUIRRELVM v) {
	return sq_throwerror(v, _SC("bad method"));
}

namespace sqobject {

// global vm
HSQUIRRELVM vm;

/// vm 初期化
HSQUIRRELVM init() {
	vm = sq_open(1024);
	sq_pushroottable(vm);
	sqstd_register_mathlib(vm);
	sqstd_register_stringlib(vm);
	sqstd_seterrorhandlers(vm);
	sq_pop(vm,1);
	return vm;
}

/// 情報保持用グローバルVMの取得
HSQUIRRELVM getGlobalVM()
{
	return vm;
}

/// vm 終了
void done()
{
	// クラス参照を解放
	SQClassType<Thread>::done(vm);
	SQClassType<Object>::done(vm);
	
	// ルートテーブルをクリア
	sq_pushroottable(vm);
	sq_clear(vm,-1);
	sq_pop(vm,1);
	sq_close(vm);
}

// ------------------------------------------------------------------
// クラス登録用マクロ
// ------------------------------------------------------------------

#ifndef USE_SQOBJECT_TEMPLATE

static SQRESULT Object_notify(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		instance->notify();
		return SQ_OK;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_notifyAll(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		instance->notifyAll();
		return SQ_OK;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_hasSetProp(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		return instance->hasSetProp(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_setDelegate(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		return instance->setDelegate(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_getDelegate(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		return instance->getDelegate(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_get(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		return instance->_get(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Object_set(HSQUIRRELVM v)
{
	Object *instance = SQClassType<Object>::getInstance(v);
	if (instance) {
		return instance->_set(v);
	}
	return ERROR_BADINSTANCE(v);
}

#endif

/**
 * クラスの登録
 * @param v squirrel VM
 */
void
Object::registerClass()
{
	SQCLASS(Object,BaseClass,SQOBJECTNAME);
	SQVCONSTRUCTOR();
	
#ifdef USE_SQOBJECT_TEMPLATE
	SQFUNC(Object,notify);
	SQFUNC(Object,notifyAll);
	SQVFUNC(Object,hasSetProp);
	SQVFUNC(Object,setDelegate);
	SQVFUNC(Object,getDelegate);
	SQVFUNC(Object,_get);
	SQVFUNC(Object,_set);
	cls.RegisterV(&Object::_get, _SC("get"));
	cls.RegisterV(&Object::_set, _SC("set"));
#else
	SQNFUNC(Object,notify);
	SQNFUNC(Object,notifyAll);
	SQNFUNC(Object,hasSetProp);
	SQNFUNC(Object,setDelegate);
	SQNFUNC(Object,getDelegate);
	cls.Register(Object_get, _SC("_get"));
	cls.Register(Object_set, _SC("_set"));
	SQNFUNC(Object,get);
	SQNFUNC(Object,set);
#endif
};

#ifndef USE_SQOBJECT_TEMPLATE

static SQRESULT Thread_exec(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		return instance->exec(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_exit(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		return instance->exit(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_stop(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		instance->stop();
		return SQ_OK;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_run(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		instance->run();
		return SQ_OK;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_getCurrentTick(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		sq_pushinteger(v,instance->getCurrentTick());
		return 1;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_getStatus(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		sq_pushinteger(v,instance->getStatus());
		return 1;
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_getExitCode(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		return instance->getExitCode(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_wait(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		return instance->wait(v);
	}
	return ERROR_BADINSTANCE(v);
}

static SQRESULT Thread_cancelWait(HSQUIRRELVM v)
{
	Thread *instance = SQClassType<Thread>::getInstance(v);
	if (instance) {
		instance->cancelWait();
		return SQ_OK;
	}
	return ERROR_BADINSTANCE(v);
}

#endif

/**
 * クラスの登録
 * @param v squirrel VM
 */
void
Thread::registerClass()
{
	SQCLASS(Thread, Object,SQTHREADNAME);
	SQVCONSTRUCTOR();

#ifdef USE_SQOBJECT_TEMPLATE
	SQVFUNC(Thread,exec);
	SQVFUNC(Thread,exit);
	SQFUNC(Thread,stop);
	SQFUNC(Thread,run);
	SQFUNC(Thread,getCurrentTick);
	SQFUNC(Thread,getStatus);
	SQVFUNC(Thread,getExitCode);
	SQVFUNC(Thread,wait);
	SQFUNC(Thread,cancelWait);
#else
	SQNFUNC(Thread,exec);
	SQNFUNC(Thread,exit);
	SQNFUNC(Thread,stop);
	SQNFUNC(Thread,run);
	SQNFUNC(Thread,getCurrentTick);
	SQNFUNC(Thread,getStatus);
	SQNFUNC(Thread,getExitCode);
	SQNFUNC(Thread,wait);
	SQNFUNC(Thread,cancelWait);
#endif
};

}

