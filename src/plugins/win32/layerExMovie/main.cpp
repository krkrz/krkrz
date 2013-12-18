#pragma comment(lib, "strmiids.lib")
#include "layerExMovie.hpp"
#include "ncbind/ncbind.hpp"

// ----------------------------------- クラスの登録

NCB_GET_INSTANCE_HOOK(layerExMovie)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		return obj;
	}

	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(layerExMovie, Layer) {
	NCB_METHOD(openMovie);
	NCB_METHOD(startMovie);
	NCB_METHOD(stopMovie);
	NCB_METHOD(isPlayingMovie);
}

// ----------------------------------- 起動・開放処理

static bool coInitialized;

/**
 * 登録処理前
 */
void PreRegistCallback()
{
	coInitialized = SUCCEEDED(CoInitialize(0));
	if (coInitialized) {
		TVPAddLog("初期化成功");
	} else {
		TVPAddLog("初期化失敗");
	}
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
	if (coInitialized) {
		CoUninitialize();
	}
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
