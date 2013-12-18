//---------------------------------------------------------------------------
/*
	このソースは、吉里吉里で使用可能な関数を増やすプラグインのテストをかねた
	プラグインの作り方の説明。
	このソースファイルの構造は以下の通り。

	・TJSから使用可能にするための関数群
	  関数群は tTJSDispatch クラスの FuncCall をオーバーライドすることで実装
	  する。ここでは４つの関数、つまり４つのクラスを実装している。

	  tAverageFunction - 引数の平均を求める
	    基本的な関数の構造、TJSからの引数やTJSに渡す値を実数に変換する方法な
	    ど

	  tSumFunction - 引数の合計を求める
	    型の判断、整数への変換、整数からの変換など

	  tMessageBoxFunction - メッセージボックスを表示する
	    TJSからの文字列をANSI文字列(ナロー文字列;ゼロ終結char*文字列)への変換
	    など

	  tGetUserNameFunction - 現在のユーザ名を取得する
	    TJSへ文字列を返すなど


	・吉里吉里とのインターフェース
	  吉里吉里やその周辺ツールが呼ぶプラグイン内の関数群
	  上記の関数を登録・削除する部分を説明している。また、吉里吉里設定ツール
	  にオプションの説明の情報を受け渡す方法について説明している。


	プラグインで例外処理を行う際の注意点については exceptest も参照のこと。
*/


/*
  VC++ 6.0 での、この形式でのプラグインの作り方
  1. 新規作成で Win32 Dynamic-Link Library のプロジェクトを「空の DLL
     プロジェクト」として作成する。
  2. 吉里吉里のソース配布パッケージの /base/win32/plugin_kit/ の
     tp_stub.cpp tp_stub.h をプロジェクトフォルダにコピーするか
     インクルード・パスを通すなどし、
     [プロジェクト|プロジェクトへ追加|ファイル]で追加する
  3. .def ファイル (このファイルの最後のほうを参照) を作成してプロジェク
     トに追加する
  4. このソースなどを参考にしてプログラムを書く
     (吉里吉里から呼ばれる V2Link と V2Unlink は必須)
*/


/*
  bcc はどうも DLL 作成時、
  ・インライン関数内
  ・ワイド文字列リテラル
  ・ワイド文字列リテラルの開始アドレスが奇数(そもそも奇数に配置するってのが
    アレ)
  ・２バイト系文字が含まれる？
  という条件がそろうと正常にその文字列を格納できないみたいなので
  注意。とりあえずはインライン関数(class { } 内に直接書いた関数を含む) で
  ワイド文字列リテラルを使うのをさければ問題はないようである。
  bcc を使わなければあまり気にしなくてよい
*/




/*
	作成した DLL を使うには :


	KAG の場合は loadplugin タグ、吉里吉里/TJS の場合は Plugins.link メソッ
	ドを使う。

@loadplugin module=basetest.dll

	あるいは

Plugins.link("basetest.dll");

	すると、average という関数が使用可能になるので、KAG 中の TJS式や TJS スク
	リプト中で

@emb exp="average(0, 1, 2)"

	あるいは

var avg = average(0, 1, 2);

	などとして使う。



	average (引数すべての平均を求める)
	sum (引数の合計を求める; 引数が実数を含めば実数の結果、それ以外は整数の結果)
	messagebox (引数を一つとり、メッセージボックスを表示する関数)
	getUserName (現在のユーザ名を返す関数)
	
*/





//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"

#include <lmcons.h>
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// テスト関数
//---------------------------------------------------------------------------
class tAverageFunction : public tTJSDispatch
{
	// 吉里吉里で利用可能な関数を簡単に作成するには、
	// tTJSDispatch を継承させ、FuncCall を実装する。

	// 例として、与えられた引数の平均 (実数で) を返す関数を実装する。

	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * AverageFunction;
	// テスト関数を保持
	// iTJSDispatch2 の派生物 ( tTJSDispatch や tAverageFunction もそう) はなる
	// べくヒープに確保 ( つまり、new で確保する ) ようにすべき。
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tAverageFunction::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	// flag       : 呼び出しフラグ
	//              無視してかまわない
	// membername : メンバ名
	//              無視してかまわないが、一応
	if(membername) return TJS_E_MEMBERNOTFOUND;
	//              と記述しておくと良い
	// hint       : メンバ名ヒント
	//              本来はmembernameのハッシュ値だが、無視してかまわない
	// result     : 結果格納先
	//              NULL の場合は結果が必要ない時
	// numparams  : パラメータの数
	// param      : パラメータ
	// objthis    : コンテキスト
	//              クラスメソッドではないので無視してかまわない

	// if(numparams != 3) return TJS_E_BADPARAMCOUNT;
		// 引数の数が合わない場合は TJS_E_BADPARAMCOUNT を返す
		// たとえば、引数が 3 でなければならない場合は 上記のコメントをはずす

	if(numparams == 0) return TJS_E_BADPARAMCOUNT;
		// この場合は引数が 0 であってはならないのでこのようにする


	// 平均を計算
	tTVReal sum = 0.0;  // tTVReal は double の typedef で tTJSVariant で使用
	for(tjs_int i = 0; i < numparams; i++)  // すべての引数に対して
	{
		sum += (tTVReal)*param[i];
			// *param[i] などは tTJSVariant 型。この型はいわゆるバリアント型
			// で、整数、実数、オブジェクト、オクテット列、文字列などを表す事
			// ができ、各型との変換関数も定義されている。
	}
	sum /= (tTVReal)numparams;


	if(result)
	{
		// result は NULL になりうるので、関数で値を返すときは
		// かならす result が NULL でないかチェックする。
		// result が NULL であってもエラーにはしないほうがよい
		*result = sum;
	}

	return TJS_S_OK;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// テスト関数 もういっこ
//---------------------------------------------------------------------------
class tSumFunction : public tTJSDispatch
{
	// 例として、与えられた引数の合計を求める関数
	// (引数の中に実数があれば答えも実数、そうでなければ答えは整数にする)

	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * SumFunction;
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tSumFunction::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if(membername) return TJS_E_MEMBERNOTFOUND;

	// 引数が 0 の場合の処理
	if(numparams == 0)
	{
		// 引数の数が0なので
		if(result) *result = (tTVInteger)0;
			// tTVInteger にキャストされた整数をtTJSVariant型に渡すと
			// 答えは整数になる
		return TJS_S_OK;
	}

	// 引数の中に実数があるかどうかをチェック
	bool real_result = false; // 実数で答えを返す場合は true にする
	tjs_int i;
	for(i = 0; i < numparams; i++)
	{
		if(param[i]->Type() == tvtReal)  // 実数ならば
		{
			real_result = true;
			break;
		}
	}

	// 合計を求めて答えを返す
	if(real_result)
	{
		// 答えは実数で返す
		tTVReal sum = 0.0;
		for(i = 0; i < numparams; i++)
		{
			sum += (tTVReal)*param[i];
		}

		if(result) *result = sum;
	}
	else
	{
		// 答えは整数で返す
		tTVInteger sum = 0;
		for(i = 0; i < numparams; i++)
		{
			sum += (tTVInteger)*param[i];
		}

		if(result) *result = sum;
	}

	return TJS_S_OK;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// テスト関数 さらにもういっこ
//---------------------------------------------------------------------------
class tMessageBoxFunction : public tTJSDispatch
{
	// 例として、簡単なメッセージボックスを表示する関数
	// tTJSVariant から ナロー文字列 ( ふつうの char * 文字列 ) を取り出す例
	// 本体側に例外を通じてエラー文字列を返す例

	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * MessageBoxFunction;
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tMessageBoxFunction::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{


	if(membername) return TJS_E_MEMBERNOTFOUND;

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	// *param[0] ( tTJSVariant 型 ) からナロー文字列を取り出す
	// TJS2 はワイド文字 (UNICODE) で文字列を管理しているので
	// ナロー文字列に変換するのはすこし手間がかかる。
	// ここでいうナロー文字列とはふつうの char * 文字列のことで、
	// 日本語のばあいは Shift-JIS コードになる。

	// *param[0] はこの時点で文字列型でなければならないが
	// 文字列型でなかった場合は例外を発生させる
	if(param[0]->Type() != tvtString)
		TVPThrowExceptionMessage(TJS_W("文字列を指定してください"));
			// TVPThrowExceptionMessage は例外を発生させる
			// この関数を呼ぶと、もうここへは戻らないので注意

	ttstr str(*param[0]); // いったん ttstr 型に代入する
	tjs_int narrow_len = str.GetNarrowStrLen(); // ナロー文字列変換後のサイズ
	if(narrow_len == -1)
		TVPThrowExceptionMessage(TJS_W("文字列の変換に失敗しました"));
			// GetNarrowStrLen は文字列中に変換できない文字を見つけると
			// -1 を返す

	char *narrow_str = new char [narrow_len + 1];
			// narrow_len はnullターミネータの文字数を含んでいないので、
			// 文字列バッファを確保する場合には +1 を忘れないように
	try
	{
		// いつどこで例外が発生するかわからないので
		// できるかぎり 例外保護を行う

		// 文字列を変換
		str.ToNarrowStr(narrow_str, narrow_len);

		// メッセージボックスを表示
		MessageBox(TVPGetApplicationWindowHandle(), narrow_str,
			"メッセージ", MB_OK);

			// TVPGetApplicationWindowHandle は吉里吉里のウィンドウのすべて
			// の owner であるウィンドウハンドルを返す。ダイアログボックス
			// を表示するときなどに親ウィンドウが必要ならば、このウィンドウ
			// ハンドルを用いるとよい
	}
	catch(...)
	{
		// 例外が発生した場合
		delete [] narrow_str; // 確保したバッファを解放
		throw; // 例外を再び投げる
	}

	delete [] narrow_str; // 確保したバッファを解放

	if(result) result->Clear(); // result->Clear() をすると void を返す

	return TJS_S_OK;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// テスト関数 さらにさらにもういっこ
//---------------------------------------------------------------------------
class tGetUserNameFunction : public tTJSDispatch
{
	// 例として、現在のユーザ名を返す関数
	// 関数から文字列を返す例

	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * GetUserNameFunction;
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tGetUserNameFunction::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if(membername) return TJS_E_MEMBERNOTFOUND;

	// GetUserName 関数を使用して、現在のユーザ名を返す。
	// GetUserName は NT 系 OS の場合 GetUserNameW という関数が
	// 使用できるので、もし NT 系 OS であればそちらを使用する。

	// OS が NT 系 OS かどうかを得る
	bool is_nt; // NT 系 OS の場合真にする

	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);
	is_nt = ovi.dwPlatformId == VER_PLATFORM_WIN32_NT;
	
	// GetUserNameW への関数ポインタを得る
	typedef BOOL (WINAPI *GetUserNameW_t)(LPWSTR lpBuffer, LPDWORD nSize);
	GetUserNameW_t pGetUserNameW = NULL; // UNICODE 版 API
	HMODULE advapi_handle = NULL; // ADVAPI32.DLL のモジュールハンドル

	if(is_nt) advapi_handle = LoadLibrary("advapi32.dll");
		// ADVAPI32.DLL を読みこむ

	try
	{
		if(advapi_handle)
		{
			// 関数ポインタを得る
			pGetUserNameW = (GetUserNameW_t)
				GetProcAddress(advapi_handle, "GetUserNameW");
			if(!pGetUserNameW)
			{
				FreeLibrary(advapi_handle);
				advapi_handle = NULL;
			}
		}

		// GetUserName を使用して現在のユーザ名を取得
		if(pGetUserNameW)
		{
			// UNICODE (ワイド文字)バージョン
			DWORD size;
			size = UNLEN + 1;
			wchar_t un[UNLEN + 1]; // ワイド文字列
			un[0] = 0;
			pGetUserNameW(un, &size);

			if(result) *result = un;
				// ゼロ終結ワイド文字列はそのまま結果に代入可能
				// (文字列用のバッファは新たに吉里吉里内部で確保されるので
				//  代入に使用した文字列バッファは解放してかまわない)
		}
		else
		{
			// ANSI (ナロー文字)バージョン
			DWORD size;
			size = UNLEN + 1;
			char un[UNLEN + 1]; // ワイド文字列
			un[0] = 0;
			GetUserNameA(un, &size);

			if(result) *result = un; // ゼロ終結ナロー文字列も代入可能
									// (自動的にワイド文字に内部で変換)
		}
	}
	catch(...)
	{
		if(advapi_handle) FreeLibrary(advapi_handle);
		throw;
	}

 	if(advapi_handle) FreeLibrary(advapi_handle);

	return TJS_S_OK;
}
//---------------------------------------------------------------------------






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
	AverageFunction = new tAverageFunction();

	// 2 AverageFunction を tTJSVariant 型に変換
	val = tTJSVariant(AverageFunction);

	// 3 すでに val が AverageFunction を保持しているので、AverageFunction は
	//   Release する
	AverageFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("average"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------



	//-----------------------------------------------------------------------
	// 1 まずオブジェクトを作成
	SumFunction = new tSumFunction();

	// 2 SumFunction を tTJSVariant 型に変換
	val = tTJSVariant(SumFunction);

	// 3 すでに val が SumFunction を保持しているので、SumFunction は
	//   Release する
	SumFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("sum"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------



	//-----------------------------------------------------------------------
	// 1 まずオブジェクトを作成
	MessageBoxFunction = new tMessageBoxFunction();

	// 2 MessageBoxFunction を tTJSVariant 型に変換
	val = tTJSVariant(MessageBoxFunction);

	// 3 すでに val が MessageBoxFunction を保持しているので、
	// MessageBoxFunction は Release する
	MessageBoxFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("messagebox"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------



	//-----------------------------------------------------------------------
	// 1 まずオブジェクトを作成
	GetUserNameFunction = new tGetUserNameFunction();

	// 2 GetUserNameFunction を tTJSVariant 型に変換
	val = tTJSVariant(GetUserNameFunction);

	// 3 すでに val が GetUserNameFunction を保持しているので、
	// GetUserNameFunction は Release する
	GetUserNameFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("getUserName"), // メンバ名 ( かならず TJS_W( ) で囲む )
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
			TJS_W("average"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("sum"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("messagebox"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("getUserName"), // メンバ名
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
static tjs_char option_message[] =
L"テストカテゴリ:テスト設定項目1;これはテスト用のオプション設定項目です。|"
L"testoption1|select,*default;デフォルトの値,1;選択可能な値1,2;選択可能な値2\n"
L"テストカテゴリ:テスト設定項目2;これはテスト用のオプション設定項目です。|"
L"testoption2|string(64),*デフォルトの文字列\n";
//---------------------------------------------------------------------------
extern "C" const wchar_t* _stdcall _export GetOptionDesc()
{
	// GetOptionDesc は、吉里吉里設定ツールに渡すオプションの情報を返すための
	// 関数。
	// 一行につき一つのオプションに関する情報を書く。
	// 書式は
	// カテゴリ:設定名;説明|オプション名|オプションとして設定可能な値
	// (区切り記号に注意)
	// 「オプション名」には '-' (ハイフン) はつけない。
	// 「オプションとして設定可能な値」は以下の書式である。
	// タイプ,値の値;簡単な説明[,値;値の簡単な説明 ...]
	// 「タイプ」は現バージョンでは select と string のみ使用可能。

	// 「タイプ」に select を指定すると、コンボボックスから値を選択することが
	// できるようになる。
	// 「値;値の簡単な説明」の先頭に * をつけるとそれがデフォルトのオプション
	// であることを示す。デフォルトのオプションは必ず一つは無くてはならない。
	// 「値」は実際に吉里吉里本体へのオプションとして渡されるものを、
	// 「値の簡単な説明」は人間が読んで理解できるものを書く。

	// 「タイプ」に string を指定すると、エディットボックスでユーザが任意の文
	// 字列を入力できるようになる。
	// 「タイプ」に string を指定する場合は、string(n),*デフォルト文字列 の
	// 形で n には最大文字数を記述する。0 で文字数制限はなくなるが、できれば
	// 制限をつけたほうがよい。
	// 「デフォルト文字列」にはデフォルトの文字列を記述する。string(n) のあと
	// の , (カンマ) も * (アステリスク) も省略できない。

	// 例は /msg/win32/option_desc_ja.txt にもある。


	return option_message;

	// 各オプション末や、最後の改行記号を忘れると大変なことになる。


	// 吉里吉里設定などは、そのプラグインに設定可能な項目があるかどうかを
	// プラグインDLLの
	// 先頭2KB以内に "(--has-option--)" という文字列があるかどうか
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// (２重引用符は含めず)
	// で判断するため、先頭2KB以内にこれを書かなければならない。
	// bcc の場合はリンカオプション /GC で /GC"(--has-option--)"
	// と記述し、VC++ の場合もリンカオプション /COMMENT で
	// /COMMENT:"(--has-option--)" と記述する。
	// BCB の IDE からは プロジェクトオプションの「リンカ(詳細)」の
	// 「イメージのコメント」に "(--has-option--)" と書く。
	// (２重引用符で囲まないとエラーになるかもしれない)

	// UPX などで圧縮するとここの部分が消されてしまうので、
	// ヘッダの未使用領域 (0x80～0x1ff の 00 で埋まっている所など) に
	// 無理矢理書くなどする。

	// 本体に渡されたオプションは TVPGetCommandLine 関数で取得できる。
	// tTJSVariant val;
	// if(TVPGetCommandLine(TJS_W("-testoption1"), &val))
	// {
	//      // ここに オプション "-testoption1" が渡されていたときの
	//      // 処理を書く。val にはその値が入っている。
	//      // /environ/win32/wuvorbis/WuVorbisMainUnit.cpp 等参照
	// }
	// TVPGetcommandLine の実行は現バージョンでは高価なのでできれば
	// この関数の呼び出しの結果はキャッシュした方がよい。

	// 吉里吉里設定は、プラグインを「吉里吉里本体と同じフォルダ」と
	// 「吉里吉里本体のと同じフォルダにある plugin というフォルダ」から
	// しか検索しない。従って設定可能なオプションを含むプラグインは
	// Releaser などでアーカイブ中に含めてはならない(もともとReleaserで
	// プラグインを中に含めることは推奨されていない)。

	// この GetOptionDesc 関数内では tp_stub に記述されているような
	// 各吉里吉里本体内の関数は利用できない。
}
//---------------------------------------------------------------------------
/*
	V2Link と V2Unlink は DLL からエクスポートされている必要がある。
	従って、.def ファイルを作成し

EXPORTS
	V2Link
	V2Unlink

	と記述してプロジェクトに追加する。

	GetOptionsDesc を使う場合はそれも追加する。
*/
//---------------------------------------------------------------------------

