#include <windows.h>
#include "tp_stub.h"
#include "sqtjsobj.h"

// sqwrapper.cpp より
extern void sq_pushvariant(HSQUIRRELVM v, tTJSVariant &variant);
extern SQRESULT sq_getvariant(HSQUIRRELVM v, int idx, tTJSVariant *result);
extern SQRESULT ERROR_KRKR(HSQUIRRELVM v, tjs_error error);
extern void SQEXCEPTION(HSQUIRRELVM v);

#include <sqfunc.h>

// 型情報
static const SQChar *tjsClassAttrName = _SC("tjsClass");

/**
 * メンバ登録処理用
 */
class MemberRegister : public tTJSDispatch /** EnumMembers 用 */
{

public:
	// コンストラクタ
	MemberRegister(HSQUIRRELVM v, tTJSVariant &tjsClassObj) : v(v), tjsClassObj(tjsClassObj) {
	};

	// EnumMember用繰り返し実行部
	// param[0] メンバ名
	// param[1] フラグ
	// param[2] メンバの値
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			if (param[2]->Type() == tvtObject) {
				const tjs_char *name = param[0]->GetString();
				tTVInteger flag = param[1]->AsInteger();
				bool staticMember = (flag & TJS_STATICMEMBER) != 0;
				iTJSDispatch2 *member = param[2]->AsObjectNoAddRef();
				if (member) {
					if (TJS_SUCCEEDED(member->IsInstanceOf(0,NULL,NULL,L"Function",NULL))) {
						registerFunction(name, *param[2], staticMember);
					} else if (TJS_SUCCEEDED(member->IsInstanceOf(0,NULL,NULL,L"Property",NULL))) {
						registerProperty(name, *param[2], staticMember);
					}
				} else {
					registNull(name);
				}
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}

protected:

	// null 登録
	void registNull(const tjs_char *functionName) {
		sq_pushstring(v, functionName, -1);
		sq_pushnull(v);
		sq_createslot(v, -3);
	}

	// ファンクション登録
	void registerFunction(const tjs_char *functionName, tTJSVariant &function, bool staticMember) {
		sq_pushstring(v, functionName, -1);
		sq_pushvariant(v, function);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticInvoker, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsInvoker, 1);
			sq_newslot(v, -3, SQFalse);
		}
	}

	// プロパティ登録
	void registerProperty(const tjs_char *propertyName, tTJSVariant &property, bool staticMember) {

		ttstr name = L"set";
		name += toupper(*propertyName);
		name += (propertyName + 1);
		sq_pushstring(v, name.c_str(), -1);
		sq_pushvariant(v, property);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticSetter, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsSetter, 1);
			sq_newslot(v, -3, SQFalse);
		}

		name = L"get";
		name += toupper(*propertyName);
		name += (propertyName + 1);
		sq_pushstring(v, name.c_str(), -1);
		sq_pushvariant(v, property);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticGetter, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsGetter, 1);
			sq_newslot(v, -3, SQFalse);
		}
	}
	
private:
	HSQUIRRELVM v;
	tTJSVariant &tjsClassObj;
};

// -----------------------------------------------------------------------

// TJSObject登録用のクラスID
int TJSObject::classId;

// 初期化用
void
TJSObject::init(HSQUIRRELVM vm)
{
	// squirrel インスタンスを TJS側で保持するためのIDを取得
	classId = TJSRegisterNativeClass(L"SquirrelClass");
	
	// TJSObjectクラスを定義
	SQTemplate<TJSObject, sqobject::Object> cls(vm);

	// メソッド登録
	sq_pushroottable(vm);
	sq_pushstring(vm, _SC("createTJSClass"), -1);
	sq_newclosure(vm, createTJSClass, 0);
	sq_createslot(vm, -3);
	sq_pushstring(vm, _SC("tjsNull"), -1);
	sq_pushuserpointer(vm, NULL);
	sq_createslot(vm, -3);
	sq_pop(vm, 1);
}

// 初期化用
void
TJSObject::done(HSQUIRRELVM vm)
{
	// クラス参照を解放
	SQClassType<TJSObject>::done(vm);
}

/**
 * クラスの登録
 * @param HSQUIRRELVM v
 */
SQRESULT
TJSObject::createTJSClass(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
	if (top < 2) {
		return sq_throwerror(v, _SC("invalid param"));
	}

	// クラスを生成
	sq_pushobject(v, SQClassType<TJSObject>::ClassObject());
	sq_newclass(v, true); // 継承する

	// メンバ登録
	const tjs_char *tjsClassName = NULL;
	tTJSVariant tjsClassObj;
	for (SQInteger i=top;i>1;i--) {
		if ((tjsClassName = sqobject::getString(v,i))) {
			TVPExecuteExpression(tjsClassName, &tjsClassObj);
			if (tjsClassObj.Type() == tvtObject &&
				TJS_SUCCEEDED(tjsClassObj.AsObjectClosureNoAddRef().IsInstanceOf(0,NULL,NULL,L"Class",NULL))) {
				MemberRegister *r = new MemberRegister(v, tjsClassObj);
				tTJSVariantClosure closure(r);
				tjsClassObj.AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
				r->Release();
			}
		}
	}

	if (tjsClassName) {
		// コンストラクタ登録
		sq_pushstring(v, _SC("constructor"), -1);
		sq_pushvariant(v, tjsClassObj);
		sq_newclosure(v, tjsConstructor, 1);
		sq_createslot(v, -3);
		// クラス属性に tjsクラスを登録
		sq_pushnull(v);
		sq_newtable(v);
		if (SQ_SUCCEEDED(sq_setattributes(v,-3))) {
			sq_pop(v,1);
			sq_pushnull(v);
			if (SQ_SUCCEEDED(sq_getattributes(v, -2))) {
				sq_pushstring(v, tjsClassAttrName, -1);
				sq_pushvariant(v, tjsClassObj);
				if (SQ_SUCCEEDED(sq_createslot(v, -3))) {
					sq_pop(v,1);
				} else {
					sq_pop(v,2);
				}
			} else {
				// XXX
				sq_pop(v,1);
			}
		} else {
			// XXX
			sq_pop(v,2);
		}
		
		// TJS機能メソッドを登録
		sq_pushstring(v, _SC("tjsIsValid"), -1);
		sq_newclosure(v, TJSObject::tjsIsValid, 0);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("tjsOverride"), -1);
		sq_newclosure(v, TJSObject::tjsOverride, 0);
		sq_setparamscheck(v, -2, _SC(".sc"));
		sq_createslot(v, -3);
	}

	return 1;
}

/**
 * squirrel から吉里吉里オブジェクトを取得
 */
bool
TJSObject::getVariant(HSQUIRRELVM v, SQInteger idx, tTJSVariant *variant)
{
	if (sq_gettype(v, idx) == OT_CLASS) {
		if (idx < 0) {
			idx = sq_gettop(v) + 1 + idx;
		}
		bool ret = false;
		// クラス属性からオブジェクト情報を引き出す
		sq_pushnull(v);
		if (SQ_SUCCEEDED(sq_getattributes(v, idx))) {
			sq_pushstring(v, tjsClassAttrName, -1);
			if (SQ_SUCCEEDED(sq_get(v,-2))) {
				if (SQ_SUCCEEDED(sq_getvariant(v,-1, variant))) {
					ret = true;
				}
				sq_pop(v,1);
			}
			sq_pop(v,1);
		} else {
			// XXX
			sq_pop(v,1);
		}
		return ret;
	} else if (sq_gettype(v, idx) == OT_INSTANCE) {
		TJSObject *obj = SQClassType<TJSObject>::getInstance(v, idx);
		if (obj && obj->instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE) {
			*variant = obj->instance;
			return true;
		}
	}
	return false;
}

/**
 * 吉里吉里オブジェクトを squirrel に登録。squirrel 側で生成されたオブジェクトの場合は元のオブジェクト情報をそのまま返す
 * @return 登録成功
 */
bool
TJSObject::pushVariant(HSQUIRRELVM v, tTJSVariant &variant)
{
	// 登録済みインスタンスかどうかの判定
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(variant.AsObjectNoAddRef()->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		// 元々 squirrel 側から登録されたオブジェクトの場合は元の squirrel オブジェクト情報をそのまま返す
		TJSObject *self = (TJSObject*)ninstance;
		self->self.push(v);
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------


//---------------------------------------------------------------------------
// missing関数
//---------------------------------------------------------------------------
class tMissingFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
									   tTJSVariant *result,
									   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		return TJSObject::missing(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * missing 処理用の口
 * TJSインスタンスにメンバが存在しなかった場合は squirrelインスタンスを参照する
 */
tjs_error TJSObject::missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
	
	if (numparams < 3) {return TJS_E_BADPARAMCOUNT;};

	// バインドされている squirrel オブジェクトを取得
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSObject *self = (TJSObject*)ninstance;

		bool ret = false;
		HSQUIRRELVM gv = sqobject::getGlobalVM();
		if (!(int)*params[0]) { // get
			self->self.push(gv); // self
			sq_pushstring(gv, params[1]->GetString(), -1); // key
			if (SQ_SUCCEEDED(sq_get(gv, -2))) {
				tTJSVariant value;
				if (SQ_SUCCEEDED(sq_getvariant(gv, -1, &value))) {
					params[2]->AsObjectClosureNoAddRef().PropSet(0, NULL, NULL, &value, NULL);
					ret = true;
				}
				sq_pop(gv, 1); // value
			}
			sq_pop(gv, 1); // self
		} else { // set
			self->self.push(gv); // self
			sq_pushstring(gv, params[1]->GetString(), -1); // key
			sq_pushvariant(gv, *params[2]);    // value
			if (SQ_SUCCEEDED(sq_set(gv, -3))) {
				ret = true;
			}
			sq_pop(gv,1); // self
		}
		if (result) {
			*result = ret;
		}
	}
	return TJS_E_NATIVECLASSCRASH;
}

//---------------------------------------------------------------------------
// callSQ関数
//---------------------------------------------------------------------------
class tCallSQFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
									   tTJSVariant *result,
									   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		return TJSObject::call(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * call 処理用の口
 * TJSインスタンスからsquirrelインスタンスのメソッドを直接呼び出す
 */
tjs_error
TJSObject::call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
				tTJSVariant *result,
				tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 1) {return TJS_E_BADPARAMCOUNT;};
	// バインドされている squirrel オブジェクトを取得
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSObject *self = (TJSObject*)ninstance;
		HSQUIRRELVM gv = sqobject::getGlobalVM();
		self->self.push(gv); // this
		sq_pushstring(gv, param[0]->GetString(), -1); // methodName
		if (SQ_SUCCEEDED(sq_get(gv, -2))) {
			self->self.push(gv); // this
			for (int i=1;i<numparams;i++) {	// 引数
				sq_pushvariant(gv, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(gv, numparams, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(gv, -1, result);
					sq_pop(gv, 1); // result
				}
				sq_pop(gv, 2); // func, this
				return TJS_S_OK;
			} else {
				sq_pop(gv, 2); // func, this
				SQEXCEPTION(gv);
			}
		} else {
			sq_pop(gv, 1); // this
			SQEXCEPTION(gv);
		}
	}
	return TJS_E_NATIVECLASSCRASH;
}


/**
 * コンストラクタ
 */
TJSObject::TJSObject(HSQUIRRELVM v, int idx, tTJSVariant &instance) : instance(instance)
{
	initSelf(v, idx);
	iTJSDispatch2 *objthis = instance.AsObjectNoAddRef();

	// TJSインスタンスにネイティブインスタンスとして登録しておく
	iTJSNativeInstance *ninstance = this;
	objthis->NativeInstanceSupport(TJS_NIS_REGISTER, classId, &ninstance);

	// callSQ メソッド登録
	tCallSQFunction *callSQ = new tCallSQFunction();
	if (callSQ) {
		tTJSVariant val(callSQ, objthis);
		objthis->PropSet(TJS_MEMBERENSURE, TJS_W("callSQ"), NULL, &val, objthis);
		callSQ->Release();
	}
	
	// missing メソッド登録
	tMissingFunction *missing = new tMissingFunction();
	if (missing) {
		tTJSVariant val(missing, objthis);
		const tjs_char *missingName = TJS_W("missing");
		objthis->PropSet(TJS_MEMBERENSURE, missingName, NULL, &val, objthis);
		missing->Release();
		// missing 有効化
		tTJSVariant name(missingName);
		objthis->ClassInstanceInfo(TJS_CII_SET_MISSING, 0, &name);
	}
}

static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
	return false;
}

// デストラクタ
TJSObject::~TJSObject()
{
}

static void TJS_USERENTRY TryInvalidate(void * data) {
	tTJSVariant *v = (tTJSVariant*)data;
	v->AsObjectClosureNoAddRef().Invalidate(0, NULL, NULL, NULL);
}

void
TJSObject::invalidate()
{
	// TJSオブジェクトを破壊して参照をクリアする
	// これにより、TJS側で回収処理が走りこのオブジェクト自体は
	// ネイティブインスタンスのクリア処理で破棄される
	if (instance.Type() == tvtObject && instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE) {
		TVPDoTryBlock(TryInvalidate, Catch, NULL, (void *)&instance);
	}
	instance.Clear();
}

/**
 * オブジェクトのリリーサ
 */
SQRESULT
TJSObject::release(SQUserPointer up, SQInteger size)
{
	TJSObject *self = (TJSObject*)up;
	if (self) {
		self->destructor();
		self->invalidate();
	}
	return SQ_OK;
}

// ---------------------------
// NativeInstance 対応用メンバ
// ---------------------------

// 生成時呼び返し
tjs_error TJS_INTF_METHOD
TJSObject::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}

// Invalidate時呼び返し
void TJS_INTF_METHOD
TJSObject::Invalidate()
{
}

// 破棄時呼び返し
void TJS_INTF_METHOD
TJSObject::Destruct()
{
	delete this;
}

// TJSの例外回避呼び出し処理用
class FuncInfo {

public:
	// コンストラクタ
	FuncInfo(HSQUIRRELVM v, tTJSVariant &instance, int offset=0) : v(v), instance(instance), argc(0), args(NULL), result(SQ_OK) {
		// 引数生成
		if (offset > 0) {
			argc = (tjs_int)(sq_gettop(v) - offset);
			if (argc > 0) {
				args = new tTJSVariant*[(size_t)argc];
				for (tjs_int i=0;i<argc;i++) {
					args[i] = new tTJSVariant();
					sq_getvariant(v, 2+i, args[i]);
				}
			}
		}
	}

	// デストラクタ
	~FuncInfo() {
		// 引数破棄
		if (args) {
			for (int i=0;i<argc;i++) {
				delete args[i];
			}
			delete[] args;
		}
	}
	
	SQRESULT create(tTJSVariant &target) {
		TVPDoTryBlock(TryCreate, Catch, Finally, (void *)this);
		target = r;
		return result;
	}

	SQRESULT exec() {
		TVPDoTryBlock(TryExec, Catch, Finally, (void *)this);
		return result;
	}

	SQRESULT setter() {
		TVPDoTryBlock(TrySetter, Catch, Finally, (void *)this);
		return result;
	}

	SQRESULT getter() {
		TVPDoTryBlock(TryGetter, Catch, Finally, (void *)this);
		return result;
	}

private:

	void _TryCreate() {
		tjs_error error;
		iTJSDispatch2 *newinstance;
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().CreateNew(0, NULL, NULL, &newinstance, argc, args, NULL))) {
			r = tTJSVariant(newinstance, newinstance);
			newinstance->Release();
			result = SQ_OK;
		} else {
			result = ERROR_KRKR(v, error);
		}
	}

	static void TJS_USERENTRY TryCreate(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryCreate();
	}

	void _TryExec() {
		tjs_error error;
		tTJSVariant method;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &method))) {
			if (TJS_SUCCEEDED(error = method.AsObjectNoAddRef()->FuncCall(0, NULL, NULL, &r, argc, args, instance.AsObjectNoAddRef()))) {
				if (r.Type() != tvtVoid) {
					sq_pushvariant(v, r);
					result = 1;
				} else {
					result = 0;
				}
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TryExec(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryExec();
	}

	void _TrySetter() {
		sq_getvariant(v, 2, &r);
		tjs_error error;
		tTJSVariant prop;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &prop))) {
			if (TJS_SUCCEEDED(error = prop.AsObjectNoAddRef()->PropSet(TJS_MEMBERENSURE, NULL, NULL, &r, instance.AsObjectNoAddRef()))) {
				result = SQ_OK;
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TrySetter(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TrySetter();
	}
	
	void _TryGetter() {
		tjs_error error;
		tTJSVariant prop;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &prop))) {
			if (TJS_SUCCEEDED(error = prop.AsObjectNoAddRef()->PropGet(0, NULL, NULL, &r, instance.AsObjectNoAddRef()))) {
				sq_pushvariant(v, r);
				result = 1;
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TryGetter(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryGetter();
	}

	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		FuncInfo *info = (FuncInfo*)data;
		info->result = sq_throwerror(info->v, desc.message.c_str());
		// 例外は常に無視
		return false;
	}

	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	HSQUIRRELVM v;
	tTJSVariant &instance;
	tjs_int argc;
	tTJSVariant **args;
	tTJSVariant r;
	SQRESULT result;
};

/**
 * TJSオブジェクトのコンストラクタ
 * 引数1 オブジェクト
 * 引数2～ 引数
 * 自由変数1 TJSクラスオブジェクト
 */
SQRESULT
TJSObject::tjsConstructor(HSQUIRRELVM v)
{
	// クラスを生成する
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -1, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj, 2);
		tTJSVariant variant;
		SQRESULT ret = info.create(variant);
		if (SQ_SUCCEEDED(ret)) {
			TJSObject *self = new TJSObject(v, 1, variant);
			if (SQ_SUCCEEDED(sq_setinstanceup(v, 1, self))) {
				sq_setreleasehook(v, 1, release);
			} else {
				delete self;
			}
		}
		return ret;
	}
	return ERROR_CREATE(v);
}

/**
 * TJSオブジェクト用のメソッド
 * 引数1 オブジェクト
 * 引数2～ 引数
 * 自由変数1 メソッド
 */
SQRESULT
TJSObject::tjsInvoker(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance, 2);
		return info.exec();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJSオブジェクト用のプロパティゲッター
 * 引数1 オブジェクト
 * 自由変数1 プロパティ
 */
SQRESULT
TJSObject::tjsGetter(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance);
		return info.getter();
	} 
	return ERROR_BADINSTANCE(v);
}

/**
 * TJSオブジェクト用のプロパティセッター
 * 引数1 オブジェクト
 * 引数2 設定値
 * 自由変数1 プロパティ
 */
SQRESULT
TJSObject::tjsSetter(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance);
		return info.setter();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJSオブジェクト用の静的メソッド
 * 引数1 オブジェクト
 * 引数2～ 引数
 * 自由変数1 クラスオブジェクト
 * 自由変数2 メソッド
 */
SQRESULT
TJSObject::tjsStaticInvoker(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj, 3);
		return info.exec();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJSオブジェクト用の静的プロパティゲッター
 * 引数1 オブジェクト
 * 自由変数1 クラスオブジェクト
 * 自由変数2 プロパティ
 */
SQRESULT
TJSObject::tjsStaticGetter(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj);
		return info.getter();
	} 
	return ERROR_BADINSTANCE(v);
}
	
/**
 * TJSオブジェクト用の静的プロパティセッター
 * 引数1 オブジェクト
 * 引数2 設定値
 * 自由変数1 クラスオブジェクト
 * 自由変数2 プロパティ
 */
SQRESULT
TJSObject::tjsStaticSetter(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj);
		return info.setter();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJSオブジェクトの有効確認
 * 引数1 オブジェクト
 */
SQRESULT 
TJSObject::tjsIsValid(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		SQBool ret = instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE ? SQTrue : SQFalse;
		sq_pushbool(v, ret);
		return 1;
	}
	sq_pushbool(v, SQFalse);
	return 1;
}

/**
 * TJSオブジェクトのオーバライド処理
 * 引数1 オブジェクト
 * 引数2 名前
 * 引数3 値。省略時は squirrel インスタンスから名前で参照
 */
SQRESULT 
TJSObject::tjsOverride(HSQUIRRELVM v)
{
	SQRESULT result;
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		tTJSVariant value;
		const SQChar *methodName = sqobject::getString(v,2);
		int n = sq_gettop(v);
		if (n >= 3) {
			// 引数で明示指定されている
			if (SQ_FAILED(result = sq_getvariant(v, 3, &value))) {
				return result;
			}
		} else {
			// 自己オブジェクトから名前で検索して登録
			sq_push(v, 1);
			sq_pushstring(v, methodName, -1);
			if (SQ_FAILED(result = sq_get(v, -2))) {
				sq_pop(v, 1); //self
				return result;
			} else {
				SQObjectType type = sq_gettype(v, -1);
				if (type == OT_CLOSURE || type == OT_NATIVECLOSURE) {
					// クロージャの場合は bindenv しておく
					sq_push(v, 1);
					if (SQ_FAILED(result = sq_bindenv(v, -2))) {
						sq_pop(v, 2); // func, self
						return result;
					} else {
						sq_remove(v, -2); // original func
					}
				}
				if (SQ_FAILED(result = sq_getvariant(v, -1, &value))) { // value
					sq_pop(v, 1); //self
					return result;
				}
				sq_pop(v, 1); // self
			}
		}
		tjs_error error;
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().PropSet(TJS_MEMBERENSURE, methodName, NULL, &value, NULL))) {
			return SQ_OK;
		} else {
			return  ERROR_KRKR(v, error);
		}
	}
	return ERROR_BADINSTANCE(v);
}
