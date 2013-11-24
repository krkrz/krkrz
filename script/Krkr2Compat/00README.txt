TITLE: 吉里吉里Ｚ向け「吉里吉里２互換実装」スクリプト
AUTHOR: 合資会社ワムソフト 三上 響

-------------------------------------------------------------------------------
●これは何か？

吉里吉里Ｚの「吉里吉里２と互換のない部分」を埋めるための
TJSスクリプトによる互換実装です。

既存スクリプトをできるだけ手を入れること無く動かすことを目的としています。

下記はダミーのプロパティを作成します。

	・Window.innerSunken
	・Window.showScrollBars

下記は必要になった時点で対応プラグインを読み込みます。

	・Window.menu, MenuItem (menu.dll)
	・KAGParser (KAGParser.dll)

下記は win32dialog.dll により実装されています。

	・フォント選択ダイアログ（Font.doUserSelect）
	・１行入力ダイアログ（System.inputString）

	・コンソールウィンドウ（Debug.console）
	・スクリプトエディタ（Shift+F2で表示されるもの）
	・Padクラス（一部プロパティについては未サポート）

下記は未実装ですが，今後対応するかもしれません。

	・コントローラ（Debug.controller）
	・監視式ウィンドウ（Shift+F3で表示されるもの）

なお，各動作について細部まで厳密にチェックはしていません。
問題があった場合は issue 登録をお願いします。


-------------------------------------------------------------------------------
●ファイルの内容について

00README.txt		このファイルです
data/startup.tjs	テストスクリプト起動用TJSファイルです
data/export.tjs		文字コード変換用のTJSファイルです
data/k2compat/*.tjs	互換スクリプト本体です。
k2compat.xp3		data/k2compat/*.tjs を utf16le-bom で収めてあります
pack_k2compat_xp3.bat	k2compat.xp3 をパックするためのバッチファイル
clean.bat		文字コード変換の一時フォルダを消すバッチファイル
runkrkr2_*.bat		吉里吉里２で起動するテストバッチファイル

	※各TJSファイルは utf8 エンコードになっています
	　実際のプロジェクトで使用する場合は k2compat.xp3 を使用するか，
	　後述の文字コード変換をしてください。


-------------------------------------------------------------------------------
●必要ファイル

テストスクリプトの動作確認や文字コード変換には
足りないファイルを用意する必要があります。

・吉里吉里Ｚから持ってくるもの：

tvpwin32.exe		全般で必要
plugin/menu.dll		互換処理に必要
plugin/KAGParser.dll	互換処理に必要

・吉里吉里２本家から持ってくるもの：

plugin/win32dialog.dll	互換処理に必要[※]
plugin/fstat.dll	文字コード変換に必要[※]
plugin/saveStruct.dll	文字コード変換に必要[※]

	[※]下記からダウンロードできます
	https://sv.kikyou.info/svn/kirikiri2/trunk/kirikiri2/bin/win32/plugin/

・吉里吉里２本家から持ってくるもの（オプショナル）：

krkr.eXe		runkrkr2_*.bat での起動に必要
krkrrel.exe		pack_k2compat_xp3.bat に必要
krdevui.dll		〃
plugin/windowEx.dll	K2COMPAT_SPEC_{DESKTOP,SCREEN}INFO指定を使う場合に必要


-------------------------------------------------------------------------------
●使い方

既存のプロジェクトで使用する場合，
まず予め下記の３つのプラグインをpluginフォルダなどへ配置しておきます。

	menu.dll
	KAGParser.dll
	win32dialog.dll

次に，startup.tjs の冒頭などに

@if (kirikiriz)
{
	var base = "※k2compatの配置位置";
	Scripts.execStorage(base+"k2compat.tjs");
	Krkr2CompatUtils.scriptBase = base;
}
@endif

といったスクリプトを埋め込みます。

ただし「"※k2compatの配置位置";」の部分は，
k2compatをどのように配置するかで書き換えてください。

・k2compat.xp3 を使用する場合

　k2compat.xp3をtvpwin32.exeと同じ場所に配置します。
　上記スクリプトは

	var base = System.exePath + "k2compat.xp3>";

　のように記述します。
　なお，XP3の暗号化モジュールを使用している場合はこの方法は使えません。


・k2compatフォルダを配置する場合

　予め後述の文字コード変換を行い，
　data_sjis/k2compat もしくは
　data_utf16le/k2compat フォルダを
　対象プロジェクトの data フォルダへコピーしておきます。
　そして上記スクリプトは

	var base = "k2compat/";

　のように記述します。
　なお，この方法の場合，（レアケースだと思いますが）修正パッチなどで
　k2compatフォルダ以下のファイルに修正が必要な場合は
　若干特殊な対応が必要になります。（後述）

・k2compatフォルダを配置して検索パスを追加する場合

　「k2compatフォルダを配置する場合」と同じようにk2compatフォルダを配置します。
　上記スクリプトは Initialize.tjs などで「k2compat」フォルダにパスを通しておき，
　パスを通した後で，

	var base = "";

　として実行してください。（配置場所は startup.tjs ではなくなります）
　こちらの場合は修正パッチでの特殊な対応は不要になります。


最後に，tvpwin32.exe -readencoding=Shift_JIS で起動してください。
プラグインなどは基本的に必要になった時点で遅延読み込みされますので，
足りないファイルがあっても起動直後においては確認できません。
ご注意ください。


-------------------------------------------------------------------------------
●条件コンパイル式について

k2compat.tjsをロードする前に下記の条件コンパイル式が定義可能です。

// 各種互換実装を無効化します

@set (K2COMPAT_PURGE_MENU = 1) // MenuItem, Window.menu (menu.dll)
@set (K2COMPAT_PURGE_KAGPARSER = 1) // KAGParser (KAGParrser.dll
@set (K2COMPAT_PURGE_FONTSELECT = 1) // Layer.font.doUserSelect
@set (K2COMPAT_PURGE_INPUTSTRING = 1) // System.inputString
@set (K2COMPAT_PURGE_WINDOWPROP = 1) // Window.innerSunken, .showScrollBars
@set (K2COMPAT_PURGE_PTDRAWDEVICE = 1) // Window.PassThroughDrawDevice
@set (K2COMPAT_PURGE_PAD = 1) // Pad
@set (K2COMPAT_PURGE_DEBUG = 1) // 下記のDebugクラス周りの実装一括
  @set (K2COMPAT_PURGE_CONSOLE = 1) // コンソール (Debug.console)
  @set (K2COMPAT_PURGE_CONTROLLER = 1) // コントローラ (Debug.controller)
  @set (K2COMPAT_PURGE_SCRIPTEDITOR = 1) // スクリプトエディタ (Debug.scripted)
  @set (K2COMPAT_PURGE_HOTKEY = 1) // Shift+F1～F4のホットキー


// System.desktop{Left,Top,Width,Height} の仕様を変更します(要windowEx.dll)
@set (K2COMPAT_SPEC_DESKTOPINFO =  1) // 常にプライマリモニタの情報を返す
@set (K2COMPAT_SPEC_DESKTOPINFO = -1) // Window.mainWindowのあるモニタ情報を返す
※未指定の場合はZ本来の仕様(全モニタ統合した座標情報)のままになります

// System.screen{Width,Height} の仕様を変更します(要windowEx.dllプラグイン)
@set (K2COMPAT_SPEC_SCREENINFO = 1) // 常にプライマリモニタの情報を返します
※未指定の場合はZ本来の仕様(mainWindowのあるScreenサイズを返す)のままになります

// k2compat.tjsのデバッグログ表示を有効にします
@set (K2COMPAT_VERBOSE = 1)

// ダミープロパティに書き込まれた場合のログ出力を抑制します
@set (K2COMPAT_PURGE_DUMMYPROP_LOG = 1)


-------------------------------------------------------------------------------
●テストスクリプトについて

直接tvpwin32.exeを起動するとデバッグウィンドウ類の動作確認ができます。

・Fileメニュー

	スクリプトの文字コード変換機能アイテムが配置されています

・FontSelectメニュー

	フォント選択画面とSystem.inputStringのテストアイテムが配置されています

・Padメニュー

	Padウィンドウのテストアイテムが配置されています

・Debugメニュー

	コンソールとスクリプトエディタの表示切り替えアイテムが配置されています


-------------------------------------------------------------------------------
●文字コード変換について

k2compatは作業の都合でutf8エンコードになっています。
既存プロジェクトで使う場合は，プロジェクト側の文字コードを
こちらに合わせてutf8に変換するか，k2compat側を既存プロジェクトに合わせるか，
もしくはどちらでも読めるutf16leに変換する必要があります。

テストスクリプトに文字コード変換ツールが内蔵されていますのでご利用ください。

	tvpwin32.exe -export=utf16le
	tvpwin32.exe -export=sjis

のどちらかのコマンドラインで起動するか，
テストウィンドウの File メニュー内「Export UTF16LE/ShiftJIS Scripts」を
選ぶことで変換可能です。

なお，どちらの変換に際しても，fstat.dll プラグインが必須です。
ShiftJISへの変換は saveStruct.dll プラグインが必須です。

変換結果は，

	data_utf16le
	data_sjis

フォルダに格納されますので，この中にある k2compat フォルダごとご利用ください。


-------------------------------------------------------------------------------
●互換性について

▽デバッグウィンドウ全般：

・Shift+F? などのホットキーでのデバッグウィンドウ類の表示ON/OFFは，
　吉里吉里のウィンドウ(Windowクラスのインスタンス)にフォーカスがある場合にのみ
　機能します。デバッグウィンドウにフォーカスがある場合は現在機能しません。

・現在，krenvprf.kep相当のデバッグウィンドウ位置等の保存機能がありません。
　別の形式で状態を保存するような対応を検討中です。

・デバッグウィンドウ上での右クリックメニューの特殊機能はありません。

・ウィンドウのアイコンが違います。

・テキストの選択範囲の色が違います。

・個別のデバッグウィンドウを最小化した時に
　タスクバーではなく画面左下にストックされます。

・Windowを１つでも作る前(Window.mainWindowがない状態)で
　コンソール等のデバッグウィンドウを表示すると，
　後からWindowを作って表示した場合に，Window側の最小化ボタンが
　各デバッグウィンドウの最小化と連動しません。
　また，タスクバーのボタンも個別の枠を取る状態になります。
＞デバッグウィンドウの親ウィンドウが無いために起こる現象で，仕様です。

・逆にWindow.mainWindowがある状態でデバッグウィンドウを表示すると，
　そのウィンドウは常にWindow.mainWindowより手前に表示されてしまいます。
＞設定で回避できないか確認中（TVPApplicationWindowが無くなったことの弊害？）


▽コンソール：

・連続したログの出力はアイドル状態になってからまとめて一括で表示されます。
＞スクロールで流れるログを見たい場合は別途cmd.exeなどを使用してください

・１行入力のeval処理の結果表示が若干異なります。
＞結果がObjectの場合に詳細情報が表示されます

・１行入力を評価した時に例外が発生した場合，入力欄がクリアされません。
＞修正を前提とした動作になっています

・ログテキスト側をクリックすると１行入力からフォーカスが外れて
　ログ側にキャレットが表示されてしまいます。(Tabキーで復帰できます）

・ログがスクロールする時に表示がチラ付く場合があります

・ログ／入力履歴の上限行数が異なります


▽Padクラス：

・ステータスバーにカーソル位置(X:Y)の表示機能はありません

・下記プロパティは new Pad() した後，visible=trueする間にのみ変更可能です。
　一度表示した後からの変更はできません。

	fontFace
	fontSize, fontHeight
	fontBold
	wordWrap

・fontHeightプロパティは恐らく画面のDPIを変更した環境では
　正しいピクセルサイズを指定できません。また負数も指定できません。
　代わりにfontSizeプロパティを使用してください。

・下記プロパティは未サポートです。（値は設定できますが意味を持ちません）

	fileName
	fontItalic
	fontUnderline
	fontStrikeOut

・visibleをtrue->false->trueとした場合にキャレット位置が失われて
　テキストが全選択状態になってしまいます。


▽スクリプトエディタ：

・Padクラスに左下の実行ボタンを有効にしたバージョンのクラスが
  「ScriptEditorPad」クラスとして実装されています。

・現状，例外発生時に自動でスクリプトエディタを自動で表示してエラー行に
　カーソルをあわせる機能はありません。
＞Exceptionクラスにエラー位置などの情報がないため，本体側の対応を検討中


