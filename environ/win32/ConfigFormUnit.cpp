//---------------------------------------------------------------------------
// Config dialog box
//---------------------------------------------------------------------------

#include "tjsCommHead.h"

#include "Resource.h"

#include "MsgIntf.h"
#include "ConfigFormUnit.h"
#include "SystemControl.h"
#include "WindowImpl.h"

#include "DebugIntf.h"
#include "ReadOptionDesc.h"
#include "SysInitIntf.h"
#include <commctrl.h>
#include <windowsx.h>
#include "Application.h"
#include "FilePathUtil.h"
#include "ApplicationSpecialPath.h"
#include "BinaryStream.h"
#include "CharacterSet.h"
/*
IDC_DESCRIPTION_EDIT : 説明テキスト
IDC_DEFAULT_BUTTON : デフォルトボタン
IDC_OPTION_COMBO
IDC_OPTIONS_TREE
*/
struct TreeItem {
	HTREEITEM    ItemHandle;
	tjs_string Text; // 項目名
	tjs_string Caption; // パラメータの名前 + 設定値
	tjs_string Parameter; // パラメータの名前
	tjs_int Value; // パラメータの設定値インデックス
	tjs_int Defalut; // パラメータのデフォルト値インデックス
	tjs_string Description; // 項目の説明
	std::vector<std::pair<tjs_string, tjs_string> > Select; // 設定可能な値のリスト
};

class ConfigFormUnit {
	HWND WindowHandle;
	HWND TreeControl;
	HWND DefaultButton;
	HWND OptionList;
	int OptionListWidth;
	int OptionListHeight;

	TreeItem* CurrentItem;

	std::vector<TreeItem> TreeItems;

private:
	// TreeControl にアイテム追加
	HTREEITEM InsertTreeItem( HTREEITEM hParent, const tjs_string& text ) {
		TV_INSERTSTRUCT tvinsert;
		ZeroMemory( &tvinsert, sizeof(tvinsert) );
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.item.mask = TVIF_TEXT;
		tvinsert.hParent = hParent;
		tvinsert.item.pszText = const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(text.c_str()));
		return TreeView_InsertItem( TreeControl, &tvinsert );
	}
	void SetTreeItem( HTREEITEM hItem, const tjs_string& text ) {
		TVITEM tvitem;
		ZeroMemory( &tvitem, sizeof(tvitem) );
		tvitem.mask = TVIF_TEXT;
		tvitem.pszText = const_cast<LPWSTR>( reinterpret_cast<LPCWSTR>( text.c_str()));
		tvitem.hItem = hItem;
		TreeView_SetItem( TreeControl, &tvitem );
	}
	// 項目を選択する
	void SelectItem( const TreeItem& item ) {
		// 説明更新
		::SetDlgItemText( WindowHandle, IDC_DESCRIPTION_EDIT, reinterpret_cast<LPCWSTR>(item.Description.c_str()) );

		// 選択肢更新
		::EnableWindow( OptionList, TRUE );
		UpdateItemList( item );
		
		// デフォルトボタン更新
		SetDefaultButtonEnable( item.Defalut != item.Value );
	}
	void ClearOptionList() {
		ComboBox_ResetContent(OptionList);
	}
	void UpdateItemList( const TreeItem& item ) {
		ClearOptionList();
		tjs_uint count = (tjs_uint)item.Select.size();
		for( tjs_uint i = 0; i < count; i++ ) {
			tjs_string itemstr = item.Select[i].second + tjs_string(TJS_W(" / ")) + item.Select[i].first;
			ComboBox_InsertString( OptionList, -1, itemstr.c_str() );
		}
		ComboBox_SetCurSel( OptionList, item.Value );
		count = count > 12 ? 12 : count;
		if( count <= 0 ) count = 1;
		int itemheight = ComboBox_GetItemHeight(OptionList);
		::SetWindowPos( OptionList, 0, 0, 0, OptionListWidth, itemheight * count + OptionListHeight + 2, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_HIDEWINDOW);
		::SetWindowPos( OptionList, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	}
	void SelectNull() {
		::SetDlgItemText( WindowHandle, IDC_DESCRIPTION_EDIT, reinterpret_cast<LPCWSTR>(TJS_W("")) );
		ClearOptionList();
		::EnableWindow( OptionList, FALSE );
		SetDefaultButtonEnable( false );
	}
	void ChangeOptionValue() {
		if( CurrentItem ) {
			bool ischange = false;
			int sel = ComboBox_GetCurSel( OptionList );
			if( sel != CB_ERR ) {
				ischange = CurrentItem->Value != sel;
				CurrentItem->Value = sel;
			}
			if( ischange ) {
				if( CurrentItem->Defalut != CurrentItem->Value ) {
					CurrentItem->Caption = tjs_string(TJS_W("* ")) + CurrentItem->Text + tjs_string(TJS_W(" : ")) + CurrentItem->Select[CurrentItem->Value].second;
					SetDefaultButtonEnable( true );
				} else {
					CurrentItem->Caption = CurrentItem->Text + tjs_string(TJS_W(" : ")) + CurrentItem->Select[CurrentItem->Value].second;
					SetDefaultButtonEnable( false );
				}
				if( CurrentItem->ItemHandle ) {
					SetTreeItem( CurrentItem->ItemHandle, CurrentItem->Caption );
				}
			}
		}
	}
	void RestoreOptionToDefaultValue() {
		if( CurrentItem ) {
			if( CurrentItem->Defalut != CurrentItem->Value ) {
				CurrentItem->Value = CurrentItem->Defalut;
				CurrentItem->Caption = CurrentItem->Text + tjs_string( TJS_W( " : " ) ) + CurrentItem->Select[CurrentItem->Value].second;
				if( CurrentItem->ItemHandle ) {
					SetTreeItem( CurrentItem->ItemHandle, CurrentItem->Caption );
				}
				ComboBox_SetCurSel( OptionList, CurrentItem->Value );
				SetDefaultButtonEnable( false );
			}
		}
	}
	// 指定フォルダにある指定拡張子のプラグインから設定情報を読み込む
	void LoadPluginOptionDesc( tTVPCommandOptionList* coreopt, const tjs_string& path, const tjs_string& ext ) {
		tjs_string exename = ExePath();
		exename = IncludeTrailingBackslash(ExtractFileDir(exename));
		tjs_string filepath = exename + path;
		tjs_string mask = filepath + TJS_W("*.") + ext;

		WIN32_FIND_DATA fd;
		HANDLE hSearch = ::FindFirstFile( reinterpret_cast<LPCWSTR>(mask.c_str()), &fd );
		if( hSearch != INVALID_HANDLE_VALUE ) {
			do {
				tTVPCommandOptionList* options = TVPGetPluginCommandDesc( (filepath + tjs_string( reinterpret_cast<tjs_char*>(fd.cFileName)) ).c_str() );
				if( options ) {
					TVPMargeCommandDesc( *coreopt, *options );
					delete options;
				}
			} while( FindNextFile( hSearch, &fd ) );
			::FindClose( hSearch );
		}
	}
public:
	ConfigFormUnit( HWND hWnd ) : WindowHandle(hWnd), CurrentItem(NULL) {
		SetDlgTexts( hWnd );
		TreeControl = ::GetDlgItem( WindowHandle, IDC_OPTIONS_TREE );
		DefaultButton = ::GetDlgItem( WindowHandle, IDC_DEFAULT_BUTTON );
		OptionList = ::GetDlgItem( WindowHandle, IDC_OPTION_COMBO );
		RECT rc;
		::GetWindowRect( OptionList, &rc );
		OptionListWidth = rc.right - rc.left;
		OptionListHeight = rc.bottom - rc.top;

		SetDefaultButtonEnable( false );
		LoadOptionTree();
	}
	~ConfigFormUnit() {
	}
	void SetDlgTexts( HWND hWnd ) {
		const UINT DLG_ITEM_IDS[] = {
			IDOK,
			IDCANCEL,
			IDC_OPTION,
			IDC_OPTION_NAME,
			IDC_OPTION_VALUE,
			IDC_OPTION_DESC,
			IDC_DEFAULT_BUTTON
		};
		const UINT DLG_STR_IDS[] = {
			IDS_DLG_OK,
			IDS_DLG_CANCEL,
			IDS_DLG_OPTION,
			IDS_DLG_OPTION_NAME,
			IDS_DLG_OPTION_VALUE,
			IDS_DLG_OPTION_DESC,
			IDS_DLG_SET_DEFAULT
		};
		const int MAX_LENGTH = 1024;
		const int num_of_items = sizeof(DLG_ITEM_IDS)/sizeof(DLG_ITEM_IDS[0]);
		HINSTANCE hInstance = ::GetModuleHandle(0);
		tjs_char buffer[MAX_LENGTH];
		for( int i = 0; i < num_of_items; i++ ) {
			HWND hBtn = ::GetDlgItem( hWnd, DLG_ITEM_IDS[i] );
			if( hBtn != NULL ) {
				int len = ::LoadString( hInstance, DLG_STR_IDS[i], buffer, 1024 );
				if( len > 0 ) {
					::SetDlgItemText( hWnd, DLG_ITEM_IDS[i], buffer );
				}
			}
		}
		int len = ::LoadString( hInstance, IDS_DLG_SETTINGS, buffer, 1024 );
		if( len > 0 ) {
			::SetWindowText( hWnd, buffer );
		}
	}
	static LRESULT WINAPI DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	LRESULT Dispatch( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void SetDefaultButtonEnable( bool enable ) {
		::EnableWindow( DefaultButton, enable ? TRUE : FALSE );
	}

	void SetDescriptionText( const tjs_string& text ) {
		::SetDlgItemText( WindowHandle, IDC_DESCRIPTION_EDIT, reinterpret_cast<LPCWSTR>(text.c_str()) );
	}
	void LoadOptionTree();
	// 現在のアイテムを設定し、それに従い表示を更新する
	void SetCurrentItem( HTREEITEM hItem ) {
		if( CurrentItem != NULL && CurrentItem->ItemHandle == hItem ) return;

		CurrentItem = NULL;
		for( std::vector<TreeItem>::iterator i = TreeItems.begin(); i != TreeItems.end(); i++ ) {
			if( (*i).ItemHandle == hItem ) {
				CurrentItem = &(*i);
				break;
			}
		}
		if( CurrentItem ) {
			SelectItem( *CurrentItem );
		} else {
			SelectNull();
		}
	}
	void ConvertReturnCode( tjs_string& text ) {
		std::vector<tjs_char> buf;
		size_t len = text.length();
		buf.reserve( len * 2 );
		for( size_t i = 0; i < len; i++ ) {
			if( text[i] != TJS_W('\n') ) {
				buf.push_back( text[i] );
			} else {
				buf.push_back( TJS_W('\r') );
				buf.push_back( TJS_W('\n') );
			}
		}
		text.assign( &(buf[0]), buf.size() );
	}
	std::string ToStringOption() {
		std::string ret;
		for( std::vector<TreeItem>::const_iterator i = TreeItems.begin() ; i != TreeItems.end(); i++ ) {
			const TreeItem& item = *i;
			if( item.Value != item.Defalut ) {
				std::string name;
				TVPUtf16ToUtf8( name, item.Parameter );
				ret += name;
				ret += "=";
				ret += EncodeString( item.Select[item.Value].first );
				ret += "\r\n";
			}
		}
		return ret;
	}
	// 設定された内容を保存する
	void SaveSetting() {
		tTJSVariant val;
		tjs_string path;
		if( TVPGetCommandLine( TJS_W("-datapath"), &val) ) {
			ttstr str(val);
			path.assign( str.c_str(), str.length() );
		}
		tjs_string exename = ExePath();
		tjs_string filename = ChangeFileExt( ExtractFileName( exename ), TJS_W(".cfu") );
		filename = ApplicationSpecialPath::GetDataPathDirectory( path, exename ) + filename;
		const char * warnings =
"; ============================================================================\r\n"
"; *DO NOT EDIT* this file unless you are understanding what you are doing.\r\n"
"; FYI:\r\n"
";  Each line consists of NAME=\"VALUE\" pair, VALUE is a series of\r\n"
";  \\xNN, where NN is hexadecimal representation of UNICODE codepoint.\r\n"
";  For example, opt=\"\\x61\\x62\\x63\\x3042\\x3044\\x3046\" means that the\r\n"
";  value of options \"opt\" is alphabets a, b, and c followed by Japanese\r\n"
";  Hiraganas A, I, and U.\r\n"
";  DO NOT PUT non-escaped value like opt=\"abc\". This doesn't work and should\r\n"
";  be like opt=\"\\x61\\x62\\x63\".\r\n"
"; ============================================================================\r\n"
"";
		tTJSBinaryStream *stream = TVPCreateBinaryStreamForWrite( ttstr(filename), TJS_W("") );
		if( stream ) {
			try {
				stream->Write( warnings, (tjs_uint)strlen(warnings) );
				std::string option = ToStringOption();
				stream->Write( option.c_str(), (tjs_uint)option.length() );
			} catch(...) {
				delete stream;
				throw;
			}
			delete stream;
		}

	}
	static int HexNum(tjs_char ch) {
		if(ch>='a' && ch<='f')return ch-'a'+10;
		if(ch>='A' && ch<='F')return ch-'A'+10;
		if(ch>='0' && ch<='9')return ch-'0';
		return -1;
	} 
	static std::string DecodeString( const tjs_string& str ) {
		if( str.empty() ) return "";
		ttstr ret;
		const tjs_char *p = str.c_str();
		if(p[0] != '\"') return ttstr(str).AsNarrowStdString();

		p++;
		while( *p ) {
			if(*p != '\\') break;
			p++;
			if(*p != 'x') break;
			p++;
			tjs_char ch = 0;
			while( true ) {
				int n = HexNum(*p);
				if(n == -1) break;
				ch <<= 4;
				ch += n;
				p++;
			}
			ret += ch;
		}
		return ret.AsNarrowStdString();
	}
	static std::string EncodeString( const tjs_string& str ) {
		if( str.empty() ) return "\"\"";
		ttstr ws(str.c_str());
		const tjs_char *p = ws.c_str();

		std::string ret = "\"";
		while( *p ) {
			char tmp[10];
			TJS_nsprintf(tmp, "\\x%X", *p);
			ret += tmp;
			p++;
		}
		return ret + "\"";
	}
};
static ConfigFormUnit* DialogForm = NULL;

// 本体とプラグインの設定情報を読込み、ツリーに反映する
void ConfigFormUnit::LoadOptionTree() {
	tTVPCommandOptionList* options = TVPGetEngineCommandDesc();
	if( options ) {
		LoadPluginOptionDesc( options, TJS_W("\\"), TJS_W("dll") );
		LoadPluginOptionDesc( options, TJS_W("\\"), TJS_W("tpm") );
#ifdef TJS_64BIT_OS
		LoadPluginOptionDesc( options, TJS_W("plugin64\\"), TJS_W("dll") );
		LoadPluginOptionDesc( options, TJS_W("plugin64\\"), TJS_W("tpm") );
#else
		LoadPluginOptionDesc( options, TJS_W("plugin\\"), TJS_W("dll") );
		LoadPluginOptionDesc( options, TJS_W("plugin\\"), TJS_W("tpm") );
#endif

		tTJSVariant val;
		HTREEITEM hFirst = NULL;
		// 有効なアイテム数をカウント
		tjs_uint itemcount = 0;
		tjs_uint count = (tjs_uint)options->Categories.size();
		for( tjs_uint i = 0; i < count; i++ ) {
			const tTVPCommandOptionCategory& category = options->Categories[i];
			tjs_uint optcount = (tjs_uint)category.Options.size();
			for( tjs_uint j = 0; j < optcount; j++ ) {
				if( category.Options[j].User ) itemcount++;
			}
		}
		TreeItems.resize(itemcount);

		tjs_uint itemidx = 0;
		for( tjs_uint i = 0; i < count; i++ ) {
			const tTVPCommandOptionCategory& category = options->Categories[i];
			tjs_uint optcount = (tjs_uint)category.Options.size();
			// まずはカテゴリに有効なアイテムがあるかチェックする
			bool hasitem = false;
			for( tjs_uint j = 0; j < optcount; j++ ) {
				if( category.Options[j].User ) {
					hasitem = true;
					break;
				}
			}
			if( hasitem == false ) continue;
			HTREEITEM hItem = InsertTreeItem( TVI_ROOT, category.Name );
			if( hFirst == NULL ) hFirst = hItem;
			for( tjs_uint j = 0; j < optcount; j++ ) {
				const tTVPCommandOption& option = category.Options[j];
				if( option.User ) {
					TreeItem& curitem = TreeItems[itemidx];
					curitem.Text = option.Caption;
					curitem.Parameter = option.Name;
					curitem.Description = tjs_string(TJS_W("-"))+option.Name+tjs_string(TJS_W("\n"))+option.Description;
					ConvertReturnCode( curitem.Description );
					tjs_uint valcount = (tjs_uint)option.Values.size();
					curitem.Select.resize( valcount );
					curitem.Defalut = -1;
					tjs_string argname( tjs_string(TJS_W("-")) + option.Name );
					tjs_string selectvalue;
					if( TVPGetCommandLine( argname.c_str(), &val) ) {
						ttstr str(val);
						selectvalue.assign( str.c_str(), str.length() );
					}
					tjs_int selectindex = -1;
					for( tjs_uint k = 0; k < valcount; k++ ) {
						std::pair<tjs_string, tjs_string>& sel = curitem.Select[k];
						const tTVPCommandOptionsValue& val = option.Values[k];
						sel.first = val.Value;
						sel.second = val.Description;
						if( val.IsDefault ) {
							curitem.Defalut = k;
						}
						if( selectindex < 0 && !selectvalue.empty() && selectvalue == val.Value ) {
							selectindex = k;
						}
					}
					if( selectindex >= 0 ) {
						curitem.Value = selectindex;
					} else if( curitem.Defalut >= 0 ) {
						curitem.Value = curitem.Defalut;
					} else {
						curitem.Value = 0;
					}

					if( curitem.Defalut != curitem.Value ) {
						curitem.Caption = tjs_string(TJS_W("* ")) + curitem.Text + tjs_string(TJS_W(" : ")) + curitem.Select[curitem.Value].second;
					} else {
						curitem.Caption = curitem.Text + tjs_string(TJS_W(" : ")) + curitem.Select[curitem.Value].second;
					}
					curitem.ItemHandle = InsertTreeItem( hItem, curitem.Caption );
					itemidx++;
				}
			}
			TreeView_Expand( TreeControl, hItem, TVE_EXPAND );
		}
		TreeView_SelectItem( TreeControl, hFirst );
		delete options;
	}
}

LRESULT ConfigFormUnit::Dispatch( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK) {
			::EndDialog(hWnd, IDOK);
			return TRUE;
		} else if( LOWORD(wParam) == IDCANCEL) {
			::EndDialog(hWnd, IDCANCEL);
			return TRUE;
		} else if( LOWORD(wParam) == IDC_DEFAULT_BUTTON) {
			RestoreOptionToDefaultValue();
			return TRUE;
		} else if( HIWORD(wParam) == CBN_SELCHANGE ) {
			ChangeOptionValue();
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	 case WM_NOTIFY:
		if( wParam == IDC_OPTIONS_TREE ) {
			// ツリーコントロールの通知で現在選択されているアイテムを認識
			NMHDR* hdr = (LPNMHDR)lParam;
			UINT code = hdr->code;
			if( code == NM_SETCURSOR || code == NM_CLICK || code == NM_SETFOCUS) {
				HTREEITEM hItem = TreeView_GetSelection( TreeControl );
				if( hItem ) SetCurrentItem( hItem );
			}
		}
		break;
	default:
		break;
	}
	return FALSE;
}

LRESULT WINAPI ConfigFormUnit::DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	if( msg == WM_INITDIALOG ) {
		if( DialogForm ) delete DialogForm;
		DialogForm = new ConfigFormUnit(hWnd);
		return TRUE;
	} else {
		if( DialogForm ) return DialogForm->Dispatch( hWnd, msg, wParam, lParam );
	}
	return FALSE;
}
//---------------------------------------------------------------------------
void TVPShowUserConfig()
{
	try {
		INT_PTR result = ::DialogBox( NULL, MAKEINTRESOURCE(IDD_CONFIG_DIALOG), NULL, (DLGPROC)ConfigFormUnit::DlgProc );
		if( result == IDOK ) {
			if( DialogForm ) {
				DialogForm->SaveSetting();
				::MessageBox( NULL, TJS_W("設定を保存しました"), TJS_W("Save Option"), MB_OK );
			}
		}
	} catch(...) {
		if( DialogForm ) {
			delete DialogForm;
			DialogForm = NULL;
		}
		throw;
	}
	if( DialogForm ) {
		delete DialogForm;
		DialogForm = NULL;
	}
}
//---------------------------------------------------------------------------




