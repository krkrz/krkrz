#ifndef __SQTJS_OBJ_H_
#define __SQTJS_OBJ_H_

#include <sqobjectclass.h>

/**
 * 吉里吉里オブジェクトを保持するsquirrelクラス
 */
class TJSObject : public sqobject::Object, iTJSNativeInstance {

public:
	/**
	 * 初期化用
	 */
	static void init(HSQUIRRELVM vm);

	/**
	 * 廃棄用
	 */
	static void done(HSQUIRRELVM vm);

	// ---------------------------------------------------------------

	/**
	 * call 処理用の口
	 * TJSインスタンスからsquirrelインスタンスのメソッドを直接呼び出す
	 */
	static tjs_error call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
						  tTJSVariant *result,
						  tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	/**
	 * missing 処理用の口
	 * TJSインスタンスにメンバが存在しなかった場合は squirrelインスタンスを参照する
	 */
	static tjs_error missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);

	// ---------------------------------------------------------------

	
	/**
	 * スタックからの吉里吉里オブジェクトの取得
	 * @param v squirrelVM
	 * @param idx インデックス
	 * @param variant 格納先
	 * @return 格納成功したら true
	 */
	static bool getVariant(HSQUIRRELVM v, SQInteger idx, tTJSVariant *variant);

	/**
	 * スタックへの吉里吉里オブジェクトの登録
	 * @parma variant オブジェクト
	 * @return 登録成功したら true
	 */
	static bool pushVariant(HSQUIRRELVM v, tTJSVariant &variant);

	// ---------------------------------------------------------------
	
	/**
	 * 吉里吉里クラスから squirrel クラスを生成
	 * @param v squirrelVM
	 */
	static SQRESULT createTJSClass(HSQUIRRELVM v);

	// ---------------------------------------------------------------
	
	/**
	 * TJSオブジェクト用のメソッド
	 * 引数1 オブジェクト
	 * 引数2～配列
	 * 自由変数1 メソッド
	 */
	static SQRESULT tjsInvoker(HSQUIRRELVM v);

	/**
	 * TJSオブジェクト用のプロパティゲッター
	 * 引数1 オブジェクト
	 * 自由変数1 プロパティ
	 */
	static SQRESULT tjsGetter(HSQUIRRELVM v);

	/**
	 * TJSオブジェクト用のプロパティセッター
	 * 引数1 オブジェクト
	 * 引数2 設定値
	 * 自由変数1 プロパティ
	 */
	static SQRESULT tjsSetter(HSQUIRRELVM v);

	/**
	 * TJSオブジェクト用の静的メソッド
	 * 引数1 オブジェクト
	 * 引数2～配列
	 * 自由変数1 クラス
	 * 自由変数2 メンバ
	 */
	static SQRESULT tjsStaticInvoker(HSQUIRRELVM v);

	/**
	 * TJSオブジェクト用の静的プロパティゲッター
	 * 引数1 オブジェクト
	 * 自由変数1 クラス
	 * 自由変数2 プロパティ
	 */
	static SQRESULT tjsStaticGetter(HSQUIRRELVM v);

	/**
	 * TJSオブジェクト用の静的プロパティセッター
	 * 引数1 オブジェクト
	 * 引数2 設定値
	 * 自由変数1 クラス
	 * 自由変数2 プロパティ
	 */
	static SQRESULT tjsStaticSetter(HSQUIRRELVM v);

	/**
	 * TJSオブジェクトの有効確認
	 * 引数1 オブジェクト
	 */
	static SQRESULT tjsIsValid(HSQUIRRELVM v);

	/**
	 * TJSオブジェクトに対するオーバライド処理
	 * 引数1 オブジェクト
	 * 引数2 名前
	 * 引数3 squirrelクロージャ
	 */
	static SQRESULT tjsOverride(HSQUIRRELVM v);

protected:
	/**
	 * コンストラクタ
	 * @param v squirrel VM
	 * @param idx オブジェクト参照元インデックス
	 * @param instance バインド対象のTJSオブジェクト
	 */
	TJSObject(HSQUIRRELVM v, int idx, tTJSVariant &instance);
	
	// デストラクタ
	~TJSObject();

	/**
	 * 破棄処理
	 */
	void invalidate();
	
	/**
	 * オブジェクトのリリーサ
	 */
	static SQRESULT release(SQUserPointer up, SQInteger size);

	/**
	 * TJSオブジェクトのコンストラクタ
	 * TJSオブジェクトのコンストラクタ
	 * 引数1 オブジェクト
	 * 引数2～ 引数
	 * 自由変数1 クラス名
	 */
	static SQRESULT tjsConstructor(HSQUIRRELVM v);

private:
	// NativeClass ID
	static int classId;
	
	// 処理対象オブジェクト
	tTJSVariant instance;
	
public:
	// NativeInstance 対応用メンバ
	virtual tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	virtual void TJS_INTF_METHOD Invalidate();
	virtual void TJS_INTF_METHOD Destruct();
};

#endif
