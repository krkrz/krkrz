Title: KAGParserEx プラグイン
Author: miahmie, wtnbgo

●これは何か？

KAGParserを置き換えて拡張するプラグインです。
安定したら本体側にcontribする予定ですが，
状況次第ではこのままで行くかもしれません。

現在のところ，拡張されるのは下記３点のみです。
なお，仕様は予告無く変更される場合があります。


・タグの複数行記述が可能になる multiLineTagEnabled プロパティの追加

　行末に "\" を記述し，次の行の先頭に ";" を入れることで，
　複数行にわたるタグを記述できます。
　※行末に "\" のみで次の行頭に ";" がないとエラーになります。

	@tag hoge=123 fuga=test \
	;    someoption=true \
	;    andmore=false

　行頭に ";" を入れるのは，旧仕様のパーサーに通しても ch タグとして
　認識されないようにコメントとする無理やりな adhoc 仕様です。

　"\" の前には必ず１つ以上のスペースを空けてください。
　空けないと前の要素とくっついて認識されてしまいます。
　また，タグ名は必ず１行目に記述します。

	; ダメな例：
	@tag hoge=123\			← hoge="123\"と認識
	@tag hoge\			← hoge\=trueと認識
	@ \				← \ というタグ名で認識

　現仕様では，[～]表記で複数行のタグを記述する場合，そのタグを閉じた以降に
　さらにタグを続けても無視されるようになっています。
　（改行タグも自動で挿入されません）

	[tag hoge=123 fuga=test \
	;    someoption=true \
	;    andmore=false][このタグは無視される]

・getNextTag() の帰り値中に "taglist" メンバを追加

　設定されている値はタグ名が記載順格納された配列です。
　記載された順にコマンド処理したい場合に有用です。

・パラメータマクロ展開機能

  paramMacros (辞書オブジェクト) に対して以下の形のマクロ情報を登録できます

　パラメータ名 => [パラメータ名,値,パラメータ名,値 ... ]

　この登録に合致するパラメータ名があった場合に、
  登録されているパラメータ一覧をそこに差し込んだものとして処理します
  パラメータ値の先頭に % や & があるとそれは実際の実行時に解釈されて
　それぞれマクロ展開/実体参照として機能します。

　関連して以下のシステムタグが追加されています

 パラメータマクロ登録
  @pmacro name=パラメータマクロ名 param1=value param2=value ...

  &や%の指定は、普通に書くと pmacro の登録時に処理されてしまう
  のでマクロ実行時に処理させたい場合はエスケープしておいて下さい

 パラメータマクロ削除
  @erasepmacro name=パラメータマクロ名


●マクロパラメータの展開「*」の挙動の違いについて

KAGParserではマクロ展開の「*」以前に書かれているオプションが上書きされて
消えてしまうというバグっぽい挙動がありますが，KAGParserExでは
paramMacros対応のため構造を変えた関係で「*」以前のパラメータも有効となります。

例：
	[macro name=hoge]
		[tag foo=bar * baz]
	[endmacro]

	において

	[hoge fuga=piyo]

	とすると，
	・KAGParser   では [tag fuga=piyo baz]
	・KAGParserEx では [tag foo=bar fuga=piyo baz]
	が渡る


●ソースについて

KAGParserのソースをそのまま流用＆改造して組み込んでいます。
ソースはsvn copyしているので，ログから本体側からの変更点を追うことができます。

	KAGParser.cpp   <- src/core/utils/KAGParser.cpp
	KAGParser.h     <- src/core/utils/KAGParser.h
	tjsHashSearch.h <- src/core/tjs/tjsHashSearch.h


●使い方

KAGParserEx.dll をリンクすると，KAGParserクラスが置き換わります。
アンリンクすると，元に戻ります。


●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。

