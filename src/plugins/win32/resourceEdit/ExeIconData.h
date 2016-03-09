
#ifndef __EXE_ICON_DATA_H__
#define __EXE_ICON_DATA_H__

#include <windows.h>
#include <vector>

class ExeIconData {
	static const int DEFAULT_ID = 2000;

	LPTSTR icon_group_;			// アイコングループ
	WORD lang_id_;				// アイコングループのロケール
	WORD min_id_;
	std::vector<WORD> icon_ids_;// アイコングループのアイコンID

	static BOOL CALLBACK EnumResNameProc( HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam );
	static BOOL CALLBACK EnumResLangProc( HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam );
public:
	ExeIconData() : icon_group_(NULL), lang_id_(USHRT_MAX), min_id_(USHRT_MAX) {}

	void SetIconGroup( LPTSTR icon ) { icon_group_ = icon; }
	LPTSTR GetIconGroup() const { 
		if( icon_group_!= NULL ) {
			return icon_group_;
		} else {
			return MAKEINTRESOURCE(DEFAULT_ID);
		}
	}

	void SetLangID( WORD id ) { lang_id_ = id; }
	WORD GetLangID() const {
		if( lang_id_ != USHRT_MAX ) {
			return lang_id_;
		} else {
			return MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
		}
	}

	int GetIconCount() const { return icon_ids_.size(); }
	WORD GetIconID( int index ) { return icon_ids_.at(index); }

	WORD GetMinIconID() const {
		if( min_id_ != USHRT_MAX ) {
			return DEFAULT_ID;
		} else {
			return min_id_;
		}
	}

	void Load(LPCTSTR pszFileName);
};


#endif // __EXE_ICON_DATA_H__
