
#include "stdafx.h"
#include "InputStringForm.h"
#include "Resource.h"

static std::string Title;
static std::string Caption;
static std::string EditText;

static LRESULT WINAPI DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
	case WM_INITDIALOG: {
		::SetWindowText( hWnd, Title.c_str() );
		::SetDlgItemText( hWnd, IDC_EDIT_TEXT, EditText.c_str() );
		::SetDlgItemText( hWnd, IDC_STATIC_PROMPT, Caption.c_str() );
		return TRUE;
	}
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK) {
			//OKƒ{ƒ^ƒ“‚ª‰Ÿ‚³‚ê‚½‚Æ‚«‚Ìˆ—
			::EndDialog(hWnd, IDOK);
			return TRUE;
		} else if(LOWORD(wParam) == IDCANCEL) {
			char str[256];
			str[0] = 0;
            GetDlgItemText( hWnd, IDC_EDIT_TEXT, str, 255 );
			Caption = std::string(str);
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;
    case WM_CLOSE:
        EndDialog(hWnd, 0);
        return TRUE;
	default:
		break;
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}
bool InputQuery( const std::string& title, const std::string& caption, std::string& edit ) {
	Title = title;
	Caption = caption;
	EditText = edit;
	INT_PTR result = ::DialogBox( NULL, MAKEINTRESOURCE(IDD_INPUT_STRING), NULL, (DLGPROC)DlgProc );
	edit = EditText;
	return result > 0;
}
