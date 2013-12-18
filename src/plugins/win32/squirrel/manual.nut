/**
 * 吉里吉里クラスの取得
 * @param className 吉里吉里クラス名指定(文字列)
 * @param ... 継承している親クラスを列挙
 * @return squirrelクラス
 *
 * 吉里吉里のクラスを squirrelクラスとして取得します。
 * このクラスは継承可能です。
 *
 * ※吉里吉里側で親クラス情報を参照生成できないため、
 * 親クラスが継承しているクラスの名前をすべて手動で列挙する必要があります。
 * またこの機能で作成した吉里吉里クラスのインスタンスが吉里吉里側から
 * 返される場合は、squirrel のクラスでのインスタンスとしてラッピングされます。
 */
function createTJSClass(className, ...);

/**
 * 吉里吉里クラスの squirrel における基底クラス構造
 * このインスタンスに対する吉里吉里側からのメンバ参照は、元の吉里吉里インスタンスのそれが呼ばれますが、
 * 存在してないメンバの場合は、missing 機能により squirrel 側オブジェクトの同名メンバが参照されます。
 * 吉里吉里から呼ばれるものも直接 squirrelインスタンスのそれにに差し替える場合は
 * tjsOverride() で強制上書きをかけることができます。イベントの登録に使います。
 */
class TJSObject extends Object {

	/**
	 * コンストラクタ
	 */
	constructor();

	/**
	 * 吉里吉里オブジェクトの有効性の確認
	 * レイヤなど吉里吉里側で強制 invalidate される可能性があるオブジェクトの状況確認に使います。
	 * @return valid なら true
	 */
	function tjsIsValid();
	
	/**
	 * 吉里吉里オブジェクトの強制オーバライド処理
	 * 吉里吉里インスタンスのメンバを強制的に上書きします。
	 * イベントなどを squirrel 側でうけたい場合に指定します
	 * 値を省略した場合は自己オブジェクトを参照します。このとき、
	 * クロージャが指定されていた場合は、自動的に bindenv(this) されたものが登録されます
	 * @param name メンバ名
	 * @param value 登録する値(省略可)
	 */
	function tjsOverride(name, value=null);
};

/**
 * TJS用の null値。tjsのメソッドに tjs の null を渡す必要がある場合などに使います。
 * squirrel の null は tjs では void 扱いです。
 */
tjsNull;
