//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
//---------------------------------------------------------------------------


/*
  プラグインでの例外処理

  プラグインの RTL (Run Time Library) と吉里吉里本体の RTL は異なるため、特に
  例外を扱う場合は注しなければならない点が何点かあります。

  プラグインで例外を再送する場合は、例外を吸収してしまわない限り問題はありま
  せん。
  つまり、以下のようなコードはダメです (メモリリークを引き起こします)。

	try {
		// 例外を発生するような処理
		...
	}
	catch(...) {
		// throw; しない
	}

  例外を再送するのは問題ありません。

	try {
		// 例外を発生するような処理
		...
	}
	catch(...) {
		// 例外を再送
		throw;
	}

  以下のように、例外の型を判断することはできません。

	try {
		// 例外を発生するような処理
		...
	}
	catch(const eTJS &e) { // そもそも eTJS が tp_stub に無いのでエラー
		...
	}

  プラグインから本体に例外を投げるには TVPThrowExceptionMessage を使用する
  ことができます。

	TVPThrowExceptionMessage(TJS_W("文字列を指定してください"));

  プラグインで本体の例外を捕捉し、メッセージを取得したい場合は TVPDoTryBlock
  を使用します。本体で発生した例外を、プラグイン側で捕捉して吸収したり、
  例外のメッセージを取得するにはこの方法を使用するしかありません。

  このプラグインでは、この TVPDoTryBlock の使用方法を説明します。
*/



//---------------------------------------------------------------------------
// テスト用コールバック関数
//---------------------------------------------------------------------------
static void TJS_USERENTRY TryBlock1(void * data)
{
	// try ブロックです。
	// この関数内で例外が発生した場合は、catchblock が呼び出されます。
	// data には、TVPDoTryBlock で指定した data 引数の値が渡されます。

	// 明らかに例外を発生させるスクリプトを実行
	TVPExecuteScript(TJS_W("var a = 3, b = 0; var c = a \\ b;"), NULL);
}
//---------------------------------------------------------------------------
static bool TJS_USERENTRY CatchBlock1(void * data, const tTVPExceptionDesc & desc)
{
	// catch ブロックです。
	// tryblock 内で例外が発生した場合はこの関数が呼び出されます。
	// desc.type は 例外の型を表し、TJS 由来の例外の場合は "eTJS" 、それ以外の場合は
	// "unknown" になっています。
	// desc.message は、例外のメッセージです。メッセージが無い場合は空文字列となっています。
	// この関数が true を返すと、例外は再送されます。
	// false を返すと、例外は吸収されます。
	// data には、TVPDoTryBlock で指定した data 引数の値が渡されます。

	TVPAddLog(TJS_W("exception type:") + desc.type + TJS_W(", message:") + desc.message);
	return *(bool*)data;
}
//---------------------------------------------------------------------------
static void TJS_USERENTRY FinallyBlock1(void * data)
{
	// finally ブロックです。
	// try ブロック内で例外が発生してもしなくても、最終的には実行される関数です。
	// data には、TVPDoTryBlock で指定した data 引数の値が渡されます。
	TVPAddLog(TJS_W("finally executed."));
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// テスト関数
//---------------------------------------------------------------------------
class tTestExceptionFunction1 : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * TestExceptionFunction1;
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTestExceptionFunction1::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	// 最初の引数 = 例外を再送するか
	bool rethrow = false;
	if(numparams >= 1)
		rethrow = (bool)(tjs_int)*param[0];

	TVPDoTryBlock(TryBlock1, CatchBlock1, FinallyBlock1, (void *)&rethrow);
	/*
		TVPDoTryBlock には、４つの引数を渡します。
		第一引数は tryblock 関数へのポインタ、第二引数は catchblock 関数への
		ポインタ、第三引数は finallyblock 関数へのポインタです。
		第四引数は、各関数へ渡される data 引数の値です。
		各関数の説明は上記の各関数をご覧ください。
	*/

	return TJS_S_OK;
}
//---------------------------------------------------------------------------



/*
 参考

 実際の TVPDoTryBlock は以下のようになっています。

void TVPDoTryBlock(
	tTVPTryBlockFunction tryblock,
	tTVPCatchBlockFunction catchblock,
	tTVPFinallyBlockFunction finallyblock,
	void *data)
{
	try
	{
		tryblock(data);
	}
	catch(const eTJS & e)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("eTJS");
		desc.message = e.GetMessage();
		if(catchblock(data, desc)) throw;
		return;
	}
	catch(...)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("unknown");
		if(catchblock(data, desc)) throw;
		return;
	}
	if(finallyblock) finallyblock(data);
}
*/






//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// AverageFunction の作成と登録
	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();



	//-----------------------------------------------------------------------
	// 1 まずオブジェクトを作成
	TestExceptionFunction1 = new tTestExceptionFunction1();

	// 2 AverageFunction を tTJSVariant 型に変換
	val = tTJSVariant(TestExceptionFunction1);

	// 3 すでに val が AverageFunction を保持しているので、AverageFunction は
	//   Release する
	TestExceptionFunction1->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("testException1"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------




	// - global を Release する
	global->Release();

	// もし、登録する関数が複数ある場合は 1 ～ 4 を繰り返す


	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


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
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	// TJS のグローバルオブジェクトに登録した average 関数などを削除する

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("testException1"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------
