TITLE: KirikiriZ 向けフォント選択ダイアログ互換スクリプト
AUTHOR: 合資会社ワムソフト 三上 響

●これは何か？

フォント選択ダイアログの互換版を win32dialog.dll にて実装したものになります。
Fontインスタンスから親レイヤを取得できないため，呼び出し方法が若干異なりますが，
それ以外はほぼ Font.doUserSelect と同じように使えます。
（ただし細かいオプションについては細部まで厳密にチェックはしていません）


●ファイルの内容について

00README.txt		このファイルです
data			テストスクリプトフォルダです
data/startup.tjs	テストスクリプト起動用TJSファイルです
data/win32dialog.tjs	win32dialog.dllのヘルパーTJSです
data/fontselect.tjs	フォント選択ダイアログの本体です
data/inputstring.tjs	System.inputString 互換のスクリプト本体です

	※各tjsファイルは utf16le-bom エンコードになっています

●動作確認方法

動作確認には足りないファイルを用意していただく必要があります。

tvpwin32.exe		β３で確認
plugin/menu.dll		〃
plugin/win32dialog.dll	吉里吉里本家レポジトリからバイナリを取得してください
			⇒ https://sv.kikyou.info/svn/kirikiri2/trunk/kirikiri2/bin/win32/plugin/win32dialog.dll

上記ファイルを置いて起動していただくと，テストウィンドウが表示されます。

Fileメニュー内

	・FontSelect(GDI)
	・FontSelect(FreeType)

を選ぶことでフォント選択ダイアログが表示されます。
GDI/FreeTypeはラスタライザのバージョンの違いによるものです。

同じくメニューのOptionを選ぶと，フォント選択のflagsを編集することができます。
（System.inputStringのサンプルにもなっています）


●既知の不具合

・TrueType以外のフォント（System,Terminal等）が描画されない（GDI/FreeTypeとも）
＞Optionで「fsfTrueTypeOnly」を外すと確認できます
　freetypeは仕様としてもfont.getListから外す等の対応があっても良いのでは


●APIについて

----------------------------------------------------------------
◆System.doFontSelect

・機能/意味
　フォント選択ダイアログボックスの表示（互換実装）

・タイプ
　Systemクラスのメソッド

・構文
　doFontSelect(layer, flags, caption, prompt, sample)

・引数
　layer : 対象となる font オブジェクトを持つレイヤーを指定します
　flags, caption, prompt, sample : Font.doUserSelect と同じパラメータです

・戻り値（※）
　ユーザが OK ボタンを選択した／もしくはフォントリストをダブルクリックした場合は
　そのフォントフェイス名称の文字列が返ります。
　それ以外のキャンセル操作をされた場合は void が返ります。

・説明
　ユーザにフォントを選択させるためのダイアログボックスを開きます。
　デフォルトで layer.font.face のフォントが選択された状態になっています。
　fsfUseFontFace を flags に指定した場合は、layer.font.height が参照され、
　その大きさで表示されます。（大きすぎると操作が困難になるので注意してください）

　使用に際しては、
	fontselect.tjs  (自動検索パス中にスクリプトを配置)
	win32dialog.tjs (〃)
	win32dialog.dll (所定のプラグインフォルダに配置)
　の3ファイルが必要です。

　また、Scripts.evalStorage("fontselect.tjs"); を実行して
　あらかじめスクリプトを読み込んでおくか、

System.doFontSelect = function {
	// 遅延読み込み
	Scripts.evalStorage("fontselect.tjs");
	return global.System.doFontSelect(...);
} incontextof global;

　などとして遅延読み込みの細工をしておくことが必要です。

----------------------------------------------------------------


※Font.doUserSelectと戻り値の仕様が若干異なりますが，
　fontselect.tjsを編集することで互換仕様に変更可能です。

例：
	var dialog = new FontSelectDialog(layer, layer.font.face, *);
	var result = dialog.open(layer.window);
	invalidate dialog;
+	if (result !== void) layer.font.face = result;
-	return result;
+	return result !== void;


