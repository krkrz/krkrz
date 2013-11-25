//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainUnit.cpp", ScriptDebuggerForm);
USEFORM("ProjectSettingUnit.cpp", ProjectSettingForm);
USEFORM("DubbggerSettingUnit.cpp", DubbggerSettingForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	{	// 二重起動抑止
		HANDLE hMutex;
		hMutex = ::CreateMutex(NULL, FALSE, "script debugger Mutex 9A7A0A70B3864EA5B9CFA711223A10AF");
		if( (hMutex != NULL) && (GetLastError () == ERROR_ALREADY_EXISTS) ) {
			::CloseHandle(hMutex);
			return 0;
		}
	}
	try
	{
		Application->Initialize();
		SetApplicationMainFormOnTaskBar(Application, true);
		Application->Title = "krkrdebg - スクリプト デバッガ -";
		Application->CreateForm(__classid(TScriptDebuggerForm), &ScriptDebuggerForm);
		Application->CreateForm(__classid(TProjectSettingForm), &ProjectSettingForm);
		Application->CreateForm(__classid(TDubbggerSettingForm), &DubbggerSettingForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
