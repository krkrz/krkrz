//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// System Initialization and Uninitialization
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <string>
#include "FilePathUtil.h"
#include <sys/sysinfo.h>
#include <chrono>
#include <thread>

#include "SysInitImpl.h"
#include "StorageIntf.h"
#include "StorageImpl.h"
#include "MsgIntf.h"
#include "GraphicsLoaderIntf.h"
#include "SystemControl.h"
#include "DebugIntf.h"
#include "tjsLex.h"
#include "LayerIntf.h"
#include "Random.h"
#include "DetectCPU.h"
#include "XP3Archive.h"
#include "ScriptMgnIntf.h"
#include "XP3Archive.h"
#include "BinaryStream.h"
#include "Application.h"
#include "Exception.h"
#include "ApplicationSpecialPath.h"
#include "TickCount.h"



//---------------------------------------------------------------------------
// global data
//---------------------------------------------------------------------------
tjs_string TVPNativeProjectDir;
tjs_string TVPNativeDataPath;
bool TVPProjectDirSelected = false;
extern tjs_real TVPCPUClock;
extern ttstr TVPGetPersonalPath();
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// System security options
//---------------------------------------------------------------------------
// system security options are held inside the executable, where
// signature checker will refer. This enables the signature checker
// (or other security modules like XP3 encryption module) to check
// the changes which is not intended by the contents author.
const static char TVPSystemSecurityOptions[] =
"-- TVPSystemSecurityOptions disablemsgmap(0):forcedataxp3(0):acceptfilenameargument(0) --";
//---------------------------------------------------------------------------
int GetSystemSecurityOption(const char *name)
{
	size_t namelen = TJS_nstrlen(name);
	const char *p = TJS_nstrstr(TVPSystemSecurityOptions, name);
	if(!p) return 0;
	if(p[namelen] == '(' && p[namelen + 2] == ')')
		return p[namelen+1] - '0';
	return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPDumpHWException()
{
	// dummy
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// random generator initializer
//---------------------------------------------------------------------------
static void TVPInitRandomGenerator()
{
	// initialize random generator
	tjs_uint32 tick = TVPGetRoughTickCount32();
	TVPPushEnvironNoise(&tick, sizeof(tjs_uint32));
	pid_t id = gettid(); // pthread
	TVPPushEnvironNoise(&id, sizeof(id));
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPInitializeBaseSystems
//---------------------------------------------------------------------------
void TVPInitializeBaseSystems()
{
	// set system archive delimiter
	tTJSVariant v;
	if(TVPGetCommandLine(TJS_W("-arcdelim"), &v))
		TVPArchiveDelimiter = ttstr(v)[0];

	// set default current directory
	{
		TVPSetCurrentDirectory( IncludeTrailingBackslash(ExtractFileDir(Application->GetExternalDataPath())) );
	}

	// load message map file
	bool load_msgmap = GetSystemSecurityOption("disablemsgmap") == 0;

	if(load_msgmap)
	{
		const tjs_char name_msgmap [] = TJS_W("msgmap.tjs");
		if(TVPIsExistentStorage(name_msgmap))
			TVPExecuteStorage(name_msgmap, NULL, false, TJS_W(""));
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// system initializer / uninitializer
//---------------------------------------------------------------------------
static tjs_uint64 TVPTotalPhysMemory = 0;
static void TVPInitProgramArgumentsAndDataPath(bool stop_after_datapath_got);
void TVPBeforeSystemInit()
{

	TVPInitProgramArgumentsAndDataPath(false); // ensure command line

	// randomize
	TVPInitRandomGenerator();

	// memory usage
	{
		struct sysinfo info;
		::sysinfo(&info);

		TVPPushEnvironNoise(&info, sizeof(info));

		TVPTotalPhysMemory = info.totalram;

		ttstr memstr( to_tjs_string(TVPTotalPhysMemory).c_str() );
		TVPAddImportantLog( TVPFormatMessage(TVPInfoTotalPhysicalMemory, memstr) );

		tTJSVariant opt;
		if(TVPGetCommandLine(TJS_W("-memusage"), &opt))
		{
			ttstr str(opt);
			if(str == TJS_W("low"))
				TVPTotalPhysMemory = 0; // assumes zero
		}

		if(TVPTotalPhysMemory <= 36*1024*1024)
		{
			// very very low memory, forcing to assume zero memory
			TVPTotalPhysMemory = 0;
		}

		if(TVPTotalPhysMemory < 48*1024*1024ULL)
		{
			// extra low memory
			if(TJSObjectHashBitsLimit > 0)
				TJSObjectHashBitsLimit = 0;
			TVPSegmentCacheLimit = 0;
			TVPFreeUnusedLayerCache = true; // in LayerIntf.cpp
		}
		else if(TVPTotalPhysMemory < 64*1024*1024)
		{
			// low memory
			if(TJSObjectHashBitsLimit > 4)
				TJSObjectHashBitsLimit = 4;
		}
	}

	// ディレクトリチェック - Android 版は以下の優先順位
	// 1. assets/config.cf を読み、そこで指定されたフォルダから読み込む(未実装)
	// 2. assets/data.xp3 から読み込む
	// 3. assets/startup.tjs から開始
	tjs_char buf[MAX_PATH];
	bool selected = false;
	AAsset* asset = AAssetManager_open( Application->getAssetManager(), "data.xp3", AASSET_MODE_UNKNOWN);
	bool result = asset != nullptr;
	if( result ) {
		AAsset_close( asset );
		asset = nullptr;
		TJS_strcpy(buf, TJS_W("asset:///data.xp3>"));
		tjs_int buflen = TJS_strlen(buf);
		buf[buflen] = TVPArchiveDelimiter, buf[buflen+1] = 0;
		selected = true;
		TVPProjectDirSelected = true;
	}
	if( !selected ) {
		asset = AAssetManager_open( Application->getAssetManager(), "startup.tjs", AASSET_MODE_UNKNOWN);
		result = asset != nullptr;
		if( result ) {
			AAsset_close( asset );
			asset = nullptr;
			TJS_strcpy(buf, TJS_W("asset:///"));
			selected = true;
			TVPProjectDirSelected = true;
		}
	}
	if( selected ) {
		TVPProjectDir = TVPNormalizeStorageName(buf);
		TVPSetCurrentDirectory(TVPProjectDir);
		TVPNativeProjectDir = buf;
	}
	if(TVPProjectDirSelected) {
		TVPAddImportantLog( TVPFormatMessage(TVPInfoSelectedProjectDirectory, TVPProjectDir) );
	}
#if 0
	tjs_char buf[MAX_PATH];
	bool bufset = false;
	bool nosel = false;
	bool forcesel = false;

	bool forcedataxp3 = GetSystemSecurityOption("forcedataxp3") != 0;
	bool acceptfilenameargument = GetSystemSecurityOption("acceptfilenameargument") != 0;

	if(!forcedataxp3 && !acceptfilenameargument)
	{
		if(TVPGetCommandLine(TJS_W("-nosel")) || TVPGetCommandLine(TJS_W("-about")))
		{
			nosel = true;
		}
		else
		{
			tjs_char exeDir[MAX_PATH];
			TJS_strcpy(exeDir, IncludeTrailingBackslash(ExtractFileDir(ExePath())).c_str());
			for(tjs_int i = 1; i<_argc; i++)
			{
				if(_argv[i][0] == '-' && _argv[i][1] == '-' && _argv[i][2] == 0)
					break;

				if(_argv[i][0] != '-')
				{
					// ::SetCurrentDirectory( exeDir );
					TJS_strncpy(buf, ttstr(_argv[i]).c_str(), MAX_PATH-1);
					buf[MAX_PATH-1] = TJS_W('\0');
					if(DirectoryExists(buf)) // is directory?
						TJS_strcat(buf, TJS_W("\\"));

					TVPProjectDirSelected = true;
					bufset = true;
					nosel = true;
				}
			}
		}
	}

	// check "-sel" option, to force show folder selection window
	if(!forcedataxp3 && TVPGetCommandLine(TJS_W("-sel")))
	{
		// sel option was set
		if(bufset)
		{
			tjs_char path[MAX_PATH];
			tjs_char *dum = 0;
			GetFullPathName(buf, MAX_PATH-1, path, &dum);
			TJS_strcpy(buf, path);
			TVPProjectDirSelected = false;
			bufset = true;
		}
		nosel = true;
		forcesel = true;
	}

	// check "content-data" directory
	if(!forcedataxp3 && !nosel)
	{
		tjs_char tmp[MAX_PATH];
		TJS_strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ExePath())).c_str());
		TJS_strcat(tmp, TJS_W("content-data"));
		if(DirectoryExists(tmp))
		{
			TJS_strcat(tmp, TJS_W("\\"));
			TJS_strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check "data.xp3" archive
 	if(!nosel)
	{
		tjs_char tmp[MAX_PATH];
		TJS_strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ExePath())).c_str());
		TJS_strcat(tmp, TJS_W("data.xp3"));
		if(FileExists(tmp))
		{
			TJS_strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check "data.exe" archive
 	if(!nosel)
	{
		tjs_char tmp[MAX_PATH];
		TJS_strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ExePath())).c_str());
		TJS_strcat(tmp, TJS_W("data.exe"));
		if(FileExists(tmp))
		{
			TJS_strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check self combined xpk archive
	if(!nosel)
	{
		if(TVPIsXP3Archive(TVPNormalizeStorageName(ExePath())))
		{
			TJS_strcpy(buf, ExePath().c_str());
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}


	// check "data" directory
	if(!forcedataxp3 && !nosel)
	{
		tjs_char tmp[MAX_PATH];
		TJS_strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ExePath())).c_str());
		TJS_strcat(tmp, TJS_W("data"));
		if(DirectoryExists(tmp))
		{
			TJS_strcat(tmp, TJS_W("\\"));
			TJS_strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// decide a directory to execute or to show folder selection
	if(!bufset)
	{
		if(forcedataxp3) throw EAbort(TJS_W("Aborted"));
		TJS_strcpy(buf, ExtractFileDir(ExePath()).c_str());
		int curdirlen = TJS_strlen(buf);
		if(buf[curdirlen-1] != TJS_W('\\')) buf[curdirlen] = TJS_W('\\'), buf[curdirlen+1] = 0;
	}

	// フォルダ選択はなし

	// check project dir and store some environmental variables
	if(TVPProjectDirSelected)
	{
		Application->SetShowMainForm( false );
	}

	tjs_int buflen = TJS_strlen(buf);
	if(buflen >= 1)
	{
		if(buf[buflen-1] != TJS_W('\\')) buf[buflen] = TVPArchiveDelimiter, buf[buflen+1] = 0;
	}

	TVPProjectDir = TVPNormalizeStorageName(buf);
	TVPSetCurrentDirectory(TVPProjectDir);
	TVPNativeProjectDir = buf;

	if(TVPProjectDirSelected)
	{
		TVPAddImportantLog( TVPFormatMessage(TVPInfoSelectedProjectDirectory, TVPProjectDir) );
	}
#endif
}
//---------------------------------------------------------------------------
void TVPSetProjectPath( const ttstr& path ) {
	if( TVPProjectDirSelected == false ) {
		TVPProjectDirSelected = true;
		TVPProjectDir = TVPNormalizeStorageName(path);
		TVPSetCurrentDirectory(TVPProjectDir);
		TVPNativeProjectDir = path.AsStdString();
		TVPAddImportantLog( TVPFormatMessage(TVPInfoSelectedProjectDirectory, TVPProjectDir) );
	}
}
//---------------------------------------------------------------------------
static void TVPDumpOptions();
//---------------------------------------------------------------------------
extern bool TVPEnableGlobalHeapCompaction;
static bool TVPHighTimerPeriod = false;
static tjs_uint TVPTimeBeginPeriodRes = 0;
//---------------------------------------------------------------------------
void TVPAfterSystemInit()
{
	// ensure datapath directory
	TVPEnsureDataPathDirectory();

	TVPAllocGraphicCacheOnHeap = false; // always false since beta 20

	// determine maximum graphic cache limit
	tTJSVariant opt;
	tjs_int64 limitmb = -1;
	if(TVPGetCommandLine(TJS_W("-gclim"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("auto"))
			limitmb = -1;
		else
			limitmb = opt.AsInteger();
	}


	if(limitmb == -1)
	{
		if(TVPTotalPhysMemory <= 32*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 48*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 64*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 96*1024*1024)
			TVPGraphicCacheSystemLimit = 4;
		else if(TVPTotalPhysMemory <= 128*1024*1024)
			TVPGraphicCacheSystemLimit = 8;
		else if(TVPTotalPhysMemory <= 192*1024*1024)
			TVPGraphicCacheSystemLimit = 12;
		else if(TVPTotalPhysMemory <= 256*1024*1024)
			TVPGraphicCacheSystemLimit = 20;
		else if(TVPTotalPhysMemory <= 512*1024*1024)
			TVPGraphicCacheSystemLimit = 40;
		else
			TVPGraphicCacheSystemLimit = tjs_uint64(TVPTotalPhysMemory / (1024*1024*10));	// cachemem = physmem / 10
		TVPGraphicCacheSystemLimit *= 1024*1024;
	}
	else
	{
		TVPGraphicCacheSystemLimit = limitmb * 1024*1024;
	}
	// 32bit なので 512MB までに制限
	if( TVPGraphicCacheSystemLimit >= 512*1024*1024 )
		TVPGraphicCacheSystemLimit = 512*1024*1024;


	if(TVPTotalPhysMemory <= 64*1024*1024)
		TVPSetFontCacheForLowMem();


	// check TVPGraphicSplitOperation option
	if(TVPGetCommandLine(TJS_W("-gsplit"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("no"))
			TVPGraphicSplitOperationType = gsotNone;
		else if(str == TJS_W("int"))
			TVPGraphicSplitOperationType = gsotInterlace;
		else if(str == TJS_W("yes") || str == TJS_W("simple"))
			TVPGraphicSplitOperationType = gsotSimple;
		else if(str == TJS_W("bidi"))
			TVPGraphicSplitOperationType = gsotBiDirection;

	}

	// check TVPDefaultHoldAlpha option
	if(TVPGetCommandLine(TJS_W("-holdalpha"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("yes") || str == TJS_W("true"))
			TVPDefaultHoldAlpha = true;
		else
			TVPDefaultHoldAlpha = false;
	}

	// check TVPJPEGFastLoad option
	if(TVPGetCommandLine(TJS_W("-jpegdec"), &opt)) // this specifies precision for JPEG decoding
	{
		ttstr str(opt);
		if(str == TJS_W("normal"))
			TVPJPEGLoadPrecision = jlpMedium;
		else if(str == TJS_W("low"))
			TVPJPEGLoadPrecision = jlpLow;
		else if(str == TJS_W("high"))
			TVPJPEGLoadPrecision = jlpHigh;

	}

	// dump option
	TVPDumpOptions();

	// initilaize x86 graphic routines
#ifdef _WIN32	// CPUごとに準備した方が良い
	TVPGL_IA32_Init();
#endif

	// timer precision
	tjs_uint prectick = 1;
	if(TVPGetCommandLine(TJS_W("-timerprec"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("high")) prectick = 1;
		if(str == TJS_W("higher")) prectick = 5;
		if(str == TJS_W("normal")) prectick = 10;
	}

        // draw thread num
        tjs_int drawThreadNum = 1;
        if (TVPGetCommandLine(TJS_W("-drawthread"), &opt)) {
          ttstr str(opt);
          if (str == TJS_W("auto"))
            drawThreadNum = 0;
          else
            drawThreadNum = (tjs_int)opt;
        }
        TVPDrawThreadNum = drawThreadNum;
#if 0
	if(prectick)
	{
		// retrieve minimum timer resolution
		TIMECAPS tc;
		timeGetDevCaps(&tc, sizeof(tc));
		if(prectick < tc.wPeriodMin)
			TVPTimeBeginPeriodRes = tc.wPeriodMin;
		else
			TVPTimeBeginPeriodRes = prectick;
		if(TVPTimeBeginPeriodRes > tc.wPeriodMax)
			TVPTimeBeginPeriodRes = tc.wPeriodMax;
		// set timer resolution
		timeBeginPeriod(TVPTimeBeginPeriodRes);
		TVPHighTimerPeriod = true;
	}

	TVPPushEnvironNoise(&TVPCPUType, sizeof(TVPCPUType));

	// set LFH
	if(TVPGetCommandLine(TJS_W("-uselfh"), &opt)) {
		ttstr str(opt);
		if(str == TJS_W("yes") || str == TJS_W("true")) {
			ULONG HeapInformation = 2;
			BOOL lfhenable = ::HeapSetInformation( GetProcessHeap(), HeapCompatibilityInformation, &HeapInformation, sizeof(HeapInformation) );
			if( lfhenable ) {
				TVPAddLog( TJS_W("(info) Enable LFH") );
			} else {
				TVPAddLog( TJS_W("(info) Cannot Enable LFH") );
			}
		}
	}
	// Global Heap Compact
	if(TVPGetCommandLine(TJS_W("-ghcompact"), &opt)) {
		ttstr str(opt);
		if(str == TJS_W("yes") || str == TJS_W("true")) {
			TVPEnableGlobalHeapCompaction = true;
		}
	}
#endif
}
//---------------------------------------------------------------------------
void TVPBeforeSystemUninit()
{
	// TVPDumpHWException(); // dump cached hw exceptoin
}
//---------------------------------------------------------------------------
void TVPAfterSystemUninit()
{
#if 0
	// restore timer precision
	if(TVPHighTimerPeriod)
	{
		timeEndPeriod(TVPTimeBeginPeriodRes);
	}
#endif
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool TVPTerminated = false;
bool TVPTerminateOnWindowClose = true;
bool TVPTerminateOnNoWindowStartup = true;
int TVPTerminateCode = 0;
//---------------------------------------------------------------------------
void TVPTerminateAsync(int code)
{
	// do "A"synchronous temination of application
	TVPTerminated = true;
	TVPTerminateCode = code;

	// posting dummy message will prevent "missing WM_QUIT bug" in Direct3D framework.
	if(TVPSystemControl) TVPSystemControl->CallDeliverAllEventsOnIdle();

	Application->Terminate();

	if(TVPSystemControl) TVPSystemControl->CallDeliverAllEventsOnIdle();
}
//---------------------------------------------------------------------------
void TVPTerminateSync(int code)
{
	// do synchronous temination of application (never return)
	TVPSystemUninit();

	Application->Terminate();
}
//---------------------------------------------------------------------------
void TVPMainWindowClosed()
{
	// called from WindowIntf.cpp, caused by closing all window.
	if( TVPTerminateOnWindowClose) TVPTerminateAsync();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// GetCommandLine
//---------------------------------------------------------------------------
static std::vector<std::string> * TVPGetEmbeddedOptions()
{
#if 0
	HMODULE hModule = ::GetModuleHandle(NULL);
	const char *buf = NULL;
	unsigned int size = 0;
	HRSRC hRsrc = ::FindResource(NULL, MAKEINTRESOURCE(IDR_OPTION), TEXT("TEXT"));
	if( hRsrc != NULL ) {
		size = ::SizeofResource( hModule, hRsrc );
		HGLOBAL hGlobal = ::LoadResource( hModule, hRsrc );
		if( hGlobal != NULL ) {
			buf = reinterpret_cast<const char*>(::LockResource(hGlobal));
		}
	}
	if( buf == NULL ) return NULL;

	std::vector<std::string> *ret = NULL;
	try {
		ret = new std::vector<std::string>();
		const char *tail = buf + size;
		const char *start = buf;
		while( buf < tail ) {
			if( buf[0] == 0x0D && buf[1] == 0x0A ) {	// CR LF
				ret->push_back( std::string(start,buf) );
				start = buf + 2;
				buf++;
			} else if( buf[0] == 0x0D || buf[0] == 0x0A ) {	// CR or LF
				ret->push_back( std::string(start,buf) );
				start = buf + 1;
			} else if( buf[0] == '\0' ) {
				ret->push_back( std::string(start,buf) );
				start = buf + 1;
				break;
			}
			buf++;
		}
		if( start < buf ) {
			ret->push_back( std::string(start,buf) );
		}
	} catch(...) {
		if(ret) delete ret;
		throw;
	}
	TVPAddImportantLog( (const tjs_char*)TVPInfoLoadingExecutableEmbeddedOptionsSucceeded );
	return ret;
#endif
	// TODO Assets から読むようにした方が良い
	return NULL;
}
//---------------------------------------------------------------------------
static std::vector<std::string> * TVPGetConfigFileOptions(const tjs_string& filename)
{
	// load .cf file
	tjs_string errmsg;
	if(!FileExists(filename))
		errmsg = (const tjs_char*)TVPFileNotFound;

	std::vector<std::string> * ret = NULL; // new std::vector<std::string>();
	if(errmsg == TJS_W(""))
	{
		try
		{
			ret = LoadLinesFromFile(filename);
		}
		catch(Exception & e)
		{
			errmsg = e.what();
		}
		catch(...)
		{
			delete ret;
			throw;
		}
	}

	if(errmsg != TJS_W(""))
		TVPAddImportantLog( TVPFormatMessage(TVPInfoLoadingConfigurationFileFailed, filename.c_str(), errmsg.c_str()) );
	else
		TVPAddImportantLog( TVPFormatMessage(TVPInfoLoadingConfigurationFileSucceeded, filename.c_str()) );

	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

static ttstr TVPParseCommandLineOne(const ttstr &i)
{
	// value is specified
	const tjs_char *p, *o;
	p = o = i.c_str();
	p = TJS_strchr(p, '=');

	if(p == NULL) { return i + TJS_W("=yes"); }

	p++;

	ttstr optname(o, p - o);

	if(*p == TJS_W('\'') || *p == TJS_W('\"'))
	{
		// as an escaped string
		tTJSVariant v;
		TJSParseString(v, &p);

		return optname + ttstr(v);
	}
	else
	{
		// as a string
		return optname + p;
	}
}
//---------------------------------------------------------------------------
std::vector <ttstr> TVPProgramArguments;
static bool TVPProgramArgumentsInit = false;
static tjs_int TVPCommandLineArgumentGeneration = 0;
static bool TVPDataPathDirectoryEnsured = false;
//---------------------------------------------------------------------------
tjs_int TVPGetCommandLineArgumentGeneration() { return TVPCommandLineArgumentGeneration; }
//---------------------------------------------------------------------------
void TVPEnsureDataPathDirectory()
{
	if(!TVPDataPathDirectoryEnsured)
	{
		TVPDataPathDirectoryEnsured = true;
		// ensure data path existence
		if(!TVPCheckExistentLocalFolder(TVPNativeDataPath.c_str()))
		{
			if(TVPCreateFolders(TVPNativeDataPath.c_str()))
				TVPAddImportantLog( TVPFormatMessage( TVPInfoDataPathDoesNotExistTryingToMakeIt, (const tjs_char*)TVPOk ) );
			else
				TVPAddImportantLog( TVPFormatMessage( TVPInfoDataPathDoesNotExistTryingToMakeIt, (const tjs_char*)TVPFaild ) );
		}
	}
}
//---------------------------------------------------------------------------
#if 0	// Android 版ではコマンドラインは読まない
static void PushAllCommandlineArguments()
{
	// store arguments given by commandline to "TVPProgramArguments"
	bool acceptfilenameargument = GetSystemSecurityOption("acceptfilenameargument") != 0;

	bool argument_stopped = false;
	if(acceptfilenameargument) argument_stopped = true;
	int file_argument_count = 0;
	for(tjs_int i = 1; i<_argc; i++)
	{
		if(argument_stopped)
		{
			ttstr arg_name_and_value = TJS_W("-arg") + ttstr(file_argument_count) + TJS_W("=")
				+ ttstr(_argv[i]);
			file_argument_count++;
			TVPProgramArguments.push_back(arg_name_and_value);
		}
		else
		{
			if(_argv[i][0] == '-')
			{
				if(_argv[i][1] == '-' && _argv[i][2] == 0)
				{
					// argument stopper
					argument_stopped = true;
				}
				else
				{
					ttstr value(_argv[i]);
					if(!TJS_strchr(value.c_str(), TJS_W('=')))
						value += TJS_W("=yes");
					TVPProgramArguments.push_back(TVPParseCommandLineOne(value));
				}
			}
		}
	}
}
#endif
//---------------------------------------------------------------------------
static void PushConfigFileOptions(const std::vector<std::string> * options)
{
	if(!options) return;
	for(unsigned int j = 0; j < options->size(); j++)
	{
		if( (*options)[j].c_str()[0] != ';') // unless comment
			TVPProgramArguments.push_back(
			TVPParseCommandLineOne(TJS_W("-") + ttstr((*options)[j].c_str())));
	}
}
//---------------------------------------------------------------------------
static void TVPInitProgramArgumentsAndDataPath(bool stop_after_datapath_got)
{
	if(!TVPProgramArgumentsInit)
	{
		TVPProgramArgumentsInit = true;


		// find options from self executable image
		const int num_option_layers = 3;
		std::vector<std::string> * options[num_option_layers];
		for(int i = 0; i < num_option_layers; i++) options[i] = NULL;
		try
		{
			// read embedded options and default configuration file
			options[0] = TVPGetEmbeddedOptions();
			options[1] = TVPGetConfigFileOptions( TJS_W("asset://config.cf") );

			// at this point, we need to push all exsting known options
			// to be able to see datapath
			//PushAllCommandlineArguments();
			PushConfigFileOptions(options[1]); // has more priority
			PushConfigFileOptions(options[0]); // has lesser priority

			// read datapath
#if 0
			tTJSVariant val;
			tjs_string config_datapath;
			if(TVPGetCommandLine(TJS_W("-datapath"), &val))
				config_datapath = ((ttstr)val).AsStdString();
			TVPNativeDataPath = ApplicationSpecialPath::GetDataPathDirectory(config_datapath, ExePath());
#endif
			//TVPNativeDataPath = TJS_W("asset://");
			TVPNativeDataPath = Application->GetExternalDataPath();

			if(stop_after_datapath_got) return;

			// read per-user configuration file
			// TODO : ユーザー設定は SharedPreference から読み込むようにするのが望ましい
			//options[2] = TVPGetConfigFileOptions(ApplicationSpecialPath::GetUserConfigFileName(config_datapath, ExePath()));

			// push each options into option stock
			// we need to clear TVPProgramArguments first because of the
			// option priority order.
			TVPProgramArguments.clear();
			//PushAllCommandlineArguments();
			//PushConfigFileOptions(options[2]); // has more priority
			PushConfigFileOptions(options[1]); // has more priority
			PushConfigFileOptions(options[0]); // has lesser priority
		} catch(...) {
			for(int i = 0; i < num_option_layers; i++)
				if(options[i]) delete options[i];
			throw;
		}
		for(int i = 0; i < num_option_layers; i++)
			if(options[i]) delete options[i];


		// set data path
		TVPDataPath = TVPNormalizeStorageName(TVPNativeDataPath);
		TVPAddImportantLog( TVPFormatMessage( TVPInfoDataPath, TVPDataPath) );

		// set log output directory
		TVPSetLogLocation(TVPNativeDataPath);

		// increment TVPCommandLineArgumentGeneration
		TVPCommandLineArgumentGeneration++;
	}
}
//---------------------------------------------------------------------------
static void TVPDumpOptions()
{
	std::vector<ttstr>::const_iterator i;
 	ttstr options( TVPInfoSpecifiedOptionEarlierItemHasMorePriority );
	if(TVPProgramArguments.size())
	{
		for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
		{
			options += TJS_W(" ");
			options += *i;
		}
	}
	else
	{
		options += (const tjs_char*)TVPNone;
	}
	TVPAddImportantLog(options);
}
//---------------------------------------------------------------------------
bool TVPGetCommandLine(const tjs_char * name, tTJSVariant *value)
{
	TVPInitProgramArgumentsAndDataPath(false);

	tjs_int namelen = TJS_strlen(name);
	std::vector<ttstr>::const_iterator i;
	for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
	{
		if(!TJS_strncmp(i->c_str(), name, namelen))
		{
			if(i->c_str()[namelen] == TJS_W('='))
			{
				// value is specified
				const tjs_char *p = i->c_str() + namelen + 1;
				if(value) *value = p;
				return true;
			}
			else if(i->c_str()[namelen] == 0)
			{
				// value is not specified
				if(value) *value = TJS_W("yes");
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------
void TVPSetCommandLine(const tjs_char * name, const ttstr & value)
{
	TVPInitProgramArgumentsAndDataPath(false);

	tjs_int namelen = TJS_strlen(name);
	std::vector<ttstr>::iterator i;
	for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
	{
		if(!TJS_strncmp(i->c_str(), name, namelen))
		{
			if(i->c_str()[namelen] == TJS_W('=') || i->c_str()[namelen] == 0)
			{
				// value found
				*i = ttstr(i->c_str(), namelen) + TJS_W("=") + value;
				TVPCommandLineArgumentGeneration ++;
				if(TVPCommandLineArgumentGeneration == 0) TVPCommandLineArgumentGeneration = 1;
				return;
			}
		}
	}

	// value not found; insert argument into front
	TVPProgramArguments.insert(TVPProgramArguments.begin(), ttstr(name) + TJS_W("=") + value);
	TVPCommandLineArgumentGeneration ++;
	if(TVPCommandLineArgumentGeneration == 0) TVPCommandLineArgumentGeneration = 1;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCheckPrintDataPath
//---------------------------------------------------------------------------
#if 0 // Android 版では行わない
bool TVPCheckPrintDataPath()
{
	// print current datapath to stdout, then exit
	for(int i=1; i<_argc; i++)
	{
		if(!strcmp(_argv[i], "-printdatapath")) // this does not refer TVPGetCommandLine
		{
			TVPInitProgramArgumentsAndDataPath(true);
			printf("%s\n", TVPNativeDataPath.c_str());

			return true; // processed
		}
	}

	return false;
}
#endif
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCheckAbout
//---------------------------------------------------------------------------
#if 0
bool TVPCheckAbout(void)
{
	if(TVPGetCommandLine(TJS_W("-about")))
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));
		tjs_char msg[80];
		TJS_snprintf(msg, sizeof(msg)/sizeof(tjs_char), TVPInfoCpuClockRoughly, (int)TVPCPUClock);
		TVPAddImportantLog(msg);

		TVPShowVersionForm();
		return true;
	}

	return false;
}
#endif
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPExecuteAsync
//---------------------------------------------------------------------------
static void TVPExecuteAsync( const tjs_string& progname)
{
#if 0
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	BOOL ret =
		CreateProcess(
			NULL,
			const_cast<LPTSTR>(progname.c_str()),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi);

	if(ret)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return;
	}

	throw Exception(ttstr(TVPExecutionFail).AsStdString());
#endif
	// TODO インテントを投げる実装にした方が良い
}
//---------------------------------------------------------------------------





