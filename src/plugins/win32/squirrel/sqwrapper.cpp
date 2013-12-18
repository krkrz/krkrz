/**
 * Squirrel ←→ 吉里吉里ブリッジ処理
 * 吉里吉里のオブジェクトは UserData として管理する
 */

#include <windows.h>
#include "tp_stub.h"
#include <squirrel.h>
#include "sqtjsobj.h"
#include <new>

// sqfunc.cpp
extern SQRESULT ERROR_BADINSTANCE(HSQUIRRELVM v);

// 外部参照

SQRESULT ERROR_KRKR(HSQUIRRELVM v, tjs_error error) {
	switch (error) {
	case TJS_E_MEMBERNOTFOUND:
		return sq_throwerror(v, _SC("member not found"));
	case TJS_E_NOTIMPL:
		return sq_throwerror(v, _SC("not implemented"));
	case TJS_E_INVALIDPARAM:
		return sq_throwerror(v, _SC("invalid param"));
	case TJS_E_BADPARAMCOUNT:
		return sq_throwerror(v, _SC("bad param count"));
	case TJS_E_INVALIDTYPE:
		return sq_throwerror(v, _SC("invalid type"));
	case TJS_E_INVALIDOBJECT:
		return sq_throwerror(v, _SC("invalid object"));
	case TJS_E_ACCESSDENYED:
		return sq_throwerror(v, _SC("access denyed"));
	case TJS_E_NATIVECLASSCRASH:
		return sq_throwerror(v, _SC("navive class crash"));
	default:
		return sq_throwerror(v, _SC("failed"));
	}
}

// 格納・取得用
void sq_pushvariant(HSQUIRRELVM v, tTJSVariant &variant);
SQRESULT sq_getvariant(HSQUIRRELVM v, int idx, tTJSVariant *result);

/**
 * エラー処理
 */
void
SQEXCEPTION(HSQUIRRELVM v)
{
	sq_getlasterror(v);
	const SQChar *str;
	sq_getstring(v, -1, &str);
	ttstr error = str;
	sq_pop(v, 1);
	TVPThrowExceptionMessage(error.c_str());
}

#define SQUIRRELOBJCLASS L"SquirrelObject"

/**
 * HSQOBJECT 用 iTJSDispatch2 ラッパー
 */
class iTJSDispatch2Wrapper : public tTJSDispatch
{
protected:
	/// 内部保持用・グローバルVMで保持するようにする
	HSQUIRRELVM gv;
	HSQOBJECT obj;

public:
	/**
	 * コンストラクタ
	 * @param obj IDispatch
	 */
	iTJSDispatch2Wrapper(HSQUIRRELVM v, int idx) {
		// グローバルに移した上で保持する
		sq_resetobject(&obj);
		gv = sqobject::getGlobalVM();
		sq_move(gv, v, idx);
		sq_getstackobj(gv,-1,&obj);
		sq_addref(gv, &obj);
		sq_pop(gv, 1);
	}
	
	/**
	 * デストラクタ
	 */
	~iTJSDispatch2Wrapper() {
		sq_release(gv, &obj);
	}

	void push(HSQUIRRELVM v) {
		sq_pushobject(v, obj);
	}

public:
	// オブジェクト生成
	tjs_error TJS_INTF_METHOD CreateNew(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		iTJSDispatch2 **result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis) {
		if (membername) {
			return TJS_E_MEMBERNOTFOUND;
		}
		if (obj._type != OT_CLASS) {
			return TJS_E_NOTIMPL;
		}
		int ret = S_FALSE;
		sq_pushobject(gv, obj);
		sq_pushroottable(gv);			// this 相当部分
		for (int i=0;i<numparams;i++) {	// パラメータ群
			sq_pushvariant(gv, *param[i]);
		}
		if (SQ_SUCCEEDED(sq_call(gv, numparams + 1, result ? SQTrue:SQFalse, SQTrue))) {
			if (result) {
				tTJSVariant var;
				sq_getvariant(gv, -1, &var);
				sq_pop(gv, 1); // newobj
				*result = var;
			}
			sq_pop(gv, 1); // obj
		} else {
			sq_pop(gv, 1); // obj
			SQEXCEPTION(gv);
		}
		return TJS_S_OK;
	}

	// オブジェクト機能呼び出し
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis
		) {
		if (membername == NULL) {
			if (!(obj._type == OT_CLOSURE ||
				  obj._type == OT_NATIVECLOSURE ||
				  obj._type == OT_GENERATOR)) {
				return TJS_E_NOTIMPL;
			}
			sq_pushobject(gv, obj);
			sq_pushroottable(gv);
			for (int i=0;i<numparams;i++) {	// パラメータ群
				sq_pushvariant(gv, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(gv, numparams + 1, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(gv, -1, result);
					sq_pop(gv, 1);
				}
				sq_pop(gv, 1);
			} else {
				sq_pop(gv, 1);
				SQEXCEPTION(gv);
			}
			return TJS_S_OK;
		} else {
			sq_pushobject(gv, obj);
			sq_pushstring(gv, membername,-1);
			if (SQ_SUCCEEDED(sq_get(gv,-2))) {
				sq_pushobject(gv, obj); // this
				for (int i=0;i<numparams;i++) {	// パラメータ群
					sq_pushvariant(gv, *param[i]);
				}
				// 帰り値がある場合
				if (SQ_SUCCEEDED(sq_call(gv, numparams + 1, result ? SQTrue:SQFalse, SQTrue))) {
					if (result) {
						sq_getvariant(gv, -1, result);
						sq_pop(gv, 1);
					}
					sq_pop(gv, 2);
				} else {
					sq_pop(gv, 2);
					SQEXCEPTION(gv);
				}
			} else {
				sq_pop(gv, 1);
				return TJS_E_MEMBERNOTFOUND;
			}
			return TJS_S_OK;
		}
	}

	// プロパティ取得
	tjs_error TJS_INTF_METHOD PropGet(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		iTJSDispatch2 *objthis) {
		// プロパティはない
		if (!membername) {
			return TJS_E_NOTIMPL;
		}
		sq_pushobject(gv, obj);
		sq_pushstring(gv, membername,-1);
		if (SQ_SUCCEEDED(sq_get(gv,-2))) {
			if (result) {
				sq_getvariant(gv, -1, result);
			}
			sq_pop(gv,2);
		} else {
			sq_pop(gv,1);
			if (sq_istable(obj) || sq_isarray(obj)) {
				// tableの場合は void を返す
				if (result) {
					result->Clear();
				}
			} else {
				return TJS_E_MEMBERNOTFOUND;
			}
		}
		return TJS_S_OK;
	}

	// プロパティ取得
	tjs_error TJS_INTF_METHOD PropGetByNum(
		tjs_uint32 flag,
		int num,
		tjs_uint32 *hint,
		tTJSVariant *result,
		iTJSDispatch2 *objthis) {
		sq_pushobject(gv, obj);
		sq_pushinteger(gv, num);
		if (SQ_SUCCEEDED(sq_get(gv,-2))) {
			if (result) {
				sq_getvariant(gv, -1, result);
			}
			sq_pop(gv,2);
		} else {
			sq_pop(gv,1);
			if (sq_istable(obj) || sq_isarray(obj)) {
				// tableの場合は void を返す
				if (result) {
					result->Clear();
				}
			} else {
				return TJS_E_MEMBERNOTFOUND;
			}
		}
		return TJS_S_OK;
	}
	
	// プロパティ設定
	tjs_error TJS_INTF_METHOD PropSet(
		tjs_uint32 flag,
		const tjs_char *membername,
		tjs_uint32 *hint,
		const tTJSVariant *param,
		iTJSDispatch2 *objthis) {
		// プロパティはない
		if (!membername) {
			return TJS_E_NOTIMPL;
		}
		sq_pushobject(gv, obj);
		sq_pushstring(gv, membername,-1);
		sq_pushvariant(gv, (tTJSVariant&)*param);
		if (sq_istable(obj) || sq_isarray(obj) || (flag & TJS_MEMBERENSURE)) {
			if (SQ_SUCCEEDED(sq_newslot(gv,-3, SQFalse))) {
				sq_pop(gv,1);
			} else {
				sq_pop(gv,1);
				SQEXCEPTION(gv);
			}
		} else {
			if (SQ_SUCCEEDED(sq_set(gv,-3))) {
				sq_pop(gv,1);
			} else {
				sq_pop(gv,1);
				SQEXCEPTION(gv);
			}
		}
		return TJS_S_OK;
	}

	// プロパティ設定
	tjs_error TJS_INTF_METHOD PropSetByNum(
		tjs_uint32 flag,
		int num,
		tjs_uint32 *hint,
		const tTJSVariant *param,
		iTJSDispatch2 *objthis) {
		sq_pushobject(gv, obj);
		sq_pushinteger(gv, num);
		sq_pushvariant(gv, (tTJSVariant&)*param);
		if (sq_istable(obj) || sq_isarray(obj) || (flag & TJS_MEMBERENSURE)) {
			if (SQ_SUCCEEDED(sq_newslot(gv,-3, SQFalse))) {
				sq_pop(gv,1);
			} else {
				sq_pop(gv,1);
				SQEXCEPTION(gv);
			}
		} else {
			if (SQ_SUCCEEDED(sq_set(gv,-3))) {
				sq_pop(gv,1);
			} else {
				sq_pop(gv,1);
				SQEXCEPTION(gv);
			}
		}
		return TJS_S_OK;
	}

	
	tjs_error TJS_INTF_METHOD IsInstanceOf(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		const tjs_char * classname,
		iTJSDispatch2 *objthis
		) {
		if (membername == NULL && wcscmp(classname, SQUIRRELOBJCLASS) == 0) {
			return TJS_S_TRUE;
		}
		return TJS_S_FALSE;
	}
};


//----------------------------------------------------------------------------
// tTJSVariant を UserData として保持するための機構
//----------------------------------------------------------------------------

/**
 * tTJSVariant用オブジェクト開放処理
 */
static SQRESULT
variantRelease(SQUserPointer up, SQInteger size)
{
	tTJSVariant *self = (tTJSVariant*)up;
	if (self) {
		self->~tTJSVariant();
	}
	return SQ_OK;
}

static const SQUserPointer TJSTYPETAG = (SQUserPointer)"TJSTYPETAG";

// iTJSDispatch2* 取得用
static bool GetVariant(HSQUIRRELVM v, int idx, tTJSVariant *result)
{
	SQUserPointer otag;
	SQUserPointer up;
	if (SQ_SUCCEEDED(sq_getuserdata(v,idx,&up,&otag)) && otag == TJSTYPETAG) {
		*result = *(tTJSVariant*)up;
		return true;
	}
	return false;
}

// iTJSDispatch2* 取得用
static iTJSDispatch2 *GetDispatch(HSQUIRRELVM v, int idx)
{
	tTJSVariant var;
	if (GetVariant(v, idx, &var)) {
		return var.AsObjectNoAddRef();
	}
	return NULL;
}

/**
 * iTJSDispatch2 用プロパティの存在確認
 * @param v squirrel VM
 */
static SQRESULT
exist(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (GetVariant(v, 1, &instance)) {
		tTJSVariant result;
		tjs_error error;
		const tjs_char *name = sqobject::getString(v,2);
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().PropGet(TJS_MEMBERMUSTEXIST, sqobject::getString(v, 2), NULL, &result, NULL))) {
			return 1;
		} else {
			return ERROR_KRKR(v, error);
		}
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * iTJSDispatch2 用プロパティの取得
 * @param v squirrel VM
 */
static SQRESULT
get(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (GetVariant(v, 1, &instance)) {
		tTJSVariant result;
		tjs_error error;
		const tjs_char *name = sqobject::getString(v,2);
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().PropGet(TJS_MEMBERMUSTEXIST, sqobject::getString(v, 2), NULL, &result, NULL))) {
			sq_pushvariant(v, result);
			return 1;
		} else {
			return ERROR_KRKR(v, error);
		}
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * iTJSDispatch2 用プロパティの設定
 * @param v squirrel VM
 */
static SQRESULT
set(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (GetVariant(v, 1, &instance)) {
		tTJSVariant result;
		sq_getvariant(v, 3, &result);
		tjs_error error;
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().PropSet(TJS_MEMBERENSURE, sqobject::getString(v, 2), NULL, &result, NULL))) {
			return SQ_OK;
		} else {
			return ERROR_KRKR(v, error);
		}
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * iTJSDispatch2 用コンストラクタの呼び出し
 * @param v squirrel VM
 */
static SQRESULT
callConstructor(HSQUIRRELVM v)
{
	// param1 オブジェクト
	// param2 オリジナルオブジェクト
	// param3 ～ 本来の引数ぽ

	iTJSDispatch2 *dispatch	= GetDispatch(v, 1);
	if (dispatch) {

		// this を取得
		iTJSDispatch2 *thisobj = GetDispatch(v, 2);
		
		tjs_int argc = (tjs_int)(sq_gettop(v) - 2);
		
		// 引数変換
		tTJSVariant **args = new tTJSVariant*[argc];
		for (int i=0;i<argc;i++) {
			args[i] = new tTJSVariant();
			sq_getvariant(v, i+3, args[i]);
		}

		SQRESULT ret = 0;
		iTJSDispatch2 *instance = NULL;
		tjs_error error;
		if (TJS_SUCCEEDED(error = dispatch->CreateNew(0, NULL, NULL, &instance, argc, args, thisobj))) {
			tTJSVariant var(instance, instance);
			sq_pushvariant(v, var);
			instance->Release();
			ret = 1;
		} else {
			ret = ERROR_KRKR(v, error);
		}
			
		// 引数破棄
		for (int i=0;i<argc;i++) {
			delete args[i];
		}
		delete[] args;

		return ret;
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * iTJSDispatch2 用メソッドの呼び出し
 * @param v squirrel VM
 */
static SQRESULT
callMethod(HSQUIRRELVM v)
{
	// param1 オブジェクト
	// param2 オリジナルオブジェクト
	// param3 ～ 本来の引数ぽ

	iTJSDispatch2 *dispatch	= GetDispatch(v, 1);
	if (dispatch) {

		// this を取得
		iTJSDispatch2 *thisobj = GetDispatch(v, 2);
		
		tjs_int argc = (tjs_int)(sq_gettop(v) - 2);
		
		// 引数変換
		tTJSVariant **args = new tTJSVariant*[argc];
		for (tjs_int i=0;i<argc;i++) {
			args[i] = new tTJSVariant();
			sq_getvariant(v, i+3, args[i]);
		}

		// メソッド呼び出し
		SQRESULT ret = 0;
		tTJSVariant result;
		tjs_error error;
		if (TJS_SUCCEEDED(error = dispatch->FuncCall(0, NULL, NULL, &result, argc, args, thisobj))) {
			if (result.Type() != tvtVoid) {
				sq_pushvariant(v, result);
				ret = 1;
			} else {
				ret = 0;
			}
		} else {
			ret = ERROR_KRKR(v, error);
		}
			
		// 引数破棄
		for (int i=0;i<argc;i++) {
			delete args[i];
		}
		delete[] args;

		return ret;
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * tTJSVariant を squirrel の空間に投入する
 * @param v squirrel VM
 * @param variant tTJSVariant
 */
void
sq_pushvariant(HSQUIRRELVM v, tTJSVariant &variant)
{
	switch (variant.Type()) {
	case tvtVoid:
		sq_pushnull(v);
		break;
	case tvtObject:
		{
			iTJSDispatch2 *obj = variant.AsObjectNoAddRef();
			if (obj == NULL) {
				// NULLの処理
				sq_pushuserpointer(v, NULL);
			} else if (!TJSObject::pushVariant(v, variant)) {
				if (obj->IsInstanceOf(0, NULL, NULL, SQUIRRELOBJCLASS, obj) == TJS_S_TRUE) {
					iTJSDispatch2Wrapper *wobj = (iTJSDispatch2Wrapper*)obj;
					wobj->push(v);
				} else {
					// UserData 確保
					tTJSVariant *self = (tTJSVariant*)sq_newuserdata(v, sizeof tTJSVariant);
					if (self) {
						new (self) tTJSVariant();
						*self = variant;
						// 開放ロジックを追加
						sq_setreleasehook(v, -1, variantRelease);
						
						// タグ登録
						sq_settypetag(v, -1, TJSTYPETAG);
						
						
						// メソッド群を追加
						sq_newtable(v);

						sq_pushstring(v, L"_exist", -1);
						sq_newclosure(v, exist, 0);
						sq_createslot(v, -3);
						
						sq_pushstring(v, L"_get", -1);
						sq_newclosure(v, get, 0);
						sq_createslot(v, -3);
						
						sq_pushstring(v, L"_set", -1);
						sq_newclosure(v, set, 0);
						sq_createslot(v, -3);
						
						sq_pushstring(v, L"_call", -1);
						if (self->AsObjectClosureNoAddRef().IsInstanceOf(0, NULL, NULL, L"Class", NULL) == TJS_S_TRUE) {
							sq_newclosure(v, callConstructor, 0);
						} else {
							sq_newclosure(v, callMethod, 0);
						}
						sq_createslot(v, -3);
						
						sq_setdelegate(v, -2);
					} else {
						sq_pushnull(v);
					}
				}
			}
		}
		break;
	case tvtString:
		sq_pushstring(v, variant.GetString(), -1);
		break;
	case tvtOctet: // XXX
		sq_pushnull(v);
		break;
	case tvtInteger:
		sq_pushinteger(v, (SQInteger)(tTVInteger)(variant));
		break;
	case tvtReal:
		sq_pushfloat(v, (SQFloat)(tTVReal)(variant));
		break;
	}
}

static void wrap(HSQUIRRELVM v, int idx, tTJSVariant *result)
{
	// ラッピング
	iTJSDispatch2 *tjsobj = new iTJSDispatch2Wrapper(v, idx);
	if (tjsobj) {
		*result = tTJSVariant(tjsobj, tjsobj);
		tjsobj->Release();
	}
}

/**
 * tTJSVariant を squirrel の空間から取得する
 * @param v squirrel VM
 * @param idx スタックのインデックス
 * @param result tTJSVariant を返す先
 */
SQRESULT
sq_getvariant(HSQUIRRELVM v, int idx, tTJSVariant *result)
{
	if (result) {
		switch (sq_gettype(v, idx)) {
		case OT_NULL: result->Clear(); break; // void
		case OT_INTEGER: { SQInteger i; sq_getinteger(v, idx, &i);	*result = (tTVInteger)i; } break;
		case OT_FLOAT:   { SQFloat f; sq_getfloat(v, idx, &f); 	    *result = (tTVReal)f; } break;
		case OT_BOOL:    { SQBool b; sq_getbool(v, idx, &b);        *result = b != SQFalse; } break;
		case OT_STRING:  { const SQChar *c; sq_getstring(v, idx, &c); *result = c; } break;
		case OT_USERDATA:
			if (!GetVariant(v, idx, result)) {
				wrap(v, idx, result);
			}
			break;
		case OT_CLASS:
		case OT_INSTANCE:
			if (!TJSObject::getVariant(v, idx, result)) {
				wrap(v, idx, result);
			}
			break;
			// through down
		case OT_TABLE:
		case OT_ARRAY:
		case OT_CLOSURE:
		case OT_NATIVECLOSURE:
		case OT_GENERATOR:
		case OT_THREAD:
		case OT_WEAKREF:
			wrap(v, idx, result);
			break;
		case OT_USERPOINTER: // null
			*result = (iTJSDispatch2*)0;
			break;
		default:
			result->Clear();
		}
		return SQ_OK;
	}
	return SQ_ERROR;
}

SQRESULT
sq_getvariant(sqobject::ObjectInfo &obj, tTJSVariant *result)
{
	HSQUIRRELVM gv = sqobject::getGlobalVM();
	obj.push(gv);
	SQRESULT ret = sq_getvariant(gv, -1, result);
	sq_pop(gv, 1);
	return ret;
}
