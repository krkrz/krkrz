//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Version information dialog box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "Resource.h"

#include "tjsCommHead.h"

#include "MsgIntf.h"
#include "VersionFormUnit.h"
#include "MainFormUnit.h"
#include "WindowImpl.h"

#include "VersionFormUnit.h"
#include "DebugIntf.h"
#include "Clipbrd.h"

//---------------------------------------------------------------------------
// TVPCopyImportantLogToClipboard
//---------------------------------------------------------------------------
void TVPCopyImportantLogToClipboard()
{
	// get DirectDraw driver information
	TVPEnsureDirectDrawObject();
	TVPDumpDirectDrawDriverInformation();

	// copy
	TVPCopyToClipboard(TVPGetImportantLog());
}

static LRESULT WINAPI DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
	case WM_INITDIALOG: {
		::SetDlgItemText( hWnd, IDC_INFOMATION_EDIT, TVPGetAboutString().AsStdString().c_str() );
		return TRUE;
	}
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK) {
			//OKÉ{É^ÉìÇ™âüÇ≥ÇÍÇΩÇ∆Ç´ÇÃèàóù
			::EndDialog(hWnd, IDOK);
			return TRUE;
		} else if(LOWORD(wParam) == IDC_COPY_INFO_BUTTON) {
			TVPCopyImportantLogToClipboard();
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
//---------------------------------------------------------------------------
void TVPShowVersionForm()
{
	// get DirectDraw driver information
	TVPEnsureDirectDrawObject();
	TVPDumpDirectDrawDriverInformation();
	::DialogBox( NULL, MAKEINTRESOURCE(IDD_INPUT_STRING), NULL, (DLGPROC)DlgProc );
}
//---------------------------------------------------------------------------




