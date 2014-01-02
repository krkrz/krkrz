●これはなに？

GDI を用いたダブルバッファリングを行う DrawDevice です。
DrawDevice のベースクラス提供と実装のサンプルも兼ねています。
ClipRect の処理を行っていないので、プライマリーレイヤーがウィンドウサイズより大きい場合など綺麗にクリッピングされないなど問題があると思います。
ゲーム用ではなく、どちらかというとツール用ですが、Vista 以降通常の描画も DirectX で行われている現在の環境では余り意味はないかもしれません。



●使い方

1. Window の drawDevice に対して指定可能です

仮想コードで書くとインターフェイスは以下のようになっています。
class GDIDrawDevice {
	property interface // read only
};

以下のようにして使用します。
-------------------------------------------
Plugins.link("gdidrawdevice.dll");
class MyWindow extends Window {
  var base;
  function MyWindow(width,height) {
    super.Window();
    setInnerSize(width, height);
    // drawdevice を差し替え
    drawDevice = new GDIDrawDevice();
     // プライマリレイヤ生成
    base = new Layer(this,null);
    base.setSize(width,height);
    add(base);
  }
};
-------------------------------------------



●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠(修正BSD)してください。

