●これはなに？

Direct3D ベースで動作する吉里吉里 drawdevice プラグインです。
複数の primaryLayer を合成して表示できます。

それぞれの primaryLayer は指定された領域に
スケーリング表示されるため、個別に解像度を変更できます。

●使い方

1. Window の drawDevice に対して指定可能です

-------------------------------------------
Plugins.link("drawdeviceD3D.dll");
var WIDTH=800;
var HEIGHT=600;
class MyWindow extends Window {
  var base;
  var base2;
  function MyWindow() {
    super.Window();
    setInnerSize(WIDTH, HEIGHT);
    // drawdevice を差し替え
    drawDevice = new DrawDeviceD3D(WIDTH,HEIGHT);
     // プライマリレイヤ生成
    base = new Layer(this,null);
    base.setSize(WIDTH,HEIGHT);
    base2 = new Layer(this,null);
    base2.setSize(WIDTH/2,HEIGHT/2); // 解像度半分
    add(base);
  }
};
-------------------------------------------

機能については manual.tjs を参照してください

●めも

とりあえず標準の PassThroughDrawDevice のコードを元に作業したので
Direct3D7 ベース。気が向いたら D3D9 で書き直し？

●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。
