
#ifndef __TVP_SCREEN_H__
#define __TVP_SCREEN_H__

#include "MsgIntf.h"

class tTVPScreen {

public:
	static int GetWidth();
	static int GetHeight();
	static int GetDesktopLeft();
	static int GetDesktopTop();
	static int GetDesktopWidth();
	static int GetDesktopHeight();
};

#endif // __TVP_SCREEN_H__
