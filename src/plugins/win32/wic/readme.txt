Title: WIC plugin
Author: Takenori Imoto

●これはなに？

Windows Imaging Component (WIC) を使って画像の読み書きを行うプラグインです。

このプラグインを読み込むとGIF/ICO/TIFFの読み込みと、TIFFの書き出しに対応する。

WIC では、BMP/JPEG/PNG/HD Photo(JPEG XR) も対応しているが、こちらは本体の機能で読み書きする。
DDS も対応しているが、Windows 8.1 以降なので現在のところこのプラグインではサポートしていない。


●使い方
Plugins.link("krwic.dll");
と記述すれば、自動的にサポートする画像フォーマットが増える。


●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。

