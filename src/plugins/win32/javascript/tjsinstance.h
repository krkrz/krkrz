#ifndef __TJSINSTANCE_H__
#define __TJSINSTANCE_H__

#include "tjsbase.h"

/**
 * 吉里吉里インスタンスを保持するJavaScript用クラス情報
 * メンバ情報をプロトタイプに登録します
 */
class TJSInstance : public TJSBase, iTJSNativeInstance {

public:
	// 初期化用
	static void init(Handle<ObjectTemplate> globalTemplate);

	/**
	 * 吉里吉里オブジェクトを Javascrip オブジェクトに変換
	 * @param result 格納先
	 * @param variant 変換元
	 * @return 変換成功したら true
	 */
	static bool getJSObject(Local<Object> &result, const tTJSVariant &variant);

	// メソッド呼び出し用
	static tjs_error getProp(Local<Object> obj, const tjs_char *membername, tTJSVariant *result);
	static tjs_error setProp(Local<Object> obj, const tjs_char *membername, const tTJSVariant *param);
	static tjs_error remove(Local<Object> obj, const tjs_char *membername);
	static tjs_error createMethod(Local<Object> obj, const tjs_char *membername, iTJSDispatch2 **result, tjs_int numparams, tTJSVariant **param);
	static tjs_error callMethod(Local<Object> obj, const tjs_char *membername, tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	// ---------------------------------------------------------------

	/**
	 * call 処理用の口
	 * TJSインスタンスからJavascriptインスタンスのメソッドを直接呼び出す
	 */
	static tjs_error call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
						  tTJSVariant *result,
						  tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	/**
	 * missing 処理用の口
	 * TJSインスタンスにメンバが存在しなかった場合は Javascriptインスタンスを参照する
	 */
	static tjs_error missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);

	// ---------------------------------------------------------------
	
	/**
	 * TJSオブジェクト用のメソッド
	 * @param args 引数
	 * @return 結果
	 */
	static Handle<Value> tjsInvoker(const Arguments& args);

	/**
	 * TJSオブジェクト用のプロパティゲッター
	 */
	static Handle<Value> tjsGetter(Local<String> property, const AccessorInfo& info);
	
	/**
	 * TJSオブジェクト用のプロパティセッター
	 */
	static void tjsSetter(Local<String> property, Local<Value> value, const AccessorInfo& info);

	/**
	 * コンストラクタ
	 * @param obj 自己オブジェクト
	 * @param instance バインド対象のTJSオブジェクト
	 */
	TJSInstance(Handle<Object> obj, const tTJSVariant &instance);
	
private:

	/**
	 * 吉里吉里クラスから Javascript クラスを生成
	 * @param args 引数
	 * @return 結果
	 */
	static Handle<Value> createTJSClass(const Arguments& args);

	/**
	 * 破棄処理
	 */
	void invalidate();
	
	/**
	 * オブジェクトのディスポーザ
	 */
	static void release(Persistent<Value> handle, void* parameter);

	/**
	 * TJSオブジェクトの有効確認
	 * @param args 引数
	 * @return 結果
	 */
	static Handle<Value> tjsIsValid(const Arguments& args);

	/**
	 * TJSオブジェクトに対するオーバライド処理
	 * @param args 引数
	 * @return 結果
	 */
	static Handle<Value> tjsOverride(const Arguments& args);
	
	/**
	 * TJSオブジェクトのコンストラクタ
	 * @param args 引数
	 * @return 結果
	 */
	static Handle<Value> tjsConstructor(const Arguments& args);

	// NativeClass ID
	static int classId;
	
	// 自己参照用
	Persistent<Object> self;

public:
	// NativeInstance 対応用メンバ
	virtual tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	virtual void TJS_INTF_METHOD Invalidate();
	virtual void TJS_INTF_METHOD Destruct();
};

#endif
