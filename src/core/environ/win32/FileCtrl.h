
#ifndef __FILE_CTRL_H__
#define __FILE_CTRL_H__

#include <shlwapi.h>
#include <winnetwk.h>
#include "tstring.h"

inline tstring IncludeTrailingBackslash( const tstring& path ) {
	if( path[path.length()-1] != _T('\\') ) {
		return tstring(path+_T("\\"));
	}
	return tstring(path);
}
inline tstring ExcludeTrailingBackslash( const tstring& path ) {
	if( path[path.length()-1] == _T('\\') ) {
		return tstring(path.c_str(),path.length()-1);
	}
	return tstring(path);
}

inline tstring ExtractFileDir( const tstring& path ) {
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	_tsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	tstring dirstr = tstring( dir );
	if( dirstr[dirstr.length()-1] != '\\' ) {
		return tstring( drive ) + dirstr;
	} else {
		return tstring( drive ) + dirstr.substr(0,dirstr.length()-1);
	}
}
inline tstring ExtractFilePath( const tstring& path ) {
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	_tsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	return tstring( drive ) + tstring( dir );
}

inline bool DirectoryExists( const tstring& path ) {
	return (0!=::PathIsDirectory(path.c_str()));
}
inline bool FileExists( const tstring& path ) {
	return ( (0!=::PathFileExists(path.c_str())) && (0==::PathIsDirectory(path.c_str())) );
}
inline tstring ChangeFileExt( const tstring& path, const tstring& ext ) {
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	_tsplitpath_s( path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, NULL, 0 );
	return tstring( drive ) + tstring( dir ) + tstring( fname ) + ext;
}
inline tstring ExtractFileName( const tstring& path ) {
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path.c_str(), NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );
	return tstring( fname ) + tstring( ext );
}
inline tstring ExpandUNCFileName( const tstring& path ) {
	tstring result;
	DWORD InfoSize = 0;
	if( ERROR_MORE_DATA == WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, NULL, &InfoSize) ) {
		UNIVERSAL_NAME_INFO* pInfo = reinterpret_cast<UNIVERSAL_NAME_INFO*>( ::GlobalAlloc(GMEM_FIXED, InfoSize) );
		DWORD ret = ::WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, pInfo, &InfoSize);
		if( NO_ERROR == ret ) {
			result = tstring(pInfo->lpUniversalName);
		}
		::GlobalFree(pInfo);
	} else {
		TCHAR fullpath[_MAX_PATH];
		result = tstring( _tfullpath( fullpath, path.c_str(), _MAX_PATH ) );
	}
	return result;
}

#endif // __FILE_CTRL_H__
