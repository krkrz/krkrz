
#ifndef __CONF_MAIN_FRAME_UNIT_H__
#define __CONF_MAIN_FRAME_UNIT_H__

#include <windows.h>
#include <shlobj.h>
#include "FileCtrl.h"
#include "tstring.h"

class TConfMainFrame {
public:
	static tstring GetSpecialFolderPath(int csidl) {
		TCHAR path[MAX_PATH+1];
		if(!SHGetSpecialFolderPath(NULL, path, csidl, false))
			return tstring();
		return tstring(path);
	}
	static inline tstring GetPersonalPath() {
		tstring path = GetSpecialFolderPath(CSIDL_PERSONAL);
		if( path.empty() ) path = GetSpecialFolderPath(CSIDL_APPDATA);

		if(path != _T("")) {
			return path;
		}
		return _T("");
	}
	static inline tstring GetAppDataPath() {
		tstring path = GetSpecialFolderPath(CSIDL_APPDATA);
		if(path != _T("") ) {
			return path;
		}
		return _T("");
	}
	static inline tstring ReplaceStringAll( tstring src, const tstring& target, const tstring& dest ) {
		int nPos = 0;
		while( (nPos = src.find(target, nPos)) != tstring::npos ) {
			src.replace( nPos, target.length(), dest );
		}
		return src;
	}

	static inline tstring GetConfigFileName( const tstring& exename ) {
		return ChangeFileExt(exename, _T(".cf"));
	}
	static tstring GetDataPathDirectory( tstring datapath, const tstring& exename ) {
		if(datapath == _T("") ) datapath = tstring(_T("$(exepath)\\savedata"));

		tstring exepath = ExcludeTrailingBackslash(ExtractFileDir(exename));
		tstring personalpath = ExcludeTrailingBackslash(GetPersonalPath());
		tstring appdatapath = ExcludeTrailingBackslash(GetAppDataPath());
		if(personalpath == _T("")) personalpath = exepath;
		if(appdatapath == _T("")) appdatapath = exepath;

		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osinfo);

		bool vista_or_later = osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osinfo.dwMajorVersion >= 6;

		tstring vistapath = vista_or_later ? appdatapath : exepath;

		datapath = ReplaceStringAll(datapath, _T("$(exepath)"), exepath);
		datapath = ReplaceStringAll(datapath, _T("$(personalpath)"), personalpath);
		datapath = ReplaceStringAll(datapath, _T("$(appdatapath)"), appdatapath);
		datapath = ReplaceStringAll(datapath, _T("$(vistapath)"), vistapath);
		return IncludeTrailingBackslash(ExpandUNCFileName(datapath));
	}
	static tstring GetUserConfigFileName( const tstring& datapath, const tstring& exename ) {
		// exepath, personalpath, appdatapath
		return GetDataPathDirectory(datapath, exename) + ExtractFileName(ChangeFileExt(exename, _T(".cfu")));
	}
};


#endif // __CONF_MAIN_FRAME_UNIT_H__
