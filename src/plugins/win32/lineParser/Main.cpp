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

// -----------------------------------------------------------------

class IFile {
public:
	virtual ~IFile() {};
	virtual bool getNextLine(ttstr &str) = 0;
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
	
	bool getNextLine(ttstr &str) {
		int c;
		string mbline;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			mbline += c;
		}
		int l = mbline.length();
		if (l > 0 || c != EOF) {
			wchar_t *buf = new wchar_t[l + 1];
			l = MultiByteToWideChar(codepage, 0,
									mbline.data(),
									mbline.length(),
									buf, l);
			buf[l] = '\0';
			str = buf;
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
		return pos < dat.length() ? dat[pos++] : EOF;
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= dat.length();
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

	bool getNextLine(ttstr &str) {
		str = L"";
		int c;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			str += c;
		}
		if (str.length() > 0 || c != EOF) {
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

#define TJS_NATIVE_CLASSID_NAME ClassID_LineParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * LineParser
 */
class NI_LineParser : public tTJSNativeInstance // ネイティブインスタンス
{
protected:
	iTJSDispatch2 *target;
	IFile *file;
	tjs_int32 lineNo;
	
public:

	/**
	 * コンストラクタ
	 */
	NI_LineParser() {
		target = NULL;
		file = NULL;
		lineNo = 0;
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
		if (target) {
			target->Release();
			target = NULL;
		}
		clear();
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

	/**
	 * 行の取得
	 * @param line 行
	 * @return 成功するとtrue
	 */
	bool getNextLine(ttstr &line) {
		if (file) {
			if (file->getNextLine(line)) {
				lineNo++;
				return true;
			} else {
				clear();
			}
		}
		return false;
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
			ttstr line;
			while (getNextLine(line)) {
				tTJSVariant var1 = tTJSVariant(line);
				tTJSVariant var2 = tTJSVariant(lineNo);
				tTJSVariant *vars[2];
				vars[0] = &var1;
				vars[1] = &var2;
				method->FuncCall(0, NULL, NULL, NULL, 2, vars, target);
			}
			method->Release();
			clear();
		}
	}

};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_LineParser()
{
	return new NI_LineParser();
}

static iTJSDispatch2 * Create_NC_LineParser()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("LineParser"), Create_NI_LineParser);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/LineParser)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_LineParser,
			/*TJS class name*/LineParser)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/LineParser)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/init)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->init(param[0]->AsStringNoAddRef());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/init)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/initStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/initStorage)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			ttstr ret;
			if (_this->getNextLine(ret) && result){
				*result = ret;
			}
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parse)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams > 0) {
				_this->init(param[0]->AsStringNoAddRef());
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parse)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parseStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
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
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_LineParser);
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

//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	
	if (global) {
		addMember(global, L"LineParser", Create_NC_LineParser());
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
extern "C" HRESULT _stdcall V2Unlink()
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
		delMember(global, L"LineParser");
		global->Release();
	}

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
