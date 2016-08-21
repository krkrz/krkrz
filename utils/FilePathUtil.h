
#ifndef __FILE_PATH_UTIL_H__
#define __FILE_PATH_UTIL_H__

#include <string>
#include <stdlib.h>

#ifdef _WIN32
#include <shlwapi.h>
#include <winnetwk.h>
#else
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#define _MAX_PATH   260 /* max. length of full pathname */
#define _MAX_DRIVE  3   /* max. length of drive component */
#define _MAX_DIR    256 /* max. length of path component */
#define _MAX_FNAME  256 /* max. length of file name component */
#define _MAX_EXT    256 /* max. length of extension component */
#include "CharacterSet.h"
#endif


inline std::wstring IncludeTrailingBackslash( const std::wstring& path ) {
	if( path[path.length()-1] != L'\\' ) {
		return std::wstring(path+L"\\");
	}
	return std::wstring(path);
}
inline std::wstring ExcludeTrailingBackslash( const std::wstring& path ) {
	if( path[path.length()-1] == L'\\' ) {
		return std::wstring(path.c_str(),path.length()-1);
	}
	return std::wstring(path);
}
// 末尾の /\ は含まない
inline std::wstring ExtractFileDir( const std::wstring& path ) {
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
#ifdef _WIN32
	_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	std::wstring dirstr = std::wstring( dir );
	if( dirstr[dirstr.length()-1] != L'\\' ) {
		return std::wstring( drive ) + dirstr;
	} else {
		return std::wstring( drive ) + dirstr.substr(0,dirstr.length()-1);
	}
#else
	std::wstring::size_type pos = path.find_last_of( L"\\/" );
	if( pos != std::wstring::npos ) {
		return path.substr( 0, pos );
	} else {
		return path;
	}
#endif
}
// 末尾の /\ を含む
inline std::wstring ExtractFilePath( const std::wstring& path ) {
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
#ifdef _WIN32
	_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	return std::wstring( drive ) + std::wstring( dir );
#else
	std::wstring::size_type pos = path.find_last_of( L"\\/" );
	if( pos != std::wstring::npos ) {
		return path.substr( 0, pos+1 );
	} else {
		return path;
	}
#endif
}

inline bool DirectoryExists( const std::wstring& path ) {
#ifdef _WIN32
	return (0!=::PathIsDirectory(path.c_str()));
#else
	std::string npath;
	if( TVPUtf16ToUtf8( npath, path ) ) {
		struct stat st;
		if( stat( npath.c_str(), &st ) == 0 ) {
			return S_ISDIR(st.st_mode );
		}
	}
	return false;
#endif
}
inline bool FileExists( const std::wstring& path ) {
#ifdef _WIN32
	return ( (0!=::PathFileExists(path.c_str())) && (0==::PathIsDirectory(path.c_str())) );
#else
	std::string npath;
	if( TVPUtf16ToUtf8( npath, path ) ) {
		return ::access( npath.c_str(), F_OK) == 0;
	}
	return false;
#endif
}
inline std::wstring ChangeFileExt( const std::wstring& path, const std::wstring& ext ) {
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
#ifdef _WIN32
	_wsplitpath_s( path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, NULL, 0 );
	return std::wstring( drive ) + std::wstring( dir ) + std::wstring( fname ) + ext;
#else
	std::wstring::size_type pos = path.find_last_of( L'.' );
	if( pos != std::wstring::npos ) {
		return path.substr( 0, pos+1 ) + ext;
	} else {
		return path + ext;
	}
#endif
}
inline std::wstring ExtractFileName( const std::wstring& path ) {
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
#ifdef _WIN32
	_wsplitpath_s( path.c_str(), NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );
	return std::wstring( fname ) + std::wstring( ext );
#else
	std::wstring::size_type pos = path.find_last_of( L"\\/" );
	if( pos != std::wstring::npos ) {
		return path.substr( pos+1 );
	} else {
		return path;
	}
#endif
}
inline std::wstring ExpandUNCFileName( const std::wstring& path ) {
#ifdef _WIN32
	std::wstring result;
	DWORD InfoSize = 0;
	if( ERROR_MORE_DATA == WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, NULL, &InfoSize) ) {
		UNIVERSAL_NAME_INFO* pInfo = reinterpret_cast<UNIVERSAL_NAME_INFO*>( ::GlobalAlloc(GMEM_FIXED, InfoSize) );
		DWORD ret = ::WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, pInfo, &InfoSize);
		if( NO_ERROR == ret ) {
			result = std::wstring(pInfo->lpUniversalName);
		}
		::GlobalFree(pInfo);
	} else {
		wchar_t fullpath[_MAX_PATH];
		result = std::wstring( _wfullpath( fullpath, path.c_str(), _MAX_PATH ) );
	}
	return result;
#else
	return path;
#endif
}

#endif // __FILE_PATH_UTIL_H__
