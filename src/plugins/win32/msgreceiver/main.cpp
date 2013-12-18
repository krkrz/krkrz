//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <string>
//---------------------------------------------------------------------------

// 実行ファイルがある場所に WHND を保存する
void
storeHWND(HWND hwnd)
{
	tTJSVariant varScripts;
	TVPExecuteExpression(TJS_W("System.exeName"), &varScripts);
	ttstr path = varScripts;
	path += ".HWND";
	IStream *stream = TVPCreateIStream(path, TJS_BS_WRITE);
	if (stream != NULL) {
		char buf[100];
		DWORD len;
		_snprintf(buf, sizeof buf, "%d", (int)hwnd);
		stream->Write(buf, strlen(buf), &len);
		stream->Release();
	}
}

//---------------------------------------------------------------------------
// メッセージ受信関数
//---------------------------------------------------------------------------
static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message)
{
	iTJSDispatch2 *obj = (iTJSDispatch2 *)userdata;

	switch(Message->Msg) {
	case TVP_WM_DETACH:
		break;
	case TVP_WM_ATTACH:
		storeHWND((HWND)Message->LParam);
		break;
	case WM_COPYDATA:
		// コピペ指示
		{
			HWND             hwndFrom = (HWND)Message->WParam;
			COPYDATASTRUCT*  pcds     = (COPYDATASTRUCT*)Message->LParam;
			std::string str((const char*)pcds->lpData, pcds->cbData);
			tTJSVariant msg = str.c_str();
			tTJSVariant *p[] = {&msg};
			obj->FuncCall(0, TJS_W("onCopyData"), NULL, NULL, 1, p, obj);
		}
		break;
	default:
		// その他は無視
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
class tWMRStartFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;
		// ウインドウを指定
		iTJSDispatch2 *obj = param[0]->AsObjectNoAddRef();
		// registerMessageReceiver を呼ぶ
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmRegister;
		proc = (tTVInteger)reinterpret_cast<tjs_int>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)obj;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL,
					  NULL, 3, p, obj);

		// ウィンドウハンドルを取得して記録
		tTJSVariant val;
		obj->PropGet(0, TJS_W("HWND"), NULL, &val, obj);
		storeHWND(reinterpret_cast<HWND>((tjs_int)(val)));
		
		return TJS_S_OK;
	}
} * WMRStartFunction;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// 終了関数
//---------------------------------------------------------------------------
class tWMRStopFunction : public tTJSDispatch
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
} * WMRStopFunction;
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

	// WMRStartFunction, WMRStopFunction の作成と登録
	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 まずオブジェクトを作成
	WMRStartFunction = new tWMRStartFunction();

	// 2 TestFunction を tTJSVariant 型に変換
	val = tTJSVariant(WMRStartFunction);

	// 3 すでに val が TestFunction を保持しているので、WMRStartFunction は
	//   Release する
	WMRStartFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("wmrStart"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);

	// 1 まずオブジェクトを作成
	WMRStopFunction = new tWMRStopFunction();

	// 2 TestFunction を tTJSVariant 型に変換
	val = tTJSVariant(WMRStopFunction);

	// 3 すでに val が TestFunction を保持しているので、WMRStopFunction は
	//   Release する
	WMRStopFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("wmrStop"), // メンバ名 ( かならず TJS_W( ) で囲む )
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
			TJS_W("wmrStart"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("wmrStop"), // メンバ名
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

