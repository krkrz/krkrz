
#include "stdafx.h"
#include "Screen.h"
#include "Application.h"

int TScreen::GetWidth() const {
	HMONITOR hMonitor = MonitorFromWindow( Application->GetMainWindowHandle(), MONITOR_DEFAULTTOPRIMARY );
	MONITORINFO monitorinfo;
	if( GetMonitorInfo( hMonitor, &monitorinfo ) != FALSE ) {
		return monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
	}
	return 0;
}
int TScreen::GetHeight() const {
	HMONITOR hMonitor = MonitorFromWindow( Application->GetMainWindowHandle(), MONITOR_DEFAULTTOPRIMARY );
	MONITORINFO monitorinfo;
	if( GetMonitorInfo( hMonitor, &monitorinfo ) != FALSE ) {
		return monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
	}
	return 0;
}

TScreen* Screen;
