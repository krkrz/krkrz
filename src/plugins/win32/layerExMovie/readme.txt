Title: layerExMovie plugin
Author: わたなべごう

●これはなに？

Layer クラスに動画再生機能拡張を行うプラグインです。
VideoOverlay クラスとは別機構の動画再生になります

画面合成タイミングにあわせた動画更新を行うので、
fpslimit プラグインなどによる負荷調整が可能になります。
小さいサイズの効果ムービーを再生するための機構です。
通常のムービー再生には VideoOverlay を利用したほうが
良いでしょう

●コンパイル方法

 DirectX Extras または Platform SDK から、DirectShow の
 サンプルの、BaseClasses をもってくる必要があります。
 プロジェクトファイルは strmbase.vcproj として準備して
 あるので、BaseClasses の内容のみもってきてください。

 コンパイルが通らず、適宜ソース変更が必要な場合があります。

●使い方

manual.tjs 参照

●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。
