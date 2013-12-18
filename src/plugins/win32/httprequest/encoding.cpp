#include <windows.h>
#include <comdef.h>
#include <mlang.h>

static bool coInitialized;
static IMultiLanguage *ml = NULL;

void
initEncoding()
{
	coInitialized = SUCCEEDED(CoInitialize(0));
	if (coInitialized) {
		::CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_ALL, IID_IMultiLanguage, (LPVOID*)&ml);
	}
}

void
doneEncoding()
{
	if (coInitialized) {
		if (ml) {
			ml->Release();
			ml = NULL;
		}
		CoUninitialize();
	}
}

/**
 * IANA のエンコード名からエンコーディングコードを取得
 * @param encoding エンコード名
 * @return エンコーディングコード
 */
int
getEncoding(const wchar_t *encoding)
{
	if (encoding && *encoding && ml) {
		MIMECSETINFO csinfo;
		if (SUCCEEDED(ml->GetCharsetInfo(_bstr_t(encoding), &csinfo))) {
			return csinfo.uiInternetEncoding;
		}
	}
	return 65001; // UTF-8
}

UINT
getWCToMBLen(int enc, const wchar_t *wc, UINT wclen)
{	
	UINT mblen = 0;
	if (ml) {
		DWORD mode;
		ml->ConvertStringFromUnicode(&mode, enc, (WCHAR*)wc, &wclen, NULL, &mblen);
	}
	return mblen;
}

void
convWCToMB(int enc, const wchar_t *wc, UINT *wclen, char *mb, UINT *mblen)
{
	if (ml) {
		DWORD mode = 0;
		ml->ConvertStringFromUnicode(&mode, enc, (WCHAR*)wc, wclen, mb, mblen);
	}
}

UINT
getMBToWCLen(int enc, const char *mb, UINT mblen)
{
	UINT wclen = 0;
	if (ml) {
		DWORD mode = 0;
		ml->ConvertStringToUnicode(&mode, enc, (CHAR*)mb, &mblen, NULL, &wclen);
	}
	return wclen;
}

void
convMBToWC(int enc, const char *mb, UINT *mblen, wchar_t *wc, UINT *wclen)
{
	if (ml) {
		DWORD mode = 0;
		ml->ConvertStringToUnicode(&mode, enc, (CHAR*)mb, mblen, wc, wclen);
	}
}
