//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// TVP Win32 Project File
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "ScriptMgnIntf.h"
#include "ScriptMgnImpl.h"
#include "tjsError.h"
#include "PluginImpl.h"
#include "SystemIntf.h"

//---------------------------------------------------------------------------
bool TVPCheckCmdDescription(void);
bool TVPCheckAbout(void);
bool TVPCheckPrintDataPath();
void TVPOnError();
//---------------------------------------------------------------------------
USEDEF("tvpwin32.def");
USERES("tvpwin32.res");
USERES("HBeamCur.res");
USE("..\..\tjs2\tjsCommHead.h", File);
USE("..\..\tjs2\tjsHashSearch.h", File);
USE("..\..\tjs2\bison\tjs.y", File);
USE("..\..\tjs2\bison\tjspp.y", File);
USELIB("..\..\..\..\Lib\zlib.lib");
USELIB("..\..\..\..\Lib\deelibpng.lib");
USELIB("..\..\..\..\Lib\libjpeg2.lib");
USEUNIT("WideNativeFuncs.cpp");
USEUNIT("DetectCPU.cpp");
USEUNIT("..\..\tjs2\tjsInterface.cpp");
USEUNIT("..\..\tjs2\tjsVariant.cpp");
USEUNIT("..\..\tjs2\tjsVariantString.cpp");
USEUNIT("..\..\tjs2\tjsString.cpp");
USEUNIT("..\..\tjs2\tjsInterCodeExec.cpp");
USEUNIT("..\..\tjs2\tjsObject.cpp");
USEUNIT("..\..\tjs2\tjsDictionary.cpp");
USEUNIT("..\..\tjs2\tjsArray.cpp");
USEUNIT("..\..\tjs2\tjsNative.cpp");
USEUNIT("..\..\tjs2\tjs.cpp");
USEUNIT("..\..\tjs2\tjsScriptCache.cpp");
USEUNIT("..\..\tjs2\tjsConfig.cpp");
USEUNIT("..\..\tjs2\tjsRegExp.cpp");
USEUNIT("..\..\tjs2\tjsScriptBlock.cpp");
USEUNIT("..\..\tjs2\tjsInterCodeGen.cpp");
USEUNIT("..\..\tjs2\tjsGlobalStringMap.cpp");
USEUNIT("..\..\tjs2\tjs.tab.cpp");
USEUNIT("..\..\tjs2\tjsLex.cpp");
USEUNIT("..\..\tjs2\tjsCompileControl.cpp");
USEUNIT("..\..\tjs2\tjspp.tab.cpp");
USEUNIT("..\..\tjs2\tjsNamespace.cpp");
USEUNIT("..\..\tjs2\tjsError.cpp");
USEUNIT("..\..\tjs2\tjsUtils.cpp");
USEUNIT("..\..\tjs2\tjsMath.cpp");
USEUNIT("..\..\tjs2\tjsRandomGenerator.cpp");
USEUNIT("..\..\tjs2\tjsMT19937ar-cok.cpp");
USEUNIT("..\..\tjs2\tjsDate.cpp");
USEUNIT("..\..\tjs2\tjsDateParser.cpp");
USEUNIT("..\..\tjs2\tjsdate.tab.cpp");
USEUNIT("..\..\tjs2\tjsException.cpp");
USEUNIT("..\..\tjs2\tjsMessage.cpp");
USEUNIT("..\..\tjs2\tjsDisassemble.cpp");
USEUNIT("..\..\tjs2\tjsDebug.cpp");
USELIB("..\..\visual\IA32\tvpgl_ia32.lib");
USEUNIT("..\..\visual\LayerBitmapIntf.cpp");
USEUNIT("..\..\visual\win32\LayerBitmapImpl.cpp");
USEUNIT("..\..\visual\LayerIntf.cpp");
USEUNIT("..\..\visual\LayerManager.cpp");
USEUNIT("..\..\visual\win32\LayerImpl.cpp");
USEUNIT("..\..\visual\win32\DInputMgn.cpp");
USEUNIT("..\..\visual\WindowIntf.cpp");
USEUNIT("..\..\visual\win32\WindowImpl.cpp");
USEUNIT("..\..\visual\GraphicsLoaderIntf.cpp");
USEUNIT("..\..\visual\LoadTLG.cpp");
USEUNIT("..\..\visual\win32\GraphicsLoaderImpl.cpp");
USEUNIT("..\..\visual\MenuItemIntf.cpp");
USEUNIT("..\..\visual\win32\MenuItemImpl.cpp");
USEUNIT("..\..\visual\ComplexRect.cpp");
USEUNIT("..\..\visual\TransIntf.cpp");
USEUNIT("..\..\visual\win32\TransImpl.cpp");
USEUNIT("..\..\visual\VideoOvlIntf.cpp");
USEUNIT("..\..\visual\win32\VideoOvlImpl.cpp");
USEUNIT("..\..\visual\Resampler.cpp");
USEUNIT("..\..\visual\tvpgl.c");
USEUNIT("..\..\visual\IA32\tvpgl_ia32_intf.c");
USEUNIT("..\..\visual\win32\PassThroughDrawDevice.cpp");
USEUNIT("..\..\visual\win32\DrawDevice.cpp");
USEUNIT("..\..\utils\DebugIntf.cpp");
USEUNIT("..\..\utils\win32\DebugImpl.cpp");
USEUNIT("..\..\utils\PadIntf.cpp");
USEUNIT("..\..\utils\win32\PadImpl.cpp");
USEUNIT("..\..\utils\TimerIntf.cpp");
USEUNIT("..\..\utils\win32\TimerImpl.cpp");
USEUNIT("..\..\utils\win32\TickCount.cpp");
USEUNIT("..\..\utils\KAGParser.cpp");
USEUNIT("..\..\utils\Random.cpp");
USEUNIT("..\..\utils\md5.c");
USEUNIT("..\..\utils\ThreadIntf.cpp");
USEUNIT("..\..\utils\win32\ThreadImpl.cpp");
USEUNIT("..\..\utils\ClipboardIntf.cpp");
USEUNIT("..\..\utils\win32\ClipboardImpl.cpp");
USEUNIT("..\..\base\XP3Archive.cpp");
USEUNIT("..\..\base\TextStream.cpp");
USEUNIT("..\..\base\UtilStreams.cpp");
USEUNIT("..\..\base\win32\FileSelector.cpp");
USEUNIT("..\..\base\win32\SusieArchive.cpp");
USEUNIT("..\..\base\StorageIntf.cpp");
USEUNIT("..\..\base\win32\StorageImpl.cpp");
USEUNIT("..\..\base\win32\ScriptMgnImpl.cpp");
USEUNIT("..\..\base\EventIntf.cpp");
USEUNIT("..\..\base\win32\EventImpl.cpp");
USEUNIT("..\..\base\SysInitIntf.cpp");
USEUNIT("..\..\base\win32\SysInitImpl.cpp");
USEUNIT("..\..\base\SystemIntf.cpp");
USEUNIT("..\..\base\win32\SystemImpl.cpp");
USEUNIT("..\..\base\PluginIntf.cpp");
USEUNIT("..\..\base\win32\PluginImpl.cpp");
USEUNIT("..\..\sound\SoundBufferBaseIntf.cpp");
USEUNIT("..\..\sound\win32\SoundBufferBaseImpl.cpp");
USEUNIT("..\..\sound\CDDAIntf.cpp");
USEUNIT("..\..\sound\win32\CDDAImpl.cpp");
USEUNIT("..\..\sound\MIDIIntf.cpp");
USEUNIT("..\..\sound\win32\MIDIImpl.cpp");
USEUNIT("..\..\sound\WaveIntf.cpp");
USEUNIT("..\..\sound\win32\WaveImpl.cpp");
USEUNIT("..\..\sound\win32\tvpsnd.c");
USEUNIT("..\..\sound\WaveLoopManager.cpp");
USEUNIT("..\..\sound\PhaseVocoderDSP.cpp");
USEUNIT("..\..\sound\PhaseVocoderFilter.cpp");
USEUNIT("..\..\sound\WaveSegmentQueue.cpp");
USEOBJ("..\..\sound\cpu\pv_sse.obj");
USEOBJ("..\..\sound\cpu\pv_def.obj");
USEUNIT("EmergencyExit.cpp");
USEUNIT("HintWindow.cpp");
USEUNIT("..\..\utils\win32\TLogViewer.cpp");
USEFORM("HaltWarnFormUnit.cpp", TVPHaltWarnForm);
USEFORM("..\..\utils\win32\ConsoleFormUnit.cpp", TVPConsoleForm);
USEFORM("..\..\utils\win32\PadFormUnit.cpp", TVPPadForm);
USEFORM("MainFormUnit.cpp", TVPMainForm);
USEFORM("WatchFormUnit.cpp", TVPWatchForm);
USEFORM("..\..\visual\win32\WindowFormUnit.cpp", TVPWindowForm);
USEFORM("..\..\visual\win32\MenuContainerFormUnit.cpp", TVPMenuContainerForm);
USEFORM("..\..\visual\win32\FontSelectFormUnit.cpp", TVPFontSelectForm);
USEFORM("VersionFormUnit.cpp", TVPVersionForm);
USEUNIT("..\..\msg\MsgIntf.cpp");
USEUNIT("..\..\msg\win32\MsgImpl.cpp");
USEUNIT("..\..\base\ScriptMgnIntf.cpp");
USEUNIT("..\..\base\win32\FuncStubs.cpp");
USEUNIT("..\..\msg\win32\OptionsDesc.cpp");
USE("..\..\base\common.h", File);
USE("..\..\utils\ObjectList.h", File);
USEUNIT("..\..\..\tools\win32\krdevui\ConfSettingsUnit.cpp");
USEFORM("..\..\..\tools\win32\krdevui\ConfMainFrameUnit.cpp", ConfMainFrame); /* TFrame: File Type */
USEUNIT("..\..\base\CharacterSet.cpp");
USEUNIT("..\..\tjs2\tjsConstArrayData.cpp");
USEUNIT("..\..\tjs2\tjsByteCodeLoader.cpp");
USEUNIT("..\..\tjs2\tjsBinarySerializer.cpp");
USEUNIT("..\..\base\BinaryStream.cpp");
//---------------------------------------------------------------------------
#ifdef TVP_SUPPORT_ERI
#	pragma link "../../../../Lib/liberina.lib"
#endif
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// try starting the program!
	bool engine_init = false;
	try
	{
		if(TVPCheckProcessLog()) return 0; // sub-process for processing object hash map log


		TVPInitScriptEngine();
		engine_init = true;

		// banner
		TVPAddImportantLog(TJS_W("Program started on ") + TVPGetOSName() +
			TJS_W(" (") + TVPGetPlatformName() + TJS_W(")"));

		// TVPInitializeBaseSystems
		TVPInitializeBaseSystems();

		Application->Initialize();

		if(TVPCheckPrintDataPath()) return 0;
		if(TVPCheckCmdDescription()) return 0;
		if(TVPExecuteUserConfig()) return 0; // userconf

		TVPSystemInit();

		if(TVPCheckAbout()) return 0; // version information dialog box;

		Application->Title = "‹g—¢‹g—¢";
		Application->CreateForm(__classid(TTVPMainForm), &TVPMainForm);
                 TVPLoadPluigins(); // load plugin module *.tpm
		if(TVPProjectDirSelected) TVPInitializeStartupScript();

		Application->Run();

		try
		{
			TVPSystemUninit();
		}
		catch(...)
		{
			// ignore errors
		}
	}
	catch (EAbort &e)
	{
		// nothing to do
	}
	catch (Exception &exception)
	{
		TVPOnError();
		if(!TVPSystemUninitCalled)
			Application->ShowException(&exception);
	}
	catch (eTJSScriptError &e)
	{
		TVPOnError();
		if(!TVPSystemUninitCalled)
			Application->ShowException(&Exception(e.GetMessage().AsAnsiString()));
	}
	catch (eTJS &e)
	{
		TVPOnError();
		if(!TVPSystemUninitCalled)
			Application->ShowException(&Exception(e.GetMessage().AsAnsiString()));
	}
	catch(...)
	{
		Application->ShowException(&Exception("Unknown error!"));
	}

	if(engine_init) TVPUninitScriptEngine();

#ifndef _DEBUG
	// delete application and exit forcely
	// this prevents ugly exception message on exit

	delete Application;
	ExitProcess(TVPTerminateCode);
#endif
	return TVPTerminateCode;
}
//---------------------------------------------------------------------------





