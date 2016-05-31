

#include "ExeIconData.h"
#include "IconFile.h"


BOOL CALLBACK ExeIconData::EnumResNameProc( HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam ) {
	ExeIconData* icon = (ExeIconData*)lParam;
	icon->SetIconGroup( lpszName );
	return FALSE;
}
BOOL CALLBACK ExeIconData::EnumResLangProc( HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam ) {
	ExeIconData* icon = (ExeIconData*)lParam;
	icon->SetLangID( wIDLanguage );
	return FALSE;
}
void ExeIconData::Load(LPCTSTR pszFileName) {
	icon_ids_.clear();
	HINSTANCE hFile = LoadLibraryEx( pszFileName, 0, LOAD_LIBRARY_AS_DATAFILE | LOAD_WITH_ALTERED_SEARCH_PATH );
	if( hFile != NULL ) {
		::EnumResourceNames( hFile, RT_GROUP_ICON, &EnumResNameProc, (LONG_PTR)this );
		LPTSTR group = GetIconGroup();
		if( group != NULL ) {
			::EnumResourceLanguages( hFile, RT_GROUP_ICON, group, (ENUMRESLANGPROC)&EnumResLangProc, (LONG_PTR)this );
			WORD lang = GetLangID();
			if( lang != USHRT_MAX ) {
				HRSRC hRes = ::FindResource(hFile, group, RT_GROUP_ICON);
				if( hRes != NULL ) {
					HGLOBAL hResLoad = ::LoadResource(hFile, hRes);
					if( hResLoad != NULL ) {
						DWORD size = ::SizeofResource(hFile, hRes);
						LPVOID lpResLock = ::LockResource(hResLoad);
						DWORD offset = sizeof(ICONDIR);
						BYTE* pos = (BYTE*)lpResLock;
						pos += offset;
						GRPICONDIRENTRY* entry = (GRPICONDIRENTRY*)pos;
						DWORD count = (size-offset) / sizeof(GRPICONDIRENTRY);
						icon_ids_.reserve( count );
						for( DWORD i = 0; i < count; i++ ) {
							WORD id = entry[i].nID;
							if( id < min_id_ ) {
								min_id_ = id;
							}
							icon_ids_.push_back( id );
						}
					}
				} 
			}
		}
		::FreeLibrary( hFile );
	}
}