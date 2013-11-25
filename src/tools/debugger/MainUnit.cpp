//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MainUnit.h"
#include "CheckDebuggeeUnit.h"
#include "ProjectSettingUnit.h"
#include <IniFiles.hpp>
#include <Controls.hpp>
#include <StrUtils.hpp>
#include <memory>
#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TScriptDebuggerForm *ScriptDebuggerForm;
//---------------------------------------------------------------------------
__fastcall TScriptDebuggerForm::TScriptDebuggerForm(TComponent* Owner)
	: TForm(Owner)
{
	memset( &proc_info_, 0, sizeof(proc_info_) );

	debuggee_check_thread_ = NULL;
	break_lineno_ = -1;
	curfile_breakpoints_ = NULL;
	debugee_hwnd_ = 0;
	is_break_ = false;
	is_break_point_dirty_ = false;
	is_request_break_ = false;

	debuggee_comm_area_addr_ = NULL;
	debuggee_comm_area_size_ = 0;

	SourceLineListBox->Canvas->Font = FontDialog->Font;
	if( SourceLineListBox->Canvas->Font->Height < 0 ) {
		SourceLineListBox->ItemHeight = -SourceLineListBox->Canvas->Font->Height + 2;
	} else {
		SourceLineListBox->ItemHeight = SourceLineListBox->Canvas->Font->Height + 2;
	}
	SourceLineListBox->DoubleBuffered = true;
	ValueListEditor->DoubleBuffered = true;
	FileTreeView->DoubleBuffered = true;

	ExecuteDebugAction->Enabled = false;
	KillDebugAction->Enabled = false;
	BreakDebugAction->Enabled = false;
	StepDebugAction->Enabled = false;
	TraceDebugAction->Enabled = false;
	ReturnDebugAction->Enabled = false;
	OverWriteAction->Enabled = false;

	::DragAcceptFiles( Handle, true );

	system_image_list_ = NULL;
	GetSystemImageList();

	// コマンドライン読み込み
	ParseCommandline();

	if( project_file_path_.IsEmpty() ) {
		AnsiString defaultPjName("debugger.sdp");
		if( FileExists(defaultPjName) ) {
			project_file_path_ = ExpandFileName( defaultPjName );
			ReadProjectFile();
		}
	}
}
//---------------------------------------------------------------------------
__fastcall TScriptDebuggerForm::~TScriptDebuggerForm()
{
	::ImageList_Destroy( system_image_list_ );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ParseCommandline()
{
	enum {
		OPT_INVALID = -1,
		OPT_PROJECT = 0,
	};
	int type = OPT_INVALID;
	int count = ParamCount();
	for( int i = 0; i < count; i++ ) {
		AnsiString param = ParamStr(1+i);
		if( param[1] == '-' ) {
			AnsiString typeName = param.SubString( 2, param.Length()-1 );
			if( typeName == AnsiString("p") ) {
				type = OPT_PROJECT;
			}
		} else {
			if( type == OPT_PROJECT ) {
				project_file_path_ = ExpandFileName( param );
				ReadProjectFile();
				type = OPT_INVALID;
			}
		}
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
AnsiString __fastcall MyReplaceText(const AnsiString& Src, const AnsiString& From, const AnsiString& To)
{
	AnsiString Result = StringReplace( Src, // 対象文字列
										From, // 元の文字
										To, // 置換後の文字
										TReplaceFlags() << rfReplaceAll << rfIgnoreCase // 置換のタイプの指定
									 );
	return Result;
}
*/
//---------------------------------------------------------------------------
template<typename string_t>
size_t split( std::list<string_t>& ret, const string_t& src, const string_t& delimiter )
{
	string_t::size_type offset = 0;
	string_t::size_type p;
	while( (p = src.find(delimiter,offset)) != src.npos ) {
		ret.push_back( src.substr(offset, p-offset) );
		offset = p + delimiter.size();
	}
	string_t tail( src.substr(offset) );
	if( tail.size() ) {
		ret.push_back( src.substr(offset) );
	}
	return ret.size();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::Dispatch(void *Message)
{
	switch( ((PMessage)Message)->Msg )
	{
	case WM_DROPFILES:
		OnFileDrop(*((TWMDropFiles*)Message));
		break;

	case WM_COPYDATA:
		OnCopyData(*((TWMCopyData*)Message));

	default:
		TForm::Dispatch(Message);
		break;
	}
}
//---------------------------------------------------------------------------
bool __fastcall TScriptDebuggerForm::IsDebuggeeHandle( HWND hwnd ) const
{
	DWORD dwPID;
	::GetWindowThreadProcessId( hwnd, &dwPID );
	return proc_info_.dwProcessId == dwPID;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::OnCopyData( TWMCopyData& Msg )
{
	if( IsDebuggeeHandle(Msg.From) == false ) return;

	debugee_hwnd_ = Msg.From;
	switch( Msg.CopyDataStruct->dwData ) {
	case DBGEV_GEE_LOG:
		LogMemo->Lines->Add( AnsiString( (wchar_t*)Msg.CopyDataStruct->lpData ) );
		break;
	case DBGEV_GEE_BREAK:
		CallStackListBox->Clear();
		ValueListEditor->Strings->Clear();
		BreakFromDebuggee( Msg.CopyDataStruct->lpData, Msg.CopyDataStruct->cbData );
		break;
	case DBGEV_GEE_STACK_TRACE:
		SetCallStack( Msg.CopyDataStruct->lpData, Msg.CopyDataStruct->cbData );
		break;
	case DBGEV_GEE_LOCAL_VALUE:
		SetLocalVariable( Msg.CopyDataStruct->lpData, Msg.CopyDataStruct->cbData );
		break;
	case DBGEV_GEE_CLASS_VALUE:
		SetLocalVariable( Msg.CopyDataStruct->lpData, Msg.CopyDataStruct->cbData );
		break;
	case DBGEV_GEE_REQUEST_SETTINGS:
		if( Msg.CopyDataStruct->cbData >= (sizeof(int)*2) ) {
			int* data = (int*)Msg.CopyDataStruct->lpData;
			debuggee_comm_area_addr_ = (void*)(data[0]);
			debuggee_comm_area_size_ = data[1];

//			SendBreakpoints(debugee_hwnd_);
//			SendExceptionFlag(debugee_hwnd_);
//			SendExec();
			PushSettingsCommand( DBGEV_GER_EXEC );
		}
		break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::PushSettingsCommand( int tailcommand )
{
	ClearCommand();

	size_t numcmd = 0;
	// ブレイクポイント用
	size_t commandsize = sizeof(int) + sizeof(DebuggerCommandMinimum) // DBGEV_GER_BREAKPOINT_START
		+ sizeof(DebuggerCommandMinimum); // DBGEV_GER_BREAKPOINT_END
	numcmd += 2;

	const Breakpoints::breakpoints& points = breakpoints_.BreakPoint;
	for( Breakpoints::const_iterator i = points.begin(); i != points.end(); ++i ) {
		size_t namelen = (i->first.size()+1) * sizeof(wchar_t);
		const BreakpointLine::lines& lines = i->second.Lines;
		size_t count = lines.size();
		if( count ) {
			size_t breakptsize = namelen + (count+1) * sizeof(int) + sizeof(DebuggerCommandMinimum);
			breakptsize = ((breakptsize + 3) / 4) * 4;	// アライメント
			commandsize += breakptsize;
			numcmd++;
		}
	}

	// 例外用
	commandsize += sizeof(DebuggerCommandMinimum) + sizeof(int);
	numcmd++;

	// 実行命令用
	commandsize += sizeof(DebuggerCommandMinimum);
	numcmd++;

	if( (int)commandsize > debuggee_comm_area_size_ ) {
		// 本来は分割して送信するようにする
		Application->MessageBox( "ブレークポイントが多すぎます", "エラー", MB_OK );
		return;
	}

	CommandBuffer	comBuff( commandsize, new char[commandsize] );
	((DebuggerHeader*)(comBuff.data_))->num_of_command_ = numcmd;
	char* startbuff = (char*)&(((DebuggerHeader*)(comBuff.data_))->commands_);

	// ブレイクポイント開始
	DebuggerCommandMinimum	breaksatrt( DBGEV_GER_BREAKPOINT_START, sizeof(DebuggerCommandMinimum) );
	memcpy( startbuff, &breaksatrt, sizeof(DebuggerCommandMinimum) );
	startbuff += sizeof(DebuggerCommandMinimum);

	// ブレイクポイントリスト
	for( Breakpoints::const_iterator i = points.begin(); i != points.end(); ++i ) {
		size_t namelen = (i->first.size()+1) * sizeof(wchar_t);
		const BreakpointLine::lines& lines = i->second.Lines;
		size_t count = lines.size();
		if( count ) {
			size_t breakptsize = namelen + (count+1) * sizeof(int) + sizeof(DebuggerCommandMinimum);
			breakptsize = ((breakptsize + 3) / 4) * 4;	// アライメント
			int* breakcommand = (int*)startbuff;

			*breakcommand = DBGEV_GER_BREAKPOINT;
			breakcommand++;
			*breakcommand = breakptsize;
			breakcommand++;

			size_t datalen = (count+1)*sizeof(int) + namelen;
			*breakcommand = datalen;
			breakcommand++;
			*breakcommand = count;
			breakcommand++;

			for( BreakpointLine::const_iterator j = lines.begin(); j != lines.end(); ++j ) {
				*breakcommand = j->second;
				breakcommand++;
			}
			wchar_t* filename = (wchar_t*)breakcommand;
			memcpy( filename, i->first.c_str(), (i->first.size()+1) * sizeof(wchar_t) );

			startbuff += breakptsize;
		}
	}
	// ブレイクポイント終了
	DebuggerCommandMinimum	breakend( DBGEV_GER_BREAKPOINT_END, sizeof(DebuggerCommandMinimum) );
	memcpy( startbuff, &breakend, sizeof(DebuggerCommandMinimum) );
	startbuff += sizeof(DebuggerCommandMinimum);

	// 例外
	DebuggerCommandException exceptionFlg(0);
	memcpy( startbuff, &exceptionFlg, sizeof(DebuggerCommandException) );
	startbuff += sizeof(DebuggerCommandException);

	// 実行命令
	DebuggerCommandMinimum tail( tailcommand, 0, sizeof(DebuggerCommandMinimum) );
	memcpy( startbuff, &tail, sizeof(DebuggerCommandMinimum) );

	command_buffer_.push_back( comBuff );

	is_break_point_dirty_ = false;
	break_lineno_ = -1;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ClearCommand()
{
	for( std::list<CommandBuffer>::iterator i = command_buffer_.begin(); i != command_buffer_.end(); ++i ) {
		delete[] (*i).data_;
	}
	command_buffer_.clear();
}
//---------------------------------------------------------------------------
class DebuggerMessage : public COPYDATASTRUCT
{
public:
	DebuggerMessage( int messageType, void* data, size_t length ) {
		dwData = messageType;
		cbData = length;
		lpData = data;
	}
};
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendBreakpoints( HWND hwnd )
{
	assert(0);	// 使えない
	{	// ブレークポイント送信開始
		DebuggerMessage	message( DBGEV_GER_BREAKPOINT_START, 0, 0 );
		::SendMessage( hwnd, WM_COPYDATA, (WPARAM)Handle, (LPARAM)&message );
	}
	{	// ブレークポイント送信
		const Breakpoints::breakpoints& points = breakpoints_.BreakPoint;
		for( Breakpoints::const_iterator i = points.begin(); i != points.end(); ++i ) {
			size_t namelen = (i->first.size()+1) * sizeof(wchar_t);
			const BreakpointLine::lines& lines = i->second.Lines;
			size_t count = lines.size();
			if( count ) {
				// 1個以上ブレークポイントがあるファイルの時送る
				size_t datalen = (count+1)*sizeof(int) + namelen;
				// ポイント数、ライン番号配列、ファイル名の順で送る
				std::vector<char>	buffer(datalen);
				int* points = (int*)&(buffer[0]);
				*points = count;
				points++;
				for( BreakpointLine::const_iterator j = lines.begin(); j != lines.end(); ++j ) {
					*points = j->second;
					points++;
				}
				wchar_t* filename = (wchar_t*)points;
				memcpy( filename, i->first.c_str(), (i->first.size()+1) * sizeof(wchar_t) );
				
				DebuggerMessage	message( DBGEV_GER_BREAKPOINT, &(buffer[0]), datalen );
				::SendMessage( hwnd, WM_COPYDATA, (WPARAM)Handle, (LPARAM)&message );
			}
        }
	}

	{	// ブレークポイント送信終了
		DebuggerMessage	message( DBGEV_GER_BREAKPOINT_END, 0, 0 );
		::SendMessage( hwnd, WM_COPYDATA, (WPARAM)Handle, (LPARAM)&message );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendExceptionFlag( HWND hwnd )
{
	assert(0);	// 使えない
	int data[1] = {0};
	DebuggerMessage	message( DBGEV_GER_EXCEPTION_FLG, (void*)data, sizeof(int) );
	::SendMessage( hwnd, WM_COPYDATA, (WPARAM)Handle, (LPARAM)&message );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SetBreakCommand()
{
	ClearCommand();
	PushSingleCommand( DBGEV_GER_BREAK );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendExec()
{
	ClearCommand();
	if( is_break_point_dirty_ ) {
		PushSettingsCommand( DBGEV_GER_EXEC );
	} else {
		PushSingleCommand( DBGEV_GER_EXEC );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendBreak()
{
	ClearCommand();
	if( is_break_point_dirty_ ) {
		PushSettingsCommand( DBGEV_GER_BREAK );
	} else {
		PushSingleCommand( DBGEV_GER_BREAK );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendStep()
{
	ClearCommand();
	if( is_break_point_dirty_ ) {
		PushSettingsCommand( DBGEV_GER_STEP);
	} else {
		PushSingleCommand( DBGEV_GER_STEP );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendTrace()
{
	ClearCommand();
	if( is_break_point_dirty_ ) {
		PushSettingsCommand( DBGEV_GER_TRACE );
	} else {
		PushSingleCommand( DBGEV_GER_TRACE );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SendReturn()
{
	ClearCommand();
	if( is_break_point_dirty_ ) {
		PushSettingsCommand( DBGEV_GER_RETURN );
	} else {
		PushSingleCommand( DBGEV_GER_RETURN );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SetLocalVariable( const void* data, size_t len )
{
	const wchar_t* tail = (const wchar_t*)( ((const char*)data) + len);
	int count = *(const int*)data;
	const wchar_t* names = (const wchar_t*)( ((char*)data) + sizeof(int) );
	for( int i = 0; i < count && names < tail; i++ ) {
		AnsiString name(names);
		while( (*names) && names < tail ) { names++; }
		names++;
		AnsiString value(names);
		while( (*names) && names < tail ) { names++; }
		names++;
		ValueListEditor->Strings->Add( name + AnsiString("=") + value );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SetCallStack( const void* data, size_t len )
{
	CallStackListBox->Clear();

	std::wstring delimiter( L" <-- " );
	std::wstring result_str( (wchar_t*)data );
	std::list<std::wstring> namelist;
	split( namelist, result_str, delimiter );
	for( std::list<std::wstring>::const_iterator itr = namelist.begin(); itr != namelist.end(); ++itr ) {
		CallStackListBox->Items->Add( AnsiString( (*itr).c_str() ) );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::BreakFromDebuggee( const void* data, size_t len )
{
	int* intbuff = (int*)data;
	int lineno = *intbuff;
	intbuff++;
	wchar_t* filename = (wchar_t*)intbuff;
	std::wstring wfilename(filename);
	std::transform( wfilename.begin(), wfilename.end(), wfilename.begin(), std::tolower );

	AnsiString file( filename );
	std::map<std::wstring,std::string>::const_iterator i = files_.find( wfilename );
	if( i != files_.end() ) {
		break_file_ = i->second.c_str();
		break_lineno_ = lineno;
		OpenScriptFile( break_file_, lineno );
	}
}
//---------------------------------------------------------------------------
// ブレーク状態になったときにコールされる
void __fastcall TScriptDebuggerForm::OnBreak()
{
	is_break_ = true;
	StepDebugAction->Enabled = true;
	TraceDebugAction->Enabled = true;
	ReturnDebugAction->Enabled = true;
	BreakDebugAction->Enabled = false;
	IsRequestBreak = false;
}
//---------------------------------------------------------------------------
// ブレーク状態解除
void __fastcall TScriptDebuggerForm::CancelBreak()
{
	is_break_ = false;
	StepDebugAction->Enabled = false;
	TraceDebugAction->Enabled = false;
	ReturnDebugAction->Enabled = false;
	BreakDebugAction->Enabled = true;
	IsRequestBreak = false;
	SourceLineListBox->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::OpenScriptFile( const AnsiString& path, int line, bool force )
{
	if( path.Length() == 0 ) return;

	if( force == false ) {
		AnsiString newfilename = ExtractFileName(path).LowerCase();
		AnsiString curfilename = FileTabSheet->Caption.LowerCase();
		if( curfilename == newfilename ) {
			if( line >= 0 ) {
				SourceLineListBox->ItemIndex = line;
			}
			SourceLineListBox->Repaint();
			return;
		}
	}

	SourceLineListBox->Items->LoadFromFile( path );
	FileTabSheet->Caption = ExtractFileName(path);
	if( line >= 0 ) {
		SourceLineListBox->ItemIndex = line;
	}
	AnsiString filename = FileTabSheet->Caption.LowerCase();
	WideString wfilename( filename );
	curfile_breakpoints_ = breakpoints_.GetBreakPointLines( std::wstring(wfilename.data()) );
	SourceLineListBox->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SetBreakPoint( int lineno )
{
	AnsiString filename = FileTabSheet->Caption.LowerCase();
	WideString wfilename( filename );
	std::wstring stwfile(wfilename.data());

	std::map<std::wstring,std::string>::const_iterator i = files_.find( stwfile );
	if( i != files_.end() ) {
		breakpoints_.SetBreakPoint( stwfile, lineno );
	}
	curfile_breakpoints_ = breakpoints_.GetBreakPointLines( stwfile );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ClearBreakPoint( int lineno )
{
	AnsiString filename = FileTabSheet->Caption.LowerCase();
	WideString wfilename( filename );
	std::wstring stwfile(wfilename.data());
	breakpoints_.ClearBreakPoint( stwfile, lineno );
}
//---------------------------------------------------------------------------
bool __fastcall TScriptDebuggerForm::IsBreakPoint( int lineno ) const
{
	AnsiString filename = FileTabSheet->Caption.LowerCase();
	WideString wfilename( filename );
	std::wstring stwfile(wfilename.data());

	std::map<std::wstring,std::string>::const_iterator i = files_.find( stwfile );
	if( i != files_.end() ) {
		return breakpoints_.IsBreakPoint( stwfile, lineno );
	}
	return false;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::OnFileDrop( TWMDropFiles& Msg )
{
	HDROP hdrop = (HDROP)Msg.Drop;
	char buff[MAX_PATH];

	AnsiString path;
	int n = ::DragQueryFile(hdrop, 0xffffffff, NULL, 0);
	for(int i = 0; i < n; i++){
		::DragQueryFile(hdrop, i, buff, 256);
		AnsiString fileName(buff);

		if( path.IsEmpty() ) {
			path = ExtractFileDir( fileName ) + AnsiString("\\");
		} else {
			fileName = path + ExtractFileName( fileName );
		}

		AnsiString ext = ExtractFileExt(fileName).LowerCase();
		DWORD attr = ::GetFileAttributes( fileName.c_str() );
		bool is_directory = (attr & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
		if( is_directory ) {
			debuggee_data_ = fileName;
			AddFilesFromDir( debuggee_data_ );
			break;  // 最初の1個しか見ない
		} else if( ext == AnsiString(".sdp") ) {
			project_file_path_ = fileName;
			ReadProjectFile();
			break;  // 最初の1個しか見ない
		} else if( ext == AnsiString(".exe") ) {
			debuggee_path_ = fileName;
			ExecuteDebugAction->Enabled = true;
			break;  // 最初の1個しか見ない
		}
	}
	::DragFinish(hdrop);
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::UpdateTreeView()
{
	FileTreeView->Items->Clear();

	TTreeNode* root = FileTreeView->Items->AddObject( NULL, debuggee_data_, NULL );
	AnsiString basePath = debuggee_data_ + AnsiString("\\");
	typedef std::list<AnsiString>::const_iterator const_iterator;
	for( const_iterator i = file_list_.begin(); i != file_list_.end(); ++i ) {
		AnsiString path( *i );
		AnsiString relPath( ExtractRelativePath( basePath, path ) );
		std::list<std::string> pathlist;
		split( pathlist, std::string(relPath.c_str()), std::string("\\") );
		TTreeNode* node = root;
		TTreeNode* parent = root;
		if( node->HasChildren ){
			node = node->getFirstChild();
		}
		for( std::list<std::string>::const_iterator j = pathlist.begin(); j != pathlist.end(); ++j ) {
			AnsiString name( (*j).c_str() );
			TTreeNode* cur = node;
			node = NULL;
			while( cur ) {
				const AnsiString& caption = cur->Text;
				if( caption == name ) {
					node = cur;
					break;
				} else {
					cur = cur->getNextSibling();
				}
			}
			if( node == NULL ) {
				node = FileTreeView->Items->AddChildObject( parent, name, NULL );
				node->ImageIndex = GetFileIconIndex( std::string("folder close") );
//				node->SelectedIndex = GetFileIconIndex( std::string("folder open") );
				node->SelectedIndex = node->ImageIndex;
				node->StateIndex = -1;
			}
			parent = node;
		}
		if( node ) {
			node->Data = (void*)&(*i);
			node->ImageIndex = GetWithAddFileIconIndex( std::string( ExtractFileExt( (*i) ).UpperCase().c_str() ) );
			node->SelectedIndex = node->ImageIndex;
		}
	}

	FileTreeView->FullExpand();
}
//---------------------------------------------------------------------------
struct AnsiStringCompare : public std::binary_function<bool, AnsiString, AnsiString> {
	bool operator ()(const AnsiString& rhs, const AnsiString& lhs ) const {
		return( rhs.AnsiCompareIC( lhs ) < 0 );
	}
};
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::AddFilesFromDir( const AnsiString& projectDir )
{
	breakpoints_.ClearAll();
	file_list_.clear();
	std::list<AnsiString> files;
	files.push_back( projectDir );
	AddFiles( files );

//	AnsiString filePath = ExtractRelativePath( basepath, ExtractFilePath( fullpath ) ).LowerCase();

	files_.clear();
	for( std::list<AnsiString>::const_iterator i = file_list_.begin(); i != file_list_.end(); ++i ) {
		AnsiString filename( ExtractFileName( (*i) ).LowerCase() );
		WideString wfilename( filename );
		files_.insert( std::map<std::wstring,std::string>::value_type( std::wstring( wfilename.data() ), std::string( (*i).c_str() ) ) );
	}

	UpdateTreeView();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::AddFiles( const std::list<AnsiString>& files )
{
	if( files.size() ) {
		for( std::list<AnsiString>::const_iterator i = files.begin(); i != files.end(); ++i ) {
			DWORD attr = ::GetFileAttributes( (*i).c_str() );
			if( (attr & FILE_ATTRIBUTE_DIRECTORY) && (attr & FILE_ATTRIBUTE_HIDDEN) == 0 ) {	// 隠しファイルは追加しない(.svn)対策
				AnsiString findDir = (*i) + AnsiString("\\*");
				std::list<AnsiString>	dirFiles;
				WIN32_FIND_DATA			FindData;
				HANDLE hFind = ::FindFirstFile( findDir.c_str(), &FindData );
				if( hFind != INVALID_HANDLE_VALUE ) {
					do {
						AnsiString	fileName( FindData.cFileName );
						if( fileName != AnsiString(".") && fileName != AnsiString("..") ) {
							dirFiles.push_back( (*i) + AnsiString("\\") + fileName );
						}
					} while( FindNextFile( hFind, &FindData ) );
					if( dirFiles.size() ) {
						AnsiStringCompare	comp;
						dirFiles.sort( comp );
						AddFiles( dirFiles );
					}
				}
			} else if( (attr & FILE_ATTRIBUTE_HIDDEN) == 0 ) {	// 隠しファイルは追加しない(.svn)対策]
				if( IsAppendFile( *i ) ) {
					file_list_.push_back( *i );
                }
			}
		}
	}
}
//---------------------------------------------------------------------------
bool __fastcall TScriptDebuggerForm::IsAppendFile( const AnsiString& path )
{
	AnsiString ext = ExtractFileExt( path ).LowerCase();
	std::vector<AnsiString>::const_iterator e = std::find( script_exts_.begin(), script_exts_.end(), ext );
	return ( e != script_exts_.end() );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ExecDebuggee()
{
	if( debuggee_path_.Length() == 0 ) return;

	if( proc_info_.dwProcessId == 0 ) {
		// まだ起動していない
		debugee_hwnd_ = 0;
		debuggee_check_thread_ = new DebuggeeCheckThread(false);
	} else {
		// 起動中
		SendExec();
    }
}
//---------------------------------------------------------------------------
AnsiString __fastcall TScriptDebuggerForm::GetDebuggeeCommandLine() const
{
	AnsiString arg( debuggee_path_ );
	if( debuggee_args_.Length() ) {
		arg += AnsiString(" ") + debuggee_args_;
	}
	if( debuggee_data_.Length() ) {
		arg += AnsiString(" ") + debuggee_data_;
	}
	return arg;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::UpdateAll()
{
	AddFilesFromDir( debuggee_data_ );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ReadProjectFile()
{
	if( project_file_path_.IsEmpty() ) return;

	breakpoints_.ClearAll();
	curfile_breakpoints_ = NULL;
	std::auto_ptr<TIniFile> projFile( new TIniFile( project_file_path_ ) );
	if( projFile.get() ) {
		// projファイルのパスをカレントディレクトリに設定する
		AnsiString filePath = ExtractFilePath(project_file_path_);
		AnsiString oldCurDir = GetCurrentDir();
		if( SetCurrentDir( filePath ) ) {
			debuggee_path_ = ExpandFileName( projFile->ReadString( "Debugee", "Path", "" ) );
			debuggee_args_ = projFile->ReadString( "Debugee", "Args", "" );
			debuggee_data_ = ExpandFileName( projFile->ReadString( "Debugee", "DataFolder", "" ) );
			debuggee_working_folder_ = ExpandFileName( projFile->ReadString( "Debugee", "WorkingFolder", "" ) );
			SetCurrentDir( oldCurDir );	// 元に戻す
		} else {
			debuggee_path_ = projFile->ReadString( "Debugee", "Path", "" );
			debuggee_args_ = projFile->ReadString( "Debugee", "Args", "" );
			debuggee_data_ = projFile->ReadString( "Debugee", "DataFolder", "" );
			debuggee_working_folder_ = projFile->ReadString( "Debugee", "WorkingFolder", "" );
		}

		AnsiString scriptExts = projFile->ReadString( "Debugee", "ScriptExt", "" );
		ReadStringListFromString( scriptExts, script_exts_ );

		UpdateAll();
		ExecuteDebugAction->Enabled = true;
		OverWriteAction->Enabled = true;
		Caption = AnsiString("krkrdebg - スクリプト デバッガ - ") + project_file_path_;
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::WriteProjectFile()
{
	if( project_file_path_.IsEmpty() ) return;

	::DeleteFile( project_file_path_.c_str() );

	std::auto_ptr<TIniFile> projFile( new TIniFile( project_file_path_ ) );
	if( projFile.get() ) {
		AnsiString pjDrive = ExtractFileDrive( project_file_path_ );
		AnsiString baseName = ExtractFilePath( project_file_path_ );

		AnsiString writePath = debuggee_path_;
		AnsiString writeDataPath = debuggee_data_;
		AnsiString writeWorkingFolder = debuggee_working_folder_;
		if( pjDrive == ExtractFileDrive( debuggee_path_ ) ) {
			writePath = ExtractRelativePath( baseName, debuggee_path_ );
		}
		if( pjDrive == ExtractFileDrive( debuggee_data_ ) ) {
			writeDataPath = ExtractRelativePath( baseName, debuggee_data_ );
		}
		if( pjDrive == ExtractFileDrive( debuggee_working_folder_ ) ) {
			writeWorkingFolder = ExtractRelativePath( baseName, debuggee_working_folder_ );
		}
		projFile->WriteString( "Debugee", "Path", writePath );
		projFile->WriteString( "Debugee", "Args", debuggee_args_ );
		projFile->WriteString( "Debugee", "DataFolder", writeDataPath );
		projFile->WriteString( "Debugee", "WorkingFolder", writeWorkingFolder );

		AnsiString scriptExts;
		WriteStringListFromString( scriptExts, script_exts_ );
		projFile->WriteString( "Debugee", "ScriptExt", scriptExts );

		OverWriteAction->Enabled = true;
		Caption = AnsiString("krkrdebg - スクリプト デバッガ - ") + project_file_path_;
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ShowLastError()
{
	LPVOID lpMsgBuf;
	::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
						(LPTSTR)&lpMsgBuf, 0, NULL );
	::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	LocalFree(lpMsgBuf);
}
//---------------------------------------------------------------------------
//! デバッギチェックスレッドからの終了通知を受ける
void __fastcall TScriptDebuggerForm::TarminateDebugeeCheckThread()
{
	debuggee_check_thread_ = NULL;
	memset( &proc_info_, 0, sizeof(proc_info_) );
	CancelBreak();
	KillDebugAction->Enabled = false;
	BreakDebugAction->Enabled = false;
	break_lineno_ = -1;
	SourceLineListBox->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::WakeupDebugee()
{
	KillDebugAction->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::AppendDebugString( const AnsiString& log )
{
	LogMemo->Lines->Add( log );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ReadStringListFromString( const AnsiString& input, std::vector<AnsiString>& vals )
{
	vals.clear();
	if( input.Length() <= 0 ) return;

	int length = input.Length() + 1;
	int start = 1;
	for( int i = 1; i < length; i++ ) {
		if( input[i] == ' ' ) {
			AnsiString ext( input.SubString( start, i - start ) );
			vals.push_back( ext );
			start = i+1;
		}
	}
	if( start != length ) {
		AnsiString ext( input.SubString( start, length - start ) );
		vals.push_back( ext );
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::WriteStringListFromString( AnsiString& input, const std::vector<AnsiString>& vals )
{
	input = "";
	for( std::vector<AnsiString>::const_iterator i = vals.begin(); i != vals.end(); ++i ) {
		input += (*i) + AnsiString(" ");
	}
	input.TrimRight();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SetProjectActionExecute(TObject *Sender)
{
	ProjectSettingForm->ExePath = debuggee_path_;
	ProjectSettingForm->Arg = debuggee_args_;
	ProjectSettingForm->ProjectFolder = debuggee_data_;
	ProjectSettingForm->WorkingFolder = debuggee_working_folder_;

	AnsiString scriptExts;
	WriteStringListFromString( scriptExts, script_exts_ );
	ProjectSettingForm->ScriptExt = scriptExts;

	ProjectSettingForm->ShowModal();
	if( ProjectSettingForm->ModalResult == mrOk ) {
		debuggee_path_ = ProjectSettingForm->ExePath;
		debuggee_args_ = ProjectSettingForm->Arg;
		debuggee_data_ = ProjectSettingForm->ProjectFolder;
		debuggee_working_folder_ = ProjectSettingForm->WorkingFolder;
		scriptExts = ProjectSettingForm->ScriptExt;
		ReadStringListFromString( scriptExts, script_exts_ );

		breakpoints_.ClearAll();
		curfile_breakpoints_ = NULL;
		AddFilesFromDir( debuggee_data_ );

		if( debuggee_path_.Length() ) {
			ExecuteDebugAction->Enabled = true;
		}
	}
}
//---------------------------------------------------------------------------
AnsiString __fastcall TScriptDebuggerForm::GetApplicationFileName()
{
	return ExtractFileName( Application->ExeName );
}
//---------------------------------------------------------------------------
AnsiString __fastcall TScriptDebuggerForm::GetApplicationFolderName()
{
	return ExtractFilePath( Application->ExeName );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SourceLineListBoxDrawItem( TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
	TCanvas *pCanvas = ((TListBox *)Control)->Canvas;
	AnsiString Text = SourceLineListBox->Items->Strings[Index];
	Text = ReplaceStr( Text, AnsiString("\t"), AnsiString("    "));

	const int BREAK_POINT_BOX_WIDTH = SourceLineListBox->ItemHeight;
	const int LINE_TEXT_WIDTH = BREAK_POINT_BOX_WIDTH * 3;
	TRect breakPointBox = Rect;
	breakPointBox.Right = breakPointBox.Left + BREAK_POINT_BOX_WIDTH;
	pCanvas->Brush->Color = clBtnFace;
	pCanvas->FillRect( breakPointBox );

	bool	isBreakPoint = false;
	if( curfile_breakpoints_ ) {
		isBreakPoint = curfile_breakpoints_->IsBreakPoint( Index );
    }
	if( isBreakPoint ) {
		pCanvas->Pen->Color = clRed;
		pCanvas->Brush->Color = clRed;

		TRect breakPointCircle = breakPointBox;
		breakPointCircle.Top += 2;
		breakPointCircle.Bottom -= 2;
		breakPointCircle.Left += 2;
		breakPointCircle.Right = breakPointCircle.Left + breakPointCircle.Bottom - breakPointCircle.Top;
		pCanvas->Ellipse( breakPointCircle );
	}

	// 行番号表示
	TRect linenoRect = Rect;
	linenoRect.Left += BREAK_POINT_BOX_WIDTH;
	linenoRect.Right = linenoRect.Left + LINE_TEXT_WIDTH;
	pCanvas->Brush->Color = clBtnFace;
	pCanvas->FillRect( linenoRect );    
	pCanvas->Font->Color = clBlack;
	AnsiString lineText(Index);
	pCanvas->TextRect( linenoRect, linenoRect.Left+2, linenoRect.Top+1, lineText );

	// 実際のテキスト表示
	TRect textRect = Rect;
	textRect.Left += BREAK_POINT_BOX_WIDTH + LINE_TEXT_WIDTH;
	int left = textRect.Left + 2;
	int top = textRect.Top;

	pCanvas->Brush->Color = clWhite;
	if( break_lineno_ == Index )
		pCanvas->Brush->Color = clRed;
	pCanvas->FillRect( textRect );
	pCanvas->Font->Color = clBlack;
	if( break_lineno_ == Index )
		pCanvas->Font->Color = clWhite;
	pCanvas->TextRect( textRect, left, top, Text);

	if( State.Contains(odSelected) ) {	// 選択行
		pCanvas->Pen->Color = clBlack;
		pCanvas->Pen->Style = psSolid;
		pCanvas->MoveTo(Rect.Left, Rect.Bottom-1);
		pCanvas->LineTo(Rect.Right-1, Rect.Bottom-1);

	}
	// 点線を消す
	if( State.Contains(odSelected) && State.Contains(odFocused) ) {
		pCanvas->DrawFocusRect(Rect);
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileTreeViewDblClick(TObject *Sender)
{
//	TPoint pos = Mouse->CursorPos;
//	TPoint clPt = FileTreeView->ScreenToClient( pos );
//	TTreeNode* node = FileTreeView->GetNodeAt( clPt.x, clPt.y );
	TTreeNode* node = FileTreeView->Selected;
	if( node && node->Data ) {
		const AnsiString* filepath = (const AnsiString*)node->Data;
		if( filepath->Length() ) {
			break_lineno_ = -1;
			OpenScriptFile( filepath->c_str(), 0, true );
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileOpenAccept(TObject *Sender)
{
	project_file_path_ = FileOpen->Dialog->FileName;
	ReadProjectFile();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileSaveAsAccept(TObject *Sender)
{
	project_file_path_ = FileSaveAs->Dialog->FileName;
	WriteProjectFile();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileSaveAsBeforeExecute(TObject *Sender)
{
	FileSaveAs->Dialog->FileName = project_file_path_;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileOpenBeforeExecute(TObject *Sender)
{
	FileOpen->Dialog->FileName = project_file_path_;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SelectTextFontActionExecute(TObject *Sender)
{
	FontDialog->Font = SourceLineListBox->Canvas->Font;
	if( FontDialog->Execute() ) {
		SourceLineListBox->Canvas->Font = FontDialog->Font;
		if( SourceLineListBox->Canvas->Font->Height < 0 ) {
			SourceLineListBox->ItemHeight = -SourceLineListBox->Canvas->Font->Height + 2;
		} else {
			SourceLineListBox->ItemHeight = SourceLineListBox->Canvas->Font->Height;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SourceLineListBoxDblClick(TObject *Sender)
{
	int index = SourceLineListBox->ItemIndex;
	if( index >= 0 ) {
		if( IsBreakPoint( index ) ) {
			ClearBreakPoint( index );
		} else {
			SetBreakPoint( index );
		}
		SourceLineListBox->Repaint();
		FileTreeView->Repaint();
		is_break_point_dirty_ = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ExecuteDebugActionExecute(TObject *Sender)
{
	ClearCommand();
	ExecDebuggee();
	CancelBreak();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::KillDebugActionExecute(TObject *Sender)
{
	SendExec();
	// 強制終了
	::TerminateProcess(proc_info_.hProcess, 0);
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::BreakDebugActionExecute(TObject *Sender)
{
	CancelBreak();
	IsRequestBreak = true;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::StepDebugActionExecute(TObject *Sender)
{
	SendStep();
	CancelBreak();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::TraceDebugActionExecute(TObject *Sender)
{
	SendTrace();
	CancelBreak();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ReturnDebugActionExecute(TObject *Sender)
{
	SendReturn();
	CancelBreak();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SerachNext( const AnsiString& word, bool caseSensitive )
{
	if( word.Length() == 0 ) return;

	TListBox* listBox = SourceLineListBox;
	TStrings* strings = listBox->Items;
	int count = strings->Count;
	int startindex = listBox->ItemIndex;
	startindex++;
	if( caseSensitive ) {
		for( int i = startindex; i < count; i++ ) {
			AnsiString lineword = strings->Strings[i];
			if( AnsiContainsStr( lineword, word ) ) {
				// hit
				listBox->ItemIndex = i;
				return;
			}
		}
	} else {
		for( int i = startindex; i < count; i++ ) {
			AnsiString lineword = strings->Strings[i];
			if( AnsiContainsText( lineword, word ) ) {
				// hit
				listBox->ItemIndex = i;
				return;
			}
		}
	}
	Application->MessageBox( (word + AnsiString("は見付かりませんでした。")).c_str(), "検索", MB_OK );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SerachPrev( const AnsiString& word, bool caseSensitive )
{
	if( word.Length() == 0 ) return;

	TListBox* listBox = SourceLineListBox;
	TStrings* strings = listBox->Items;
	int startindex = listBox->ItemIndex;
	startindex--;
	if( caseSensitive ) {
		for( int i = startindex; i >= 0; i-- ) {
			AnsiString lineword = strings->Strings[i];
			if( AnsiContainsStr( lineword, word ) ) {
				// hit
				listBox->ItemIndex = i;
				return;
			}
		}
	} else {
		for( int i = startindex; i >= 0; i-- ) {
			AnsiString lineword = strings->Strings[i];
			if( AnsiContainsText( lineword, word ) ) {
				// hit
				listBox->ItemIndex = i;
				return;
			}
		}
	}
	Application->MessageBox( (word + AnsiString("は見付かりませんでした。")).c_str(), "検索", MB_OK );
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SearchPrevActionExecute(TObject *Sender)
{
	SerachPrev(search_word_);
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SearchActionExecute(TObject *Sender)
{
	FindDialog->FindText = search_word_;
	FindDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FindDialogFind(TObject *Sender)
{
	search_word_ = FindDialog->FindText;
	if( search_word_.Length() ) {
		SearchNextAction->Enabled = true;
		SearchPrevAction->Enabled = true;
		bool casesensitive = FindDialog->Options.Contains(frMatchCase);
		if( FindDialog->Options.Contains(frDown) ) {
			SerachNext(search_word_,casesensitive);
		} else {
			SerachPrev(search_word_,casesensitive);
		}
	} else {
		SearchNextAction->Enabled = false;
		SearchPrevAction->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::SearchNextActionExecute(TObject *Sender)
{
	SerachNext(search_word_);
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::ClearBreakpointsActionExecute( TObject *Sender)
{
	breakpoints_.ClearAll();
	curfile_breakpoints_ = NULL;
	SourceLineListBox->Repaint();
	FileTreeView->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::FileTreeViewCustomDrawItem( TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw)
{
	DefaultDraw = true;
	if( Node && Node->Data ) {
		const AnsiString* filepath = (const AnsiString*)Node->Data;
		AnsiString filename = ExtractFileName(*filepath).LowerCase();
		WideString wfilename( filename );
		if( breakpoints_.HasBreakPoint( std::wstring( wfilename.data() ) ) ) {
			Sender->Canvas->Font->Color = clRed;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::GetSystemImageList()
{
	HICON hIcon;
	TIcon* Icon;
	SHFILEINFO sfi;

	// フォルダアイコン(クローズ)
	system_image_list_ = (HIMAGELIST)::SHGetFileInfo("C:\\WINDOWS", 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX|SHGFI_ICON );
	if( system_image_list_ != 0 ) {
		if( sfi.iIcon >= 0 ) {
			hIcon = ::ImageList_GetIcon( system_image_list_, sfi.iIcon, ILD_NORMAL );
			if( hIcon ) {
				Icon = new TIcon;
				Icon->Handle = hIcon;
				int index = FileIconImageList->AddIcon( Icon );
				Icon->ReleaseHandle();
				delete Icon;
				::DestroyIcon(hIcon);
				if( index >= 0 ) {
					AddFileIconIndex( std::string("folder close"), index );
                }
			}
		}
//		::ImageList_Destroy( hSystemImageList );
	}

	// フォルダアイコン(オープン)
//	hSystemImageList = (HIMAGELIST)::SHGetFileInfo("C:\\WINDOWS", 0, &sfi, sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_ICON|SHGFI_OPENICON);
	if( system_image_list_ != 0 ) {
		SHGetFileInfo("C:\\WINDOWS", 0, &sfi, sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_ICON|SHGFI_OPENICON);
		if( sfi.iIcon >= 0 ) {
			hIcon = ::ImageList_GetIcon( system_image_list_, sfi.iIcon, ILD_NORMAL);
			if( hIcon ) {
				Icon = new TIcon;
				Icon->Handle = hIcon;
				int index = FileIconImageList->AddIcon( Icon );
				Icon->ReleaseHandle();
				delete Icon;
				::DestroyIcon(hIcon);
				if( index >= 0 ) {
					AddFileIconIndex( std::string("folder open"), index );
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
bool __fastcall TScriptDebuggerForm::AddFileIconIndex( const std::string& name, int index )
{
	typedef std::pair<std::map<std::string,int>::iterator,bool>	ret_t;
	ret_t ret = file_icon_index_.insert( std::map<std::string,int>::value_type( name, index ) );
	return ret.second;
}
//---------------------------------------------------------------------------
int __fastcall TScriptDebuggerForm::GetFileIconIndex( const std::string& name )
{
	std::map<std::string,int>::iterator i = file_icon_index_.find( name );
	if( i != file_icon_index_.end() ) {
		return i->second;
	}
	return -1;
}
//---------------------------------------------------------------------------
// 既にあったらそれを取得して、無かったら新規取得、追加する
int __fastcall TScriptDebuggerForm::GetWithAddFileIconIndex( const std::string& ext )
{
	std::map<std::string,int>::iterator i = file_icon_index_.find( ext );
	if( i != file_icon_index_.end() ) {
		return i->second;
	}
	int result = -1;
	if( system_image_list_ != 0 ) {
		SHFILEINFO sfi;
		std::string name( std::string("*") + ext );
		::SHGetFileInfo( name.c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX|SHGFI_ICON|SHGFI_USEFILEATTRIBUTES );
		if( sfi.iIcon >= 0 ) {
			HICON hIcon = ::ImageList_GetIcon( system_image_list_, sfi.iIcon, ILD_NORMAL);
			if( hIcon ) {
				TIcon* Icon = new TIcon;
				Icon->Handle = hIcon;
				result = FileIconImageList->AddIcon( Icon );
				Icon->ReleaseHandle();
				delete Icon;
				::DestroyIcon(hIcon);
				if( result >= 0 ) {
					AddFileIconIndex( ext, result );
				}
			}
		}
	}
	return result;
}
//---------------------------------------------------------------------------
void __fastcall TScriptDebuggerForm::OverWriteActionExecute(TObject *Sender)
{
	WriteProjectFile();
}
//---------------------------------------------------------------------------

