#include "layerExCairo.hpp"
#include "ncbind/ncbind.hpp"

// ----------------------------------- クラスの登録

NCB_GET_INSTANCE_HOOK(layerExCairo)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		if (obj) obj->reset();						// メソッドを呼ぶ前に必ず呼ばれる
		return (_obj = obj);						//< デストラクタで使用したい場合はプライベート変数に保存
	}

	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
		if (_obj) _obj->redraw();					// メソッドを呼んだ後に必ず呼ばれる
	}

private:
	ClassT *_obj;
}; // 実体は class 定義なので ; を忘れないでね

// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(layerExCairo, Layer) {
}

// ----------------------------------- 起動・開放処理

/**
 * 登録処理前
 */
void PreRegistCallback()
{
}

/**
 * 登録処理後
 */
void PostRegistCallback()
{
}

/**
 * 開放処理前
 */
void PreUnregistCallback()
{
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
}

NCB_PRE_REGIST_CALLBACK(   PreRegistCallback);
NCB_POST_REGIST_CALLBACK(  PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK( PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
