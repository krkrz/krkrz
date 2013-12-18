#include "IDispatchWrapper.hpp"
#include <stdarg.h>

static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

// {125DF12C-67F5-4d9e-BFD3-6C59B0348952}
static const GUID IID_iTJSDispatch2 = 
{ 0x125df12c, 0x67f5, 0x4d9e, { 0xbf, 0xd3, 0x6c, 0x59, 0xb0, 0x34, 0x89, 0x52 } };

// オブジェクト名の対応をとるためのマッピング情報
// とりあえずこのクラスに閉じるってことで static 保持で対応

#pragma warning(disable: 4786)
#include <map>
using namespace std;
static map<ttstr,int> methodMap;
static map<int,ttstr> methodIdMap;

/**
 * 名前から一意の数値をえる
 * @param name 名前
 * @return アトム値
 */
static int
getAtom(const tjs_char *name)
{
	ttstr atomName(name);
	map<ttstr,int>::const_iterator n = methodMap.find(atomName);
	if (n != methodMap.end()) {
		return n->second;
	}
	int ret = methodMap.size() + 1;
	methodMap[atomName] = ret;
	methodIdMap[ret] = atomName;
	return ret;
}

/**
 * アトム値から名前を得る
 * @param atom アトム値
 * @return 名前
 */
static const tjs_char *
getName(int atom)
{
	map<int,ttstr>::const_iterator n = methodIdMap.find(atom);
	if (n != methodIdMap.end()) {
		return n->second.c_str();	
	}
	return NULL;
}

/**
 * VARIANT を tTJSVariant に格納する
 * @param result 変換先
 * @param variant 変換元
 */
void
IDispatchWrapper::storeVariant(tTJSVariant &result, VARIANT &variant)
{
	result.Clear();
	switch (variant.vt) {
	case VT_NULL:
		result = (iTJSDispatch2*)NULL;
		break;
	case VT_I8:
		result = variant.llVal;
		break;
	case VT_I4:
		result = (tjs_int32)variant.lVal;
		break;
	case VT_UI1:
		result = (tjs_int32)variant.bVal;
		break;
	case VT_I2:
		result = (tjs_int32)variant.iVal;
		break;
	case VT_R4:
		result = (double)variant.fltVal;
		break;
	case VT_R8:
		result = variant.dblVal;
		break;
	case VT_BOOL:
		result = (variant.boolVal == VARIANT_TRUE);
		break;
	case VT_BSTR:
		result = variant.bstrVal;
		break;
	case VT_ARRAY | VT_UI1:
		{
			SAFEARRAY *psa = variant.parray;
			unsigned char *p;
			if (SUCCEEDED(SafeArrayAccessData(psa, (LPVOID*)&p))) {
				// p;
				//psa->rgsabound->cElements;
				// XXX variant にどう入れよう？
				SafeArrayUnaccessData(psa);
			}
		}
		break;
	case VT_UNKNOWN:
	case VT_DISPATCH:
		if (variant.punkVal) {
			iTJSDispatch2 *obj = NULL;
			if (variant.punkVal->QueryInterface(IID_iTJSDispatch2, (void**)&obj) == S_OK) {
				result = obj;
				obj->Release();
			} else {
				IDispatch *dispatch = NULL;
				if (variant.punkVal->QueryInterface(IID_IDispatch, (void**)&dispatch) == S_OK) {
					iTJSDispatch2 *obj = new iTJSDispatch2Wrapper(dispatch);
					result = obj;
					obj->Release();
					dispatch->Release();
				}
			}
		} else {
			result = (iTJSDispatch2*)NULL;
		}
		break;
	case VT_BYREF | VT_I8:
		result = *variant.pllVal;
		break;
	case VT_BYREF | VT_I4:
		result = (tjs_int32)*variant.plVal;
		break;
	case VT_BYREF | VT_UI1:
		result = (tjs_int32)*variant.pbVal;
		break;
	case VT_BYREF | VT_I2:
		result = (tjs_int32)*variant.piVal;
		break;
	case VT_BYREF | VT_R4:
		result = *variant.pfltVal;
		break;
	case VT_BYREF | VT_R8:
		result = *variant.pdblVal;
		break;
	case VT_BYREF | VT_BOOL:
		result = (*variant.pboolVal == VARIANT_TRUE);
		break;
	case VT_BYREF | VT_BSTR:
		result = *variant.pbstrVal;
		break;
	case VT_BYREF | VT_ARRAY | VT_UI1:
		{
			SAFEARRAY *psa = *(variant.pparray);
			const tjs_uint8 *p;
			if (SUCCEEDED(SafeArrayAccessData(psa, (LPVOID*)&p))) {
				result = tTJSVariant(p, psa->rgsabound->cElements);
				SafeArrayUnaccessData(psa);
			}
		}
		break;
	case VT_BYREF | VT_UNKNOWN:
	case VT_BYREF | VT_DISPATCH:
		if (*(variant.ppunkVal)) {
			iTJSDispatch2 *obj = NULL;
			if ((*(variant.ppunkVal))->QueryInterface(IID_iTJSDispatch2, (void**)&obj) == S_OK) {
				result = obj;
				obj->Release();
			} else {
				IDispatch *dispatch = NULL;
				if ((*(variant.ppunkVal))->QueryInterface(IID_IDispatch, (void**)&dispatch) == S_OK) {
					iTJSDispatch2 *obj = new iTJSDispatch2Wrapper(dispatch);
					result = obj;
					obj->Release();
					dispatch->Release();
				}
			}
		} else {
			result = (iTJSDispatch2*)NULL;
		}
		break;
	case (VT_BYREF | VT_VARIANT):
		storeVariant(result, *variant.pvarVal);
	default:
		;//log(L"unkown result type");
	}
}

/**
 * tTJSVariant を VARIANT に格納する
 * @param result 格納先
 * @param variant 格納元
 */
void
IDispatchWrapper::storeVariant(VARIANT &result, tTJSVariant &variant)
{
	VariantClear(&result);
	switch (variant.Type()) {
	case tvtVoid:
		//log(L"void");
		break;
	case tvtObject:
		// オブジェクト
		//log(L"object");
		{
			switch (variant.Type()) {
			case tvtObject:
				V_VT(&result)       = VT_DISPATCH;
				V_DISPATCH(&result) = new IDispatchWrapper(variant.AsObjectNoAddRef());
				break;
			default:
				V_VT(&result) = VT_NULL;
				break;
			}
		}
		break;
	case tvtString:
		//log(L"string");
		// 文字列
		V_VT(&result) = VT_BSTR;
		V_BSTR(&result) = SysAllocString(variant.GetString());
		break;
	case tvtOctet:
		//log(L"octet");
		// オクテット列
		{
			tTJSVariantOctet *octet = variant.AsOctetNoAddRef();
			if (octet) {
				SAFEARRAY *psa;
				SAFEARRAYBOUND sabound;
				sabound.lLbound = 0;
				sabound.cElements = octet->GetLength();
				if ((psa = SafeArrayCreate(VT_UI1, 1, &sabound))) {
					// データをコピーする
					unsigned char *p;
					if (SUCCEEDED(SafeArrayAccessData(psa, (LPVOID*)&p))) {
						memcpy(p, octet->GetData(), octet->GetLength());
						SafeArrayUnaccessData(psa);
						V_VT(&result) = VT_ARRAY | VT_UI1;
						V_ARRAY(&result) = psa;
					} else {
						SafeArrayDestroy(psa);
					}
				} else {
					// メモリ不足例外
				}
			}
		}
		break;
	case tvtInteger:
		// COM は 64bit をサポートしていない!!
		// ということで COM に渡す引数で整数値は 32bit を超えないように注意すること（涙）
		//log(L"integer");
		V_VT(&result) = VT_I4;
		V_I4(&result) = (int)(variant);
		break;
	case tvtReal:
		//log(L"real");
		V_VT(&result) = VT_R8;
		V_R8(&result) = (double)(variant);
		break;
	}
}

//----------------------------------------------------------------------------
// コンストラクタ・デストラクタ


IDispatchWrapper::IDispatchWrapper(iTJSDispatch2 *obj) 
{
	refCount = 1;
	this->obj = obj;
	obj->AddRef();
}

IDispatchWrapper::~IDispatchWrapper()
{
	obj->Release();
}

int
IDispatchWrapper::Construct(iTJSDispatch2 *obj, VARIANT *pvarResult, int argc, VARIANT *argv, tjs_error &err)
{
	//log(L"construct");

	int ret = S_OK;
	
	// 引数変換
	tTJSVariant **args = new tTJSVariant*[argc];
	for (int i=0;i<argc;i++) {
		args[i] = new tTJSVariant();
		storeVariant(*args[i], argv[argc - i - 1]);
	}
	
	// コンストラクタ呼び出し
	iTJSDispatch2 *instance = NULL;
	try {
		if (TJS_SUCCEEDED(err = Try_iTJSDispatch2_CreateNew(obj, 0, NULL, NULL, &instance, argc, args, obj))) {
			if (instance) {
				if (pvarResult) {
					V_VT(pvarResult)       = VT_DISPATCH;
					V_DISPATCH(pvarResult) = new IDispatchWrapper(instance);
				}
				instance->Release();
			} else {
				if (pvarResult) {
					V_VT(pvarResult) = VT_NULL;
				}
			}
		}
	} catch(...) {
		ret = DISP_E_EXCEPTION;
	}
	
	// 引数破棄
	for (int i=0;i<argc;i++) {
		delete args[i];
	}
	delete[] args;

	return ret;
}

HRESULT
IDispatchWrapper::InvokeEx(iTJSDispatch2 *obj, const tjs_char *memberName, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pei, DISPID id)
{
	if (!pdispparams) return E_POINTER;
	
	// 引数パラメータ設定
	int      argc = pdispparams->cArgs;
	VARIANT *argv = pdispparams->rgvarg;

	HRESULT ret = S_OK;
	tjs_error err;
	ttstr errmesg;

	if ((wFlags & DISPATCH_CONSTRUCT)) {

		// コンストラクタ
		ret = Construct(obj, pvarResult, argc, argv, err);
		
	} else if ((wFlags & DISPATCH_METHOD)) {

		// メソッド呼び出し
		//log(L"FuncCall %ls", memberName);
		
		// 引数変換
		tTJSVariant **args = new tTJSVariant*[argc];
		for (int i=0;i<argc;i++) {
			args[i] = new tTJSVariant();
			storeVariant(*args[i], argv[argc - i - 1]);
		}
		
		// オブジェクト実行用のスクリプトクラスを生成
		tTJSVariant result;
		try {
			if (TJS_SUCCEEDED(err = Try_iTJSDispatch2_FuncCall(obj, 0, memberName, (tjs_uint32*)&id, &result, argc, args, obj))) {
				if (pvarResult) {
					storeVariant(*pvarResult, result);
				}
			}
		} catch(const tTVPExceptionDesc &desc) {
			errmesg = desc.message.IsEmpty() ? desc.type : desc.message;
			ret = DISP_E_EXCEPTION;
		} catch(...) {
			ret = DISP_E_EXCEPTION;
		}
		
		// 引数破棄
		for (int i=0;i<argc;i++) {
			delete args[i];
		}
		delete[] args;
		
	} else if ((wFlags & DISPATCH_PROPERTYGET)) {
		
		//log(L"PropGet %ls", memberName);
		
		tTJSVariant result;
		try {
			if (TJS_SUCCEEDED(err = Try_iTJSDispatch2_PropGet(obj, 0, memberName, (tjs_uint32*)&id, &result, obj))) {
				if (pvarResult) {
					storeVariant(*pvarResult, result);
				}
			}
		} catch(const tTVPExceptionDesc &desc) {
			errmesg = desc.message.IsEmpty() ? desc.type : desc.message;
			ret = DISP_E_EXCEPTION;
		} catch(...)    {
			ret = DISP_E_EXCEPTION;
		}
	} else if ((wFlags & DISPATCH_PROPERTYPUT) && argc > 0) {

		//log(L"PropPut %ls", memberName);
		
		tTJSVariant arg;
		storeVariant(arg, argv[0]);
		try {
			err = Try_iTJSDispatch2_PropSet(obj, TJS_MEMBERENSURE, memberName, (tjs_uint32*)&id, &arg, obj);
		} catch(const tTVPExceptionDesc &desc) {
			errmesg = desc.message.IsEmpty() ? desc.type : desc.message;
			ret = DISP_E_EXCEPTION;
		} catch(...) {
			ret = DISP_E_EXCEPTION;
		}
	}
	
	if (ret == DISP_E_EXCEPTION) {
		if (pei) {
			memset(pei, 0, sizeof(EXCEPINFO));
			pei->wCode = 0x0201;
			pei->bstrSource = SysAllocString(L"TJScript");
			pei->bstrDescription = SysAllocString(errmesg.c_str());
			pei->scode = DISP_E_EXCEPTION;
		}
	} else if (TJS_FAILED(err)) {
		switch (err) {
		case TJS_E_MEMBERNOTFOUND:
			ret = DISP_E_MEMBERNOTFOUND;
			break;
		case TJS_E_BADPARAMCOUNT:
			ret = DISP_E_BADPARAMCOUNT;
			break;
		case TJS_E_INVALIDPARAM:
			ret = DISP_E_BADVARTYPE;
			break;
		case TJS_E_NOTIMPL:
		case TJS_E_INVALIDTYPE:
		case TJS_E_INVALIDOBJECT:
		case TJS_E_ACCESSDENYED:
		case TJS_E_NATIVECLASSCRASH:
		default:
			ret = S_FALSE;
		}
	}
	
	return ret;
}

//----------------------------------------------------------------------------
// IUnknown 実装

HRESULT STDMETHODCALLTYPE
IDispatchWrapper::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) {
	if (riid == IID_iTJSDispatch2) {
		if (ppvObject == NULL)
			return E_POINTER;
		*ppvObject = (void*)obj;
		obj->AddRef();
		return S_OK;
	} else if (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IDispatchEx) {
		if (ppvObject == NULL)
			return E_POINTER;
		*ppvObject = this;
		AddRef();
		return S_OK;
	} else {
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

ULONG STDMETHODCALLTYPE
IDispatchWrapper::AddRef()
{
	refCount++;
	return refCount;
}
	
ULONG STDMETHODCALLTYPE
IDispatchWrapper::Release()
{
	int ret = --refCount;
	if (ret <= 0) {
		delete this;
		ret = 0;
	}
	return ret;
}

//-------------------------------------------------------------------
// IDispatch 実装

HRESULT
IDispatchWrapper::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	HRESULT ret = S_OK;
	UINT i;
	for (i=0; i<cNames; i++) {
		const tjs_char *memberName = rgszNames[i];
		if (true) {
			rgdispid[i] = getAtom(memberName);
		} else {
			rgdispid[i] = DISPID_UNKNOWN;
			ret = DISP_E_UNKNOWNNAME;
		}
	}
	return ret;
}

HRESULT
IDispatchWrapper::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
						 DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	return InvokeEx(dispidMember, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, NULL);
}

HRESULT
IDispatchWrapper::GetDispID( 
	/* [in] */ BSTR bstrName,
	/* [in] */ DWORD grfdex,
	/* [out] */ DISPID *pid)
{
	const tjs_char *memberName = bstrName;
	if (true) {
		*pid = getAtom(memberName);
		return S_OK;
	} else {
		*pid = DISPID_UNKNOWN;
		return DISP_E_UNKNOWNNAME;
	}
}

HRESULT
IDispatchWrapper::InvokeEx( 
	/* [in] */ DISPID id,
	/* [in] */ LCID lcid,
	/* [in] */ WORD wFlags,
	/* [in] */ DISPPARAMS *pdispparams,
	/* [out] */ VARIANT *pvarResult,
	/* [out] */ EXCEPINFO *pei,
	/* [unique][in] */ IServiceProvider *pspCaller)
{
	return InvokeEx(obj, id == DISPID_VALUE ? NULL : getName(id), wFlags, pdispparams, pvarResult, pei, id);
}

HRESULT
IDispatchWrapper::DeleteMemberByDispID( 
		/* [in] */ DISPID id)
{
	const tjs_char *memberName = getName(id);
	HRESULT ret = S_FALSE;
	if (memberName) {
		try {
			if (TJS_SUCCEEDED(Try_iTJSDispatch2_DeleteMember(obj, TJS_MEMBERMUSTEXIST, memberName, (tjs_uint32*)&id, obj))) {
				ret = S_OK;
			}
		} catch(...)  {
			log(L"DeleteMember で例外");
		}
	}
	return ret;
}

HRESULT
IDispatchWrapper::GetMemberName( 
	/* [in] */ DISPID id,
	/* [out] */ BSTR *pbstrName)
{
	const tjs_char *memberName = getName(id);
	if (true) {
		if (pbstrName) {
			*pbstrName = SysAllocString(memberName);
		}
		return S_OK;
	} else {
		return DISP_E_UNKNOWNNAME;
	}
}

class GetEnumCaller : public tTJSDispatch /** EnumMembers 用 */
{
protected:
	vector<DISPID> *methodEnums;
public:
	GetEnumCaller(vector<DISPID> *methodEnums) : methodEnums(methodEnums) {};
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		//log(L"enumMemberCallback");
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER)) {
				methodEnums->push_back(getAtom(param[0]->GetString()));
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
};

HRESULT
IDispatchWrapper::GetNextDispID( 
	/* [in] */ DWORD grfdex,
	/* [in] */ DISPID id,
	/* [out] */ DISPID *pid)
{
	if (id == DISPID_STARTENUM) {
		methodEnums.clear();
		methodEnumsCount = 0;
		tTJSVariantClosure closure(new GetEnumCaller(&methodEnums));
		try {
			Try_iTJSDispatch2_EnumMembers(obj, TJS_ENUM_NO_VALUE, &closure, obj);
		} catch (...) {
			log(L"EnumMembers で例外");
		}
		closure.Release();
	}
	if (methodEnums.size() > methodEnumsCount) {
		*pid = methodEnums[methodEnumsCount++];
		return S_OK;
	} else {
		return S_FALSE;
	}
}

// -------------------------------------------------------------------------------------------

/**
 * 実行処理本体
 * @param dispatch IDispatch
 * @param wFlags 実行種別
 * @param membername メンバ名
 * @param result 結果格納先
 * @param numparams パラメータ数
 * @param param パラメータ
 */
tjs_error
iTJSDispatch2Wrapper::Invoke(IDispatch *dispatch,
							 DWORD wFlags,
							 const tjs_char *membername,
							 tTJSVariant *result,
							 tjs_int numparams,
							 tTJSVariant **param
							 )
{
	LCID lcid = LOCALE_SYSTEM_DEFAULT;

	DISPID dispId;
	HRESULT hr;

	if (membername == NULL) {
		if (wFlags & DISPATCH_METHOD) {
			dispId = DISPID_VALUE;
		} else {
			return TJS_E_NOTIMPL;
		}
	} else {
		// DISPID の取得
		BSTR wcmdname = SysAllocString(membername);
		hr = dispatch->GetIDsOfNames(IID_NULL, &wcmdname, 1, lcid, &dispId);
		SysFreeString(wcmdname);
		if (FAILED(hr)) {
			return TJS_E_MEMBERNOTFOUND;
		}
	}
	
	// 呼び出し用パラメータ変換
	DISPPARAMS dispParams;
	memset(&dispParams, 0, sizeof(DISPPARAMS));
	dispParams.cArgs = numparams;
	dispParams.rgvarg = (VARIANT*)malloc(sizeof VARIANTARG * numparams);
	for (int i=0; i<numparams; i++) {
		VariantInit(&dispParams.rgvarg[numparams - i - 1]);
		IDispatchWrapper::storeVariant(dispParams.rgvarg[numparams - i - 1], *param[i]);
	}
	
	// PROPERTYPUT 用特殊処理
	DISPID dispIDParam = DISPID_PROPERTYPUT;
	if (wFlags & DISPATCH_PROPERTYPUT) {
		dispParams.cNamedArgs = 1;
		dispParams.rgdispidNamedArgs = &dispIDParam;
	}
	
	// 呼び出し処理
	VARIANT invokeResult;
	VariantInit(&invokeResult);
	EXCEPINFO excepInfo;
	hr = dispatch->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, wFlags, &dispParams, &invokeResult, &excepInfo, NULL);
	
	// 後片付け
	for (int i=0; i<numparams; i++) {
		VariantClear(&dispParams.rgvarg[i]);
	}
	free(dispParams.rgvarg);
	
	// エラー処理
	tjs_error ret;
	if (SUCCEEDED(hr)) {
		if (result) {
			IDispatchWrapper::storeVariant(*result, invokeResult);
		}
		VariantClear(&invokeResult);
		ret = TJS_S_OK;
	} else {
		// XXX 例外どうしよう
		switch (hr) {
		case DISP_E_MEMBERNOTFOUND:
			ret = TJS_E_MEMBERNOTFOUND;
			//log(TJS_W("member not found"));
			break;
		case DISP_E_BADPARAMCOUNT:
			ret = TJS_E_BADPARAMCOUNT;
			//log(TJS_W("bad param count"));
			break;
		case DISP_E_BADVARTYPE:
			ret = TJS_E_INVALIDPARAM;
			//log(TJS_W("bad var type"));
			break;
		case DISP_E_EXCEPTION:
			{
				ttstr msg = excepInfo.bstrSource;
				msg += L":";
				msg += excepInfo.bstrDescription;
				TVPThrowExceptionMessage(msg.c_str());
			}
			break;
		default:
			log(TJS_W("other error"));
			ret = TJS_E_FAIL;
		}
	}
	return ret;
}
