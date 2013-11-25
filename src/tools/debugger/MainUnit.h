//---------------------------------------------------------------------------

#ifndef MainUnitH
#define MainUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <ValEdit.hpp>
#include <ActnCtrls.hpp>
#include <ActnList.hpp>
#include <ActnMan.hpp>
#include <ActnMenus.hpp>
#include <StdActns.hpp>
#include <ToolWin.hpp>
#include <XPStyleActnCtrls.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <Buttons.hpp>
#include <map>
#include <string>
#include <vector>
#include <list>
#include "debugger.h"
#include "CommandBuffer.h"
//---------------------------------------------------------------------------
class DebuggeeCheckThread;
class TScriptDebuggerForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *MainPanel;
	TPanel *LogPanel;
	TMemo *LogMemo;
	TSplitter *LogSplitter;
	TPanel *ToolPanel;
	TPanel *TreePanel;
	TSplitter *TreeSplitter;
	TPanel *CodePropPanel;
	TTreeView *FileTreeView;
	TStatusBar *StatusBar;
	TPanel *CodePanel;
	TPanel *PropPanel;
	TSplitter *CodePropSplitter;
	TPageControl *CodePageControl;
	TPanel *ValuePanel;
	TSplitter *ValueStackSplitter;
	TPanel *CallStackPanel;
	TValueListEditor *ValueListEditor;
	TListBox *CallStackListBox;
	TLabel *CallStackLabel;
	TTabSheet *FileTabSheet;
	TListBox *SourceLineListBox;
	TActionManager *ActionManager;
	TActionMainMenuBar *ActionMainMenuBar;
	TFileOpen *FileOpen;
	TFileSaveAs *FileSaveAs;
	TFileExit *FileExit;
	TAction *SetProjectAction;
	TFontDialog *FontDialog;
	TAction *SelectTextFontAction;
	TAction *ExecuteDebugAction;
	TAction *KillDebugAction;
	TAction *BreakDebugAction;
	TAction *StepDebugAction;
	TAction *TraceDebugAction;
	TAction *ReturnDebugAction;
	TAction *SearchPrevAction;
	TAction *SetDebuggerOptAction;
	TAction *OverWriteAction;
	TAction *ClearBreakpointsAction;
	TImageList *ToolbarImageList;
	TBitBtn *ExecBitBtn;
	TBitBtn *KillBitBtn;
	TBitBtn *StepBitBtn;
	TBitBtn *TraceBitBtn;
	TBitBtn *ReturnBitBtn;
	TBitBtn *BreakBitBtn;
	TFindDialog *FindDialog;
	TAction *SearchNextAction;
	TImageList *FileIconImageList;
	void __fastcall SetProjectActionExecute(TObject *Sender);
	void __fastcall SourceLineListBoxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
	void __fastcall FileTreeViewDblClick(TObject *Sender);
	void __fastcall FileOpenAccept(TObject *Sender);
	void __fastcall FileSaveAsAccept(TObject *Sender);
	void __fastcall FileSaveAsBeforeExecute(TObject *Sender);
	void __fastcall FileOpenBeforeExecute(TObject *Sender);
	void __fastcall SelectTextFontActionExecute(TObject *Sender);
	void __fastcall SourceLineListBoxDblClick(TObject *Sender);
	void __fastcall ExecuteDebugActionExecute(TObject *Sender);
	void __fastcall KillDebugActionExecute(TObject *Sender);
	void __fastcall BreakDebugActionExecute(TObject *Sender);
	void __fastcall StepDebugActionExecute(TObject *Sender);
	void __fastcall TraceDebugActionExecute(TObject *Sender);
	void __fastcall ReturnDebugActionExecute(TObject *Sender);
	void __fastcall SearchPrevActionExecute(TObject *Sender);
	void __fastcall SearchActionExecute(TObject *Sender);
	void __fastcall FindDialogFind(TObject *Sender);
	void __fastcall SearchNextActionExecute(TObject *Sender);
	void __fastcall ClearBreakpointsActionExecute(TObject *Sender);
	void __fastcall FileTreeViewCustomDrawItem(TCustomTreeView *Sender,
          TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall OverWriteActionExecute(TObject *Sender);
private:	// ユーザー宣言
	std::map<std::wstring,std::string>	files_;
//	std::vector<AnsiString>				append_ext_;
	std::list<AnsiString>				file_list_;
	std::vector<AnsiString>				script_exts_;
	PROCESS_INFORMATION		proc_info_;
	AnsiString				project_file_path_;
	AnsiString				debuggee_path_;
	AnsiString				debuggee_args_;
	AnsiString				debuggee_data_;
	AnsiString				debuggee_working_folder_;
	HWND					debugee_hwnd_;

	DebuggeeCheckThread*	debuggee_check_thread_;
	bool					is_break_point_dirty_;

	AnsiString				break_file_;
	int						break_lineno_;

	AnsiString				search_word_;

	Breakpoints				breakpoints_;
	BreakpointLine*			curfile_breakpoints_;
	bool					is_break_;
	bool					is_request_break_;

	
	HIMAGELIST				system_image_list_;

	std::map<std::string,int>	file_icon_index_;

	void __fastcall OnFileDrop( TWMDropFiles& Msg );
	void __fastcall OnCopyData( TWMCopyData& Msg );
	bool __fastcall IsDebuggeeHandle( HWND hwnd ) const;
	bool __fastcall IsAppendFile( const AnsiString& path );

	void __fastcall SendBreakpoints( HWND hwnd );
	void __fastcall SendExceptionFlag( HWND hwnd );

	void __fastcall SendExec();
	void __fastcall SendBreak();
	void __fastcall SendStep();
	void __fastcall SendTrace();
	void __fastcall SendReturn();

	void __fastcall AddFilesFromDir( const AnsiString& projectDir );
	void __fastcall AddFiles( const std::list<AnsiString>& files );

	void __fastcall ExecDebuggee();

	void __fastcall ReadProjectFile();
	void __fastcall WriteProjectFile();

	void __fastcall BreakFromDebuggee( const void* data, size_t len );
	void __fastcall SetCallStack( const void* data, size_t len );
	void __fastcall SetLocalVariable( const void* data, size_t len );

	void __fastcall UpdateAll();
	void __fastcall UpdateTreeView();

	void __fastcall ShowLastError();

	AnsiString __fastcall GetDebuggeeCommandLine() const;

	void __fastcall ReadStringListFromString( const AnsiString& input, std::vector<AnsiString>& vals );
	void __fastcall WriteStringListFromString( AnsiString& input, const std::vector<AnsiString>& vals );

	void __fastcall OpenScriptFile( const AnsiString& path, int line = 0, bool force = false );

	void __fastcall SetBreakPoint( int lineno );
	void __fastcall ClearBreakPoint( int lineno );
	bool __fastcall IsBreakPoint( int lineno ) const;

	bool __fastcall AddFileIconIndex( const std::string& name, int index );
	int __fastcall GetFileIconIndex( const std::string& name );
	int __fastcall GetWithAddFileIconIndex( const std::string& ext );

	static const int DEBUGGER_COMM_AREA_MAX = 1024 * 1024;
	// 以下のような構造で、ブレークから復帰後にデバッガから上の領域に書き込まれる。
	// 領域を超えないようにするのはデバッガの責任
	struct DebuggerCommand {
		int		command_;
		int		next_offset_;	//!< このコマンドの先頭から次のデータまでのオフセット(アライメントしておくこと)
		int		size_;			//!< data のサイズ
		char	data_[1];
	};
	struct DebuggerCommandException {
		int		command_;
		int		next_offset_;	//!< このコマンドの先頭から次のデータまでのオフセット(アライメントしておくこと)
		int		size_;			//!< data のサイズ
		int		flag_;
		DebuggerCommandException( int flag )
		: command_(DBGEV_GER_EXCEPTION_FLG), next_offset_(sizeof(DebuggerCommandException)), size_(sizeof(int)), flag_(flag)
		{}
	};
	struct DebuggerCommandMinimum {
		int		command_;
		int		next_offset_;	//!< このコマンドの先頭から次のデータまでのオフセット(アライメントしておくこと)
		int		size_;			//!< data のサイズ

		DebuggerCommandMinimum( int command, int offset, int size = 0 )
		 : command_(command), next_offset_(offset), size_(size)
		{}
	};
	struct DebuggerHeader {
		int				num_of_command_;
		DebuggerCommand	commands_;

		void SetSingleCommand( int command ) {
			num_of_command_ = 1;
			commands_.command_ = command;
			commands_.next_offset_ = 0;
			commands_.size_ = 0;
		}
	};
	std::list<CommandBuffer>	command_buffer_;

	void __fastcall PushSingleCommand( int command ) {
		DebuggerHeader	debcmd;
		debcmd.SetSingleCommand( command );
		char* buff = new char[sizeof(debcmd)];
		memcpy( buff, &debcmd, sizeof(debcmd) );
		command_buffer_.push_back( CommandBuffer(sizeof(debcmd),buff) );
		break_lineno_ = -1;
	}
	void __fastcall PushSettingsCommand( int tailcommand );

	void*	debuggee_comm_area_addr_;
	int		debuggee_comm_area_size_;

	void __fastcall ClearCommand();

	void __fastcall SerachNext( const AnsiString& word, bool caseSensitive = true );
	void __fastcall SerachPrev( const AnsiString& word, bool caseSensitive = true );

	void __fastcall GetSystemImageList();

	void __fastcall ParseCommandline();
	AnsiString __fastcall GetApplicationFileName();
	AnsiString __fastcall GetApplicationFolderName();

public:		// ユーザー宣言
	__fastcall TScriptDebuggerForm(TComponent* Owner);
	virtual __fastcall ~TScriptDebuggerForm();
	virtual void __fastcall Dispatch(void *Message);

	//! デバッギ起動引数
	__property AnsiString DebuggeeCommandLine  = { read=GetDebuggeeCommandLine };
	//! デバッギワークフォルダ
	__property AnsiString DebuggeeWorkingFolder  = { read=debuggee_working_folder_ };
    //! アプリケーションファイル名
    __property AnsiString ApplicationFileName  = { read=GetApplicationFileName };
    //! アプリケーションフォルダ名
    __property AnsiString ApplicationFolderName  = { read=GetApplicationFolderName};

	__property bool IsRequestBreak = { read=is_request_break_, write=is_request_break_ };

//	__property int DebuggerCommand  = { read=GetDebuggerCommand,write=SetDebuggerCommand };
//	__property int DebuggerCommandSize = { read=GetDebuggerCommandSize,write=SetDebuggerCommandSize };
//	char* __fastcall GetDebuggerCommandData() { return dubugger_comm_area_.cmd.data_; }
//  	char* __fastcall GetDebuggerCommandArea() { return &dubugger_comm_area_; }
//	int __fastcall GetDebuggerCommandAreaSize() { return dubugger_comm_area_.cmd.size_ + sizeof(int)*2; }

	void* __fastcall GetDebugeeAreaAddr() { return debuggee_comm_area_addr_; }
	int __fastcall GetDebugeeAreaSize() { return debuggee_comm_area_size_; }

	//! デバッギチェックスレッドからの終了通知を受ける
	void __fastcall TarminateDebugeeCheckThread();
	void __fastcall WakeupDebugee();
	void __fastcall AppendDebugString( const AnsiString& log );
	void __fastcall SetProcInfo( const PROCESS_INFORMATION& info ) {
		proc_info_ = info;
	}

	bool IsExistCommand() const { return command_buffer_.size(); }
	bool GetFirstCommand( CommandBuffer& cmd ) {
		if( IsExistCommand() ) {
			std::list<CommandBuffer>::iterator i = command_buffer_.begin();
			cmd = (*i);
			command_buffer_.pop_front();
			return true;
		} else {
			cmd.data_ = 0;
			cmd.size_ = 0;
			return false;
		}
	}
	void __fastcall OnBreak();
	void __fastcall CancelBreak();

	void __fastcall SetBreakCommand();
};
//---------------------------------------------------------------------------
extern PACKAGE TScriptDebuggerForm *ScriptDebuggerForm;
//---------------------------------------------------------------------------
#endif
