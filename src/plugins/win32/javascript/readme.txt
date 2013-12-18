Title: Javascript Plugin
Author: わたなべごう

●これはなに？

V8 JavaScript Engine (http://code.google.com/p/v8/) の吉里吉里バインドです。

●コンパイル方法

(1) このフォルダ直下に、V8 Javascript Engine を "v8" という名前でチェックアウト
(2) VisualStudio 用にコンパイルして v8/v8.lib, v8/v8_g.lib を準備
(3) javascript.sln を開いてコンパイル

●システム概要

操作の詳細は manual.tjs / manual.js を参照してください。

◇名前空間

・Javascript のグローバル空間は吉里吉里全体に対して１つだけ存在します。
　
　Javascript 用のスクリプトの実行はこのグローバル空間上でおこなわれ、
　定義されたファンクションやクラスもこのグローバル空間に登録されていきます。

・TJS2 のグローバル空間を Javascript 側から "krkr" で参照できます。

・Javascript のグローバル空間を TJS2 側から "jsglobal" で参照できます。

・整数、実数、文字列などのプリミティブ値は値渡しになります。
・TJS2 の void は javascript の undefined と対応します
・TJS2 の null は javascript の null と対応します
・オブジェクトは相互に呼び出し可能です

◇Scripts 拡張

Javascript の実行用メソッドが Scripts クラスに拡張されます。
これにより外部の Javascript ファイルを読み込んで実行可能になります

◇吉里吉里クラスの javascriptプロトタイプクラス化

吉里吉里のクラスを javascript のプロトタイプクラスとして
継承可能な状態で扱うことができます。

・createTJSClass()で、TJSのクラスを内部的に保持する 
　Javascriptクラス生成関数を作成することができます。

  tjsOverride() でTJSインスタンスに対して直接 javascript メソッドを
  登録できます

  TJSインスタンス側に callJS() として javascript インスタンスの
　メソッドを明示的に呼び出す命令が拡張されます。

  TJSインスタンス側では missing 機能が設定され、存在しないメンバが
　参照された場合は javascript インスタンスの同名メンバが参照されます。
  TJSインスタンス内部からのイベント呼び出しにもこれが適用されるため、
　TJSインスタンス中に定義がなければ自動的に javascript インスタンスの
　それが呼び出されます

  ※squirrel 実装とは異なりこの機能で取得した吉里吉里クラスの
  吉里吉里側で生成されたインスタンスが返される場合のラッピング処理は
　行われませんのでご注意ください

◇デバッガ機能

  enableDebugJS() でデバッガを有効にすると、
　外部からの TCP/IP接続でリモートデバッグ可能になります。
　※リモートデバッガとして V8 付属の d8.exe が使えます。

　デバッガによって Javascript の実行が中断している間は吉里吉里全体が
　停止状態になり、画面の更新やイベント処理なども停止するので注意が必要です。

　逆に Javascript 実行状態でない間は、そのままではデバッガに対する
　応答が帰りません。TJS 側で、processDebugJS() を定期的に呼び出して
　通信を処理させる必要があるので注意してください。

●ライセンス

このプラグイン自体のライセンスは吉里吉里本体に準拠してください。

This license applies to all parts of V8 that are not externally
maintained libraries.  The externally maintained libraries used by V8
are:

  - Dtoa, located under third_party/dtoa.  This code is copyrighted by
    David M. Gay and released under an MIT license.

  - Strongtalk assembler, the basis of the files assembler-arm-inl.h,
    assembler-arm.cc, assembler-arm.h, assembler-ia32-inl.h,
    assembler-ia32.cc, assembler-ia32.h, assembler.cc and assembler.h.
    This code is copyrighted by Sun Microsystems Inc. and released
    under a 3-clause BSD license.

  - Valgrind client API header, located at third_party/valgrind/valgrind.h
    This is release under the BSD license.

These libraries have their own licenses; we recommend you read them,
as their terms may differ from the terms below.

Copyright 2006-2009, Google Inc. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Google Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
