#include "ncbind/ncbind.hpp"

#ifdef _DEBUG
#define dm(msg) TVPAddLog(msg)
#else
#define dm(msg)
#endif

class SystemRegistory
{
public:
	SystemRegistory(){}

	static tjs_error TJS_INTF_METHOD writeRegistory(
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
};

NCB_ATTACH_CLASS(SystemRegistory, System)
{
	RawCallback("writeRegValue", &Class::writeRegistory, 0);
};