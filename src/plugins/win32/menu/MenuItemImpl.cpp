//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "MenuItem" class implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "EventIntf.h"
#include "MenuItemImpl.h"
#include "MsgIntf.h"
#include "WindowIntf.h"

//---------------------------------------------------------------------------
// tTJSNI_MenuItem
//---------------------------------------------------------------------------
tTJSNI_MenuItem::tTJSNI_MenuItem()
{
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_MenuItem::Construct(tjs_int numparams,
	tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	inherited::Construct(numparams, param, tjs_obj);

	// create or attach MenuItem object
	if(Window)
	{
		MenuItem = Window->GetRootMenuItem();
	}
	else
	{
		MenuItem = new TMenuItem(Application);
		MenuItem->OnClick = MenuItemClick;
	}

	// fetch initial caption
	if(!Window && numparams >= 2)
	{
		Caption = *param[1];
		MenuItem->Caption = Caption.AsAnsiString();
	}

	return S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_MenuItem::Invalidate()
{
	bool dodeletemenuitem = (Window == NULL);

	// invalidate inherited
	inherited::Invalidate();  // this sets Owner = NULL

	// delete VCL object
	if(dodeletemenuitem) delete MenuItem, MenuItem = NULL;
}
//---------------------------------------------------------------------------
void __fastcall tTJSNI_MenuItem::MenuItemClick(TObject * sender)
{
	// VCL event handler
	// post to the event queue
	TVPPostInputEvent(new tTVPOnMenuItemClickInputEvent(this));
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::CanDeliverEvents() const
{
	// returns whether events can be delivered
	if(!MenuItem) return false;
	bool enabled = true;
	TMenuItem *item = MenuItem;
	while(item)
	{
		if(!item->Enabled)
		{
			enabled = false;
			break;
		}
		item = item->Parent;
	}
	return enabled;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::Add(tTJSNI_MenuItem * item)
{
	if(MenuItem && item->MenuItem)
	{
		MenuItem->Add(item->MenuItem);
		AddChild((tTJSNI_BaseMenuItem*)item);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::Insert(tTJSNI_MenuItem *item, tjs_int index)
{
	if(MenuItem && item->MenuItem)
	{
		MenuItem->Insert(index, item->MenuItem);
		AddChild((tTJSNI_BaseMenuItem*)item);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::Remove(tTJSNI_MenuItem *item)
{
	if(MenuItem && item->MenuItem)
	{
		int index = MenuItem->IndexOf(item->MenuItem);
		if(index == -1) TVPThrowExceptionMessage(TVPNotChildMenuItem);

		MenuItem->Delete(index);
		RemoveChild((tTJSNI_BaseMenuItem*)item);
	}
}
//---------------------------------------------------------------------------
HMENU tTJSNI_MenuItem::GetMenuItemHandleForPlugin() const
{
	if(!MenuItem) return NULL;
	return MenuItem->Handle;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::GetIndex() const
{
	if(!MenuItem) return NULL;
	return MenuItem->MenuIndex;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetIndex(tjs_int newIndex)
{
	if(!MenuItem) return;
	MenuItem->MenuIndex = newIndex;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetCaption(const ttstr & caption)
{
	if(!MenuItem) return;
	Caption = caption;
	MenuItem->AutoHotkeys = maManual;
	MenuItem->Caption = caption.AsAnsiString();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::GetCaption(ttstr & caption) const
{
	if(!MenuItem) caption.Clear();
	caption = Caption;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetChecked(bool b)
{
	if(!MenuItem) return;
	MenuItem->Checked = b;
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetChecked() const
{
	if(!MenuItem) return false;
	return MenuItem->Checked;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetEnabled(bool b)
{
	if(!MenuItem) return;
	MenuItem->Enabled = b;
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetEnabled() const
{
	if(!MenuItem) return false;
	return MenuItem->Enabled;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetGroup(tjs_int g)
{
	if(!MenuItem) return;
	MenuItem->GroupIndex = (BYTE)g;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::GetGroup() const
{
	if(!MenuItem) return 0;
	return MenuItem->GroupIndex;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetRadio(bool b)
{
	if(!MenuItem) return;
	MenuItem->RadioItem = b;
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetRadio() const
{
	if(!MenuItem) return false;
	return MenuItem->RadioItem;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetShortcut(const ttstr & shortcut)
{
	if(!MenuItem) return;
	MenuItem->ShortCut = TextToShortCut(shortcut.AsAnsiString());
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::GetShortcut(ttstr & shortcut) const
{
	if(!MenuItem) shortcut.Clear();
	shortcut = ShortCutToText(MenuItem->ShortCut);
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetVisible(bool b)
{
	if(!MenuItem) return;
	if(Window) Window->SetMenuBarVisible(b); else MenuItem->Visible = b;
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetVisible() const
{
	if(!MenuItem) return false;
	if(Window) return Window->GetMenuBarVisible(); else return MenuItem->Visible;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::TrackPopup(tjs_uint32 flags, tjs_int x, tjs_int y) const
{
	if(!MenuItem) return 0;

	HWND  hWindow;
	if (GetRootMenuItem() && GetRootMenuItem()->GetWindow()) {
		hWindow = GetRootMenuItem()->GetWindow()->GetMenuOwnerWindowHandle();
	} else {
		return 0;
	}
	HMENU hMenuItem = GetMenuItemHandleForPlugin();

	// we assume where that x and y are in client coordinates.
	// TrackPopupMenuEx requires screen coordinates, so here converts them.
	POINT scrPoint;	// screen
	scrPoint.x = x;
	scrPoint.y = y;
	BOOL rvScr = ::ClientToScreen(hWindow, &scrPoint);
	if (!rvScr)
	{
		// TODO
	}

	BOOL rvPopup = TrackPopupMenuEx(hMenuItem, flags, scrPoint.x, scrPoint.y, hWindow, NULL);
	if (!rvPopup)
	{
		// TODO
		// should raise an exception when the API fails
	}

	return rvPopup;
}


//---------------------------------------------------------------------------
// tTJSNC_MenuItem::CreateNativeInstance
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_MenuItem::CreateNativeInstance()
{
	return new tTJSNI_MenuItem();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCreateNativeClass_MenuItem
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_MenuItem()
{
	tTJSNativeClass *cls = new tTJSNC_MenuItem();
	static tjs_uint32 TJS_NCM_CLASSID;
	TJS_NCM_CLASSID = tTJSNC_MenuItem::ClassID;

//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(HMENU)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		if (result) *result = (tTVInteger)(tjs_uint)_this->GetMenuItemHandleForPlugin();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, HMENU)
//---------------------------------------------------------------------------


	return cls;
}
//---------------------------------------------------------------------------

