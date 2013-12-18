#include "ncbind/ncbind.hpp"
#include <errno.h>
#include <string>

#ifdef _DEBUG
#define dm(msg) TVPAddLog(msg)
#else
#define dm(msg)
#endif

static const char *HEX = "0123456789ABCDEF";

struct System
{
	static tjs_error TJS_INTF_METHOD writeRegValue(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis)
	{
		if(numparams < 2)
			return TJS_E_BADPARAMCOUNT;

		// ルートキーを確定
		ttstr		key	= param[0]->AsStringNoAddRef();
		tjs_int		len = key.length();
		ttstr		hkey= "";
		tjs_int		i;
		for(i=0; i<len; i++)
		{
			if(key[i] == '\\')
				break;
			hkey	+= key[i];
		}
		hkey.ToUppserCase();
		dm(hkey);
		HKEY	hKey	= HKEY_CURRENT_USER;
		if(hkey[5] == 'C')
		{
			if(hkey[6] == 'L')
				hKey	= HKEY_CLASSES_ROOT;
			else if(hkey[13] == 'C')
				hKey	= HKEY_CURRENT_CONFIG;
			else if(hkey[23] == 'U')
				hKey	= HKEY_CURRENT_USER;
		}
		else if(hkey[5] == 'L')
			hKey	= HKEY_LOCAL_MACHINE;
		else if(hkey[5] == 'U')
			hKey	= HKEY_USERS;
		else if(hkey[5] == 'P')
			hKey	= HKEY_PERFORMANCE_DATA;
		else if(hkey[5] == 'D')
			hKey	= HKEY_DYN_DATA;

		//	キー名、値名を取り出す
		tjs_int	j;
		for(j=len-1; j>=0; j--)
		{
			if(key[j] == '\\')
				break;
		}
		ttstr	keyname	= "";
		for(i++; i<j; i++)
			keyname	+= key[i];
		ttstr	valname	= "";
		for(j++; j<len; j++)
			valname	+= key[j];
		dm(keyname);
		dm(valname);

		DWORD	dwDisposition;
		LONG	res;
		res	= RegCreateKeyEx(hKey, keyname.c_str(), 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
		if(res != ERROR_SUCCESS)
			return TJS_E_FAIL;

		switch(param[1]->Type())
		{
		case tvtString:
			{
				ttstr	value	= param[1]->AsStringNoAddRef();
				res	= RegSetValueEx(hKey, valname.c_str(), 0, REG_SZ,
					(LPBYTE)value.c_str(), (DWORD)value.length()*2+1);
			}
			break;
		case tvtInteger:
			{
				tjs_uint32	value	= (tjs_uint32)param[1]->AsInteger();
				res	= RegSetValueEx(hKey, valname.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(tjs_uint32));
			}
			break;
		}
		RegCloseKey(hKey);

		return TJS_S_OK;
	}

	// System.readEnvValue
	static tjs_error TJS_INTF_METHOD readEnvValue(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (p[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;
		ttstr name(p[0]->AsStringNoAddRef());
		if (name == TJS_W("")) return TJS_E_INVALIDPARAM;
		if (r) {
			r->Clear();
			DWORD len = ::GetEnvironmentVariableW(name.c_str(), NULL, 0);
			if (!len) return TJS_S_OK;
			
			tjs_char *tmp = new tjs_char[len];
			if (!tmp) return TJS_E_FAIL;
			ZeroMemory(tmp, len);
			DWORD res = ::GetEnvironmentVariableW(name.c_str(), tmp, len);
			//		if (res != len-1) TVPAddImportantLog(TJS_W("環境変数長が一致しません"));
			*r = ttstr(tmp);
			delete[] tmp;
		}
		return TJS_S_OK;
	}

	// System.writeEnvValue
	static tjs_error TJS_INTF_METHOD writeEnvValue(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 2) return TJS_E_BADPARAMCOUNT;
		if (p[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;
		ttstr name(p[0]->AsStringNoAddRef());
		if (name == TJS_W("")) return TJS_E_INVALIDPARAM;
		ttstr value(p[1]->AsStringNoAddRef());
		if (r) {
			r->Clear();
			DWORD len = ::GetEnvironmentVariableW(name.c_str(), NULL, 0);
			if (len >= 0) {
				tjs_char *tmp = new tjs_char[len];
				if (!tmp) return TJS_E_FAIL;
				ZeroMemory(tmp, len);
				::GetEnvironmentVariableW(name.c_str(), tmp, len);
				//		if (res != len-1) TVPAddImportantLog(TJS_W("環境変数長が一致しません"));
				*r = ttstr(tmp);
				delete[] tmp;
			}
		}
		::SetEnvironmentVariableW(name.c_str(), value.c_str());
		return TJS_S_OK;
	}
	
	// System.expandEnvString
	static tjs_error TJS_INTF_METHOD expandEnvString(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (r) {
			ttstr src(p[0]->AsStringNoAddRef());
			r->Clear();
			DWORD len = ::ExpandEnvironmentStrings(src.c_str(), NULL, 0);
			if (!len) return TJS_E_FAIL;
			
			tjs_char *tmp = new tjs_char[len];
			if (!tmp) return TJS_E_FAIL;
			ZeroMemory(tmp, len);
			DWORD res = ::ExpandEnvironmentStrings(src.c_str(), tmp, len);
			//		if (res != len) TVPAddImportantLog(TJS_W("展開長が一致しません"));
			*r = ttstr(tmp);
			delete[] tmp;
		}
		return TJS_S_OK;
	}

	// urlencode処理
	static tjs_error TJS_INTF_METHOD urlencode(tTJSVariant *result,
											   tjs_int numparams,
											   tTJSVariant **param,
											   iTJSDispatch2 *objthis) {
		if (numparams > 0 && result) {
			bool utf8 = !(numparams> 1 && (int)*param[1] == 0);
			ttstr str = *param[0];
			tjs_int len;
			char *dat;
			if (utf8) {
				const tjs_char *s = str.c_str();
				len = TVPWideCharToUtf8String(s, NULL);
				dat = new char [len+1];
				try {
					TVPWideCharToUtf8String(s, dat);
					dat[len] = '\0';
				}
				catch(...)	{
					delete [] dat;
					throw;
				}
			} else {
				len = str.GetNarrowStrLen();
				dat = new char[len+1];
				try {
					str.ToNarrowStr(dat, len+1);
				}
				catch(...)	{
					delete [] dat;
					throw;
				} 
				delete [] dat;
			}
			ttstr os;
			for (int i=0; i<len; i++) {
				char c = dat[i];
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
					(c >= '0' && c <= '9') ||
					c == '-' || c == '_' || c == '.' || c == '~') {
					os += c;
				} else {
					os += (tjs_char) '%';
					os += (tjs_char) HEX[(c >> 4) & 0x0f];
					os += (tjs_char) HEX[c & 0x0f];
				}
			}
			*result = os;
			delete [] dat;
		}
		return TJS_S_OK;
	}

	
	// urldecode処理
	static tjs_error TJS_INTF_METHOD urldecode(tTJSVariant *result,
											   tjs_int numparams,
											   tTJSVariant **param,
											   iTJSDispatch2 *objthis) {

		if (numparams > 0 && result) {
			bool utf8 = !(numparams> 1 && (int)*param[1] == 0);
			ttstr str = *param[0];
			tjs_int len = str.length();
			std::string os;
			for (int i=0;i<len;i++) {
				int ch = str[i];
				if (ch > 0xff) {
					return TJS_E_INVALIDPARAM;
				}
				if (ch == '%') {
					if (i + 2 >= len) {
						return TJS_E_INVALIDPARAM;
					}
					char buf[3];
					buf[0] = (char)str[i+1];
					buf[1] = (char)str[i+2];
					buf[2] = '\0';
					long n = strtol(buf, NULL, 16);
					if (errno == ERANGE) {
						return TJS_E_INVALIDPARAM;
					}
					os += (char)n;
					i+=2;
				} else {
					os += (char)ch;
				}
			}
			if (utf8) {
				const char *s = os.c_str();
				tjs_int len = TVPUtf8ToWideCharString(s, NULL);
				if (len > 0) {
					tjs_char *dat = new tjs_char[len+1];
					try {
						TVPUtf8ToWideCharString(s, dat);
						dat[len] = TJS_W('\0');
					}
					catch(...) {
						delete [] dat;
						throw;
					}
					*result = ttstr(dat);
					delete [] dat;
				}				
			} else {
				*result = ttstr(os.c_str());
			}
		}
		return TJS_S_OK;
	}

	// はいいいえの確認
	static tjs_error TJS_INTF_METHOD confirm(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		ttstr message = *param[0];
		ttstr caption;
		HWND parent = ::TVPGetApplicationWindowHandle();
		if (numparams > 2) {
			iTJSDispatch2 *window = param[2]->AsObjectNoAddRef();
			if (window->IsInstanceOf(0, NULL, NULL, L"Window", window) != TJS_S_TRUE) {
				TVPThrowExceptionMessage(L"InvalidObject");
			}
			tTJSVariant val;
			window->PropGet(0, TJS_W("HWND"), NULL, &val, window);
			parent = reinterpret_cast<HWND>((tjs_int)(val));
		}
		if (numparams > 1) {
			caption = *param[1];
		}
		int ret = ::MessageBox(parent, message.c_str(), caption.c_str(), MB_YESNO);
		if (result) {
			*result = (ret == IDYES);
		}
		return TJS_S_OK;
	}
};

NCB_ATTACH_FUNCTION(writeRegValue,      System, System::writeRegValue);
NCB_ATTACH_FUNCTION(readEnvValue,       System, System::readEnvValue);
NCB_ATTACH_FUNCTION(writeEnvValue,      System, System::writeEnvValue);
NCB_ATTACH_FUNCTION(expandEnvString,    System, System::expandEnvString);
NCB_ATTACH_FUNCTION(urlencode,          System, System::urlencode);
NCB_ATTACH_FUNCTION(urldecode,          System, System::urldecode);
NCB_ATTACH_FUNCTION(getAboutString,     System, TVPGetAboutString);
NCB_ATTACH_FUNCTION(confirm,            System, System::confirm);

