//
// squirrel 疑似スレッド処理支援ライブラリ
//

/**
 * 基底オブジェクト
 *
 * ●プロパティ機能
 * Object を継承したオブジェクトは、通常の squirrel のオブジェクトにはないプロパティ機能が
 * 拡張されています。該当するメンバが存在しなかった場合、自動的に getter/setter
 * ファンクションを探して、それがあればそれを呼び出して値が処理されます。
 * val = obj.name; → val = obj.getName();
 * obj.name = val; → obj.setName(val)
 *
 * ●delegate機能
 * Object を継承したオブジェクトは、squirrel のテーブル/ユーザデータがサポートしてるような
 * delegate 機能を使うことができます。委譲先のオブジェクトは、squirrel の標準機能同様の
 * テーブルの他、別のオブジェクトインスタンスが指定可能です。
 * 委譲オブジェクトがインスタンスの場合は、クロージャを参照する際、そのインスタンスを環境
 * として bindenv された状態で取得されます(TJSのデフォルトの挙動と同じようになります)。
 * テーブルの場合は環境を再設定しないので、クロージャは元のオブジェクトの環境で実行されます
 *
 * ●wait機能
 * スレッド(Thread)はオブジェクトを「待つ」ことができます。
 * オブジェクトに対する待ちは、オブジェクトが notify/notifyAll することで解除されます。
 * オブジェクトが破棄される時には notifyAll() が実行されます。
 *
 * ●イベント機能(C++側機能)
 * Object を継承して作成した C++ オブジェクト中から callEvent() を呼び出すことで、
 * 該当オブジェクトの squirrel メソッドを呼び出して値を取得させることができます。
 */ 
class Object {

	/**
	 * コンストラクタ
	 * @param delegate 処理を委譲するオブジェクトを指定します。
	 * Object の諸機能が正常に動作するためには
	 * 継承クラスのコンストラクタではかならず Object.constructor() を呼び出す必要があります。
	 */
	constructor(delegate=null);

	/**
	 * デストラクタ
	 * 定義してあるとオブジェクト破棄直前に呼び出されます
	 */
    function destructor();
  
	/**
	 * このオブジェクトに対する委譲を設定します(コンストラクタ指定と同機能)
	 * @param delegate 委譲先オブジェクト
	 */
	function setDelegate(delegate=null);

	/**
	 * このオブジェクトに対する委譲を取得します。
	 * @return 委譲先オブジェクト
	 */
	function getDelegate();
	
	/**
	 * @param name プロパティ名
	 * @return 指定された名前のプロパティの setter があれば true
	 */
	function hasSetProp(name);
	
	/**
	 * このオブジェクトを待っているスレッド1つに終了を通知する
	 * ※待ちが古いものから順に処理されます
	 */
	function notify();

	/**
	 * このオブジェクトを待っている全スレッドに終了を通知する
	 * ※このメソッドはオブジェクト廃棄時にも実行されます。
	 */
	function notifyAll();

	/**
	 * プロパティの値を取得する。プロパティ名に対応する getter メソッドを呼び出して
	 * その値を返します。_get として登録されているものの別名です。
	 * @param propName プロパティ名
	 * @return プロパティの値
	 */
	function get(propName);

	/**
	 * プロパティの値を設定する。プロパティ名に対応する setter メソッドを呼び出して
	 * 値を設定します。_set として登録されているものの別名です。
	 * @param propName プロパティ名
	 * @param calue プロパティの値
	 */
	function set(propName, value);
};

enum {
	// スレッドのステート
	NONE = 0;     // 無し
	LOADING_FILE = 1;  // ファイル読み込み中
	LOADING_FUNC = 2;  // 関数読み込み中
	STOP = 3;     // 停止中
	RUN  = 4;      // 実行中
	WAIT = 5;     // 処理待ち
} TEREADSTATUS;

/**
 * スレッド制御用オブジェクト
 * 疑似スレッドを制御するためのオブジェクトです。
 *
 * ●スレッド実行機能
 * exec でスクリプトをスレッド実行させることができます。
 * スレッドを実行中の場合、仮にユーザの参照がなくなってもシステムが参照を維持します。
 * 指定されたのが定義済みの関数の場合は、スレッドのステートは直ちに「RUN」になり、
 * 次の実行処理単位から実行が開始されます。
 * ファイルから実行する場合は、ファイルロードが完了するまで実行開始が遅延する場合があります。
 * ロード中はスレッドのステートが「LOADING」になります。
 * 
 * ●wait機能
 * スレッドは実行処理を一時停止して「待つ」ことができます。この状態のステートは「WAIT」です。
 *
 * - 時間待ち: 指定された時間(tick値)以上の間実行を停止します。
 * - トリガ待ち: 指定されてトリガ(文字列指定)が送られてくるまで実行を停止します。
 * - オブジェクト待ち: 指定されたObject型のオブジェクトから notify() をうけるまで実行を停止します。
 * 
 * オブジェクトの notify() のタイミングはオブジェクトの実装次第です。
 * ただし、オブジェクト破棄時は自動的に notifyAll() されます。
 * スレッドも Objectなので、スレッドから別のスレッドを待つことができます。
 * スレッドは実行終了時およびオブジェクト破棄時に notifyAll() を実行します。
 */
class Thread extends Object {

	/**
	 * コンストラクタ
	 * @param delegate 処理を委譲するオブジェクトを指定します。
	 * @param func スレッドを生成後実行するファンクションまたはファイル名
	 * @param ... 引数
	 */
	constructor(delegate=null, func=null, ...);

	/**
	 * @return このスレッドの実行時間(tick値)wo
	 */
	function getCurrentTick();

	/**
	 * @return このスレッドの実行ステータス NONE/LOADING_FILE/LOADING_FUNC/STOP/RUN/WAIT
	 */
	function getStatus();

	/**
	 * @return このスレッドの終了/suspendコード
	 * スクリプトから return, suspend, exit() された時の指定値が格納されています
	 */
	function getExitCode();

	/**
	 * スレッドの実行開始
	 * @param func 呼び出すグローバル関数またはスクリプトファイル名
	 */
	function exec(func, ...);

	/**
	 * スレッドの終了
	 * @param exitCode 終了コード
	 */
	function exit(exitCode);

	/**
	 * スレッドの一時停止
	 */
	function stop();

	/**
	 * 一時停止したスレッドの再開
	 */
	function run();

	/**
	 * スレッドの実行待ち。いずれかの条件で解除されます。引数を指定しなかった場合でも、
	 * いったん処理が中断して、システム側のイベント/更新処理完了後に復帰します。
	 * @param param 待ち条件指定 文字列:トリガ待ち オブジェクト:オブジェクト待ち 数値:時間待ち(tick値)※最小の指定が有効
	 */
	function wait(param, ...);

	/**
	 * 現在の待ちをすべて強制的にキャンセルします
	 */
	function cancelWait();
};

// ------------------------------------------------
// スレッド系グローバルメソッド
// ------------------------------------------------

/**
 * @return 稼働中スレッドの一覧(配列)を返します。
 */
function getThreadList();

/**
 * @return 現在実行中のスレッド(Thread)を返します。
 */
function getCurrentThread();

/**
 * @return 現在実行中のスレッドの実行時間(tick値)
 */
function getCurrentTick();

/**
 * @return 現在実行中のスレッドの前回実行時からの経過時間(tick値)
 */
function getDiffTick();

/**
 * 新しいスレッドを生成して実行します。Thread(null,func, ...) と等価です。
 * @param func 呼び出すメソッド、またはファイル名
 * @return 新規スレッド制御オブジェクト(Thread)
 * @param ... 引数
 */
function fork(func, ...);

/**
 * 現在実行中のスレッドを別の実行に切り替えます
 * @param func 呼び出すグローバル関数またはスクリプトファイル名
 * @param ... 引数
 */
function exec(func, ...);

/**
 * 現在実行中のスレッドを終了します
 * @param exitCode 終了コード
 */
function exit(exitCode);

/**
 * 現在実行中のスレッドから、別のスクリプトを実行してその終了を待ちます。
 * @param func 呼び出すグローバル関数またはスクリプトファイル名
 * @param ... 引数
 * @return 呼び出したスクリプトの終了コード (exit()で指定したもの、または最後の return の値)
 */
function system(func, ...);

/**
 * 現在実行中のスレッドの実行待ち。いずれかの条件で解除されます。引数を指定しなかった場合でも、
 * いったん処理が中断して、システム側のイベント/更新処理完了後に復帰します。
 * @param param 待ち条件指定 文字列:トリガ待ち オブジェクト:オブジェクト待ち 数値:時間待ち(tick値)※最小の指定が有効
 * @return wait解除の原因。待ちが無いかキャンセルされた場合は null
 */
function wait(param, ...);

/**
 * トリガ送信
 * 全スレッドに対してトリガを送信します。
 * 該当するトリガを待っていたスレッドの待ちが解除されます。
 * @param trigger トリガ名
 */
function notify(trigger);

// ------------------------------------------------
// Continuous Handler
// ------------------------------------------------

/**
 * スレッド処理前後に呼び出されるファンクションを登録する。
 * ファンクションは function(currentTick, diffTick) の形で呼び出されます。
 * この呼び出しはスレッドによるものではないため、処理中に suspend() / wait() を
 * 呼ぶとエラーになるので注意してください。必ず1度で呼びきれるものを渡す必要があります。
 * currentTick, diffTick は、その回のスレッド呼び出し処理冒頭での値になります。
 * @param func 登録するファンクション
 * @param type 0:スレッド処理の前 1:スレッド処理の後
 */
function addContinuousHandler(func, type=1);

/**
 * 描画処理前に呼び出されるファンクションを登録解除する
 * @param func 登録解除するファンクション
 */
function removeContinuousHandler(func, type=1);

/**
 * 全 continuous handler を解除する
 */
function clearContinuousHandler();


// ------------------------------------------------
// ベースVM操作
// ------------------------------------------------

/**
 * ベースVM上でスクリプトを実行する。
 * この呼び出しはスレッドによるものではないため、処理中に suspend() / wait() を
 * 呼ぶとエラーになるので注意してください。必ず1度で呼びきれるものを渡す必要があります。
 * @param func グローバル関数。※ファイルは指定できません
 * @param ... 引数
 */
function execOnBase(func);
