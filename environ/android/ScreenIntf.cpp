//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Screen" TJS Class implementation
//---------------------------------------------------------------------------

#include "tjsCommHead.h"

#include <algorithm>
#include "MsgIntf.h"
#include "ScreenIntf.h"
#include "LayerIntf.h"
#include "DebugIntf.h"
#include "EventIntf.h"
#include "LayerBitmapIntf.h"
#include "LayerIntf.h"
#include "SysInitIntf.h"
#include "LayerManager.h"
#include "EventImpl.h"

#include "Application.h"

//---------------------------------------------------------------------------
/*
void TVPClearAllScreenInputEvents()
{
	std::vector<tTJSNI_Screen*>::iterator i;
	for(i = TVPWindowVector.begin(); i!=TVPWindowVector.end(); i++)
	{
		(*i)->ClearInputEvents();
	}
}
*/
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Input Events
//---------------------------------------------------------------------------
// For each input event tag
tTVPUniqueTagForInputEvent tTVPOnMouseDownInputEvent          ::Tag;
tTVPUniqueTagForInputEvent tTVPOnMouseUpInputEvent            ::Tag;
tTVPUniqueTagForInputEvent tTVPOnMouseMoveInputEvent          ::Tag;
tTVPUniqueTagForInputEvent tTVPOnMouseWheelInputEvent         ::Tag;
tTVPUniqueTagForInputEvent tTVPOnKeyDownInputEvent            ::Tag;
tTVPUniqueTagForInputEvent tTVPOnKeyUpInputEvent              ::Tag;
tTVPUniqueTagForInputEvent tTVPOnKeyPressInputEvent           ::Tag;
tTVPUniqueTagForInputEvent tTVPOnActivateEvent                ::Tag;
tTVPUniqueTagForInputEvent tTVPOnTouchDownInputEvent          ::Tag;
tTVPUniqueTagForInputEvent tTVPOnTouchUpInputEvent            ::Tag;
tTVPUniqueTagForInputEvent tTVPOnTouchMoveInputEvent          ::Tag;
tTVPUniqueTagForInputEvent tTVPOnTouchScalingInputEvent       ::Tag;
tTVPUniqueTagForInputEvent tTVPOnTouchRotateInputEvent        ::Tag;
tTVPUniqueTagForInputEvent tTVPOnMultiTouchInputEvent         ::Tag;
tTVPUniqueTagForInputEvent tTVPOnDisplayRotateInputEvent      ::Tag;
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTJSNI_Screen
//---------------------------------------------------------------------------
tTJSNI_Screen::tTJSNI_Screen()
{
	ObjectVectorLocked = false;
}
//---------------------------------------------------------------------------
tTJSNI_Screen::~tTJSNI_Screen()
{
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_Screen::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	Owner = tjs_obj; // no addref

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD
tTJSNI_Screen::Invalidate()
{
	// remove all events
	TVPCancelSourceEvents(Owner);
	TVPCancelInputEvents(this);

	// clear all screen update events
	TVPRemoveScreenUpdate((tTJSNI_Screen*)this);

	// invalidate all registered objects
	ObjectVectorLocked = true;
	std::vector<tTJSVariantClosure>::iterator i;
	for(i = ObjectVector.begin(); i != ObjectVector.end(); i++) {
		// invalidate each --
		// objects may throw an exception while invalidating,
		// but here we cannot care for them.
		try {
			i->Invalidate(0, NULL, NULL, NULL);
			i->Release();
		} catch(eTJSError &e) {
			TVPAddLog(e.GetMessage()); // just in case, log the error
		}
	}

	// remove all events (again)
	TVPCancelSourceEvents(Owner);
	TVPCancelInputEvents(this);

	// clear all screen update events (again)
	TVPRemoveScreenUpdate((tTJSNI_Screen*)this);

	inherited::Invalidate();
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::FireOnActivate(bool activate_or_deactivate)
{
	// fire Screen.onActivate or Screen.onDeactivate event
	TVPPostInputEvent( new tTVPOnActivateEvent(this, activate_or_deactivate), TVP_EPT_REMOVE_POST ); // to discard redundant events
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb,
	tjs_uint32 flags)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[4] = { x, y, (tjs_int64)mb, (tjs_int64)flags };
		static ttstr eventname(TJS_W("onMouseDown"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 4, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb,
	tjs_uint32 flags)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[4] = { x, y, (tjs_int)mb, (tjs_int)flags };
		static ttstr eventname(TJS_W("onMouseUp"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 4, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		static ttstr eventname(TJS_W("onMouseMove"));
			tTJSVariant arg[3] = { x, y, (tjs_int64)flags };
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_DISCARDABLE|TVP_EPT_IMMEDIATE
			/*discardable!!*/,
			3, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnTouchDown( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[5] = { x, y, cx, cy, (tjs_int64)id };
		static ttstr eventname(TJS_W("onTouchDown"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 5, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[5] = { x, y, cx, cy, (tjs_int64)id };
		static ttstr eventname(TJS_W("onTouchUp"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 5, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnTouchMove( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[5] = { x, y, cx, cy, (tjs_int64)id };
		static ttstr eventname(TJS_W("onTouchMove"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 5, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnTouchScaling( tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[5] = { startdist, curdist, cx, cy, flag };
		static ttstr eventname(TJS_W("onTouchScaling"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 5, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnTouchRotate( tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[6] = { startangle, curangle, dist, cx, cy, flag };
		static ttstr eventname(TJS_W("onTouchRotate"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 6, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnMultiTouch() {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		static ttstr eventname(TJS_W("onMultiTouch"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 0, NULL);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnKeyDown(tjs_uint key, tjs_uint32 shift)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[2] = { (tjs_int)key, (tjs_int)shift };
		static ttstr eventname(TJS_W("onKeyDown"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 2, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnKeyUp(tjs_uint key, tjs_uint32 shift)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[2] = { (tjs_int)key, (tjs_int)shift };
		static ttstr eventname(TJS_W("onKeyUp"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 2, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnKeyPress(tjs_char key)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tjs_char buf[2];
		buf[0] = (tjs_char)key;
		buf[1] = 0;
		tTJSVariant arg[1] = { buf };
		static ttstr eventname(TJS_W("onKeyPress"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 1, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnMouseWheel(tjs_uint32 shift, tjs_int delta,
	tjs_int x, tjs_int y)
{
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[4] = { (tjs_int)shift, delta, x, y };
		static ttstr eventname(TJS_W("onMouseWheel"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 4, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnActivate(bool activate_or_deactivate)
{
	if(!CanDeliverEvents()) return;

	if(Owner)
	{
		static ttstr a_eventname(TJS_W("onActivate"));
		static ttstr d_eventname(TJS_W("onDeactivate"));
		TVPPostEvent(Owner, Owner, activate_or_deactivate?a_eventname:d_eventname,
			0, TVP_EPT_IMMEDIATE, 0, NULL);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::OnDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution ) {
	if(!CanDeliverEvents()) return;
	if(Owner)
	{
		tTJSVariant arg[5] = { orientation, rotate, bpp, hresolution, vresolution };
		static ttstr eventname(TJS_W("onDisplayRotate"));
		TVPPostEvent(Owner, Owner, eventname, 0, TVP_EPT_IMMEDIATE, 5, arg);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::ClearInputEvents()
{
	TVPCancelInputEvents(this);
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::UpdateContent()
{
	if( DrawDevice ) {
		// is called from event dispatcher
		DrawDevice->Update();

		if( !WaitVSync ) DrawDevice->Show();

 		EndUpdate();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::RecheckInputState()
{
	// slow timer tick (about 1 sec interval, inaccurate)
	if( DrawDevice ) DrawDevice->RecheckInputState();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Screen::RequestUpdate()
{
	// is called from primary layer

	// post update event to self
	TVPPostScreenUpdate((tTJSNI_Screen*)this);
}
//---------------------------------------------------------------------------


//---- methods

//---------------------------------------------------------------------------
void tTJSNI_Screen::Add(tTJSVariantClosure clo)
{
	if(ObjectVectorLocked) return;
	if(ObjectVector.end() == std::find(ObjectVector.begin(), ObjectVector.end(), clo))
	{
		ObjectVector.push_back(clo);
		clo.AddRef();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Screen::Remove(tTJSVariantClosure clo)
{
	if(ObjectVectorLocked) return;
	std::vector<tTJSVariantClosure>::iterator i;
	i = std::find(ObjectVector.begin(), ObjectVector.end(), clo);
	if(i != ObjectVector.end())
	{
		clo.Release();
		ObjectVector.erase(i);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNC_Screen : TJS Screen class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Screen::ClassID = -1;
tTJSNC_Screen::tTJSNC_Screen() : tTJSNativeClass(TJS_W("Screen"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Screen) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Screen,
	/*TJS class name*/Screen)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Screen)
//----------------------------------------------------------------------




//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/update)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
	tTVPUpdateType type = utNormal;
	if(numparams >= 2 && param[1]->Type() != tvtVoid)
		type = (tTVPUpdateType)(tjs_int)*param[0];
	_this->Update(type);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/update)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/add)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	_this->Add(clo);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/add)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/remove)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	_this->Remove(clo);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/remove)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/postInputEvent)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	if(numparams < 1)  return TJS_E_BADPARAMCOUNT;
	ttstr eventname;
	iTJSDispatch2 * eventparams = NULL;

	eventname = *param[0];
	if(numparams >= 2) eventparams = param[1]->AsObjectNoAddRef();

	_this->PostInputEvent(eventname, eventparams);

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/postInputEvent)
//----------------------------------------------------------------------



//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseEnter)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(0, "onMouseEnter", objthis);
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseEnter)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseLeave)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(0, "onMouseLeave", objthis);
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseLeave)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseDown)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(4, "onMouseDown", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("button");
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseDown)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseUp)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(4, "onMouseUp", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("button");
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseUp)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseMove)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(3, "onMouseMove", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseMove)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMouseWheel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(4, "onMouseWheel", objthis);
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_MEMBER("delta");
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMouseWheel)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onTouchDown)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(5, "onTouchDown", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("cx");
	TVP_ACTION_INVOKE_MEMBER("cy");
	TVP_ACTION_INVOKE_MEMBER("id");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onTouchDown)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onTouchUp)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(5, "onTouchUp", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("cx");
	TVP_ACTION_INVOKE_MEMBER("cy");
	TVP_ACTION_INVOKE_MEMBER("id");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onTouchUp)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onTouchMove)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(5, "onTouchMove", objthis);
	TVP_ACTION_INVOKE_MEMBER("x");
	TVP_ACTION_INVOKE_MEMBER("y");
	TVP_ACTION_INVOKE_MEMBER("cx");
	TVP_ACTION_INVOKE_MEMBER("cy");
	TVP_ACTION_INVOKE_MEMBER("id");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onTouchMove)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onTouchScaling)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(5, "onTouchScaling", objthis);
	TVP_ACTION_INVOKE_MEMBER("startdistance");
	TVP_ACTION_INVOKE_MEMBER("currentdistance");
	TVP_ACTION_INVOKE_MEMBER("cx");
	TVP_ACTION_INVOKE_MEMBER("cy");
	TVP_ACTION_INVOKE_MEMBER("flag");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onTouchScaling)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onTouchRotate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(6, "onTouchRotate", objthis);
	TVP_ACTION_INVOKE_MEMBER("startangle");
	TVP_ACTION_INVOKE_MEMBER("currentangle");
	TVP_ACTION_INVOKE_MEMBER("distance");
	TVP_ACTION_INVOKE_MEMBER("cx");
	TVP_ACTION_INVOKE_MEMBER("cy");
	TVP_ACTION_INVOKE_MEMBER("flag");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onTouchRotate)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onMultiTouch)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(0, "onMultiTouch", objthis);
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onMultiTouch)

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onKeyDown)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(2, "onKeyDown", objthis);
	TVP_ACTION_INVOKE_MEMBER("key");
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onKeyDown)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onKeyUp)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(2, "onKeyUp", objthis);
	TVP_ACTION_INVOKE_MEMBER("key");
	TVP_ACTION_INVOKE_MEMBER("shift");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onKeyUp)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onKeyPress)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(1, "onKeyPress", objthis);
	TVP_ACTION_INVOKE_MEMBER("key");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onKeyPress)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onActivate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(0, "onActivate", objthis);
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onActivate)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onDeactivate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(0, "onDeactivate", objthis);
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onDeactivate)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onDisplayRotate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

	TVP_ACTION_INVOKE_BEGIN(5, "onDisplayRotate", objthis);
	TVP_ACTION_INVOKE_MEMBER("orientation");
	TVP_ACTION_INVOKE_MEMBER("angle");
	TVP_ACTION_INVOKE_MEMBER("bpp");
	TVP_ACTION_INVOKE_MEMBER("width");
	TVP_ACTION_INVOKE_MEMBER("height");
	TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onDisplayRotate)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(width)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
		*result = _this->GetWidth();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(width)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(height)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
		*result = _this->GetHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(height)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(focusedLayer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
		tTJSNI_BaseLayer *lay = _this->GetDrawDevice()->GetFocusedLayer();
		if(lay && lay->GetOwnerNoAddRef())
			*result = tTJSVariant(lay->GetOwnerNoAddRef(), lay->GetOwnerNoAddRef());
		else
			*result = tTJSVariant((iTJSDispatch2*)NULL);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);

		tTJSNI_BaseLayer *to = NULL;

		if(param->Type() != tvtVoid)
		{
			tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
			if(clo.Object)
			{
				if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
					tTJSNC_Layer::ClassID, (iTJSNativeInstance**)&to)))
					TVPThrowExceptionMessage(TVPSpecifyLayer);
			}
		}

		_this->GetDrawDevice()->SetFocusedLayer(to);

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(focusedLayer)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(primaryLayer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
		tTJSNI_BaseLayer *pri = _this->GetDrawDevice()->GetPrimaryLayer();
		if(!pri) TVPThrowExceptionMessage(TVPScreenHasNoLayer);

		if(pri && pri->GetOwnerNoAddRef())
			*result = tTJSVariant(pri->GetOwnerNoAddRef(), pri->GetOwnerNoAddRef());
		else
			*result = tTJSVariant((iTJSDispatch2*)NULL);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(primaryLayer)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(layerTreeOwnerInterface)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Screen);
		*result = reinterpret_cast<tjs_int64>(static_cast<iTVPLayerTreeOwner*>(_this));
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(layerTreeOwnerInterface)
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS

}
//---------------------------------------------------------------------------

