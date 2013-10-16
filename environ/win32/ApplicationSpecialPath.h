
#ifndef __APPLICATION_SPECIAL_PATH_H__
#define __APPLICATION_SPECIAL_PATH_H__

#include <shlobj.h>
#include "FilePathUtil.h"

class ApplicationSpecialPath {
public:
	static std::wstring GetSpecialFolderPath(int csidl) {
		wchar_t path[MAX_PATH+1];
		if(!SHGetSpecialFolderPath(NULL, path, csidl, false))
			return std::wstring();
		return std::wstring(path);
	}
	static inline std::wstring GetPersonalPath() {
		std::wstring path = GetSpecialFolderPath(CSIDL_PERSONAL);
		if( path.empty() ) path = GetSpecialFolderPath(CSIDL_APPDATA);

		if(path != L"") {
			return path;
		}
		return L"";
	}
	static inline std::wstring GetAppDataPath() {
		std::wstring path = GetSpecialFolderPath(CSIDL_APPDATA);
		if(path != L"" ) {
			return path;
		}
		return L"";
	}
	static inline std::wstring ReplaceStringAll( std::wstring src, const std::wstring& target, const std::wstring& dest ) {
		int nPos = 0;
		while( (nPos = src.find(target, nPos)) != std::wstring::npos ) {
			src.replace( nPos, target.length(), dest );
		}
		return src;
	}

	static inline std::wstring GetConfigFileName( const std::wstring& exename ) {
		return ChangeFileExt(exename, L".cf");
	}
	static std::wstring GetDataPathDirectory( std::wstring datapath, const std::wstring& exename ) {
		if(datapath == L"" ) datapath = std::wstring(L"$(exepath)\\savedata");

		std::wstring exepath = ExcludeTrailingBackslash(ExtractFileDir(exename));
		std::wstring personalpath = ExcludeTrailingBackslash(GetPersonalPath());
		std::wstring appdatapath = ExcludeTrailingBackslash(GetAppDataPath());
		if(personalpath == L"") personalpath = exepath;
		if(appdatapath == L"") appdatapath = exepath;

		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osinfo);

		bool vista_or_later = osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osinfo.dwMajorVersion >= 6;

		std::wstring vistapath = vista_or_later ? appdatapath : exepath;

		datapath = ReplaceStringAll(datapath, L"$(exepath)", exepath);
		datapath = ReplaceStringAll(datapath, L"$(personalpath)", personalpath);
		datapath = ReplaceStringAll(datapath, L"$(appdatapath)", appdatapath);
		datapath = ReplaceStringAll(datapath, L"$(vistapath)", vistapath);
		return IncludeTrailingBackslash(ExpandUNCFileName(datapath));
	}
	static std::wstring GetUserConfigFileName( const std::wstring& datapath, const std::wstring& exename ) {
		// exepath, personalpath, appdatapath
		return GetDataPathDirectory(datapath, exename) + ExtractFileName(ChangeFileExt(exename, L".cfu"));
	}
};


#endif // __APPLICATION_SPECIAL_PATH_H__
