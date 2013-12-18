#include <stdio.h>
#include "ncbind/ncbind.hpp"
#include <vector>

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdaux.h>
#include <sqthread.h>
#include <sqcont.h>
#include "sqtjsobj.h"

// squirrel 上での TJS2のグローバル空間の参照名
#define KIRIKIRI_GLOBAL L"krkr"
// TJS2 上での squirrel のグローバル空間の参照名
#define SQUIRREL_GLOBAL L"sqglobal"

// コピーライト表記
static const char *copyright =
"\n------ Squirrel Copyright START ------\n"
"Copyright (c) 2003-2009 Alberto Demichelis\n"
"------ Squirrel Copyright END ------";

static HSQUIRRELVM vm = NULL;

#include <tchar.h>

/**
 * ログ出力用 for squirrel
 */
static void printFunc(HSQUIRRELVM v, const SQChar* format, ...)
{
	va_list args;
	va_start(args, format);
	TCHAR msg[1024];
	_vsntprintf_s(msg, 1024, _TRUNCATE, format, args);
	TCHAR *p = msg;
	int c;
	int n = 0;
	while ((c = *(p+n))) {
		if (c == '\n') {
			TVPAddLog(ttstr(p,n));
			p += n;
			n = 0;
			p++;
		} else {
			n++;
		}
	}
	if (n > 0) {
		TVPAddLog(ttstr(p));
	}
	va_end(args);
}

//---------------------------------------------------------------------------
// squirrel -> TJS2 ブリッジ用
//---------------------------------------------------------------------------

extern void sq_pushvariant(HSQUIRRELVM v, tTJSVariant &variant);
extern SQRESULT sq_getvariant(HSQUIRRELVM v, int idx, tTJSVariant *result);
extern SQRESULT sq_getvariant(sqobject::ObjectInfo &obj, tTJSVariant *result);
extern void SQEXCEPTION(HSQUIRRELVM v);

/**
 * Squirrel の グローバル空間に登録処理を行う
 */
static void registerglobal(HSQUIRRELVM v, const SQChar *name, tTJSVariant &variant)
{
	sq_pushroottable(v);
	sq_pushstring(v, name, -1);
	sq_pushvariant(v, variant);
	sq_createslot(v, -3); 
	sq_pop(v, 1);
}

/**
 * Squirrel の グローバル空間から削除処理を行う
 */
static void unregisterglobal(HSQUIRRELVM v, const SQChar *name)
{
	sq_pushroottable(v);
	sq_pushstring(v, name, -1);
	sq_deleteslot(v, -2, SQFalse); 
	sq_pop(v, 1);
}

//---------------------------------------------------------------------------

#include "../json/Writer.hpp"

// squirrel 予約語一覧
static const char *reservedKeys[] = 
{
	"break",
	"case",
	"catch",
	"class",
	"clone",
	"continue",
	"const",
	"default",
	"delegate",
	"delete",
	"else",
	"enum",
	"extends",
	"for",
	"function",
	"if",
	"in",
	"local",
	"null",
	"resume",
	"return",
	"switch",
	"this",
	"throw",
	"try",
	"typeof" ,
	"while",
	"parent",
	"yield",
	"constructor",
	"vargc",
	"vargv" ,
	"instanceof",
	"true",
	"false",
	"static",
	NULL
};

#include <set>
using namespace std;

// 予約語一覧
set<ttstr> reserved;

// 予約語の登録
static void initReserved()
{
	const char **p = reservedKeys;
	while (*p != NULL) {
		reserved.insert(ttstr(*p));
		p++;
	}
}

// 予約語かどうか
static bool isReserved(const tjs_char *key)
{
	return reserved.find(ttstr(key)) != reserved.end();
}

static bool isal(int c) {
	return (c >= 'a' && c <= 'z' ||
			c >= 'A' && c <= 'Z' ||
			c == '_');
}


static bool isaln(int c) {
	return (c >= '0' && c <= '9' ||
			c >= 'a' && c <= 'z' ||
			c >= 'A' && c <= 'Z' ||
			c == '_');
}

static bool
isSimpleString(const tjs_char *str)
{
	const tjs_char *p = str;
	if (!isal(*p)) {
		return false;
	}
	while (*p != '\0') {
		if (!isaln(*p)) {
			return false;
		}
		p++;
	}
	return true;
}

static void
quoteString(const tjs_char *str, IWriter *writer)
{
	if (str) {
		writer->write((tjs_char)'"');
		const tjs_char *p = str;
		int ch;
		while ((ch = *p++)) {
			if (ch == '"') {
				writer->write(L"\\\"");
			} else if (ch == '\\') {
				writer->write(L"\\\\");
			} else if (ch == 0x08) {
			  writer->write(L"\\b");
			} else if (ch == 0x0c) {
			  writer->write(L"\\f");
			} else if (ch == 0x0a) {
			  writer->write(L"\\n");
			} else if (ch == 0x0d) {
			  writer->write(L"\\r");
			} else if (ch == 0x09) {
			  writer->write(L"\\t");
			} else if (ch < 0x20) {
			  wchar_t buf[256];
			  swprintf(buf, 255, L"\\u%04x", ch);
			  writer->write(buf);
			} else {
				writer->write((tjs_char)ch);
			}
		}
		writer->write((tjs_char)'"');
	} else {
		writer->write(L"\"\"");
	}
}

static void getVariantString(tTJSVariant &var, IWriter *writer);

/**
 * 辞書の内容表示用の呼び出しロジック
 */
class DictMemberDispCaller : public tTJSDispatch /** EnumMembers 用 */
{
protected:
	IWriter *writer;
	bool first;
public:
	DictMemberDispCaller(IWriter *writer) : writer(writer) { first = true; };
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER)) {
				if (first) {
					first = false;
				} else {
					writer->write((tjs_char)',');
//					writer->newline();
				}
				const tjs_char *name = param[0]->GetString();
				if (!isReserved(name) && isSimpleString(name)) {
					writer->write(name);
					writer->write((tjs_char)'=');
				} else {
					writer->write((tjs_char)'[');
					quoteString(name, writer);
					writer->write(L"]=");
				}
				getVariantString(*param[2], writer);
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
};

static void getDictString(iTJSDispatch2 *dict, IWriter *writer)
{
	writer->write((tjs_char)'{');
	//writer->addIndent();
	DictMemberDispCaller *caller = new DictMemberDispCaller(writer);
	tTJSVariantClosure closure(caller);
	dict->EnumMembers(TJS_IGNOREPROP, &closure, dict);
	caller->Release();
	//writer->delIndent();
	writer->write((tjs_char)'}');
}

// Array クラスメンバ
static iTJSDispatch2 *ArrayCountProp   = NULL;   // Array.count

// 配列の数を取得
tjs_int getArrayCount(iTJSDispatch2 *array) {
	tTJSVariant result;
	if (TJS_SUCCEEDED(ArrayCountProp->PropGet(0, NULL, NULL, &result, array))) {
		return (tjs_int)result.AsInteger();
	}
	return 0;
}

// 配列から文字列を取得
void getArrayString(iTJSDispatch2 *array, int idx, ttstr &store)
{
	tTJSVariant result;
	if (array->PropGetByNum(TJS_IGNOREPROP, idx, &result, array) == TJS_S_OK) {
		store = result.GetString();
	}
}

static void getArrayString(iTJSDispatch2 *array, IWriter *writer)
{
	writer->write((tjs_char)'[');
	//writer->addIndent();
	tjs_int count = getArrayCount(array);
	for (tjs_int i=0; i<count; i++) {
		if (i != 0) {
			writer->write((tjs_char)',');
			writer->newline();
		}
		tTJSVariant result;
		if (array->PropGetByNum(TJS_IGNOREPROP, i, &result, array) == TJS_S_OK) {
			getVariantString(result, writer);
		}
	}
	//writer->delIndent();
	writer->write((tjs_char)']');
}

static void
getVariantString(tTJSVariant &var, IWriter *writer)
{
	switch(var.Type()) {

	case tvtVoid:
		writer->write(L"null");
		break;
		
	case tvtObject:
		{
			iTJSDispatch2 *obj = var.AsObjectNoAddRef();
			if (obj == NULL) {
				writer->write(L"null");
			} else if (obj->IsInstanceOf(TJS_IGNOREPROP,NULL,NULL,L"Array",obj) == TJS_S_TRUE) {
				getArrayString(obj, writer);
			} else {
				getDictString(obj, writer);
			}
		}
		break;
		
	case tvtString:
		quoteString(var.GetString(), writer);
		break;

	case tvtInteger:
		writer->write((tTVInteger)var);
		break;

	case tvtReal:
		writer->write((tTVReal)var);
		break;

	default:
		writer->write(L"null");
		break;
	};
}

//---------------------------------------------------------------------------

/**
 * Scripts クラスへの Squirrel 実行メソッドの追加
 */
class ScriptsSquirrel {

public:
	ScriptsSquirrel(){};

	/**
	 * squirrel スクリプトの読み込み
	 * @param script スクリプト
	 * @throw コンパイルに失敗
	 * @return 読み込まれたスクリプト
	 */
	static tjs_error TJS_INTF_METHOD load(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		if (SQ_SUCCEEDED(sq_compilebuffer(vm, param[0]->GetString(), param[0]->AsString()->GetLength(), L"TEXT", SQTrue))) {
			sq_getvariant(vm, -1, result);
			sq_pop(vm, 1);
		} else {
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}


	/**
	 * squirrel スクリプトの実行
	 * @param script スクリプト
	 * @param ... 引数
	 * @throw コンパイルに失敗
	 * @throw 実行に失敗
	 * @return 実行結果
	 */
	static tjs_error TJS_INTF_METHOD exec(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		if (SQ_SUCCEEDED(sq_compilebuffer(vm, param[0]->GetString(), param[0]->AsString()->GetLength(), L"TEXT", SQTrue))) {
			sq_pushroottable(vm); // this
			for (int i=1;i<numparams;i++) {	// 引数
				sq_pushvariant(vm, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(vm, numparams, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(vm, -1, result);
					sq_pop(vm, 1);
				}
				sq_pop(vm, 1);
			} else {
				sq_pop(vm, 1);
				SQEXCEPTION(vm);
			}
		} else {
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}

	/**
	 * squirrel スクリプトのファイルからの読み込み
	 * @param filename ファイル名
	 * @throw ファイルの読み込みに失敗
	 * @return 読み込まれたスクリプト
	 */
	static tjs_error TJS_INTF_METHOD loadStorage(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		if (SQ_SUCCEEDED(sqstd_loadfile(vm, param[0]->GetString(), SQTrue))) {
			sq_getvariant(vm, -1, result);
			sq_pop(vm, 1);
		} else {
			SQEXCEPTION(vm);
		} 
		return TJS_S_OK;
	}
	
	/**
	 * squirrel スクリプトのファイルからの実行
	 * @param filename ファイル名
	 * @param ... 引数
	 * @throw ファイルの読み込みに失敗
	 * @throw 呼び出しに失敗
	 * @return 実行結果
	 */
	static tjs_error TJS_INTF_METHOD execStorage(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		if (SQ_SUCCEEDED(sqstd_loadfile(vm, param[0]->GetString(), SQTrue))) {
			sq_pushroottable(vm); // this
			for (int i=1;i<numparams;i++) {	// 引数
				sq_pushvariant(vm, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(vm, numparams, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(vm, -1, result);
					sq_pop(vm, 1);
				}
				sq_pop(vm, 1);
			} else {
				sq_pop(vm, 1);
				SQEXCEPTION(vm);
			}
		} else {
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}

	/**
	 * Squirrel スクリプトのスレッド実行。
	 * @param text スクリプトが格納された文字列
	 * @param ... 引数
	 * @throw コンパイルに失敗
	 * @throw スレッド生成に失敗
	 * @return Threadオブジェクト
	 */
	static tjs_error TJS_INTF_METHOD fork(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		sq_pushroottable(vm); // root
		sq_pushstring(vm, SQTHREADNAME, -1);
		if (SQ_SUCCEEDED(sq_get(vm,-2))) { // class
			sq_pushroottable(vm); // 引数:self(root)
			sq_pushnull(vm); // delegate
			if (SQ_SUCCEEDED(sq_compilebuffer(vm, param[0]->GetString(), param[0]->AsString()->GetLength(), L"TEXT", SQTrue))) {
				for (int i=1;i<numparams;i++) {	// 引数
					sq_pushvariant(vm, *param[i]);
				}
				if (SQ_SUCCEEDED(sq_call(vm, numparams+2, SQTrue, SQTrue))) { // コンストラクタ呼び出し
					sq_getvariant(vm, -1, result);
					sq_pop(vm, 3); // thread, class, root
				} else {
					sq_pop(vm, 2); // class, root
					SQEXCEPTION(vm);
				}
			} else {
				sq_pop(vm, 4); // delegate, self, class, root
				SQEXCEPTION(vm);
			}
		} else {
			sq_pop(vm,1); // root
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}

	/**
	 * Squirrel スクリプトのファイルからのスレッド実行。
	 * @param filename スクリプトが格納されたファイル
	 * @param ... 引数
	 * @throw スレッド生成に失敗
	 * @return Threadオブジェクト
	 */
	static tjs_error TJS_INTF_METHOD forkStorage(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		sq_pushroottable(vm); // root
		sq_pushstring(vm, SQTHREADNAME, -1);
		if (SQ_SUCCEEDED(sq_get(vm,-2))) { // class
			sq_pushroottable(vm); // 引数 self(root)
			sq_pushnull(vm);      // 引数 delegate
			sq_pushstring(vm, param[0]->GetString(), -1); // 引数 func
			for (int i=1;i<numparams;i++) {	// 引数
				sq_pushvariant(vm, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(vm, numparams+2, SQTrue, SQTrue))) { // コンストラクタ呼び出し
				sq_getvariant(vm, -1, result);
				sq_pop(vm, 3); // thread, class, root
			} else {
				sq_pop(vm, 2); // class, root
				SQEXCEPTION(vm);
			}
		} else {
			sq_pop(vm,1); // root
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}

	/**
	 * スレッド終了コールバックを呼び出す
	 */
	static void onThreadDone(sqobject::ObjectInfo th, void *userData) {
		tTJSVariant *method = (tTJSVariant*)userData;
		th.push(vm);

		// TJSの例外回避呼び出し処理用
		class FuncInfo {
			
		public:
			/**
			 * コンストラクタ
			 * @param method 呼び出し対象メソッド
			 * @param HSQUIRRELVM v 引数参照用VM
			 * @param argc PUSHしてある引数の数
			 */
			FuncInfo(tTJSVariant &method, HSQUIRRELVM v, int argc) : method(method), argc(argc), args(NULL) {
				// 引数生成
				if (argc > 0) {
					args = new tTJSVariant*[(size_t)argc];
					for (tjs_int i=0;i<argc;i++) {
						args[i] = new tTJSVariant();
						sq_getvariant(v, i-argc, args[i]);
					}
					sq_pop(v, argc);
				}
			}
			
			// デストラクタ
			~FuncInfo() {
				// 引数破棄
				if (args) {
					for (int i=0;i<argc;i++) {
						delete args[i];
					}
					delete[] args;
				}
			}
			
			void exec() {
				TVPDoTryBlock(TryExec, Catch, Finally, (void *)this);
			}
			
		private:
			
			void _TryExec() {
				tjs_error error;
				if (TJS_SUCCEEDED(error = method.AsObjectClosureNoAddRef().FuncCall(0, NULL, NULL, NULL, argc, args, NULL))) {
				}
			}
			
			static void TJS_USERENTRY TryExec(void * data) {
				FuncInfo *info = (FuncInfo*)data;
				info->_TryExec();
			}
			
			static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
				FuncInfo *info = (FuncInfo*)data;
				TVPAddLog(desc.message.c_str());
				return false;
			}
			
			static void TJS_USERENTRY Finally(void * data) {
			}

		private:
			tjs_int argc;
			tTJSVariant **args;
			tTJSVariant method;
		} func(*method, vm, 1);
		func.exec();
	}
	
	/**
	 * squirrelスレッドの実行
	 */
	static tjs_error TJS_INTF_METHOD drive(tTJSVariant *result,
										   tjs_int numparams,
										   tTJSVariant **param,
										   iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		sqobject::Thread::update((int)*param[0]);
		sqobject::beforeContinuous();
		int count;
		if (numparams > 1) {
			count = sqobject::Thread::main(onThreadDone, param[1]);
		} else {
			count = sqobject::Thread::main();
		}
		sqobject::afterContinuous();
		if (result) {
			*result = count;
		}
		return TJS_S_OK;
	}

	/**
	 * トリガ実行
	 * @param name トリガ名
	 */
	static tjs_error TJS_INTF_METHOD trigger(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		sqobject::Thread::trigger(param[0]->GetString());
		return TJS_S_OK;
	}
	
	/**
	 * squirrel グローバルメソッドの呼び出し
	 * @param name メソッド名
	 * @param ... 引数
	 * @throw メソッドの取得に失敗
	 * @throw メソッドの呼び出しに失敗
	 * @return 実行結果
	 */
	static tjs_error TJS_INTF_METHOD call(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		sq_pushroottable(vm);
		sq_pushstring(vm, param[0]->GetString(), -1);
		if (SQ_SUCCEEDED(sq_get(vm, -2))) { // ルートテーブルから関数を取得
			sq_pushroottable(vm); // this
			for (int i=1;i<numparams;i++) {	// 引数
				sq_pushvariant(vm, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(vm, numparams, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(vm, -1, result);
					sq_pop(vm, 1); // result
				}
				sq_pop(vm, 2); // func, root
			} else {
				sq_pop(vm, 2); // func, root
				SQEXCEPTION(vm);
			}
		} else {
			sq_pop(vm, 1); // root
			SQEXCEPTION(vm);
		}
		return S_OK;
	}

	
	/**
	 * squirrel スクリプトのコンパイル処理
	 * @param text スクリプトが格納された文字列
	 * @store store バイナリクロージャ格納先ファイル
	 * @throw コンパイルに失敗
	 * @throw 書き出しに失敗
	 * @return エラー文字列または void
	 */
	static tjs_error TJS_INTF_METHOD compile(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		SQInteger endian = numparams >= 3 ? (int)*param[3] : 0;
		if (SQ_SUCCEEDED(sq_compilebuffer(vm, param[0]->GetString(), param[0]->AsString()->GetLength(), L"TEXT", SQTrue))) {
			if (SQ_SUCCEEDED(sqstd_writeclosuretofile(vm, param[1]->GetString(), endian))) {
				sq_pop(vm, 1);
			} else {
				sq_pop(vm, 1);
				SQEXCEPTION(vm);
			} 
		} else {
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}

	/**
	 * Squirrel コードのコンパイル処理
	 * @param コンパイル元ファイル
	 * @store バイナリクロージャ格納先ファイル
	 * @throw ファイルの読み込みに失敗
	 * @throw 書き出しに失敗
	 * @return エラー文字列または void
	 */
	static tjs_error TJS_INTF_METHOD compileStorage(tTJSVariant *result,
													tjs_int numparams,
													tTJSVariant **param,
													iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		SQInteger endian = numparams >= 3 ? (int)*param[3] : 0;
		if (SQ_SUCCEEDED(sqstd_loadfile(vm, param[0]->GetString(), SQTrue))) {
			if (SQ_SUCCEEDED(sqstd_writeclosuretofile(vm, param[1]->GetString(), endian))) {
				sq_pop(vm, 1);
			} else {
				sq_pop(vm, 1);
				SQEXCEPTION(vm);
			} 
		} else {
			SQEXCEPTION(vm);
		} 
		return TJS_S_OK;
	}
	
	/**
	 * squirrel 形式でのオブジェクトの保存
	 * @param filename ファイル名
	 * @param obj オブジェクト
	 * @param utf true なら UTF-8 で出力
	 * @param newline 改行コード 0:CRLF 1:LF
	 * @return 実行結果
	 */
	static tjs_error TJS_INTF_METHOD save(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		IFileWriter writer(param[0]->GetString(),
						   numparams > 2 ? (int)*param[2] != 0: false,
						   numparams > 3 ? (int)*param[3] : 0
						   );
		writer.write(L"return ");
		getVariantString(*param[1], &writer);
		return TJS_S_OK;
	}

	/**
	 * squirrel 形式で文字列化
	 * @param obj オブジェクト
	 * @param newline 改行コード 0:CRLF 1:LF
	 * @return 実行結果
	 */
	static tjs_error TJS_INTF_METHOD toString(tTJSVariant *result,
											  tjs_int numparams,
											  tTJSVariant **param,
											  iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		if (result) {
			IStringWriter writer(numparams > 1 ? (int)*param[1] : 0);
			getVariantString(*param[0], &writer);
			*result = writer.buf;
		}
		return TJS_S_OK;
	}

	/**
	 * squirrel の名前空間に登録
	 * @param name オブジェクト名
	 * @param obj オブジェクト
	 */
	static tjs_error TJS_INTF_METHOD registerSQ(tTJSVariant *result,
											  tjs_int numparams,
											  tTJSVariant **param,
											  iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		if (numparams > 1) {
			registerglobal(vm, param[0]->GetString(), *param[1]);
		} else {
			tTJSVariant var;
			TVPExecuteExpression(param[0]->GetString(), &var);
			registerglobal(vm, param[0]->GetString(), var);
		}
		return TJS_S_OK;
	}

	/**
	 * squirrel の名前空間から削除
	 * @param name オブジェクト名
	 * @param obj オブジェクト
	 */
	static tjs_error TJS_INTF_METHOD unregisterSQ(tTJSVariant *result,
												tjs_int numparams,
												tTJSVariant **param,
												iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		unregisterglobal(vm, param[0]->GetString());
		return TJS_S_OK;
	}

	/**
	 * @return squirrelのスレッド数を返す
	 */
	static tjs_error TJS_INTF_METHOD getThreadCount(tTJSVariant *result,
													  tjs_int numparams,
													  tTJSVariant **param,
													  iTJSDispatch2 *objthis) {
		if (result) {
			*result = sqobject::Thread::getThreadCount();
		}
		return TJS_S_OK;
	}

	/**
	 * Squirrel用比較
	 * 2つのオブジェクトを squirrel 的に比較します
	 * @param obj1 オブジェクトその1
	 * @param obj2 オブジェクトその2
	 * @return 比較結果 >0: obj1>obj2 ==0:obj1==obj2 <0:obj1<obj2
	 */
	static tjs_error TJS_INTF_METHOD compare(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		if (result) {
			sq_pushvariant(vm, *param[0]);
			sq_pushvariant(vm, *param[1]);
			*result = sq_cmp(vm);
			sq_pop(vm, 2);
		}
		return TJS_S_OK;
	}

};

NCB_ATTACH_CLASS(ScriptsSquirrel, Scripts) {

	RawCallback("loadSQ",        &ScriptsSquirrel::load,        TJS_STATICMEMBER);
	RawCallback("loadStorageSQ", &ScriptsSquirrel::loadStorage, TJS_STATICMEMBER);
	RawCallback("execSQ",        &ScriptsSquirrel::exec,        TJS_STATICMEMBER);
	RawCallback("execStorageSQ", &ScriptsSquirrel::execStorage, TJS_STATICMEMBER);
	RawCallback("forkSQ",        &ScriptsSquirrel::fork,        TJS_STATICMEMBER);
	RawCallback("forkStorageSQ", &ScriptsSquirrel::forkStorage, TJS_STATICMEMBER);
	RawCallback("driveSQ",       &ScriptsSquirrel::drive,       TJS_STATICMEMBER);
	RawCallback("triggerSQ",     &ScriptsSquirrel::trigger,     TJS_STATICMEMBER);
	RawCallback("callSQ",        &ScriptsSquirrel::call,        TJS_STATICMEMBER);
	RawCallback("saveSQ",        &ScriptsSquirrel::save,        TJS_STATICMEMBER);
	RawCallback("toSQString",    &ScriptsSquirrel::toString,    TJS_STATICMEMBER);
	RawCallback("registerSQ",       &ScriptsSquirrel::registerSQ,      TJS_STATICMEMBER);
	RawCallback("unregisterSQ",     &ScriptsSquirrel::unregisterSQ,    TJS_STATICMEMBER);
	RawCallback("compileSQ",        &ScriptsSquirrel::compile,        TJS_STATICMEMBER);
	RawCallback("compileStorageSQ", &ScriptsSquirrel::compileStorage, TJS_STATICMEMBER);
	RawCallback("getThreadCountSQ", &ScriptsSquirrel::getThreadCount, TJS_STATICMEMBER);
	RawCallback("compareSQ", &ScriptsSquirrel::compare, TJS_STATICMEMBER);
};

/**
 * Squirrel 用 Continuous ハンドラクラス
 * 直接 squirrel のメソッドを呼び出す Continuous ハンドラを生成する
 */
class SQContinuous : public tTVPContinuousEventCallbackIntf {

protected:
	// 内部オブジェクト参照保持用
	HSQOBJECT obj;

public:
	/**
	 * コンストラクタ
	 * @param name 名称
	 */
	SQContinuous(const tjs_char *name) {
		sq_resetobject(&obj);          // ハンドルを初期化
		sq_pushroottable(vm);
		sq_pushstring(vm, name, -1);
		if (SQ_SUCCEEDED(sq_get(vm, -2))) { // ルートテーブルから関数を取得
			sq_getstackobj(vm, -1, &obj); // 位置-1からオブジェクトハンドルを得る
			sq_addref(vm, &obj);          // オブジェクトへの参照を追加
			sq_pop(vm, 2);
		} else {
			sq_pop(vm, 1);
			SQEXCEPTION(vm);
		}
	}

	/**
	 * デストラクタ
	 */
	~SQContinuous() {
		stop();
		sq_release(vm, &obj);  
	}

	/**
	 * 呼び出し開始
	 */
	void start() {
		stop();
		TVPAddContinuousEventHook(this);
	}

	/**
	 * 呼び出し停止
	 */
	void stop() {
		TVPRemoveContinuousEventHook(this);
	}

	/**
	 * Continuous コールバック
	 * 吉里吉里が暇なときに常に呼ばれる
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick) {
		sq_pushobject(vm, obj);
		sq_pushroottable(vm);
		sq_pushinteger(vm, (SQInteger)tick); // 切り捨て御免
		if (SQ_SUCCEEDED(sq_call(vm, 2, SQFalse, SQTrue))) {
			sq_pop(vm, 1);
		} else {
			stop();
			sq_pop(vm, 1);
			SQEXCEPTION(vm);
		}
	}
};

NCB_REGISTER_CLASS(SQContinuous) {
	NCB_CONSTRUCTOR((const tjs_char *));
	NCB_METHOD(start);
	NCB_METHOD(stop);
};


/**
 * 直接 squirrel のグローバルファンクションを呼び出すことができるラッパー
 * 変換処理を介さずに処理できるので効率が良い
 */
class SQFunction {

protected:
	// 内部オブジェクト参照保持用
	HSQOBJECT obj;

public:
	/**
	 * コンストラクタ
	 * @param name 名称
	 */
	SQFunction(const tjs_char *name) {
		sq_resetobject(&obj);          // ハンドルを初期化
		sq_pushroottable(vm);
		sq_pushstring(vm, name, -1);
		if (SQ_SUCCEEDED(sq_get(vm, -2))) { // ルートテーブルから関数を取得
			sq_getstackobj(vm, -1, &obj); // 位置-1からオブジェクトハンドルを得る
			sq_addref(vm, &obj);          // オブジェクトへの参照を追加
			sq_pop(vm, 2);
		} else {
			sq_pop(vm, 1);
			SQEXCEPTION(vm);
		}
	}

	/**
	 * デストラクタ
	 */
	~SQFunction() {
		sq_release(vm, &obj);  
	}

	/**
	 * ファンクションとして呼び出し
	 */
	static tjs_error TJS_INTF_METHOD call(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  SQFunction *objthis) {
		sq_pushobject(vm, objthis->obj); // func
		sq_pushroottable(vm); // this
		for (int i=0;i<numparams;i++) { // 引数
			sq_pushvariant(vm, *param[i]);
		}
		if (SQ_SUCCEEDED(sq_call(vm, numparams + 1, result ? SQTrue:SQFalse, SQTrue))) {
			if (result) {
				sq_getvariant(vm, -1, result);
				sq_pop(vm, 1); // result
			}
			sq_pop(vm, 1); // func
		} else {
			sq_pop(vm, 1); // func
			SQEXCEPTION(vm);
		}
		return TJS_S_OK;
	}
};

NCB_REGISTER_CLASS(SQFunction) {
	NCB_CONSTRUCTOR((const tjs_char *));
	RawCallback("call", &SQFunction::call, 0);
};

//---------------------------------------------------------------------------

/**
 * 登録処理前
 */
static void PreRegistCallback()
{
	// Copyright 表示
	TVPAddImportantLog(ttstr(copyright));
	// 予約語登録
	initReserved();
	// squirrel 登録
	vm = sqobject::init();
	
	// 出力用
	sq_setprintfunc(vm, printFunc);

	// その他の基本ライブラリの登録
	sq_pushroottable(vm);
	sqstd_register_iolib(vm);
	sqstd_register_bloblib(vm);
	sq_pop(vm, 1);
	
	// オブジェクト機構初期化
	sqobject::Object::registerClass();
	sqobject::Thread::registerClass();
	sqobject::Thread::registerGlobal();

	TJSObject::init(vm);
	
	// スレッド機構初期化
	sqobject::Thread::init();
 	sqobject::registerContinuous();
	
	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global) {
		// 吉里吉里のグローバルに Squirrel のグローバルを登録する
		{
			tTJSVariant result;
			sq_pushroottable(vm);
			sq_getvariant(vm, -1, &result);
			sq_pop(vm, 1);
			global->PropSet(TJS_MEMBERENSURE, SQUIRREL_GLOBAL, NULL, &result, global);
		}
		// Squirrel の グローバルに吉里吉里の グローバルを登録する
		tTJSVariant var(global, global);
		registerglobal(vm, KIRIKIRI_GLOBAL, var);
		global->Release();
	}

	// Array.count を取得
	{
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Array"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		tTJSVariant val;
		if (TJS_FAILED(dispatch->PropGet(TJS_IGNOREPROP,
										 TJS_W("count"),
										 NULL,
										 &val,
										 dispatch))) {
			TVPThrowExceptionMessage(L"can't get Array.count");
		}
		ArrayCountProp = val.AsObject();
	}
}

/**
 * 開放処理前
 */
static void PostUnregistCallback()
{
	if (ArrayCountProp) {
		ArrayCountProp->Release();
		ArrayCountProp = NULL;
	}
	
	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global)	{
		// Squirrel の グローバルから吉里吉里のグローバルを削除
		unregisterglobal(vm, KIRIKIRI_GLOBAL);
		// squirrel の global への登録を解除
		global->DeleteMember(0, SQUIRREL_GLOBAL, NULL, global);
		global->Release();
	}

	// スレッド系停止
	sqobject::doneContinuous();
	sqobject::Thread::done();

	TJSObject::done(vm);

	// squirrel 終了
	sqobject::done();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
