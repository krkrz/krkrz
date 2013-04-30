//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Event Handling and Dispatching
//---------------------------------------------------------------------------
#ifndef EventImplH
#define EventImplH

#include "EventIntf.h"


//---------------------------------------------------------------------------
TJS_EXP_FUNC_DEF(bool, TVPGetWaitVSync, ());
TJS_EXP_FUNC_DEF(void, TVPEnsureVSyncTimingThread, ());
TJS_EXP_FUNC_DEF(void, TVPReleaseVSyncTimingThread, ());
//---------------------------------------------------------------------------
#endif
