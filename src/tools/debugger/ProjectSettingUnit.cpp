//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ProjectSettingUnit.h"  
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProjectSettingForm *ProjectSettingForm;
//---------------------------------------------------------------------------
__fastcall TProjectSettingForm::TProjectSettingForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::SetExePath( const AnsiString& val )
{
	ExeEdit->Text = val;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TProjectSettingForm::GetExePath() const
{
	return ExeEdit->Text;
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::SetArg( const AnsiString& val )
{
	ArgEdit->Text = val;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TProjectSettingForm::GetArg() const
{
	return ArgEdit->Text;
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::SetProjectFolder( const AnsiString& val )
{
	ProjectFolderEdit->Text = val;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TProjectSettingForm::GetProjectFolder() const
{
	return ProjectFolderEdit->Text;
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::SetWorkingFolder( const AnsiString& val )
{
	WorkingFolderEdit->Text = val;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TProjectSettingForm::GetWorkingFolder() const
{
	return WorkingFolderEdit->Text;
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::SetScriptExt( const AnsiString& val )
{
	ScriptrExtEdit->Text = val;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TProjectSettingForm::GetScriptExt() const
{
	return ScriptrExtEdit->Text;
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::BrowseExeButtonClick(TObject *Sender)
{
	ExeOpenDialog->FileName = ExePath;
	if( ExeOpenDialog->Execute() ) {
		ExePath = ExeOpenDialog->FileName;
	}
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::BrowseProjectFolderButtonClick(TObject *Sender)
{
	WideString Root="デスクトップ";
	AnsiString	directory( ProjectFolder );
	if( SelectDirectory("プロジェクトフォルダを選択してください", Root, directory) ) {
		ProjectFolder = directory;
	}
}
//---------------------------------------------------------------------------
void __fastcall TProjectSettingForm::BrowseWorkingFolderButtonClick(TObject *Sender)
{
	WideString Root="デスクトップ";
	AnsiString	directory( WorkingFolder );
	if( SelectDirectory("作業フォルダを選択してください", Root, directory) ) {
		WorkingFolder = directory;
	}
}
//---------------------------------------------------------------------------
