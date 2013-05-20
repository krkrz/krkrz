#include <windows.h>
#include "tp_stub.h"
#include "MenuItemIntf.h"

static std::map<HWND,iTJSDispatch2*> MENU_LIST;
static void AddMenuDispatch( HWND hWnd, iTJSDispatch2* menu ) {
	MENU_LIST.insert( std::map<HWND, iTJSDispatch2*>::value_type( hWnd, menu ) );
}
static iTJSDispatch2* GetMenuDispatch( HWND hWnd ) {
	std::map<HWND, iTJSDispatch2*>::iterator i = MENU_LIST.find( hWnd );
	if( i != MENU_LIST.end() ) {
		return i->second;
	}
	return NULL;
}
static void DelMenuDispatch( HWND hWnd ) {
	MENU_LIST.erase(hWnd);
}
/**
 * メニューの中から既に存在しなくなったWindowについているメニューオブジェクトを削除する
 */
static void UpdateMenuList() {
	std::map<HWND, iTJSDispatch2*>::iterator i = MENU_LIST.begin();
	for( ; i != MENU_LIST.end(); ) {
		HWND hWnd = i->first;
		BOOL exist = ::IsWindow( hWnd );
		if( exist == 0 ) {
			// 既になくなったWindow
			std::map<HWND, iTJSDispatch2*>::iterator target = i;
			i++;
			iTJSDispatch2* menu = target->second;
			MENU_LIST.erase( target );
			menu->Release();
			TVPDeleteAcceleratorKeyTable( hWnd );
		} else {
			i++;
		}
	}
}
class WindowMenuProperty : public tTJSDispatch {
	tjs_error TJS_INTF_METHOD PropGet( tjs_uint32 flag,	const tjs_char * membername, tjs_uint32 *hint, tTJSVariant *result,	iTJSDispatch2 *objthis ) {
		tTJSVariant var;
		if( TJS_FAILED(objthis->PropGet(0, TJS_W("HWND"), NULL, &var, objthis)) ) {
			return TJS_E_INVALIDOBJECT;
		}
		HWND hWnd = (HWND)(tjs_int64)var;
		iTJSDispatch2* menu = GetMenuDispatch( hWnd );
		if( menu == NULL ) {
			UpdateMenuList();
			menu = TVPCreateMenuItemObject(objthis);
			AddMenuDispatch( hWnd, menu );
		}
		*result = tTJSVariant(menu, menu);
		return TJS_S_OK;
	}
	tjs_error TJS_INTF_METHOD PropSet( tjs_uint32 flag, const tjs_char *membername,	tjs_uint32 *hint, const tTJSVariant *param,	iTJSDispatch2 *objthis ) {
		return TJS_E_ACCESSDENYED;
	}
} *gWindowMenuProperty;
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved) {
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" __declspec(dllexport) HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	{
		gWindowMenuProperty = new WindowMenuProperty();
		val = tTJSVariant(gWindowMenuProperty);
		gWindowMenuProperty->Release();
		tTJSVariant win;
		if( TJS_SUCCEEDED(global->PropGet(0,TJS_W("Window"),NULL,&win,global)) ) {
			iTJSDispatch2* obj = win.AsObjectNoAddRef();
			obj->PropSet(TJS_MEMBERENSURE,TJS_W("menu"),NULL,&val,obj);
		}

		//-----------------------------------------------------------------------
		iTJSDispatch2 * tjsclass = TVPCreateNativeClass_MenuItem();
		val = tTJSVariant(tjsclass);
		tjsclass->Release();
		global->PropSet( TJS_MEMBERENSURE, TJS_W("MenuItem"), NULL, &val, global );
		//-----------------------------------------------------------------------
		
	}

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
extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	/*
		ただし、クラスの場合、厳密に「オブジェクトが使用中である」ということを
		知るすべがありません。基本的には、Plugins.unlink によるプラグインの解放は
		危険であると考えてください (いったん Plugins.link でリンクしたら、最後ま
		でプラグインを解放せず、プログラム終了と同時に自動的に解放させるのが吉)。
	*/

	// プロパティ開放
	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// メニューは解放されないはずなので、明示的には解放しない

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember( 0, TJS_W("MenuItem"), NULL, global );
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------
