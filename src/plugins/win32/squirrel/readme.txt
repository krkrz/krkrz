Title: Squirrel Plugin
Author: わたなべごう

●これはなに？

Squirrel (http://squirrel-lang.org/) の吉里吉里バインドです。

Squirrel 組み込み用オブジェクト指向言語です。
文法的には C 言語風で、TJS2 と構造も概念もよく似ています。

Squirrel は、協調スレッド（コルーチン）をサポートしており、
スクリプトの実行処理を任意のタイミングで中断できるため、
ゲーム用のロジックを組むのに非常に適しています。

●システム概要

◇名前空間

・Squirrel のグローバル空間は吉里吉里全体に対して１つだけ存在します。
　
　Squirrel 用のスクリプトの実行はこのグローバル空間上でおこなわれ、
　定義されたファンクションやクラスもこのグローバル空間に登録されていきます。

・TJS2 のグローバル空間を Squirrel 側から "::krkr" で参照できます。

・Squirrel のグローバル空間を TJS2 側から "sqglobal" で参照できます。

・I/O 空間は OS直接ではなく、TJS のストレージ空間が参照されます。

　ファイル名も TJS のストレージ名になります。
　stdin/stdout/stderr は利用できません

◇TJS2/Squirrel値変換ルール

・整数、実数、文字列などのプリミティブ値は値渡しになります。

・TJS2 の void は squirrel の null と対応します

・TJSの null は squirrel では値0の userpointer と対応します

  この値は squirrel ではグローバル変数 tjsNull で参照可能です。

・TJS2オブジェクト(iTJSDispatch2*) は、Squirrel では userData として参照可能です

　メタメソッド exist/get/set/call を通じて操作可能です。
　クラスオブジェクトを call した場合は、TJS2 側で
　インスタンスが作成されそれを UserData で参照したものが帰ります
  Dictionary/Arrayの存在しないメンバ参照はエラーになります

・squirrel オブジェクトは、TJS2 側では iTJSDispatch2 として参照可能です

  PropGet/PropGetByNum/PropSet/PropSetByNum/FuncCall/CreateNew 
  を通じて操作可能です。incontextof 指定は無視されます。
  table / array の存在しないメンバを参照した場合は void が帰ります
  table / array に対する書き込みは create 扱いになります

・createTJSClass()で、TJSのクラスを Squirrelクラスとして扱うことができます

  - このクラスから作られた squirrel インスタンスは TJS2側に渡る時に
　　内包した TJS インスタンスの参照が渡されます

  - TJS2 を経由して Squirrel 側に値が戻るときは、
　　そのまま元の Squirrelインスタンスが帰ります。

  - tjsOverride() で生成されたTJSインスタンス部に直接メソッドを登録できます

  - TJSインスタンス側に callSQ() として対応する squirrel インスタンスの
　  メソッドを明示的に呼び出す命令が拡張されます。

  - TJSインスタンス側では missing 機能が設定され、存在しないメンバが
　  参照された場合は squirrel インスタンスの同名メンバが参照されます。
    TJSインスタンス内部からのイベント呼び出しにもこれが適用されるため、
　  TJSインスタンス中に定義がなければ自動的に squirrel インスタンスの
　  それが呼び出されます
  
  - この形で作られたTJSインスタンスは squirrel インスタンスが破棄される
    ときに invalidate されます

・Scripts.registerSQ() で TJS2 の値を squirrel 側に登録できます。

◇標準ライブラリ

・Squirrel 標準ライブラリのうち以下のものが利用可能です

  - I/O
  - blob
  - math
  - string

●使用方法

◇Scripts 拡張

Squirrel スクリプトの実行機能や、オブジェクトを Squirrel の書式で
文字列化したり、ファイルに保存したりするメソッドが Scripts クラス
に拡張されます。詳細は manual.tjs を参照してください

◇SQFunction 拡張

Squirrel のglobalファンクションを直接呼び出せるように保持するクラスです。
TJS2 ラッピングによる余分な負荷なしに呼び出し処理を行うことができます。
詳細は manual.tjs を参照してください。

◇SQContinous 拡張

Squirrel のglobalファンクションを直接呼び出す Continuous Handler を
保持するクラスです。
TJS2 ラッピングによる余分な負荷なしに呼び出し処理を行うことができます。
詳細は manual.tjs を参照してください。

◇吉里吉里クラスの squirrelクラス化

吉里吉里のクラスを squirrel のクラスとして継承可能な状態で
扱うことができます。詳細は manual.nut を参照してください。

◇スレッド拡張他

squirrelによる複数のスレッドの並列実行処理が実装されています。
このために利用できる内部クラス Object / Thread が定義されています。
詳細は squirrel/sqobject/manual.nut を参照してください。

このスレッド処理を稼働させる場合、continuous handler などから
定期的に Scripts.driveSQ() を呼び出す必要があります。
また、処理の強制中断はないため、個別のスレッドで定期的に wait() を
行わないかぎりフリーズ状態となるので注意する必要があります。
※TJS同様、容易に busy loop を引き起こします

◇起動時スクリプト実行

完全に squirrel で吉里吉里を制御させる場合は、
以下のような startup.tjs を準備します。
startup.nut がスレッド起動され、それ以降 squirrel 
スレッドが無くなるまで動作を継続します。

-----------------------------------------------------------------------------
Plugins.link("squirrel.dll");
System.exitOnNoWindowStartup = false; // 起動時ウインドウ無し終了の抑制
System.exitOnWindowClose = false; // メインウインドウが閉じる時の終了の抑制

// メインループ登録
var prevTick = System.getTickCount();
function main(tick)
{
	if (Scripts.driveSQ(tick - prevTick) == 0) {
		// squirrel スレッドが全て終了したら止める
		System.terminate();
	}
	prevTick = tick;
}
System.addContinuousHandler(main);

// 引数を処理
var argc = 0;
var args = [];
var arg;
while ((arg = System.getArgument("-arg" + argc)) !== void) {
	args.add(arg);
	argc++;
}
// squirrel のスクリプトを起動
Scripts.forkStorageSQ("startup.nut", args*);
-----------------------------------------------------------------------------

※System.exitOnNoWindowStartup は リビジョン4577以降の吉里吉里でのみ使えます

●ライセンス

Squirrel は いわゆる zlib/libpngスタイルライセンスです。

Copyright (c) 2003-2009 Alberto Demichelis

This software is provided 'as-is', without any 
express or implied warranty. In no event will the 
authors be held liable for any damages arising from 
the use of this software.

Permission is granted to anyone to use this software 
for any purpose, including commercial applications, 
and to alter it and redistribute it freely, subject 
to the following restrictions:

		1. The origin of this software must not be 
		misrepresented; you must not claim that 
		you wrote the original software. If you 
		use this software in a product, an 
		acknowledgment in the product 
		documentation would be appreciated but is 
		not required.

		2. Altered source versions must be plainly 
		marked as such, and must not be 
		misrepresented as being the original 
		software.

		3. This notice may not be removed or 
		altered from any source distribution.
-----------------------------------------------------
END OF COPYRIGHT

このプラグイン自体のライセンスは吉里吉里本体に準拠してください。
