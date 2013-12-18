#ifndef SQRATFUNC_H
#define SQRATFUNC_H

#include "sqobjectclass.h"

/**
 * sqobject::sqobject を継承したオブジェクト用に sqrat あわせでテンプレートを作成
 */

namespace sqobject {

	/**
	 * 引数なしコンストラクタ用アロケータ
	 */
	template <class C>
	class DefaultConstructor {
	public:
		static SQRESULT New(HSQUIRRELVM vm) {
			C *instance = new C();
			if (instance) {
				instance->initSelf(vm, 1);
				sq_setinstanceup(vm, 1, instance);
				sq_setreleasehook(vm, 1, &Delete);
				return SQ_OK;
			} else {
				return SQ_ERROR;
			}
		}
		
		static SQInteger Copy(HSQUIRRELVM vm, SQInteger idx, const C* value) {
			return SQ_ERROR;
		}

		static SQInteger Init(HSQUIRRELVM vm, SQInteger idx, C* instance) {
			if (instance) {
				instance->initSelf(vm, idx);
				sq_setinstanceup(vm, idx, instance);
				sq_setreleasehook(vm, idx, &Delete);
				return SQ_OK;
			} else {
				return SQ_ERROR;
			}
		}
		
		static SQRESULT Delete(SQUserPointer up, SQInteger size) {
			C* self = (C*)up;
			if (self){ 
				self->destructor();
				delete self;
			}
			return SQ_OK;
		}
	};
	
	/**
	 * HSQUIRRELVMを引数にとるコンストラクタ用のアロケータ
	 */
	template <class C>
	class VMConstructor : public DefaultConstructor<C> {
	public:
		static SQRESULT New(HSQUIRRELVM vm) {
			C *instance = new C(vm);
			if (instance) {
				instance->initSelf(vm);
				sq_setinstanceup(vm, 1, instance);
				sq_setreleasehook(vm, 1, &DefaultConstructor<C>::Delete);
				return SQ_OK;
			} else {
				return SQ_ERROR;
			}
		}
	};

	/**
	 * ファクトリー形式コンストラクタ用のアロケータ
	 */
	template <class C>
	class Factory : public DefaultConstructor<C> {
	public:
		static SQRESULT New(HSQUIRRELVM vm) {
			C *instance = NULL;
			SQRESULT ret = C::factory(vm, &instance);
			if (SQ_SUCCEEDED(ret)) {
				instance->initSelf(vm);
				sq_setinstanceup(vm, 1, instance);
				sq_setreleasehook(vm, 1, &DefaultConstructor<C>::Delete);
				return SQ_OK;
			} else {
				return ret;
			}
		}
	};

	
	/**
	 * コンストラクタなしオブジェクト用アロケータ。newすると例外
	 */
	template <class C>
	class NOConstructor : public DefaultConstructor<C> {
	public:
		static SQRESULT New(HSQUIRRELVM v) {
			return sq_throwerror(v, "can't create instance");
		}
	};

	// ---------------------------------------------------------------------------

	/**
	 * _get/_set に割り込みする処理。
	 * sqrat のプロパティ処理が失敗した場合はインスタンスの _set/_get を
	 * 呼び出すようにする
	 */
	template<class C>
	struct OverrideSetGet {
		
		static SQInteger _get(HSQUIRRELVM vm) {
			// Find the get method in the get table
			sq_push(vm, 2);
			if (SQ_FAILED( sq_rawget(vm,-2) )) {
				SQUserPointer p = NULL;
				sq_getinstanceup(vm, 1, &p, NULL);
				C* ptr = (C*)p;
				return ptr ? ptr->_get(vm) : SQ_ERROR;
			}
			
			// push 'this'
			sq_push(vm, 1);
			
			// Call the getter
			sq_call(vm, 1, true, Sqrat::ErrorHandling::IsEnabled());
			return 1;
		}

		static SQInteger _set(HSQUIRRELVM vm) {
			// Find the set method in the set table
			sq_push(vm, 2);
			if (SQ_FAILED( sq_rawget(vm,-2) )) {
				SQUserPointer p = NULL;
				sq_getinstanceup(vm, 1, &p, NULL);
				C* ptr = (C*)p;
				return ptr ? ptr->_set(vm) : SQ_ERROR;
			}
			
			// push 'this'
			sq_push(vm, 1);
			sq_push(vm, 3);
			
			// Call the setter
			sq_call(vm, 2, false, Sqrat::ErrorHandling::IsEnabled());
			
			return 0;
		}

		static void Func(HSQUIRRELVM vm) {
			// push the class
			sq_pushobject(vm, Sqrat::ClassType<C>::ClassObject());
			
			// sqrat の機能をオーバライドする

			// override _set
			sq_pushstring(vm, _SC("_set"), -1);
			sq_pushobject(vm, Sqrat::ClassType<C>::SetTable()); // Push the set table as a free variable
			sq_newclosure(vm, _set, 1);
			sq_newslot(vm, -3, false);
			// override _get
			sq_pushstring(vm, _SC("_get"), -1);
			sq_pushobject(vm, Sqrat::ClassType<C>::GetTable()); // Push the get table as a free variable
			sq_newclosure(vm, _get, 1);
			sq_newslot(vm, -3, false);

			// _get / _set で登録したものは後から参照できないので、
			// スクリプトの継承先でオーバライドするときのため別名で登録しておく

			// override _set
			sq_pushstring(vm, _SC("set"), -1);
			sq_pushobject(vm, Sqrat::ClassType<C>::SetTable()); // Push the set table as a free variable
			sq_newclosure(vm, _set, 1);
			sq_newslot(vm, -3, false);
			// override _get
			sq_pushstring(vm, _SC("get"), -1);
			sq_pushobject(vm, Sqrat::ClassType<C>::GetTable()); // Push the get table as a free variable
			sq_newclosure(vm, _get, 1);
			sq_newslot(vm, -3, false);
			sq_pop(vm, 1);
		}
	};
	
}; // namespace

// ----------------------------------------------
// クラス登録用マクロ
// ----------------------------------------------

// 非継承クラス
#define SQCLASS_NOCONSTRUCTOR(Target, Name)\
  Sqrat::Class<Target, Sqrat::CopyOnly<Target> > cls(sqobject::getGlobalVM());\
  Sqrat::RootTable(sqobject::getGlobalVM()).Bind(Name, cls);

// Object継承クラス
#define SQCLASSEX(Target, Parent, Name)\
  Sqrat::DerivedClass<Target, Parent, sqobject::DefaultConstructor<Target> > cls(sqobject::getGlobalVM());\
  sqobject::OverrideSetGet<Target>::Func(sqobject::getGlobalVM());\
  Sqrat::RootTable(sqobject::getGlobalVM()).Bind(Name, cls);

// Object継承クラス・VM指定コンストラクタ
#define SQCLASSEX_VCONSTRUCTOR(Target, Parent, Name)\
  Sqrat::DerivedClass<Target, Parent, sqobject::VMConstructor<Target> > cls(sqobject::getGlobalVM());\
  sqobject::OverrideSetGet<Target>::Func(sqobject::getGlobalVM());\
  Sqrat::RootTable(sqobject::getGlobalVM()).Bind(Name, cls);

// Object継承クラス・ファクトリ形式
#define SQCLASSEX_FACTORY(Target, Parent, Name)\
  Sqrat::DerivedClass<Target, Parent, sqobject::Factory<Target> > cls(sqobject::getGlobalVM());\
  sqobject::OverrideSetGet<Target>::Func(sqobject::getGlobalVM());\
  Sqrat::RootTable(sqobject::getGlobalVM()).Bind(Name, cls);

// Object継承クラス・コンストラクタ無し
#define SQCLASSEX_NOCONSTRUCTOR(Target, Parent, Name)\
  Sqrat::DerivedClass<Target, Parent, sqobject::NOConstructor<Target> > cls(sqobject::getGlobalVM());\
  sqobject::OverrideSetGet<Target>::Func(sqobject::getGlobalVM());\
  Sqrat::RootTable(sqobject::getGlobalVM()).Bind(Name, cls);

// Object継承クラス
#define SQCLASSOBJ(Target, Name) SQCLASSEX(Target, Object, Name)
#define SQCLASSOBJ_VCONSTRUCTOR(Target, Name) SQCLASSEX_VCONSTRUCTOR(Target, Object, Name)
#define SQCLASSOBJ_FACTORY(Target, Name) SQCLASSEX_FACTORY(Target, Object, Name)
#define SQCLASSOBJ_NOCONSTRUCTOR(Target, Name) SQCLASSEX_NOCONSTRUCTOR(Target, Object, Name)

// ----------------------------------------------
// ファンクション登録用マクロ
// ----------------------------------------------

#define SQFUNC(Class, Name)  cls.Func(_SC(#Name), &Class::Name)
#define SQFUNCNAME(Class, Method, Name)  cls.Func(_SC(#Name), &Class::Method)
#define SQSFUNC(Class, Name) cls.StaticFunc(_SC(#Name), &Class::Name)
#define SQSFUNCNAME(Class, Method, Name) cls.StaticFunc(_SC(#Name), &Class::Method)
#define SQVFUNC(Class, Name) cls.VarArgFunc(_SC(#Name), &Class::Name)
#define SQVFUNCNAME(Class, Method, Name) cls.VarArgFunc(_SC(#Name), &Class::Method)
#define SQSVFUNC(Class, Name) cls.SquirrelFunc(_SC(#Name), &Class::Name, true)
#define SQSVFUNCNAME(Class, Method, Name) cls.SquirrelFunc(_SC(#Name), &Class::Method, true)

#endif
