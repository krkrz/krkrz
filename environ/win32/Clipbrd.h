
#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include <windows.h>

class TClipboard {
public:
	bool HasFormat( UINT t ) {
		bool result = false;
		if( OpenClipboard(0) ) {
			result = 0!=IsClipboardFormatAvailable(t);
			CloseClipboard();
		}
		return result;
	}
};

extern class TClipboard* Clipboard;

inline class TClipboard* GetClipboard() { return Clipboard; }

inline void TVPCopyToClipboard(const ttstr & unicode)
{
	if( ::OpenClipboard(0) ) {
		HGLOBAL ansihandle = NULL;
		HGLOBAL unicodehandle = NULL;
		try
		{
			// store ANSI string
			tstring ansistr = unicode.AsStdString();
			int ansistrlen = (ansistr.length() + 1)*sizeof(TCHAR);
			ansihandle = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, ansistrlen);
			if(!ansihandle) throw std::exception("copying to clipboard failed.");

			TCHAR *mem = (TCHAR*)GlobalLock(ansihandle);
			if(mem) _tcsncpy_s(mem, ansistrlen, ansistr.c_str(),ansistrlen);
			GlobalUnlock(ansihandle);

			::SetClipboardData( CF_TEXT, ansihandle );
			ansihandle = NULL;

			// store UNICODE string
			unicodehandle = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
				(unicode.GetLen() + 1) * sizeof(tjs_char));
			if(!unicodehandle)  throw std::exception("copying to clipboard failed.");

			tjs_char *unimem = (tjs_char*)GlobalLock(unicodehandle);
			if(unimem) TJS_strcpy(unimem, unicode.c_str());
			GlobalUnlock(unicodehandle);

			::SetClipboardData( CF_UNICODETEXT, unicodehandle );
			unicodehandle = NULL;

		}
		catch(...)
		{
			if(ansihandle) GlobalFree(ansihandle);
			if(unicodehandle) GlobalFree(unicodehandle);
			::CloseClipboard();
			throw;
		}
		::CloseClipboard();
	}
}
#endif // __CLIPBOARD_H__
