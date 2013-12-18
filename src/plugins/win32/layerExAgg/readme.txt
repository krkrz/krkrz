Title: csvparser plugin
Author: わたなべごう

●これはなに？

吉里吉里で Agg を使う試み

●コンパイル方法

このフォルダのファイルのコンパイルには以下のライブラリが必要になります

・libexpat 2.0.0  ( 1.95.8 も可 )

  $(EXPAT_HOME) として配置。
  ※標準のバイナリパッケージをインストールするとここに入ります

  スタティックリンクするようにしてあります。
　ダイナミックリンクするには、
　・ライブラリ参照フォルダを $(EXPAT_HOME)\Libs に変更
　・リンクするライブラリ指定を libexpatMT.lib から libexpat.lib に変更
　・プリプロセッサでの XML_STATIC の定義を解除
　とすればOKです。実行時に libexpat.dll が吉里吉里実行ファイルフォルダに
  ないと、アクセス例外をおこします。

　1.95.8 を使う場合はプロジェクトの設定でインクルードファイルのディレク
　トリとライブラリのディレクトリを書き換えてください。

・AGG 2.3

  Anti Grain Geometry 

　(1) カレントフォルダに agg23/ として配置します
　   ※標準のWIN用配布を展開しただけのものです

  (2) agg23.diff を patch あて
     SVG のリーダを IStream に対応させるためのパッチです

●ライセンス

Agg 2.3 は Agg 独自ライセンスで配布されています

The Anti-Grain Geometry Project
A high quality rendering engine for C++
http://antigrain.com

Anti-Grain Geometry - Version 2.3 
Copyright (C) 2002-2005 Maxim Shemanarev (McSeem) 

Permission to copy, use, modify, sell and distribute this software 
is granted provided this copyright notice appears in all copies. 
This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.

このプラグイン自体のライセンスは吉里吉里本体に準拠してください。
