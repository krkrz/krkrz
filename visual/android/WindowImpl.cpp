//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "WindowIntf.h"


//---------------------------------------------------------------------------
// tTJSNI_Window
//---------------------------------------------------------------------------
tTJSNI_Window::tTJSNI_Window()
{
#if 0
	VSyncTimingThread = NULL;
	Form = NULL;
#endif
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_Window::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = tTJSNI_BaseWindow::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;
#if 0
	if( numparams >= 1 && param[0]->Type() == tvtObject ) {
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		tTJSNI_Window *win = NULL;
		if(clo.Object != NULL) {
			if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,tTJSNC_Window::ClassID, (iTJSNativeInstance**)&win)))
				TVPThrowExceptionMessage(TVPSpecifyWindow);
			if(!win) TVPThrowExceptionMessage(TVPSpecifyWindow);
		}
		Form = new TTVPWindowForm(Application, this, win);
	} else {
		Form = new TTVPWindowForm(Application, this);
	}
#endif
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::Invalidate()
{
	tTJSNI_BaseWindow::Invalidate();
#if 0
	if( VSyncTimingThread )
	{
		delete VSyncTimingThread;
		VSyncTimingThread = NULL;
	}
	if(Form)
	{
		Form->InvalidateClose();
		Form = NULL;
	}

	// remove all events
	TVPCancelSourceEvents(Owner);
	TVPCancelInputEvents(this);

#endif
	// Set Owner null
	Owner = NULL;
}
