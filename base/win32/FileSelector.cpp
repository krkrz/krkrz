//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// File Selector dialog box
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <cderr.h>
#include <commdlg.h>

#include "MsgIntf.h"
#include "StorageImpl.h"
#include "WindowImpl.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"
#include "tjsGlobalStringMap.h"

#include "TVPScreen.h"


//---------------------------------------------------------------------------
// TVPSelectFile related
//---------------------------------------------------------------------------
#define TVP_OLD_OFN_STRUCT_SIZE 76
//---------------------------------------------------------------------------
static tjs_int TVPLastScreenWidth = 0;
static tjs_int TVPLastScreenHeight = 0;
static tjs_int TVPLastOFNLeft = -30000;
static tjs_int TVPLastOFNTop = -30000;
static UINT_PTR APIENTRY TVPOFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam,
	LPARAM lParam)
{
	if(uiMsg == WM_INITDIALOG)
	{
		int left, top;
		HWND parent = GetParent(hdlg);
		if((TVPLastOFNLeft == -30000 && TVPLastOFNTop == -30000) ||
			TVPLastScreenWidth != tTVPScreen::GetWidth() || TVPLastScreenHeight != tTVPScreen::GetHeight() )
		{
			// center the window
			RECT rect;
			GetWindowRect(parent, &rect);
			left = ((tTVPScreen::GetWidth() - rect.right + rect.left) / 2);
			top = ((tTVPScreen::GetHeight() - rect.bottom + rect.top) / 3);
		}
		else
		{
			// set last position
			left = TVPLastOFNLeft;
			top = TVPLastOFNTop;
		}

		TVPLastScreenWidth = tTVPScreen::GetWidth();
		TVPLastScreenHeight = tTVPScreen::GetHeight();

		SetWindowPos(parent, 0,
			left,
			top,
			0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	}
	else if(uiMsg == WM_DESTROY ||
		(uiMsg == WM_NOTIFY && ((OFNOTIFY*)lParam)->hdr.code == CDN_FILEOK))
	{
		HWND parent = GetParent(hdlg);
		RECT rect;
		GetWindowRect(parent, &rect);
		TVPLastOFNLeft = rect.left;
		TVPLastOFNTop = rect.top;
	}
	return 0;
}
//---------------------------------------------------------------------------
static void TVPPushFilterPair(std::vector<tjs_string> &filters, tjs_string filter)
{
	tjs_string::size_type vpos = filter.find_first_of(TJS_W("|"));
	if( vpos != tjs_string::npos )
	{
		tjs_string name = filter.substr(0, vpos);
		tjs_string wild = filter.c_str() + vpos+1;
		filters.push_back(name);
		filters.push_back(wild);
	}
	else
	{
		filters.push_back(filter);
		filters.push_back(filter);
	}
}
//---------------------------------------------------------------------------
bool TVPSelectFile(iTJSDispatch2 *params)
{
	// show open dialog box
	// NOTE: currently this only shows ANSI version of file open dialog.
	tTJSVariant val;
	tjs_char* filter = NULL;
	tjs_char* filename = NULL;
	tjs_string initialdir;
	tjs_string title;
	tjs_string defaultext;
	BOOL result;

	try
	{
		// prepare OPENFILENAME structure

		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = TVPGetModalWindowOwnerHandle();
		if( ofn.hwndOwner == INVALID_HANDLE_VALUE ) {
			ofn.hwndOwner = NULL;
		}
		ofn.hInstance = NULL;

		// set application window position to current window position
		

		// get filter
		ofn.lpstrFilter = NULL;

		static ttstr filter_name(TJSMapGlobalStringMap(TJS_W("filter")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, filter_name.c_str(), filter_name.GetHint(),
			&val, params)))
		{
			std::vector<tjs_string> filterlist;
			if(val.Type() != tvtObject)
			{
				TVPPushFilterPair(filterlist, ttstr(val).AsStdString());
			}
			else
			{
				iTJSDispatch2 * array = val.AsObjectNoAddRef();
				tjs_int count;
				tTJSVariant tmp;
				static ttstr count_name(TJSMapGlobalStringMap(TJS_W("count")));
				if(TJS_SUCCEEDED(array->PropGet(TJS_MEMBERMUSTEXIST,
					count_name.c_str(), count_name.GetHint(), &tmp, array)))
					count = tmp;
				else
					count = 0;

				for(tjs_int i = 0; i < count; i++)
				{
					if(TJS_SUCCEEDED(array->PropGetByNum(TJS_MEMBERMUSTEXIST,
						i, &tmp, array)))
					{
						TVPPushFilterPair(filterlist, ttstr(tmp).AsStdString());
					}
				}
			}

			// create filter buffer
			tjs_int bufsize = 2;
			for(std::vector<tjs_string>::iterator i = filterlist.begin(); i != filterlist.end(); i++)
			{
				bufsize += (tjs_int)(i->length() + 1);
			}

			filter = new tjs_char[bufsize];

			tjs_char* p = filter;
			for(std::vector<tjs_string>::iterator i = filterlist.begin(); i != filterlist.end(); i++)
			{
				TJS_strcpy(p, i->c_str());
				p += i->length() + 1;
			}
			*(p++) = 0;
			*(p++) = 0;

			ofn.lpstrFilter = filter;
		}

		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;

		static ttstr filterIndex_name(TJSMapGlobalStringMap(TJS_W("filterIndex")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, filterIndex_name.c_str(), filterIndex_name.GetHint(), &val, params)))
			ofn.nFilterIndex = (tjs_int)val;
		else
			ofn.nFilterIndex = 0;

		// filenames
		filename = new tjs_char[MAX_PATH + 1];
 		filename[0] = 0;

 		static ttstr name_name(TJSMapGlobalStringMap(TJS_W("name")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, name_name.c_str(), name_name.GetHint(), &val, params)))
		{
			ttstr lname(val);
			if(!lname.IsEmpty())
			{
				lname = TVPNormalizeStorageName(lname);
				TVPGetLocalName(lname);
				tjs_string name = lname.AsStdString();
				TJS_strncpy(filename, name.c_str(), MAX_PATH);
				filename[MAX_PATH] = 0;
			}
		}

		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH + 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;

		// initial dir
		ofn.lpstrInitialDir = NULL;
		static ttstr initialDir_name(TJSMapGlobalStringMap(TJS_W("initialDir")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, initialDir_name.c_str(), initialDir_name.GetHint(), &val, params)))
		{
			ttstr lname(val);
			if(!lname.IsEmpty())
			{
				lname = TVPNormalizeStorageName(lname);
				TVPGetLocalName(lname);
				initialdir = lname.AsStdString();
				ofn.lpstrInitialDir = initialdir.c_str();
			}
		}
	
		// title
		static ttstr title_name(TJSMapGlobalStringMap(TJS_W("title")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, title_name.c_str(), title_name.GetHint(), &val, params)))
		{
			title = ttstr(val).AsStdString();
			ofn.lpstrTitle = title.c_str();
		}
		else
		{
			ofn.lpstrTitle = NULL;
		}

		// flags
		bool issave = false;
		static ttstr save_name(TJSMapGlobalStringMap(TJS_W("save")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, save_name.c_str(), save_name.GetHint(), &val, params)))
			issave = val.operator bool();

		ofn.Flags = OFN_ENABLEHOOK|OFN_EXPLORER|OFN_NOCHANGEDIR|
			OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_ENABLESIZING;


		if(!issave)
			ofn.Flags |= OFN_FILEMUSTEXIST;
		else
			ofn.Flags |= OFN_OVERWRITEPROMPT;

		// default extension
		static ttstr defaultExt_name(TJSMapGlobalStringMap(TJS_W("defaultExt")));
		if(TJS_SUCCEEDED(params->PropGet(TJS_MEMBERMUSTEXIST, defaultExt_name.c_str(), defaultExt_name.GetHint(), &val, params)))
		{
			defaultext = ttstr(val).AsStdString();
			ofn.lpstrDefExt = defaultext.c_str();
		}
		else
		{
			ofn.lpstrDefExt = NULL;
		}

		// hook proc
		ofn.lpfnHook = TVPOFNHookProc;

		// show dialog box
		if(!issave)
			result = GetOpenFileName(&ofn);
		else
			result = GetSaveFileName(&ofn);


		if(!result && CommDlgExtendedError() == CDERR_STRUCTSIZE)
		{
			// for old windows
			// set lStructSize to old Windows' structure size
			ofn.lStructSize = TVP_OLD_OFN_STRUCT_SIZE;
			if(!issave)
				result = GetOpenFileName(&ofn);
			else
				result = GetSaveFileName(&ofn);
		}

		if(result)
		{
			// returns some informations

			// filter index
			val = (tjs_int)ofn.nFilterIndex;
			static ttstr filterIndex_name(TJSMapGlobalStringMap(TJS_W("filterIndex")));
			params->PropSet(TJS_MEMBERENSURE, filterIndex_name.c_str(), filterIndex_name.GetHint(), &val, params);

			// file name
			val = TVPNormalizeStorageName(ttstr(filename));
			static ttstr name_name(TJSMapGlobalStringMap(TJS_W("name")));
			params->PropSet(TJS_MEMBERENSURE, name_name.c_str(), name_name.GetHint(), &val, params);
		}

	}
	catch(...)
	{
		if(filter) delete [] filter;
		if(filename) delete [] filename;
		throw;
	}

	delete [] filter;
	delete [] filename;

	return 0!=result;
}
//---------------------------------------------------------------------------






