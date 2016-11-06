
#ifndef __APPLICATION_SPECIAL_PATH_H__
#define __APPLICATION_SPECIAL_PATH_H__

#include "FilePathUtil.h"
#include "Application.h"

class ApplicationSpecialPath {
public:
	static inline tjs_string GetPersonalPath() {
		return Application->GetInternalDataPath();
	}
	static inline tjs_string GetAppDataPath() {
		return Application->GetExternalDataPath();
	}
	static inline tjs_string ReplaceStringAll( tjs_string src, const tjs_string& target, const tjs_string& dest ) {
		int nPos = 0;
		while( (nPos = src.find(target, nPos)) != tjs_string::npos ) {
			src.replace( nPos, target.length(), dest );
		}
		return src;
	}

	static inline tjs_string GetConfigFileName( const tjs_string& exename ) {
		return ChangeFileExt(exename, TJS_W(".cf"));
	}
	static tjs_string GetDataPathDirectory( tjs_string datapath, const tjs_string& exename ) {
		//if(datapath == TJS_W("") ) datapath = tjs_string(TJS_W("$(exepath)\\savedata"));
		if(datapath == TJS_W("") ) datapath = tjs_string(TJS_W("$(appdatapath)\\savedata"));

		//tjs_string exepath = ExcludeTrailingBackslash(ExtractFileDir(exename));
		tjs_string personalpath = ExcludeTrailingBackslash(GetPersonalPath());
		tjs_string appdatapath = ExcludeTrailingBackslash(GetAppDataPath());
		//if(personalpath == TJS_W("")) personalpath = exepath;
		//if(appdatapath == TJS_W("")) appdatapath = exepath;

		//datapath = ReplaceStringAll(datapath, TJS_W("$(exepath)"), exepath);
		//datapath = ReplaceStringAll(datapath, TJS_W("$(vistapath)"), vistapath);
		datapath = ReplaceStringAll(datapath, TJS_W("$(personalpath)"), personalpath);
		datapath = ReplaceStringAll(datapath, TJS_W("$(appdatapath)"), appdatapath);
		return IncludeTrailingBackslash(ExpandUNCFileName(datapath));
	}
	static tjs_string GetUserConfigFileName( const tjs_string& datapath, const tjs_string& exename ) {
		// exepath, personalpath, appdatapath
		return GetDataPathDirectory(datapath, exename) + ExtractFileName(ChangeFileExt(exename, TJS_W(".cfu")));
	}
};


#endif // __APPLICATION_SPECIAL_PATH_H__
