
#ifndef __EDIT_RESOURCE_H__
#define __EDIT_RESOURCE_H__

#include <windows.h>

enum {
	ERR_EXE_SUCCESS = 1,
	ERR_EXE_WRITE_OPEN_ERROR = -4,
	ERR_EXE_ADD_ICON_ERROR = -5,
	ERR_EXE_ADD_ICON_GROUP_ERROR = -6,
	ERR_EXE_WRITE_CHANGES_ERROR = -7,
	ERR_EXE_FILE_READ_ERROR = -8,
};


int ReplaceIcon( LPCTSTR pszExeFilePath, LPCTSTR pszIconFilePath );
int UpdateBinaryResource( LPCTSTR pszExeFilePath, BYTE* buff, DWORD length,  LPCTSTR pszType, LPCTSTR pszName );
int UpdateBinaryResource( LPCTSTR pszExeFilePath, LPCTSTR pszBinaryFilePath, LPCTSTR pszType, LPCTSTR pszName );

#endif // __EDIT_RESOURCE_H__
