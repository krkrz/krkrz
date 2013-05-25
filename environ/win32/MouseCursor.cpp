
#include "MouseCursor.h"
#include "WindowFormUnit.h"

const LPTSTR MouseCursor::CURSORS[CURSOR_EOT] = {
	IDC_APPSTARTING,
	IDC_ARROW,
	IDC_CROSS,
	IDC_HAND,
	IDC_IBEAM,
	IDC_HELP,
	IDC_NO,
	IDC_SIZEALL,
	IDC_SIZENESW,
	IDC_SIZENS,
	IDC_SIZENWSE,
	IDC_SIZEWE,
	IDC_UPARROW,
	IDC_WAIT,
};
HCURSOR MouseCursor::CURSOR_HANDLES[CURSOR_EOT];
bool MouseCursor::is_cursor_hide_ = false;

void MouseCursor::Initialize() {
	for( int i = 0; i < CURSOR_EOT; i++ ) {
		CURSOR_HANDLES[i] = ::LoadCursor( NULL, CURSORS[i] );
	}
}

void MouseCursor::SetMouseCursor( int index ) {
	if( is_cursor_hide_ && index != crNone ) {
		::ShowCursor( TRUE );
		is_cursor_hide_ = false;
	}
	switch( index ) {
	case crDefault:
		::SetCursor( CURSOR_HANDLES[CURSOR_ARROW] );
		break;
	case crNone:
		if( is_cursor_hide_ != true ) {
			::ShowCursor( FALSE );
			is_cursor_hide_ = true;
		}
		break;
	case crArrow:
		::SetCursor( CURSOR_HANDLES[CURSOR_ARROW] );
		break;
	case crCross:
		::SetCursor( CURSOR_HANDLES[CURSOR_CROSS] );
		break;
	case crIBeam:
		::SetCursor( CURSOR_HANDLES[CURSOR_IBEAM] );
		break;
	case crSize:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZEALL] );
		break;
	case crSizeNESW:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZENESW] );
		break;
	case crSizeNS:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZENS] );
		break;
	case crSizeNWSE:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZENWSE] );
		break;
	case crSizeWE:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZEWE] );
		break;
	case crUpArrow:
		::SetCursor( CURSOR_HANDLES[CURSOR_UPARROW] );
		break;
	case crHourGlass:
		::SetCursor( CURSOR_HANDLES[CURSOR_WAIT] );
		break;
	case crHelp:
		::SetCursor( CURSOR_HANDLES[CURSOR_HELP] );
		break;
	case crHandPoint:
		::SetCursor( CURSOR_HANDLES[CURSOR_HAND] );
		break;
	case crSizeAll:
		::SetCursor( CURSOR_HANDLES[CURSOR_SIZEALL] );
		break;
	case crNo:
		::SetCursor( CURSOR_HANDLES[CURSOR_NO] );
		break;
	case crDrag:
	case crNoDrop:
	case crHSplit:
	case crVSplit:
	case crMultiDrag:
	case crSQLWait:
	case crAppStart:
	case crHBeam:
	default:
		::SetCursor( CURSOR_HANDLES[CURSOR_ARROW] );
		break;
	}
}
void MouseCursor::SetCursor() {
	if( hCursor_ != INVALID_HANDLE_VALUE ) {
		::SetCursor( hCursor_ );
		if( is_cursor_hide_ ) {
			::ShowCursor( TRUE );
			is_cursor_hide_ = false;
		}
	} else if( cursor_index_ != INVALID_CURSOR_INDEX ) {
		SetMouseCursor( cursor_index_ );
	}
}

void MouseCursor::SetCursorIndex( int index ) {
	cursor_index_ = index;
}