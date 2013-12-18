#ifndef _idispatch_wrapper_
#define _idispatch_wrapper_

#include <windows.h>
#include <DispEx.h>
#include "tp_stub.h"

#pragma warning(disable: 4786)
#include <vector>
using namespace std;

/**
 * iTJSDispatch2 用 IDispatch ラッパー
 */
class IDispatchWrapper : public IDispatchEx
{
public:
	static void	storeVariant(tTJSVariant &result, VARIANT &variant);
	static void storeVariant(VARIANT &result, tTJSVariant &variant);

protected:
	int refCount;
	iTJSDispatch2 *obj;
	vector<DISPID> methodEnums;
	int methodEnumsCount;

	static int Construct(iTJSDispatch2 *obj, VARIANT *pvarResult, int argc, VARIANT *argv, tjs_error &err);
public:
	static HRESULT InvokeEx(iTJSDispatch2 *obj, const tjs_char *methodName, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, DISPID id=0);
	
public:
	IDispatchWrapper(iTJSDispatch2 *obj);
	~IDispatchWrapper();
	
	
	//----------------------------------------------------------------------------
	// IUnknown 実装
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	
	//-------------------------------------------------------------------
	// IDispatch 実装
public:
	STDMETHOD (GetTypeInfoCount) (UINT* pctinfo) { return	E_NOTIMPL; }
	STDMETHOD (GetTypeInfo) (UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {	return	E_NOTIMPL; }
	STDMETHOD (GetIDsOfNames) (REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
	STDMETHOD (Invoke) (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

	//-------------------------------------------------------------------
	// IDispatchEx 実装

	virtual HRESULT STDMETHODCALLTYPE GetDispID( 
		/* [in] */ BSTR bstrName,
		/* [in] */ DWORD grfdex,
		/* [out] */ DISPID *pid);
	
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx( 
		/* [in] */ DISPID id,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [in] */ DISPPARAMS *pdp,
		/* [out] */ VARIANT *pvarRes,
		/* [out] */ EXCEPINFO *pei,
		/* [unique][in] */ IServiceProvider *pspCaller);
	
	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
		/* [in] */ BSTR bstrName,
		/* [in] */ DWORD grfdex) {
		DISPID id;
		HRESULT hr;
		if (SUCCEEDED(hr = GetDispID(bstrName, grfdex, &id))) {
			return DeleteMemberByDispID(id);
		} else {
			return hr;
		}
	}
	
	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
		/* [in] */ DISPID id);
	
	virtual HRESULT STDMETHODCALLTYPE GetMemberProperties( 
		/* [in] */ DISPID id,
		/* [in] */ DWORD grfdexFetch,
		/* [out] */ DWORD *pgrfdex) {
		return E_NOTIMPL;
	}
	
	virtual HRESULT STDMETHODCALLTYPE GetMemberName( 
		/* [in] */ DISPID id,
		/* [out] */ BSTR *pbstrName);
	
	virtual HRESULT STDMETHODCALLTYPE GetNextDispID( 
		/* [in] */ DWORD grfdex,
		/* [in] */ DISPID id,
		/* [out] */ DISPID *pid);
	
	virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent( 
		/* [out] */ IUnknown **ppunk) {
		return E_NOTIMPL;
	}
};

/**
 * IDispatch 用 iTJSDispatch2 ラッパー
 */
class iTJSDispatch2Wrapper : public tTJSDispatch
{
public:
	// IDispatch 用呼び出し処理
	static tjs_error Invoke(IDispatch *dispatch,
							DWORD wFlags,
							const tjs_char *membername,
							tTJSVariant *result,
							tjs_int numparams,
							tTJSVariant **param
							);

protected:
	/// 内部保持用
	IDispatch *obj;

	/// 実行処理
	inline tjs_error Invoke(DWORD wFlags,
							const tjs_char * membername,
							tTJSVariant *result,
							tjs_int numparams,
							tTJSVariant **param) {
		return Invoke(obj, wFlags, membername, result, numparams, param);
	}
	
public:

	/**
	 * コンストラクタ
	 * @param obj IDispatch
	 */
	iTJSDispatch2Wrapper(IDispatch *obj) : obj(obj) {
		obj->AddRef();
	}

	/**
	 * デストラクタ
	 */
	~iTJSDispatch2Wrapper() {
		obj->Release();
	}

	//----------------------------------------------------------------------------
	// iTJSDispatch2 実装
public:
	
	tjs_error TJS_INTF_METHOD
	FuncCall(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis
		) {
		return Invoke(DISPATCH_METHOD|DISPATCH_PROPERTYGET, membername, result, numparams, param);
	}

	tjs_error TJS_INTF_METHOD
	PropGet(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		iTJSDispatch2 *objthis
		) {
		return Invoke(DISPATCH_PROPERTYGET, membername, result, 0, NULL);
	}
	
	tjs_error TJS_INTF_METHOD
	PropSet(
		tjs_uint32 flag,
		const tjs_char *membername,
		tjs_uint32 *hint,
		const tTJSVariant *param,
		iTJSDispatch2 *objthis
		) {
		return Invoke(DISPATCH_PROPERTYPUT, membername, NULL, 1, (class tTJSVariant**)&param);
	}
};


/**
 * IDispatch 直下メソッド用 iTJSDispatch2 ラッパー
 */
class iTJSDispatch2WrapperForMethod : public tTJSDispatch
{
protected:
	IDispatch *dispatch;
	ttstr method;
	
public:
	/// コンストラクタ
	iTJSDispatch2WrapperForMethod(IDispatch *dispatch, const tjs_char *method) : dispatch(dispatch), method(method) {
		if (dispatch) {
			dispatch->AddRef();
		}
	}
	
	/// デストラクタ
	~iTJSDispatch2WrapperForMethod() {
		if (dispatch) {
			dispatch->Release();
		}
	}

	/// 関数呼び出し
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (membername != NULL) {
			return DISP_E_MEMBERNOTFOUND;
		}
		return iTJSDispatch2Wrapper::Invoke(dispatch, DISPATCH_METHOD|DISPATCH_PROPERTYGET, method.c_str(), result, numparams, param);
	}
};

#endif
