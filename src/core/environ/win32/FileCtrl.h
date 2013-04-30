
#ifndef __FILE_CTRL_H__
#define __FILE_CTRL_H__

#include <shlwapi.h>
#include <winnetwk.h>

inline std::string IncludeTrailingBackslash( const std::string& path ) {
	if( path[path.length()-1] != '\\' ) {
		return std::string(path+"\\");
	}
	return std::string(path);
}
inline std::string ExcludeTrailingBackslash( const std::string& path ) {
	if( path[path.length()-1] == '\\' ) {
		return std::string(path.c_str(),path.length()-1);
	}
	return std::string(path);
}

inline std::string ExtractFileDir( const std::string& path ) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	std::string dirstr = std::string( dir );
	if( dirstr[dirstr.length()-1] != '\\' ) {
		return std::string( drive ) + dirstr;
	} else {
		return std::string( drive ) + dirstr.substr(0,dirstr.length()-1);
	}
}
inline std::string ExtractFilePath( const std::string& path ) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	return std::string( drive ) + std::string( dir );
}

inline bool DirectoryExists( const std::string& path ) {
	return (0!=::PathIsDirectory(path.c_str()));
}
inline bool FileExists( const std::string& path ) {
	return ( (0!=::PathFileExists(path.c_str())) && (0==::PathIsDirectory(path.c_str())) );
}
inline std::string ChangeFileExt( const std::string& path, const std::string& ext ) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	_splitpath_s( path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, NULL, 0 );
	return std::string( drive ) + std::string( dir ) + std::string( fname ) + ext;
}
inline std::string ExtractFileName( const std::string& path ) {
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath_s( path.c_str(), NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );
	return std::string( fname ) + std::string( ext );
}
inline std::string ExpandUNCFileName( const std::string& path ) {
	std::string result;
	DWORD InfoSize = 0;
	if( ERROR_MORE_DATA == WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, NULL, &InfoSize) ) {
		UNIVERSAL_NAME_INFO* pInfo = reinterpret_cast<UNIVERSAL_NAME_INFO*>( ::GlobalAlloc(GMEM_FIXED, InfoSize) );
		DWORD ret = ::WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, pInfo, &InfoSize);
		if( NO_ERROR == ret ) {
			result = std::string(pInfo->lpUniversalName);
		}
		::GlobalFree(pInfo);
	} else {
		char fullpath[_MAX_PATH];
		result = std::string( _fullpath( fullpath, path.c_str(), _MAX_PATH ) );
	}
	return result;
}

#endif // __FILE_CTRL_H__
