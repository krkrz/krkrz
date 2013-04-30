
#ifndef __CONF_MAIN_FRAME_UNIT_H__
#define __CONF_MAIN_FRAME_UNIT_H__

#include <windows.h>
#include <shlobj.h>
#include "FileCtrl.h"

class TConfMainFrame {
public:
	static std::string GetSpecialFolderPath(int csidl) {
		char path[MAX_PATH+1];
		if(!SHGetSpecialFolderPathA(NULL, path, csidl, false))
			return std::string();
		return std::string(path);
	}
	static inline std::string GetPersonalPath() {
		std::string path = GetSpecialFolderPath(CSIDL_PERSONAL);
		if( path.empty() ) path = GetSpecialFolderPath(CSIDL_APPDATA);

		if(path != "") {
			return path;
		}
		return "";
	}
	static inline std::string GetAppDataPath() {
		std::string path = GetSpecialFolderPath(CSIDL_APPDATA);
		if(path != "") {
			return path;
		}
		return "";
	}
	static inline std::string ReplaceStringAll( std::string src, const std::string& target, const std::string& dest ) {
		int nPos = 0;
		while( (nPos = src.find(target, nPos)) != std::string::npos ) {
			src.replace( nPos, target.length(), dest );
		}
		return src;
	}

	static inline std::string GetConfigFileName( const std::string& exename ) {
		return ChangeFileExt(exename, ".cf");
	}
	static std::string GetDataPathDirectory( std::string datapath, const std::string& exename ) {
		if(datapath == "") datapath = std::string("$(exepath)\\savedata");

		std::string exepath = ExcludeTrailingBackslash(ExtractFileDir(exename));
		std::string personalpath = ExcludeTrailingBackslash(GetPersonalPath());
		std::string appdatapath = ExcludeTrailingBackslash(GetAppDataPath());
		if(personalpath == "") personalpath = exepath;
		if(appdatapath == "") appdatapath = exepath;

		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osinfo);

		bool vista_or_later = osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osinfo.dwMajorVersion >= 6;

		std::string vistapath = vista_or_later ? appdatapath : exepath;

		datapath = ReplaceStringAll(datapath, "$(exepath)", exepath);
		datapath = ReplaceStringAll(datapath, "$(personalpath)", personalpath);
		datapath = ReplaceStringAll(datapath, "$(appdatapath)", appdatapath);
		datapath = ReplaceStringAll(datapath, "$(vistapath)", vistapath);
		return IncludeTrailingBackslash(ExpandUNCFileName(datapath));
	}
	static std::string GetUserConfigFileName( const std::string& datapath, const std::string& exename ) {
		// exepath, personalpath, appdatapath
		return GetDataPathDirectory(datapath, exename) + ExtractFileName(ChangeFileExt(exename, ".cfu"));
	}
};


#endif // __CONF_MAIN_FRAME_UNIT_H__
