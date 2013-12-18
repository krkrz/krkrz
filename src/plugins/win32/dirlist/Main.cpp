//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// 指定されたディレクトリ内のファイルの一覧を得る関数
//---------------------------------------------------------------------------
class tGetDirListFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(membername) return TJS_E_MEMBERNOTFOUND;

		// 引数 : ディレクトリ
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr dir(*param[0]);

		if(dir.GetLastChar() != TJS_W('/'))
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));

		// OSネイティブな表現に変換
		dir = TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);

		// Array クラスのオブジェクトを作成
		iTJSDispatch2 * array;

		{
			tTJSVariant result;
			TVPExecuteExpression(TJS_W("[]"), &result);
			// なにか TJS スクリプトで出来そうなことをC++でやるのが面倒ならば
			// このように TJS 式を実行してしまうのが手っ取り早い
			array = result.AsObject();
		}

		try
		{
			//
			char nfile[MAX_PATH + 1];
			char ndir[MAX_PATH + 1];
			char nwildcard[MAX_PATH + 1];

			// dir を ndir に変換
			int dir_narrow_len = dir.GetNarrowStrLen();
			if(dir_narrow_len >= MAX_PATH - 3)
				TVPThrowExceptionMessage(TJS_W("Too long directory name."));

			dir.ToNarrowStr(ndir, MAX_PATH);

			// FindFirstFile を使ってファイルを列挙
			strcpy(nwildcard, ndir);
			strcat(nwildcard, "*.*");

			WIN32_FIND_DATA data;
			HANDLE handle = FindFirstFile(nwildcard, &data);
			if(handle != INVALID_HANDLE_VALUE)
			{
				tjs_int count = 0;
				do
				{
					strcpy(nfile, ndir);
					strcat(nfile, data.cFileName);

					if(GetFileAttributes(nfile) & FILE_ATTRIBUTE_DIRECTORY)
					{
						// ディレクトリの場合は最後に / をつける
						strcpy(nfile, data.cFileName);
						strcat(nfile, "/");
					}
					else
					{
						// 普通のファイルの場合はそのまま
						strcpy(nfile, data.cFileName);
					}

					// 配列に追加する
					tTJSVariant val((ttstr)(nfile));
					array->PropSetByNum(0, count++, &val, array);

				} while(FindNextFile(handle, &data));
				FindClose(handle);
			}
			else
			{
				TVPThrowExceptionMessage(TJS_W("Directory not found."));
			}

			if(result)
				*result = tTJSVariant(array, array);
		}
		catch(...)
		{
			array->Release();
			throw;
		}

		array->Release();

		// 戻る
		return TJS_S_OK;
	}
} * GetDirListFunction;
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

	// GetDirListFunction の作成と登録
	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 まずオブジェクトを作成
	GetDirListFunction = new tGetDirListFunction();

	// 2 GetDirListFunction を tTJSVariant 型に変換
	val = tTJSVariant(GetDirListFunction);

	// 3 すでに val が GetDirListFunction を保持しているので、GetDirListFunction は
	//   Release する
	GetDirListFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("getDirList"), // メンバ名 ( かならず TJS_W( ) で囲む )
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

	// TJS のグローバルオブジェクトに登録した getDirList 関数を削除する

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
			TJS_W("getDirList"), // メンバ名
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



