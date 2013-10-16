
#include "tjsCommHead.h"
#include "InputStringForm.h"
#include "Resource.h"

static std::wstring Title;
static std::wstring Caption;
static std::wstring EditText;

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
			wchar_t str[256];
			str[0] = 0;
            GetDlgItemText( hWnd, IDC_EDIT_TEXT, str, 255 );
			Caption = std::wstring(str);
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
bool InputQuery( const std::wstring& title, const std::wstring& caption, std::wstring& edit ) {
	Title = title;
	Caption = caption;
	EditText = edit;
	INT_PTR result = ::DialogBox( NULL, MAKEINTRESOURCE(IDD_INPUT_STRING), NULL, (DLGPROC)DlgProc );
	edit = EditText;
	return result > 0;
}
