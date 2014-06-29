●使い方

吉里吉里Ｚでメニュー(MenuItem, Window.menu)を拡張します。
下記のようにしてWindowインスタンス作成前に本プラグインをリンクしてください。

@if (kirikiriz)
Plugins.link("menu.dll");
@endif

リンク後の基本的なメニューの機能は吉里吉里２と同様です。


●ショートカットに指定可能な文字一覧
※ショートカットとして機能するかは別問題で、そちらは未確認のため注意

Backspace
Tab
Num 5
Enter
Shift
Ctrl
Alt
Esc
変換
無変換
Space
Page Up
Page Down
End
Home
Left
Up
Right
Down
Sys Req
Insert
Delete
0
1
2
3
4
5
6
7
8
9
A
B
C
D
E
F
G
H
I
J
K
L
M
N
O
P
Q
R
S
T
U
V
W
X
Y
Z
Num 0
Num 1
Num 2
Num 3
Num 4
Num 5
Num 6
Num 7
Num 8
Num 9
Num *
Num +
Num -
Num Del
/
F1
F2
F3
F4
F5
F6
F7
F8
F9
F10
F11
F12
Pause
Scroll Lock
Shift
Shift
Ctrl
Ctrl
Alt
Alt
M
D
C
B
P
Q
J
G
F
:
;
,
-
.
/
@
[
\
]
^
\
Caps Lock
ひらがな
半角/全角


●吉里吉里２との互換性について

・フルスクリーンでメニューバーの自動消去／表示機能がありません
＞TJS側で予め消去するなどの対応を行ってください

・下記ショートカット向けプロパティが拡張されます

global.MenuItem.textToKeycode = %[ ... ];
global.MenuItem.keycodeToText =  [ ... ];

MenuItem.shortcutのショートカットキーの文字列の変換テーブルです。
仮想キーコードとテキストの相互変換に使用されます。
（辞書の方はキーを全部小文字にして使用してください）

textToKeycode[text.toLowerCase()] = VK_*;
keycodeToText[VK_*] = text;

詳細はプラグインリンク後の上記辞書／配列の中身を確認してください。
中身を書き換えることができますが、MenuItem生成後での変更は行わないでください。

吉里吉里２互換用に予め下記が追加されています。
textToKeycode["BkSp".toLowerCase()] = VK_BACK;
textToKeycode["PgUp".toLowerCase()] = VK_PRIOR;
textToKeycode["PgDn".toLowerCase()] = VK_NEXT;

・上記変換テーブルの仕様により、MenuItem.shortcutの表記が変わる場合があります
（文字列⇒仮想キーコードが多対１のため、設定後は正規化されます）

	var item = new MenuItem(window, "shortcut normalize test");
	item.shortcut = "Shift+BkSp";
	Debug.message(item.shortcut);
	// -> Shift+Backspace


