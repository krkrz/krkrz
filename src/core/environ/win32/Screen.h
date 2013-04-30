
#ifndef __T_SCREEN_H__
#define __T_SCREEN_H__

#include <winuser.h>

class TScreen {
public:
	TScreen();
	int GetWidth() const;
	int GetHeight() const;
};

extern TScreen* Screen;

#endif // __T_SCREEN_H__
