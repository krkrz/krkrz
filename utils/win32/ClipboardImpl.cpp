//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Clipboard Class interface
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "ClipboardImpl.h"

#include <Clipbrd.hpp>
#include "TLogViewer.h"

//---------------------------------------------------------------------------
// clipboard related functions
//---------------------------------------------------------------------------
bool TVPClipboardHasFormat(tTVPClipboardFormat format)
{
	switch(format)
	{
	case cbfText:
		return Clipboard()->HasFormat(CF_TEXT) ||
			Clipboard()->HasFormat(CF_UNICODETEXT); // ANSI text or UNICODE text
	default:
		return false;
	}
}
//---------------------------------------------------------------------------
void TVPClipboardSetText(const ttstr & text)
{
	// this is already implemented in TLogViewer
	TVPCopyToClipboard(text);
}
//---------------------------------------------------------------------------
bool TVPClipboardGetText(ttstr & text)
{
	if(!OpenClipboard(Application->Handle)) return false;

	bool result = false;
	try
	{
		// select CF_UNICODETEXT or CF_TEXT
		UINT formats[2] = { CF_UNICODETEXT, CF_TEXT};
		int format = GetPriorityClipboardFormat(formats, 2);

		if(format == CF_UNICODETEXT)
		{
			// try to read unicode text
			HGLOBAL hglb = (HGLOBAL)GetClipboardData(CF_UNICODETEXT);
			if(hglb != NULL)
			{
				const tjs_char *p = (const tjs_char *)GlobalLock(hglb);
				if(p)
				{
					try
					{
						text = ttstr(p);
						result = true;
					}
					catch(...)
					{
						GlobalUnlock(hglb);
						throw;
					}
					GlobalUnlock(hglb);
				}
			}
		}
		else if(format == CF_TEXT)
		{
			// try to read ansi text
			HGLOBAL hglb = (HGLOBAL)GetClipboardData(CF_TEXT);
			if(hglb != NULL)
			{
				const char *p = (const char *)GlobalLock(hglb);
				if(p)
				{
					try
					{
						text = ttstr(p);
						result = true;
					}
					catch(...)
					{
						GlobalUnlock(hglb);
						throw;
					}
					GlobalUnlock(hglb);
				}
			}
		}
	}
	catch(...)
	{
		CloseClipboard();
		throw;
	}
	CloseClipboard();

	return result;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNC_Clipboard
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_Clipboard::CreateNativeInstance()
{
	return NULL;
}
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Clipboard()
{
	tTJSNativeClass *cls = new tTJSNC_Clipboard();

	return cls;
}
//---------------------------------------------------------------------------
