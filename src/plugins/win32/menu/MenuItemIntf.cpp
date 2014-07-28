//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "MenuItem" class interface
//---------------------------------------------------------------------------


#include "MenuItemIntf.h"

extern iTJSDispatch2* textToKeycodeMap;
extern iTJSDispatch2* keycodeToTextList;

static int TextToShortCut( ttstr text ) {
	// ignore case
	text.ToLowerCase();

	int virt = 0;
	const wchar_t* top  = text.c_str();
	const wchar_t* tail = top;
	const wchar_t* ret = NULL;
	if( (ret = wcsstr( top, L"shift+" )) != NULL ) {
		virt |= FSHIFT;
		if( tail < (ret + 6) ) tail = (ret + 6);
	}
	if( (ret = wcsstr( top, L"ctrl+" )) != NULL ) {
		virt |= FCONTROL;
		if( tail < (ret + 5) ) tail = (ret + 5);
	}
	if( (ret = wcsstr( top, L"alt+" )) != NULL ) {
		virt |= FALT;
		if( tail < (ret + 4) ) tail = (ret + 4);
	}
	iTJSDispatch2 *dict = textToKeycodeMap;
	if( dict ) {
		tTJSVariant var;
		if( TJS_SUCCEEDED(dict->PropGet(0, tail, NULL, &var, dict)) ) {
			virt |= FVIRTKEY;
			return (virt << 16) | (var.operator tjs_int() & 0xFFFF);
		}
	}
	return (virt << 16);
}
static ttstr ShortCutToText( int key ) {
	ttstr str;
	int virt = key >> 16;
	if( virt & FSHIFT ) {
		str += TJS_W("Shift+");
	}
	if( virt & FCONTROL ) {
		str += TJS_W("Ctrl+");
	}
	if( virt & FALT ) {
		str += TJS_W("Alt+");
	}
	key &= 0xFFFF;
	if( key >= 8 && key <= 255 ) {
		iTJSDispatch2 *array = keycodeToTextList;
		if( array ) {
			tTJSVariant var;
			if( TJS_SUCCEEDED(array->PropGetByNum(0, key, &var, array)) ) {
				str += var.GetString();
			}
		}
	}
	return str;
}
static ttstr MakeCaptionWithShortCut( ttstr const &caption, int key ) {
	if( caption == TJS_W("-") ) return caption;

	ttstr shortcut( ShortCutToText( key ) );
	if( shortcut.IsEmpty() ) return caption;

	const wchar_t *text = caption.c_str();
	const wchar_t *tab  = wcschr( text, L'\t' );
	return ( (tab != NULL) ? ttstr( text, tab-text ) : ttstr( caption ) ) + TJS_W("\t") + shortcut;
}


extern const tjs_char* TVPSpecifyWindow;
extern const tjs_char* TVPSpecifyMenuItem;
extern const tjs_char* TVPInternalError;
extern const tjs_char* TVPNotChildMenuItem;

#define TVPThrowInternalError \
	TVPThrowExceptionMessage(TVPInternalError, __FILE__,  __LINE__)

/**
	* メッセージ受信関数本体
	* @param userdata ユーザデータ(この場合ネイティブオブジェクト情報)
	* @param Message ウインドウメッセージ情報
	*/
static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message) {
	tTJSNI_MenuItem *obj = (tTJSNI_MenuItem*)userdata;
	switch (Message->Msg) {
	case TVP_WM_DETACH: // ウインドウが切り離された
		break; 
	case TVP_WM_ATTACH: // ウインドウが設定された
		break;
	case WM_COMMAND: {
		WORD wID = Message->WParam & 0xFFFF;
		WindowMenuItem::OnClickHandler( wID );
		break;
	}
	default:
		break;
	}
	return false;
}

#define TJS_NATIVE_CLASSID_NAME ClassID_MenuItem
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
// tTJSNI_MenuItem
//---------------------------------------------------------------------------
tTJSNI_MenuItem::tTJSNI_MenuItem()
{
	Owner = NULL;
	OwnerWindow = NULL;
	Parent = NULL;
	ChildrenArrayValid = false;
	ChildrenArray = NULL;
	ArrayClearMethod = NULL;

	HWnd = (HWND)INVALID_HANDLE_VALUE;
	ActionOwner.Object = ActionOwner.ObjThis = NULL;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_MenuItem::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	ActionOwner = param[0]->AsObjectClosure();
	Owner = tjs_obj;

	if( numparams >= 2 ) {
		if( param[1]->Type() == tvtObject ) {
			// may be Window instance
			tTJSVariant var;
			iTJSDispatch2* win = param[1]->AsObjectNoAddRef();
			if( TJS_FAILED(win->PropGet(0, TJS_W("HWND"), NULL, &var, win)) ) {
				TVPThrowExceptionMessage(TVPSpecifyWindow);
			}
			HWnd = (HWND)(tjs_int64)var;
			OwnerWindow = win;
		}
	}

	// create or attach MenuItem object
	if( OwnerWindow ) {
		HMENU hMenu = ::CreateMenu();
		MenuItem = new WindowMenuItem(this,HWnd,hMenu);
		::SetMenu( HWnd, hMenu );
		iTJSDispatch2 *obj = OwnerWindow;
		// registerMessageReceiver を呼ぶ
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmRegister;
		proc = (tTVInteger)reinterpret_cast<tjs_int64>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)this;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL, NULL, 3, p, obj);
	} else {
		tTJSVariant var;
		iTJSDispatch2* win = param[0]->AsObjectNoAddRef();
		if( win == NULL ) {
			TVPThrowExceptionMessage(TVPSpecifyWindow);
		}
		if( TJS_FAILED(win->PropGet(0, TJS_W("HWND"), NULL, &var, win)) ) {
			TVPThrowExceptionMessage(TVPSpecifyWindow);
		}
		HWnd = (HWND)(tjs_int64)var;
		MenuItem = new WindowMenuItem(this,HWnd);
	}

	// fetch initial caption
	if( !OwnerWindow && numparams >= 2 ) {
		Caption = *param[1];
		MenuItem->SetCaption( Caption.c_str() );
	}

	return S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_MenuItem::Invalidate()
{
	bool dodeletemenuitem = (OwnerWindow == NULL);

	TVPCancelSourceEvents(Owner);

	{ // locked
		tObjectListSafeLockHolder<tTJSNI_MenuItem> holder(Children);
		tjs_int count = Children.GetSafeLockedObjectCount();
		for(tjs_int i = 0; i < count; i++)
		{
			tTJSNI_MenuItem *item = Children.GetSafeLockedObjectAt(i);
			if(!item) continue;

			if(item->Owner)
			{
				item->Owner->Invalidate(0, NULL, NULL, item->Owner);
				item->Owner->Release();
			}
		}
	} // locked

//	Owner = NULL;
	OwnerWindow = NULL;
	Parent = NULL;

	if(ChildrenArray) ChildrenArray->Release(), ChildrenArray = NULL;
	if(ArrayClearMethod) ArrayClearMethod->Release(), ArrayClearMethod = NULL;

	ActionOwner.Release();
	ActionOwner.ObjThis = ActionOwner.Object = NULL;

	inherited::Invalidate();

	if(dodeletemenuitem) delete MenuItem, MenuItem = NULL;
}
//---------------------------------------------------------------------------
tTJSNI_MenuItem * tTJSNI_MenuItem::CastFromVariant(const tTJSVariant & from)
{
	if(from.Type() == tvtObject)
	{
		// is this Window instance ?
		tTJSVariantClosure clo = from.AsObjectClosureNoAddRef();
		if(clo.Object == NULL) TVPThrowExceptionMessage(TVPSpecifyMenuItem);
		tTJSNI_MenuItem *menuitem = NULL;
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, ClassID_MenuItem, (iTJSNativeInstance**)&menuitem)))
			TVPThrowExceptionMessage(TVPSpecifyMenuItem);
		return menuitem;
	}
	TVPThrowExceptionMessage(TVPSpecifyMenuItem);
	return NULL;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::AddChild(tTJSNI_MenuItem *item)
{
	if(Children.Add(item))
	{
		ChildrenArrayValid = false;
		if(item->Owner) item->Owner->AddRef();
		item->Parent = this;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::RemoveChild(tTJSNI_MenuItem *item)
{
	if(Children.Remove(item))
	{
		ChildrenArrayValid = false;
		if(item->Owner) item->Owner->Release();
		item->Parent = NULL;
	}
}
//---------------------------------------------------------------------------
iTJSDispatch2 * tTJSNI_MenuItem::GetChildrenArrayNoAddRef()
{
	if(!ChildrenArray)
	{
		// create an Array object
		iTJSDispatch2 * classobj;
		ChildrenArray = TJSCreateArrayObject(&classobj);
		try
		{
			tTJSVariant val;
			tjs_error er;
			er = classobj->PropGet(0, TJS_W("clear"), NULL, &val, classobj);
				// retrieve clear method
			if(TJS_FAILED(er)) TVPThrowInternalError;
			ArrayClearMethod = val.AsObject();
		}
		catch(...)
		{
			ChildrenArray->Release();
			ChildrenArray = NULL;
			classobj->Release();
			throw;
		}
		classobj->Release();
	}

	if(!ChildrenArrayValid)
	{
		// re-create children list
		ArrayClearMethod->FuncCall(0, NULL, NULL, NULL, 0, NULL, ChildrenArray);
			// clear array

		{ // locked
			tObjectListSafeLockHolder<tTJSNI_MenuItem> holder(Children);
			tjs_int count = Children.GetSafeLockedObjectCount();
			tjs_int itemcount = 0;
			for(tjs_int i = 0; i < count; i++)
			{
				tTJSNI_MenuItem *item = Children.GetSafeLockedObjectAt(i);
				if(!item) continue;

				iTJSDispatch2 * dsp = item->Owner;
				tTJSVariant val(dsp, dsp);
				ChildrenArray->PropSetByNum(TJS_MEMBERENSURE, itemcount, &val,
					ChildrenArray);
				itemcount++;
			}
		} // locked

		ChildrenArrayValid = true;
	}

	return ChildrenArray;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::OnClick(void)
{
	// fire onClick event
	if(!CanDeliverEvents()) return;

	// also check window
	tTJSNI_MenuItem *item = this;
	while(!item->OwnerWindow)
	{
		if(!item->Parent) break;
		item = item->Parent;
	}
	if(!item->OwnerWindow) return;
	//if(!item->OwnerWindow->CanDeliverEvents()) return;

	// fire event
	static ttstr eventname(TJS_W("onClick"));
	TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_NORMAL, 0, NULL);
}
//---------------------------------------------------------------------------
tTJSNI_MenuItem * tTJSNI_MenuItem::GetRootMenuItem() const
{
	tTJSNI_MenuItem * current = const_cast<tTJSNI_MenuItem*>(this);
	tTJSNI_MenuItem * parent = current->GetParent();
	while (parent)
	{
		current = parent;
		parent = current->GetParent();
	}
	return current;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void tTJSNI_MenuItem::MenuItemClick()
{
	// post to the event queue
	static ttstr eventname(TJS_W("fireClick"));
	TVPPostEvent(Owner, Owner, eventname, 0, 0, 0, NULL);
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::CanDeliverEvents() const
{
	// returns whether events can be delivered
	if(!MenuItem) return false;
	bool enabled = true;
	WindowMenuItem *item = MenuItem;
	while(item)
	{
		if(!item->GetEnabled())
		{
			enabled = false;
			break;
		}
		item = item->GetParent();
	}
	return enabled;
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::Add(tTJSNI_MenuItem * item)
{
	if(MenuItem && item->MenuItem)
	{
		MenuItem->Add(item->MenuItem);
		AddChild(item);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::Insert(tTJSNI_MenuItem *item, tjs_int index)
{
	if(MenuItem && item->MenuItem)
	{
		MenuItem->Insert(index, item->MenuItem);
		AddChild(item);
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
		RemoveChild(item);
	}
}
//---------------------------------------------------------------------------
HMENU tTJSNI_MenuItem::GetMenuItemHandleForPlugin() const
{
	if(!MenuItem) return NULL;
	return MenuItem->GetHandle();
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::GetIndex() const
{
	if(!MenuItem) return NULL;
	return MenuItem->GetMenuIndex();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetIndex(tjs_int newIndex)
{
	if(!MenuItem) return;
	MenuItem->SetMenuIndex( newIndex );
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetCaption(const ttstr & caption)
{
	if(!MenuItem) return;
	Caption = caption;
	//MenuItem->AutoHotkeys = maManual;
	MenuItem->SetCaption( MakeCaptionWithShortCut( caption, MenuItem->GetShortCut() ).c_str() );
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::GetCaption(ttstr & caption) const
{
	if(!MenuItem) caption.Clear();
	else caption = Caption; //MenuItem->GetCaption();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetChecked(bool b)
{
	if(!MenuItem) return;
	MenuItem->SetChecked( b );
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetChecked() const
{
	if(!MenuItem) return false;
	return MenuItem->GetChecked();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetEnabled(bool b)
{
	if(!MenuItem) return;
	MenuItem->SetEnabled( b );
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetEnabled() const
{
	if(!MenuItem) return false;
	return MenuItem->GetEnabled();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetGroup(tjs_int g)
{
	if(!MenuItem) return;
	MenuItem->SetGroupIndex( g );
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::GetGroup() const
{
	if(!MenuItem) return 0;
	return MenuItem->GetGroupIndex();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetRadio(bool b)
{
	if(!MenuItem) return;
	MenuItem->SetRadioItem( b );
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetRadio() const
{
	if(!MenuItem) return false;
	return MenuItem->GetRadioItem();
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetShortcut(const ttstr & shortcut)
{
	if(!MenuItem) return;
	MenuItem->SetShortCut( TextToShortCut(shortcut.c_str()) );
	MenuItem->SetCaption( MakeCaptionWithShortCut( Caption, MenuItem->GetShortCut() ).c_str() );
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::GetShortcut(ttstr & shortcut) const
{
	if(!MenuItem) shortcut.Clear();
	shortcut = ShortCutToText(MenuItem->GetShortCut() );
}
//---------------------------------------------------------------------------
void tTJSNI_MenuItem::SetVisible(bool b)
{
	if(!MenuItem) return;
	if(OwnerWindow) {
		bool update = false;
		if( GetVisible() ) {
			if( !b ) {
				::SetMenu( HWnd, NULL );
				update = true;
			}
		} else {
			if( b ) {
				::SetMenu( HWnd, GetRootMenuItem()->GetMenuItemHandleForPlugin() );
				update = true;
			}
		}
		// Window->SetMenuBarVisible(b);

		// redraw window
		if( update ) {
			::RedrawWindow( HWnd, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE | RDW_UPDATENOW );
		}
	} else {
		MenuItem->SetVisible( b );
	}
}
//---------------------------------------------------------------------------
bool tTJSNI_MenuItem::GetVisible() const
{
	if(!MenuItem) return false;
	if(OwnerWindow) {

		return ::GetMenu(HWnd) != NULL; // Window->GetMenuBarVisible(); 
	} else return MenuItem->GetVisible();
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_MenuItem::TrackPopup(tjs_uint32 flags, tjs_int x, tjs_int y) const
{
	if(!MenuItem) return 0;

	HWND  hWindow;
	if( GetRootMenuItem()  ) {
		hWindow = HWnd;
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


static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_MenuItem() {
	return new tTJSNI_MenuItem();
}

//---------------------------------------------------------------------------
// tTJSNC_MenuItem
//---------------------------------------------------------------------------

iTJSDispatch2 * TVPCreateNativeClass_MenuItem() {
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("MenuItem"), Create_NI_MenuItem);
	
	/// メンバ定義
	TJS_BEGIN_NATIVE_MEMBERS(MenuItem) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_MenuItem,
	/*TJS class name*/MenuItem)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/MenuItem)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/add)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSNI_MenuItem * item = tTJSNI_MenuItem::CastFromVariant(*param[0]);
	_this->Add(item);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/add)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/insert)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;
	tTJSNI_MenuItem * item = tTJSNI_MenuItem::CastFromVariant(*param[0]);
	tjs_int index = *param[1];
	_this->Insert(item, index);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/insert)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/remove)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSNI_MenuItem * item = tTJSNI_MenuItem::CastFromVariant(*param[0]);
	_this->Remove(item);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/remove)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/popup) // not trackPopup
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 3) return TJS_E_BADPARAMCOUNT;
	tjs_uint32 flags = static_cast<tjs_uint32>((tTVInteger)*param[0]);
	tjs_int x = *param[1];
	tjs_int y = *param[2];
	tjs_int rv = _this->TrackPopup(flags, x, y);
	if (result) *result = rv;
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/popup) // not trackPopup
//---------------------------------------------------------------------------

//-- events

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onClick)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
		/*var. type*/tTJSNI_MenuItem);

	tTJSVariantClosure obj = _this->GetActionOwnerNoAddRef();
	if(obj.Object) {
		if( numparams < 0 ) return TJS_E_BADPARAMCOUNT;
		tjs_int arg_count = 0;
		iTJSDispatch2 *evobj = TVPCreateEventObject(TJS_W("onClick"), objthis, objthis );
		tTJSVariant evval(evobj, evobj);
		evobj->Release();

		tTJSVariant *pevval = &evval;
		obj.FuncCall( 0, TJS_W("action"), NULL, result, 1, &pevval, NULL);
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onClick)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/fireClick)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	_this->OnClick();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/fireClick)
//----------------------------------------------------------------------

//--properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(caption)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		ttstr res;
		_this->GetCaption(res);
		*result = res;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetCaption(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(caption)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(checked)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		*result = _this->GetChecked();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetChecked(param->operator bool());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(checked)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(enabled)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		*result = _this->GetEnabled();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetEnabled(param->operator bool());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(enabled)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(group)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		*result = _this->GetGroup();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetGroup((tjs_int)*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(group)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(radio)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		*result = _this->GetRadio();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetRadio(param->operator bool());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(radio)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(shortcut)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		ttstr res;
		_this->GetShortcut(res);
		*result = res;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetShortcut(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(shortcut)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(visible)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		*result = _this->GetVisible();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetVisible(param->operator bool());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(visible)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(parent)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		tTJSNI_MenuItem *parent = _this->GetParent();
		if(parent)
		{
			iTJSDispatch2 *dsp = parent->GetOwnerNoAddRef();
			*result = tTJSVariant(dsp, dsp);
		}
		else
		{
			*result = tTJSVariant((iTJSDispatch2 *)NULL);
		}
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(parent)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(children)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		iTJSDispatch2 *dsp = _this->GetChildrenArrayNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(children)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(root) // not rootMenuItem
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		tTJSNI_MenuItem * root = _this->GetRootMenuItem();
		if (root)
		{
			iTJSDispatch2 *dsp = root->GetOwnerNoAddRef();
			if (result) *result = tTJSVariant(dsp, dsp);
		}
		else
		{
			if (result) *result = tTJSVariant((iTJSDispatch2 *)NULL);
		}
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(root) // not rootMenuItem
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(window)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		iTJSDispatch2 * window = _this->GetWindow();
		if (window)
		{
			iTJSDispatch2 *dsp = window;
			if (result) *result = tTJSVariant(dsp, dsp);
		}
		else
		{
			if (result) *result = tTJSVariant((iTJSDispatch2 *)NULL);
		}
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(window)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(index)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		if (result) *result = (tTVInteger)_this->GetIndex();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
		_this->SetIndex((tjs_int)(*param));
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(index)
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
TJS_END_NATIVE_PROP_DECL(HMENU)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(textToKeycode)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		if (result) *result = tTJSVariant(textToKeycodeMap, textToKeycodeMap);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(textToKeycode)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(keycodeToText)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		if (result) *result = tTJSVariant(keycodeToTextList, keycodeToTextList);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(keycodeToText)
//----------------------------------------------------------------------
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS

	return classobj;
}
//---------------------------------------------------------------------------

#undef TJS_NATIVE_CLASSID_NAME


//---------------------------------------------------------------------------
iTJSDispatch2 * TVPCreateMenuItemObject(iTJSDispatch2 * window) {
	struct tHolder {
		iTJSDispatch2 * Obj;
		tHolder() { Obj = TVPCreateNativeClass_MenuItem(); }
		~tHolder() { Obj->Release(); }
	} static menuitemclass;

	iTJSDispatch2 *out;
	tTJSVariant param(window);
	tTJSVariant *pparam[2] = {&param, &param};
	if(TJS_FAILED(menuitemclass.Obj->CreateNew(0, NULL, NULL, &out, 2, pparam,menuitemclass.Obj)))
		TVPThrowExceptionMessage(TVPInternalError,
			TJS_W("TVPCreateMenuItemObject"));

	return out;
}
//---------------------------------------------------------------------------

