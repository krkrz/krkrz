//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "MenuItem" class interface
//---------------------------------------------------------------------------
#ifndef MenuItemIntfH
#define MenuItemIntfH

#include "WindowMenu.h"
#include "tp_stub.h"
#include "ObjectList.h"
#include <map>

//---------------------------------------------------------------------------
// tTJSNI_MenuItem
//---------------------------------------------------------------------------
class tTJSNI_MenuItem : public tTJSNativeInstance {
	typedef tTJSNativeInstance inherited;

	tObjectList<tTJSNI_MenuItem> Children;

	WindowMenuItem * MenuItem;

	ttstr Caption;
	ttstr Shortcut;

protected:
	iTJSDispatch2 *OwnerWindow;
	HWND HWnd;

	iTJSDispatch2 *Owner;
	tTJSVariantClosure ActionOwner; // object to send action

	tTJSNI_MenuItem *Parent;

	bool ChildrenArrayValid;
	iTJSDispatch2 * ChildrenArray;
	iTJSDispatch2 * ArrayClearMethod;

public:
	void MenuItemClick();

public:
	tTJSNI_MenuItem();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	static tTJSNI_MenuItem * CastFromVariant(const tTJSVariant & from);

protected:
	virtual bool CanDeliverEvents() const;

protected:
	void AddChild(tTJSNI_MenuItem *item);
	void RemoveChild(tTJSNI_MenuItem *item);

public:
	void Add(tTJSNI_MenuItem * item);
	void Insert(tTJSNI_MenuItem *item, tjs_int index);
	void Remove(tTJSNI_MenuItem *item);

	void SetCaption(const ttstr & caption);
	void GetCaption(ttstr & caption) const;

	void SetChecked(bool b);
	bool GetChecked() const;

	void SetEnabled(bool b);
	bool GetEnabled() const;

	void SetGroup(tjs_int g);
	tjs_int GetGroup() const;

	void SetRadio(bool b);
	bool GetRadio() const;

	void SetShortcut(const ttstr & shortcut);
	void GetShortcut(ttstr & shortcut) const;

	void SetVisible(bool b);
	bool GetVisible() const;

	tjs_int GetIndex() const;
	void SetIndex(tjs_int newIndex);

	tjs_int TrackPopup(tjs_uint32 flags, tjs_int x, tjs_int y) const;

//-- interface to plugin
	HMENU GetMenuItemHandleForPlugin() const;

public:
	tTJSVariantClosure GetActionOwnerNoAddRef() const { return ActionOwner; }

	iTJSDispatch2 * GetOwnerNoAddRef() const { return Owner; }

	tTJSNI_MenuItem * GetParent() const { return Parent; }

	tTJSNI_MenuItem * GetRootMenuItem() const;

	iTJSDispatch2 * GetWindow() const { return OwnerWindow; }

	iTJSDispatch2 * GetChildrenArrayNoAddRef();


public:
	void OnClick(void); // fire onClick event
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
extern iTJSDispatch2 * TVPCreateNativeClass_MenuItem();
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
extern iTJSDispatch2 * TVPCreateMenuItemObject(iTJSDispatch2 * window);
//---------------------------------------------------------------------------

#endif
