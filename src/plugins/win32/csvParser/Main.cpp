#include <windows.h>
#include "tp_stub.h"
#include <stdio.h>
#include <string>

using namespace std;

/**
 * ログ出力用
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

//---------------------------------------------------------------------------

// Array クラスメンバ
static iTJSDispatch2 *ArrayClearMethod   = NULL;   // Array.clear

// -----------------------------------------------------------------

class IFile {
public:
	virtual ~IFile() {};
	virtual bool addNextLine(ttstr &str) = 0;
};

class IFileStorage : public IFile {

	IStream *in;
	char buf[8192];
	ULONG pos;
	ULONG len;
	bool eofFlag;
	int codepage;
	
public:
	IFileStorage(tTJSVariantString *filename, int codepage) : codepage(codepage) {
		in = TVPCreateIStream(filename, TJS_BS_READ);
		if(!in) {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + *filename).c_str());
		}
		pos = 0;
		len = 0;
		eofFlag = false;
	}

	~IFileStorage() {
		if (in) {
			in->Release();
			in = NULL;
		}
	}
 #ifdef getc
  #undef getc
 #endif
	int getc() {
		if (pos < len) {
			return buf[pos++];
		} else {
			if (!in || eofFlag) {
				return EOF;
			} else {
				pos = 0;
				if (in->Read(buf, sizeof buf, &len) == S_OK) {
					eofFlag = len < sizeof buf;
				} else {
					eofFlag = true;
					len = 0;
				}
				return getc();
			}
		}
	}

 #ifdef ungetc
  #undef ungetc
 #endif
	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= len && eofFlag;
	}

	/**
	 * 改行チェック
	 */
	bool endOfLine(int c) {
		bool eol = (c =='\r' || c == '\n');
		if (c == '\r'){
			c = getc();
			if (!eof() && c != '\n') {
				ungetc();
			}
		}
		return eol;
	}
	
	bool addNextLine(ttstr &str) {
		int c;
		string mbline;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			mbline += c;
		}
		int l = (int)mbline.length();
		if (l > 0 || c != EOF) {
			wchar_t *buf = new wchar_t[l + 1];
			l = MultiByteToWideChar(codepage, 0,
									mbline.data(),
									(int)mbline.length(),
									buf, l);
			buf[l] = '\0';
			str += buf;
			delete buf;
			return true;
		} else {
			return false;
		}
	}
};

class IFileStr : public IFile {

	ttstr dat;
	ULONG pos;

public:
	IFileStr(tTJSVariantString *str) {
		dat = *str;
		pos = 0;
	}

	int getc() {
		return pos < (ULONG)dat.length() ? dat[pos++] : EOF;
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= (ULONG)dat.length();
	}

	/**
	 * 改行チェック
	 */
	bool endOfLine(tjs_char c) {
		bool eol = (c =='\r' || c == '\n');
		if (c == '\r'){
			c = getc();
			if (!eof() && c != '\n') {
				ungetc();
			}
		}
		return eol;
	}

	bool addNextLine(ttstr &str) {
		int l = 0;
		int c;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			str += c;
			l++;
		}
		if (l > 0 || c != EOF) {
			return true;
		} else {
			return false;
		}
	}
};

// -----------------------------------------------------------------

static void
addMember(iTJSDispatch2 *dispatch, const tjs_char *name, iTJSDispatch2 *member)
{
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		name, // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&var, // 登録する値
		dispatch // コンテキスト
		);
}

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGet(TJS_IGNOREPROP,
									 name,
									 NULL,
									 &val,
									 dispatch))) {
		ttstr msg = TJS_W("can't get member:");
		msg += name;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
}

static bool
isValidMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	return dispatch->IsValid(TJS_IGNOREPROP,
							 name,
							 NULL,
							 dispatch) == TJS_S_TRUE;
}

static void
delMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	dispatch->DeleteMember(
		0, // フラグ ( 0 でよい )
		name, // メンバ名
		NULL, // ヒント
		dispatch // コンテキスト
		);
}

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_CSVParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * CSVParser
 */
class NI_CSVParser : public tTJSNativeInstance // ネイティブインスタンス
{
protected:
	iTJSDispatch2 *target;
	IFile *file;
	tjs_int32 lineNo;

	// 区切り文字
	tjs_char separator;

	// 改行文字
	ttstr newline;
	
	// 行情報(ワイドキャラで処理する)
	ttstr line;
	
	bool addline() {
		return file->addNextLine(line);
	}
	
	// 文字さがし
	int find(ttstr &line, tjs_char ch, int start) {
		int i;
		for (i=start; i < line.length(); i++) {
			if (ch == line[i]) {
				return i;
			}
		}
		return i;
	}

	// 分割処理
	void split(iTJSDispatch2 *fields) {

		ttstr fld;
		int i, j;
		tjs_int cnt = 0;
	
		if (line.length() == 0) {
			return;
		}
		i = 0;
		do {
			if (i < line.length() && line[i] == '"') {
				++i;
				fld = L"";
				j = i;
				do {
					for (;j < line.length(); j++){
						if (line[j] == '"' && line[++j] != '"'){
							int k = find(line, separator, j);
							if (k > line.length()) {
								k = line.length();
							}
							for (k -= j; k-- > 0;)
								fld += line[j++];
							goto next;
						}
						fld += line[j];
					}
					// 改行追加処理
					fld += newline;
				} while (addline());
			} else {
				j = find(line, separator, i);
				if (j > line.length()) {
					j = line.length();
				}
				fld = ttstr(line.c_str() + i, j-i);
			}
		next:
			{
				// 登録
				tTJSVariant var(fld);
				fields->PropSetByNum(TJS_MEMBERENSURE, cnt++, &var, fields);
			}
			i = j + 1;
		} while (j < line.length());
	}
	
public:

	/**
	 * コンストラクタ
	 */
	NI_CSVParser() {
		target = NULL;
		file = NULL;
		lineNo = 0;
		separator = ',';
		newline = L"\r\n";
	}

	~NI_CSVParser() {
		clear();
	}

	/**
	 * TJS コンストラクタ
	 * @param numparams パラメータ数
	 * @param param
	 * @param tjs_obj this オブジェクト
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		if (numparams > 0) {
			target = param[0]->AsObject();
			if (numparams > 1) {
				separator = (tjs_int)*param[1];
				if (numparams > 2) {
					newline = *param[2];
				}
			}
		}
		return S_OK;
	}

	/**
	 * ファイルクローズ処理
	 */
	void clear() {
		if (file) {
			delete file;
			file = NULL;
		}
	}
	
	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		clear();
		if (target) {
			target->Release();
			target = NULL;
		}
	}

	/**
	 * パーサの初期化処理
	 */
	void init(tTJSVariantString *text) {
		clear();
		file = new IFileStr(text);
		lineNo = 0;
	}

	/**
	 * 初期化処理
	 */
	void initStorage(tTJSVariantString *filename, bool utf8=false) {
		clear();
		file = new IFileStorage(filename, utf8 ? CP_UTF8 : CP_ACP);
		lineNo = 0;
	}


	// 1行読み出し
	bool getNextLine(tTJSVariant *result = NULL) {
		bool ret = false;
		if (file) {
			line = L"";
			if (addline()) {
				lineNo++;
				iTJSDispatch2 *fields = TJSCreateArrayObject();
				split(fields);
				if (result) {
					*result = tTJSVariant(fields,fields);
				}
				fields->Release();
				ret = true;
			} else {
				clear();
			}
		}
		return ret;
	}
	
	/**
	 * 現在の行番号の取得
	 * @return 行番号
	 */
	tjs_int32 getLineNumber() {
		return lineNo;
	}
	
	/**
	 * パースの実行
	 */
	void parse(iTJSDispatch2 *objthis) {
		iTJSDispatch2 *target = this->target ? this->target : objthis;
		if (file && isValidMember(target, L"doLine")) {
			iTJSDispatch2 *method = getMember(target, L"doLine");
			tTJSVariant result;
			while (getNextLine(&result)) {
				tTJSVariant var2 = tTJSVariant(lineNo);
				tTJSVariant *vars[2];
				vars[0] = &result;
				vars[1] = &var2;
				method->FuncCall(0, NULL, NULL, NULL, 2, vars, target);
			}
			clear();
			method->Release();
		}
	}

};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_CSVParser()
{
	return new NI_CSVParser();
}

static iTJSDispatch2 * Create_NC_CSVParser()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("CSVParser"), Create_NI_CSVParser);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/CSVParser)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_CSVParser,
			/*TJS class name*/CSVParser)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/CSVParser)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/init)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->init(param[0]->AsStringNoAddRef());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/init)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/initStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/initStorage)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			_this->getNextLine(result);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parse)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams > 0) {
				_this->init(param[0]->AsStringNoAddRef());
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parse)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parseStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams > 0) {
				_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parseStorage)

		TJS_BEGIN_NATIVE_PROP_DECL(currentLineNumber)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_CSVParser);
				*result = _this->getLineNumber();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentLineNumber)

	TJS_END_NATIVE_MEMBERS

	// 定数の登録

	/*
	 * この関数は classobj を返します。
	 */
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

#ifndef CSVPARSER_NO_V2LINK
//---------------------------------------------------------------------------

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" __declspec(dllexport) HRESULT __stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	
	if (global) {

		// Arary クラスメンバー取得
		{
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("Array"), &varScripts);
			iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
			// メンバ取得
			ArrayClearMethod = getMember(dispatch, TJS_W("clear"));
		}

		addMember(global, L"CSVParser", Create_NC_CSVParser());
		global->Release();
	}
			
	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で
	// 管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、
	// プラグインは解放できないと言うことになる。

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if (global)	{
		delMember(global, L"CSVParser");
		if (ArrayClearMethod) {
			ArrayClearMethod->Release();
			ArrayClearMethod = NULL;
		}
		global->Release();
	}

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}

#endif // CSVPARSER_NO_V2LINK
