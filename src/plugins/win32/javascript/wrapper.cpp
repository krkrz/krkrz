/**
 * Javascript ←→ 吉里吉里ブリッジ処理
 * 吉里吉里のオブジェクトは XXXX として管理する
 */

#include "tjsobj.h"
#include "tjsinstance.h"

// 値の格納・取得用
Local<Value> toJSValue(const tTJSVariant &variant);
tTJSVariant toVariant(Handle<Value> value);

#define JSOBJECTCLASS L"JavascriptObject"

/**
 * Javascript object 用 iTJSDispatch2 ラッパー
 */
class iTJSDispatch2Wrapper : public tTJSDispatch
{
public:
	/**
	 * コンストラクタ
	 * @param obj IDispatch
	 */
	iTJSDispatch2Wrapper(Handle<Object> obj) {
		this->obj = Persistent<Object>::New(obj);
	}
	
	/**
	 * デストラクタ
	 */
	~iTJSDispatch2Wrapper() {
		obj.Dispose();
	}

	/**
	 * 保持してる値を返す
	 */
	Local<Value> getValue() {
		return *obj;
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
		return TJSInstance::createMethod(obj->ToObject(), membername, result, numparams, param);
	}

	// メソッド呼び出し
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis
		) {
		return TJSInstance::callMethod(obj->ToObject(), membername, result, numparams, param, objthis);
	}

	// プロパティ取得
	tjs_error TJS_INTF_METHOD PropGet(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		iTJSDispatch2 *objthis) {
		if (!membername) {
			return TJS_E_NOTIMPL;
		}
		return TJSInstance::getProp(obj->ToObject(), membername, result);
	}

	// プロパティ設定
	tjs_error TJS_INTF_METHOD PropSet(
		tjs_uint32 flag,
		const tjs_char *membername,
		tjs_uint32 *hint,
		const tTJSVariant *param,
		iTJSDispatch2 *objthis) {
		return TJSInstance::setProp(obj->ToObject(), membername, param);
	}

	// メンバ削除
	tjs_error TJS_INTF_METHOD DeleteMember(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint,
		iTJSDispatch2 *objthis) {
		return TJSInstance::remove(obj->ToObject(), membername);
	}

	tjs_error TJS_INTF_METHOD IsInstanceOf(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		const tjs_char * classname,
		iTJSDispatch2 *objthis
		) {
		if (membername == NULL && wcscmp(classname, JSOBJECTCLASS) == 0) {
			return TJS_S_TRUE;
		}
		return TJS_S_FALSE;
	}

protected:
	/// 内部保持用
	Persistent<Object> obj;
};

//----------------------------------------------------------------------------
// 変換用
//----------------------------------------------------------------------------

/**
 * tTJSVariant を squirrel の空間に投入する
 * @param result javascrpt value
 * @param variant tTJSVariant
 */
Local<Value>
toJSValue(const tTJSVariant &variant)
{
	switch (variant.Type()) {
	case tvtVoid:
		return *Undefined();
	case tvtObject:
		{
			iTJSDispatch2 *obj = variant.AsObjectNoAddRef();
			if (obj == NULL) {
				// NULLの処理
				return *Null();
			} else if (obj->IsInstanceOf(0, NULL, NULL, JSOBJECTCLASS, obj) == TJS_S_TRUE) {
				// Javascript ラッピングオブジェクトの場合
				iTJSDispatch2Wrapper *wobj = (iTJSDispatch2Wrapper*)obj;
				return wobj->getValue();
			} else {
				Local<Object> result;
				if (TJSInstance::getJSObject(result, variant)) {
					// 登録済みインスタンスの場合
					return result;
				}
				// 単純ラッピング
				return TJSObject::toJSObject(variant);
			}
		}
		break;
	case tvtString:
		return String::New(variant.GetString(), -1);
	case tvtOctet:
		return *Null();
	case tvtInteger:
	case tvtReal:
		return Number::New((tTVReal)variant);
	}
	return *Undefined();
}

tTJSVariant
toVariant(Handle<Object> object, Handle<Object> context)
{
	tTJSVariant result;
	iTJSDispatch2 *tjsobj = new iTJSDispatch2Wrapper(object);
	iTJSDispatch2 *tjsctx = new iTJSDispatch2Wrapper(context);
	if (tjsobj && tjsctx) {
		result = tTJSVariant(tjsobj, tjsctx);
		tjsobj->Release();
		tjsctx->Release();
	} else {
		if (tjsobj) { tjsobj->Release(); };
		if (tjsctx) { tjsctx->Release(); };
	}
	return result;
}

tTJSVariant
toVariant(Handle<Object> object)
{
	tTJSVariant result;
	iTJSDispatch2 *tjsobj = new iTJSDispatch2Wrapper(object);
	if (tjsobj) {
		result = tTJSVariant(tjsobj, tjsobj);
		tjsobj->Release();
	}
	return result;
}

/**
 * javascript値を tTJSVariant に変換する
 * @param value Javascript値
 * @return tTJSVariant
 */
tTJSVariant
toVariant(Handle<Value> value)
{
	tTJSVariant result;
	if (value->IsNull()) {
		result = (iTJSDispatch2*)0;
	} else if (value->IsTrue()) {
		result = true;
	} else if (value->IsFalse()) {
		result = false;
	} else if (value->IsString()) {
		String::Value str(value);
		result = *str;
	} else if (value->IsFunction() || value->IsArray() || value->IsDate()) {
		// 単純ラッピング
		result = toVariant(value->ToObject());
	} else if (value->IsObject()) {
		Local<Object> obj = value->ToObject();
		if (!TJSBase::getVariant(result, obj)) {
			// 単純ラッピング
			result = toVariant(obj);
		}
	} else if (value->IsBoolean()) {
		result = value->BooleanValue();
	} else if (value->IsNumber()) {
		result = value->NumberValue();
	} else if (value->IsInt32()) {
		result = value->Int32Value();
	} else if (value->IsUint32()) {
		result = (tTVInteger)value->Uint32Value();
	}
	// value->IsUndefined()
	// value->IsExternal()
	return result;
}
