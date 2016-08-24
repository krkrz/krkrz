
#ifndef __APPLICATION_SPECIAL_PATH_H__
#define __APPLICATION_SPECIAL_PATH_H__

#include "FilePathUtil.h"
#include "Application.h"

class ApplicationSpecialPath {
public:
	static inline std::wstring GetPersonalPath() {
		return Application->GetInternalDataPath();
	}
	static inline std::wstring GetAppDataPath() {
		return Application->GetExternalDataPath();
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
		//if(datapath == L"" ) datapath = std::wstring(L"$(exepath)\\savedata");
		if(datapath == L"" ) datapath = std::wstring(L"$(appdatapath)\\savedata");

		//std::wstring exepath = ExcludeTrailingBackslash(ExtractFileDir(exename));
		std::wstring personalpath = ExcludeTrailingBackslash(GetPersonalPath());
		std::wstring appdatapath = ExcludeTrailingBackslash(GetAppDataPath());
		//if(personalpath == L"") personalpath = exepath;
		//if(appdatapath == L"") appdatapath = exepath;

		//datapath = ReplaceStringAll(datapath, L"$(exepath)", exepath);
		//datapath = ReplaceStringAll(datapath, L"$(vistapath)", vistapath);
		datapath = ReplaceStringAll(datapath, L"$(personalpath)", personalpath);
		datapath = ReplaceStringAll(datapath, L"$(appdatapath)", appdatapath);
		return IncludeTrailingBackslash(ExpandUNCFileName(datapath));
	}
	static std::wstring GetUserConfigFileName( const std::wstring& datapath, const std::wstring& exename ) {
		// exepath, personalpath, appdatapath
		return GetDataPathDirectory(datapath, exename) + ExtractFileName(ChangeFileExt(exename, L".cfu"));
	}
};


#endif // __APPLICATION_SPECIAL_PATH_H__
