#include "ncbind/ncbind.hpp"
#include <vector>
#include <algorithm>

/**
 * メソッド追加用
 */
class ScriptsAdd {

public:
	ScriptsAdd(){};

	/**
	 * メンバ名一覧の取得
	 */
	static tjs_error TJS_INTF_METHOD getKeys(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);
	/**
	 * メンバの個数の取得
	 */
	static tjs_error TJS_INTF_METHOD getCount(tTJSVariant *result,
											  tjs_int numparams,
											  tTJSVariant **param,
											  iTJSDispatch2 *objthis);
	/**
	 * コンテキストの取得
	 */
	static tTJSVariant getObjectContext(tTJSVariant obj);

	/**
	 * コンテキストが null かどうか判定
	 */
	static bool isNullContext(tTJSVariant obj);
	
	//----------------------------------------------------------------------
	// 構造体比較関数
	static bool equalStruct(tTJSVariant v1, tTJSVariant v2);

	//----------------------------------------------------------------------
	// 構造体比較関数(数字の比較はゆるい)
	static bool equalStructNumericLoose(tTJSVariant v1, tTJSVariant v2);

	//----------------------------------------------------------------------
	// 全配列・辞書巡回
	static tjs_error TJS_INTF_METHOD foreach(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);

	//----------------------------------------------------------------------
	// hash値取得
	static tjs_error TJS_INTF_METHOD getMD5HashString(tTJSVariant *result,
													  tjs_int numparams,
													  tTJSVariant **param,
													  iTJSDispatch2 *objthis);


	//----------------------------------------------------------------------
	// オブジェクト複製
	static tTJSVariant clone(tTJSVariant v1);
};

/**
 * 辞書のキー一覧取得用
 */
class DictMemberGetCaller : public tTJSDispatch /** EnumMembers 用 */
{
public:
	DictMemberGetCaller(iTJSDispatch2 *array) : array(array) {};
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
			tTVInteger flag = param[1]->AsInteger();
			static tjs_uint addHint = NULL;
			if (!(flag & TJS_HIDDENMEMBER)) {
				array->FuncCall(0, TJS_W("add"), &addHint, 0, 1, &param[0], array);
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
protected:
	iTJSDispatch2 *array;
};


//----------------------------------------------------------------------
// 辞書を作成
tTJSVariant createDictionary(void)
{
	iTJSDispatch2 *obj = TJSCreateDictionaryObject();
	tTJSVariant result(obj, obj);
	obj->Release();
	return result;
}

//----------------------------------------------------------------------
// 配列を作成
tTJSVariant createArray(void)
{
	iTJSDispatch2 *obj = TJSCreateArrayObject();
	tTJSVariant result(obj, obj);
	obj->Release();
	return result;
}

//----------------------------------------------------------------------
// 辞書の要素を全比較するCaller
class DictMemberCompareCaller : public tTJSDispatch
{
public:
	tTJSVariantClosure &target;
	bool match;

	DictMemberCompareCaller(tTJSVariantClosure &_target)
		 : target(_target)
		   , match(true) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (result)
			*result = true;
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				const tjs_char *key = param[0]->GetString();
				tTJSVariant value = *param[2];
				tTJSVariant targetValue;
				if (target.PropGet(0, key, NULL, &targetValue, NULL)
					== TJS_S_OK) {
					match = match && ScriptsAdd::equalStruct(value, targetValue);
					if (result)
						*result = match;
				}
			}
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// 辞書の要素を全比較するCaller(数字の比較はゆるい)
class DictMemberCompareNumericLooseCaller : public tTJSDispatch
{
public:
	tTJSVariantClosure &target;
	bool match;

	DictMemberCompareNumericLooseCaller(tTJSVariantClosure &_target)
		 : target(_target)
		   , match(true) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (result)
			*result = true;
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				const tjs_char *key = param[0]->GetString();
				tTJSVariant value = *param[2];
				tTJSVariant targetValue;
				if (target.PropGet(0, key, NULL, &targetValue, NULL)
					== TJS_S_OK) {
					match = match && ScriptsAdd::equalStructNumericLoose(value, targetValue);
					if (result)
						*result = match;
				}
			}
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// 辞書を巡回するcaller
class DictIterateCaller : public tTJSDispatch
{
public:
	iTJSDispatch2 *func;
	iTJSDispatch2 *functhis;
	tTJSVariant **paramList;
	tjs_int paramCount;
	tTJSVariant breakResult;

	DictIterateCaller(iTJSDispatch2 *func,
					  iTJSDispatch2 *functhis,
					  tTJSVariant **_paramList,
					  tjs_int _paramCount)
		 : func(func), functhis(functhis)
		   , paramList(_paramList)
		   , paramCount(_paramCount) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		breakResult.Clear();
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				paramList[0] = param[0];
				paramList[1] = param[2];
				(void)func->FuncCall(0, NULL, NULL, &breakResult, paramCount, paramList, functhis);
			}
		}
		if (result) {
			*result = breakResult.Type() == tvtVoid;
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// 変数
tjs_uint32 countHint;

	/**
	 * メンバ名一覧の取得
	 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getKeys(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	if (result) {
		iTJSDispatch2 *array = TJSCreateArrayObject();
		DictMemberGetCaller *caller = new DictMemberGetCaller(array);
		tTJSVariantClosure closure(caller);
		param[0]->AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
		caller->Release();
		*result = tTJSVariant(array, array);
		array->Release();
	}
	return TJS_S_OK;
}

/**
 * メンバの個数の取得
 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getCount(tTJSVariant *result,
					 tjs_int numparams,
					 tTJSVariant **param,
					 iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	if (result) {
		tjs_int count;
		param[0]->AsObjectClosureNoAddRef().GetCount(&count, NULL, NULL, NULL);
		*result = count;
	}
	return TJS_S_OK;
}


/**
 * コンテキストの取得
 */
tTJSVariant
ScriptsAdd::getObjectContext(tTJSVariant obj)
{
	iTJSDispatch2 *objthis = obj.AsObjectClosureNoAddRef().ObjThis;
	return tTJSVariant(objthis, objthis);
}

/**
 * コンテキストが null かどうか判定
 */
bool
ScriptsAdd::isNullContext(tTJSVariant obj)
{
	return obj.AsObjectClosureNoAddRef().ObjThis == NULL;
}

//----------------------------------------------------------------------
// 構造体比較関数
bool
ScriptsAdd::equalStruct(tTJSVariant v1, tTJSVariant v2)
{
	// タイプがオブジェクトなら特殊判定
	if (v1.Type() == tvtObject
		&& v2.Type() == tvtObject) {
		if (v1.AsObjectNoAddRef() == v2.AsObjectNoAddRef())
			return true;

		tTJSVariantClosure &o1 = v1.AsObjectClosureNoAddRef();
		tTJSVariantClosure &o2 = v2.AsObjectClosureNoAddRef();

		// 関数どうしなら特別扱いで関数比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE)
			return v1.DiscernCompare(v2);

		// Arrayどうしなら全項目を比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			// 長さが一致していなければ比較失敗
			tTJSVariant o1Count, o2Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			(void)o2.PropGet(0, L"count", &countHint, &o2Count, NULL);
			if (! o1Count.DiscernCompare(o2Count))
				return false;
			// 全項目を順番に比較
			tjs_int count = o1Count;
			tTJSVariant o1Val, o2Val;
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &o1Val, NULL);
				(void)o2.PropGetByNum(TJS_IGNOREPROP, i, &o2Val, NULL);
				if (! equalStruct(o1Val, o2Val))
					return false;
			}
			return true;
		}

		// Dictionaryどうしなら全項目を比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			// 項目数が一致していなければ比較失敗
			tjs_int o1Count, o2Count;
			(void)o1.GetCount(&o1Count, NULL, NULL, NULL);
			(void)o2.GetCount(&o2Count, NULL, NULL, NULL);
			if (o1Count != o2Count)
				return false;
			// 全項目を順番に比較
			DictMemberCompareCaller *caller = new DictMemberCompareCaller(o2);
			tTJSVariantClosure closure(caller);
			tTJSVariant(o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL));
			bool result = caller->match;
			caller->Release();
			return result;
		}
	}

	return v1.DiscernCompare(v2);
}

//----------------------------------------------------------------------
// 構造体比較関数(数字の比較はゆるい)
bool
ScriptsAdd::equalStructNumericLoose(tTJSVariant v1, tTJSVariant v2)
{
	// タイプがオブジェクトなら特殊判定
	if (v1.Type() == tvtObject
		&& v2.Type() == tvtObject) {
		if (v1.AsObjectNoAddRef() == v2.AsObjectNoAddRef())
			return true;

		tTJSVariantClosure &o1 = v1.AsObjectClosureNoAddRef();
		tTJSVariantClosure &o2 = v2.AsObjectClosureNoAddRef();

		// 関数どうしなら特別扱いで関数比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE)
			return v1.DiscernCompare(v2);

		// Arrayどうしなら全項目を比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			// 長さが一致していなければ比較失敗
			tTJSVariant o1Count, o2Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			(void)o2.PropGet(0, L"count", &countHint, &o2Count, NULL);
			if (! o1Count.DiscernCompare(o2Count))
				return false;
			// 全項目を順番に比較
			tjs_int count = o1Count;
			tTJSVariant o1Val, o2Val;
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &o1Val, NULL);
				(void)o2.PropGetByNum(TJS_IGNOREPROP, i, &o2Val, NULL);
				if (! equalStructNumericLoose(o1Val, o2Val))
					return false;
			}
			return true;
		}

		// Dictionaryどうしなら全項目を比較
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			// 項目数が一致していなければ比較失敗
			tjs_int o1Count, o2Count;
			(void)o1.GetCount(&o1Count, NULL, NULL, NULL);
			(void)o2.GetCount(&o2Count, NULL, NULL, NULL);
			if (o1Count != o2Count)
				return false;
			// 全項目を順番に比較
			DictMemberCompareNumericLooseCaller *caller = new DictMemberCompareNumericLooseCaller(o2);
			tTJSVariantClosure closure(caller);
			tTJSVariant(o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL));
			bool result = caller->match;
			caller->Release();
			return result;
		}
	}

	// 数字の場合は
	if ((v1.Type() == tvtInteger || v1.Type() == tvtReal)
		&& (v2.Type() == tvtInteger || v2.Type() == tvtReal))
		return v1.NormalCompare(v2);

	return v1.DiscernCompare(v2);
}

//----------------------------------------------------------------------
// 全配列・辞書巡回
tjs_error TJS_INTF_METHOD
ScriptsAdd::foreach(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure &obj = param[0]->AsObjectClosureNoAddRef();
	tTJSVariantClosure &funcClosure = param[1]->AsObjectClosureNoAddRef();

	// 実行対象関数を選択
	// 無名関数なら this コンテキストで動作させる
	iTJSDispatch2 *func     = funcClosure.Object;
	iTJSDispatch2 *functhis = funcClosure.ObjThis;
	if (functhis == 0) {
		functhis = objthis;
	}

	// 配列の場合
	if (obj.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {

		tTJSVariant key, value;
		tTJSVariant **paramList = new tTJSVariant *[numparams];
		paramList[0] = &key;
		paramList[1] = &value;
		for (tjs_int i = 2; i < numparams; i++)
			paramList[i] = param[i];

		tTJSVariant arrayCount;
		(void)obj.PropGet(0, L"count", &countHint, &arrayCount, NULL);
		tjs_int count = arrayCount;

		tTJSVariant breakResult;
		for (tjs_int i = 0; i < count; i++) {
			key = i;
			breakResult.Clear();
			(void)obj.PropGetByNum(TJS_IGNOREPROP, i, &value, NULL);
			(void)func->FuncCall(0, NULL, NULL, &breakResult, numparams, paramList, functhis);
			if (breakResult.Type() != tvtVoid) {
				break;
			}
		}
		if (result) {
			*result = breakResult;
		}
		
		delete[] paramList;

	} else {

		tTJSVariant **paramList = new tTJSVariant *[numparams];
		for (tjs_int i = 2; i < numparams; i++)
			paramList[i] = param[i];

		DictIterateCaller *caller = new DictIterateCaller(func, functhis, paramList, numparams);
		tTJSVariantClosure closure(caller);
		obj.EnumMembers(TJS_IGNOREPROP, &closure, NULL);
		if (result) {
			*result = caller->breakResult;
		}
		caller->Release();

		delete[] paramList;
	}
	return TJS_S_OK;
}


/**
 * octet の MD5ハッシュ値の取得
 * @param octet 対象オクテット
 * @return ハッシュ値（32文字の16進数ハッシュ文字列（小文字））
 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getMD5HashString(tTJSVariant *result,
							 tjs_int numparams,
							 tTJSVariant **param,
							 iTJSDispatch2 *objthis) {
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSVariantOctet *octet = param[0]->AsOctetNoAddRef();

	TVP_md5_state_t st;
	TVP_md5_init(&st);
	TVP_md5_append(&st, octet->GetData(), (int)octet->GetLength());
	
	tjs_uint8 buffer[16];
	TVP_md5_finish(&st, buffer);

	tjs_char ret[32+1], *hex = TJS_W("0123456789abcdef");
	for (tjs_int i=0; i<16; i++) {
		ret[i*2  ] = hex[(buffer[i] >> 4) & 0xF];
		ret[i*2+1] = hex[(buffer[i]     ) & 0xF];
	}
	ret[32] = 0;
	if (result) *result = ttstr(ret);
	return TJS_S_OK;
}


//----------------------------------------------------------------------
// 辞書の要素を全cloneするCaller
class DictMemberCloneCaller : public tTJSDispatch
{
public:
	DictMemberCloneCaller(iTJSDispatch2 *dict) : dict(dict) {};
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		tTJSVariant value = ScriptsAdd::clone(*param[2]);
		dict->PropSet(TJS_MEMBERENSURE|(tjs_int)*param[1], param[0]->GetString(), 0, &value, dict);
		if (result)
			*result = true;
		return TJS_S_OK;
	}
protected:
	iTJSDispatch2 *dict;
};

//----------------------------------------------------------------------
// 構造体比較関数
tTJSVariant
ScriptsAdd::clone(tTJSVariant obj)
{
	// タイプがオブジェクトなら細かく判定
	if (obj.Type() == tvtObject) {

		tTJSVariantClosure &o1 = obj.AsObjectClosureNoAddRef();
		
		// Arrayの複製
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			iTJSDispatch2 *array = TJSCreateArrayObject();
			tTJSVariant o1Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			tjs_int count = o1Count;
			tTJSVariant val;
			tTJSVariant *args[] = {&val};
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &val, NULL);
				val = ScriptsAdd::clone(val);
				static tjs_uint addHint = 0;
				(void)array->FuncCall(0, TJS_W("add"), &addHint, 0, 1, args, array);
			}
			tTJSVariant result(array, array);
			array->Release();
			return result;
		}
		
		// Dictionaryの複製
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			DictMemberCloneCaller *caller = new DictMemberCloneCaller(dict);
			tTJSVariantClosure closure(caller);
			o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL);
			caller->Release();
			tTJSVariant result(dict, dict);
			dict->Release();
			return result;
		}

		// cloneメソッドの呼び出しに成功すればそれを返す
		tTJSVariant result;
		static tjs_uint cloneHint = 0;
		if (o1.FuncCall(0, L"clone", &cloneHint, &result, 0, NULL, NULL)== TJS_S_TRUE) {
			return result;
		}
	}
	
	return obj;
}

NCB_ATTACH_CLASS(ScriptsAdd, Scripts) {
	RawCallback("getObjectKeys", &ScriptsAdd::getKeys, TJS_STATICMEMBER);
	RawCallback("getObjectCount", &ScriptsAdd::getCount, TJS_STATICMEMBER);
	NCB_METHOD(getObjectContext);
	NCB_METHOD(isNullContext);
	NCB_METHOD(equalStruct);
	NCB_METHOD(equalStructNumericLoose);
	RawCallback("foreach", &ScriptsAdd::foreach, TJS_STATICMEMBER);
	RawCallback("getMD5HashString", &ScriptsAdd::getMD5HashString, TJS_STATICMEMBER);
	NCB_METHOD(clone);
};

NCB_ATTACH_FUNCTION(rehash, Scripts, TJSDoRehash);
