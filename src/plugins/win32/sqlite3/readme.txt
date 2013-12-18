Title: sqlite plugin
Author: わたなべごう

●これはなに？

組み込みSQLデータベースエンジン sqlite (http://www.sqlite.org) 
の吉里吉里バインドです。

●使い方

・単純な使い方の場合は Sqlite オブジェクトを使います。
・ステート処理を行いたい場合は SqliteStatement オブジェクトを使います。
・別スレッドで処理させたい場合は SqliteThread オブジェクトを使います。

詳細は manual.tjs を参照してください。

●SQL拡張

 cnt(a,b)   文字列 a に b が含まれれば真
 ncnt(a,b)  正規化された文字列 a に 正規化された文字列 b が含まれてれば真

●ライセンス

sqlite は public domain です。
このプラグイン自体のライセンスは吉里吉里本体に準拠してください。
