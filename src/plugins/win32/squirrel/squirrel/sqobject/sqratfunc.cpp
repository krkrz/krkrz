/**
 * sqrat 版の sqobject 実装
 *
 * sqrat を使った Object, Thread 登録処理の実装例です。
 * sqrat の機能をつかって継承を処理しています。
 */
#include "sqratfunc.h"
#include "sqobjectclass.h"
#include "sqthread.h"

#include <sqstdstring.h>
#include <sqstdmath.h>
#include <sqstdaux.h>

using namespace sqobject;

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
	sq_close(vm);
}

// デストラクタ登録用
static SQRESULT destructor(HSQUIRRELVM v) {
	return SQ_OK;
}

/**
 * Object クラスの登録
 */
void
Object::registerClass()
{
	Sqrat::Class<Object, sqobject::VMConstructor<Object> > cls(vm, (SQUserPointer)SQOBJECTNAME);
	cls.SquirrelFunc(_SC("destructor"), ::destructor);
	// sqrat の set/get を上書きして sqobject 機能と整合をとる
	sqobject::OverrideSetGet<Object>::Func(vm);
	Sqrat::RootTable(vm).Bind(SQOBJECTNAME, cls);

	SQFUNC(Object,notify);
	SQFUNC(Object,notifyAll);
	SQVFUNC(Object,hasSetProp);
	SQVFUNC(Object,setDelegate);
	SQVFUNC(Object,getDelegate);
};

/**
 * Thread クラスの登録
 */
void
Thread::registerClass()
{
	SQCLASSOBJ_VCONSTRUCTOR(Thread,SQTHREADNAME);
	SQVFUNC(Thread,exec);
	SQVFUNC(Thread,exit);
	SQFUNC(Thread,stop);
	SQFUNC(Thread,run);
	SQFUNC(Thread,getCurrentTick);
	SQFUNC(Thread,getStatus);
	SQVFUNC(Thread,getExitCode);
	SQVFUNC(Thread,wait);
	SQFUNC(Thread,cancelWait);
};

}
