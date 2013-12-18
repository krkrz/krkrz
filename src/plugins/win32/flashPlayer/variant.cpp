#include <windows.h>
#include <DispEx.h>
#include "tp_stub.h"

/**
 * VARIANT ‚ð tTJSVariant ‚ÉŠi”[‚·‚é
 * @param result •ÏŠ·æ
 * @param variant •ÏŠ·Œ³
 */
void
storeVariant(tTJSVariant &result, VARIANT &variant)
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
				// XXX variant ‚É‚Ç‚¤“ü‚ê‚æ‚¤H
				SafeArrayUnaccessData(psa);
			}
		}
		break;
	case VT_UNKNOWN:
	case VT_DISPATCH:
		result = (iTJSDispatch2*)NULL;
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
		result = (iTJSDispatch2*)NULL;
		break;
	case (VT_BYREF | VT_VARIANT):
		storeVariant(result, *variant.pvarVal);
	default:
		;//log(L"unkown result type");
	}
}
