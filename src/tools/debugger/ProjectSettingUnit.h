//---------------------------------------------------------------------------

#ifndef ProjectSettingUnitH
#define ProjectSettingUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TProjectSettingForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TEdit *ExeEdit;
	TLabel *ExeLabel;
	TButton *BrowseExeButton;
	TLabel *ArgLabel;
	TEdit *ArgEdit;
	TLabel *ProjectFolderLabel;
	TEdit *ProjectFolderEdit;
	TButton *BrowseProjectFolderButton;
	TLabel *WorkingFolderLabel;
	TEdit *WorkingFolderEdit;
	TButton *BrowseWorkingFolderButton;
	TButton *OKButton;
	TButton *CancelButton;
	TOpenDialog *ExeOpenDialog;
	TLabel *ScriptExtLabel;
	TEdit *ScriptrExtEdit;
	void __fastcall BrowseExeButtonClick(TObject *Sender);
	void __fastcall BrowseProjectFolderButtonClick(TObject *Sender);
	void __fastcall BrowseWorkingFolderButtonClick(TObject *Sender);
private:	// ユーザー宣言

	void __fastcall SetExePath( const AnsiString& val );
	AnsiString __fastcall GetExePath() const;
	void __fastcall SetArg( const AnsiString& val );
	AnsiString __fastcall GetArg() const;
	void __fastcall SetProjectFolder( const AnsiString& val );
	AnsiString __fastcall GetProjectFolder() const;
	void __fastcall SetWorkingFolder( const AnsiString& val );
	AnsiString __fastcall GetWorkingFolder() const;            
	void __fastcall SetScriptExt( const AnsiString& val );
	AnsiString __fastcall GetScriptExt() const;


public:		// ユーザー宣言
	__fastcall TProjectSettingForm(TComponent* Owner);

	__property AnsiString ExePath = { write=SetExePath,read=GetExePath };
	__property AnsiString Arg = { write=SetArg,read=GetArg };
	__property AnsiString ProjectFolder = { write=SetProjectFolder,read=GetProjectFolder };
	__property AnsiString WorkingFolder = { write=SetWorkingFolder,read=GetWorkingFolder };
	__property AnsiString ScriptExt = { write=SetScriptExt,read=GetScriptExt };
};
//---------------------------------------------------------------------------
extern PACKAGE TProjectSettingForm *ProjectSettingForm;
//---------------------------------------------------------------------------
#endif
