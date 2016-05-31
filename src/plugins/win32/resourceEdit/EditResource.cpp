
#include "EditResource.h"
#include "IconFile.h"
#include "ExeIconData.h"

// http://www.geocities.jp/asumaroyuumaro/program/tips/ExeIconChange.html

int ReplaceIcon( LPCTSTR pszExeFilePath, LPCTSTR pszIconFilePath ) {
	// read exe icon infomation
	ExeIconData icondata;
	icondata.Load( pszExeFilePath );
	
	// read icon file
	IconFile iconFile;
	int err;
	if( (err = iconFile.Load( pszIconFilePath )) != IconFile::ERR_ICON_SUCCESS ) {
		return err;
	}

	// start icon update
	HANDLE hUpdate = ::BeginUpdateResource( pszExeFilePath, FALSE);
	if( hUpdate == NULL ) {
		return ERR_EXE_WRITE_OPEN_ERROR;	// Could not open file for writing.
	}

	// delete icons
	for( int i = 0; i < icondata.GetIconCount(); i++ ) {
		WORD id = icondata.GetIconID(i);
		::UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(id), icondata.GetLangID(), NULL, 0 );
	}

	WORD iconBaseId = icondata.GetMinIconID();
	// add icons
	BOOL result;
	for( int i = 0; i < iconFile.GetImageCount(); i++ ) {
		result = ::UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(iconBaseId+i), icondata.GetLangID(),
			iconFile.GetImageData(i), iconFile.GetImageSize(i));
		if( result == FALSE ) {
			return ERR_EXE_ADD_ICON_ERROR;	// Could not add resource(RT_ICON).
		} 
	}
	// update icon group
	result = ::UpdateResource(hUpdate, RT_GROUP_ICON, icondata.GetIconGroup(), icondata.GetLangID(),
		iconFile.CreateIconGroupData(iconBaseId), iconFile.SizeOfIconGroupData());
	if( result == FALSE ) {
		return ERR_EXE_ADD_ICON_GROUP_ERROR;	// Could not add resource(RT_GROUP_ICON).
	}

	// close and commit icon update
	if( !::EndUpdateResource(hUpdate, FALSE) ) {
		return ERR_EXE_WRITE_CHANGES_ERROR;	// Could not write changes to file.
	}
	return ERR_EXE_SUCCESS;
}

int UpdateBinaryResource( LPCTSTR pszExeFilePath, BYTE* buff, DWORD length,  LPCTSTR pszType, LPCTSTR pszName ) {
	// start update
	HANDLE hUpdate = ::BeginUpdateResource( pszExeFilePath, FALSE);
	if( hUpdate == NULL ) {
		return ERR_EXE_WRITE_OPEN_ERROR;	// Could not open file for writing.
	}

	// update binary
	BOOL result = ::UpdateResource(hUpdate, pszType, pszName, MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), buff, length);
	if( result == FALSE ) {
		return ERR_EXE_WRITE_CHANGES_ERROR;	// Could not add resource.
	}

	// close and commit update
	if( !::EndUpdateResource(hUpdate, FALSE) ) {
		return ERR_EXE_WRITE_CHANGES_ERROR;	// Could not write changes to file.
	}
	return ERR_EXE_SUCCESS;
}

int UpdateBinaryResource( LPCTSTR pszExeFilePath, LPCTSTR pszBinaryFilePath, LPCTSTR pszType, LPCTSTR pszName ) {
	// read binary file
	BYTE* buff = NULL;
	DWORD fileSize = 0;
	{
		HANDLE hFile = ::CreateFile(pszBinaryFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE) return ERR_EXE_FILE_READ_ERROR;
		fileSize = ::GetFileSize( hFile, NULL );
		if( fileSize == 0 ) return ERR_EXE_FILE_READ_ERROR;
		DWORD dwRead;
		buff = new BYTE[fileSize];
		if( buff == NULL ) return ERR_EXE_FILE_READ_ERROR;
		BOOL result = ::ReadFile(hFile, buff, fileSize, &dwRead, NULL);
		if( result == FALSE || fileSize != dwRead) {
			delete[] buff;
			return ERR_EXE_FILE_READ_ERROR;
		}
		::CloseHandle(hFile);
	}
	int result = UpdateBinaryResource( pszExeFilePath, buff, fileSize, pszType, pszName );
	delete[] buff;
	return result;
}
