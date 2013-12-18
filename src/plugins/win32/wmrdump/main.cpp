//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
//---------------------------------------------------------------------------

/*
	Window.registerMessageReceiver で取得できたウィンドウメッセージを
	コンソールに垂れ流すテストプログラム。
*/


/*
	吉里吉里の Window クラスで生成されるウィンドウは HWND プロパティで
	そのウィンドウハンドルを得ることができるが、内部実装の都合上、
	このウィンドウハンドルはいくつかのプロパティの変更やメソッドの実行で
	作り直される可能性がある。
	これを検知するために Window.registerMessageReceiver でメッセージ受信関数を
	登録し、 TVP_WM_DETACH と TVP_WM_ATTACH を監視することができる。
*/


//---------------------------------------------------------------------------
// メッセージ受信関数
//---------------------------------------------------------------------------
static bool __stdcall MyReceiver
	(void *userdata, tTVPWindowMessage *Message)
{
	// この関数には(興味があるないに関わらず)非常に大量のメッセージが通過するため
	// 関係ないメッセージについてはパフォーマンスをあまり低下させないような実装が望まれる

	switch(Message->Msg)
	{
	case TVP_WM_DETACH:
		// ウィンドウハンドルが作り直されるか、あるいは
		// ウィンドウが破棄される直前にこのメッセージが来る。
		// 吉里吉里の Window クラスのウィンドウの子ウィンドウとして
		// ウィンドウを貼り付けた場合はこのタイミングでウィンドウを
		// 取り外さなければならない。
		// WPARAM は 1 ならばウィンドウが破棄されるとき、0 ならば
		// ウィンドウが作り直されるときである。
		// TVP_WM_DETACH と TVP_WM_ATTACH の間にも
		// メッセージが到着することがあるが無視すべき。
		if(Message->WParam == 1)
			TVPAddLog(TJS_W("TVP_WM_DETACH: closing"));
		else if(Message->WParam == 0)
			TVPAddLog(TJS_W("TVP_WM_DETACH: rebuilding"));
		break;

	case TVP_WM_ATTACH:
		// ウィンドウが作り直された後にこのメッセージが来る。
		// LPARAM には 新しいウィンドウハンドルが入っている。
		// 子ウィンドウをまた取り付ける場合はこのタイミングでまた取り付ける
		// ことができる。
		TVPAddLog(TJS_W("TVP_WM_ATTACH: new window handle ") +
			TJSInt32ToHex(Message->LParam, 8));
		break;

	default:
		// その他のメッセージ。
		TVPAddLog(TJS_W("Message: msg=") + TJSInt32ToHex(Message->Msg, 8) +
			TJS_W(", WParam=") + TJSInt32ToHex(Message->WParam, 8) +
			TJS_W(", LParam=") + TJSInt32ToHex(Message->LParam, 8));
		break;


	}


	return false;
		/* true を返すと 吉里吉里のウィンドウはそのメッセージに関知しなくなる。
		   TVP_WM_DETACH や TVP_WM_ATTACH への応答に関しては戻り値は無視される */
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// 開始関数
//---------------------------------------------------------------------------
class tWMRDumpStartFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		// *param[0] は Window クラスのオブジェクトである必要がある
		iTJSDispatch2 *obj = param[0]->AsObjectNoAddRef();
			// param[0] がオブジェクトを既に addref しているので
			// AsObject ではなく AsObjectNoAddRef を用いる
			// ( AsObjectNoAddRef は参照カウンタをいじらないので
			//  オブジェクトを使い終わっても Release する必要はないが
			//  すくなくとも「オブジェクトを必要としている期間中は」
			//  オブジェクトが存在し続けることが確実な場合のみに使用すべき )

		// registerMessageReceiver を呼ぶ
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmRegister;
		proc = (tTVInteger)reinterpret_cast<tjs_int>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)0;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL,
			NULL, 3, p, obj);

		// ウィンドウハンドルを取得してみたりする
		tTJSVariant val;
		HWND hwnd;
		obj->PropGet(0, TJS_W("HWND"), NULL, &val, obj);
		hwnd = reinterpret_cast<HWND>((tjs_int)(val));

		TVPAddLog(TJS_W("Window handle ") +
			TJSInt32ToHex(reinterpret_cast<tjs_int>(hwnd), 8));

		return TJS_S_OK;
	}
} * WMRDumpStartFunction;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// 終了関数
//---------------------------------------------------------------------------
class tWMRDumpStopFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		// *param[0] は Window クラスのオブジェクトである必要がある
		iTJSDispatch2 *obj = param[0]->AsObjectNoAddRef();

		// registerMessageReceiver を呼ぶ
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmUnregister;
		proc = (tTVInteger)reinterpret_cast<tjs_int>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)0;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL,
			NULL, 3, p, obj);

		return TJS_S_OK;
	}
} * WMRDumpStopFunction;
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// WMRDumpStartFunction, WMRDumpStopFunction の作成と登録
	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 まずオブジェクトを作成
	WMRDumpStartFunction = new tWMRDumpStartFunction();

	// 2 TestFunction を tTJSVariant 型に変換
	val = tTJSVariant(WMRDumpStartFunction);

	// 3 すでに val が TestFunction を保持しているので、WMRDumpStartFunction は
	//   Release する
	WMRDumpStartFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("wmrStartDump"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);

	// 1 まずオブジェクトを作成
	WMRDumpStopFunction = new tWMRDumpStopFunction();

	// 2 TestFunction を tTJSVariant 型に変換
	val = tTJSVariant(WMRDumpStopFunction);

	// 3 すでに val が TestFunction を保持しているので、WMRDumpStopFunction は
	//   Release する
	WMRDumpStopFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("wmrStopDump"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);


	// - global を Release する
	global->Release();

	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	TVPAddLog(TVPPluginGlobalRefCount);
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
	TVPAddLog(TVPPluginGlobalRefCount);
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	// TJS のグローバルオブジェクトに登録した関数を削除する

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
			TJS_W("wmrStartDump"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("wmrStopDump"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
			// 登録した関数が複数ある場合は これを繰り返す
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

