●これはなに？

吉里吉里から Irrlicht を取り扱うためのプラグインです。

●動作上の特記事項

・DirectX9 専用で構築されています。コンパイルには DirectX9 SDK が必要です。

  ※DirectX9 が使えない場合は drawdevice 設定時に例外がおこります

・Irrlicht からのファイルアクセスは吉里吉里のファイル空間に対して行われます

●使い方

1. Window の drawDevice に対して指定可能です

-------------------------------------------
Plugins.link("krrlicht.dll");
var WIDTH=800;
var HEIGHT=600;
class MyWindow extends Window {
  var base;
  function MyWindow() {
    super.Window();
    setInnerSize(WIDTH, HEIGHT);
    // drawdevice を差し替え
    drawDevice = new Irrlicht.DrawDevice(WIDTH,HEIGHT);
     // プライマリレイヤ生成
    base = new Layer(this,null);
    base.setSize(WIDTH,HEIGHT);
    add(base);
  }
};
-------------------------------------------

2. レイヤに対して描画が実行できます

-------------------------------------------
var win = new Window();
win.visible = true;
var irr = new Irrlicht.SimpleDevice(win, 100, 100);
var layer = new Layer(win, null);

// XXX 一度イベントループに入ってデバイスが実体化してから実行する必要あり
irr.updateToLayer(layer);
-------------------------------------------

3. 子ウインドウとして Irrlicht を配置できます

-------------------------------------------
var win = new Window();
win.visible = true;
var irr = new Irrlicht.Window(win, 10, 10, 100, 100);
irr.visible = true;
-------------------------------------------

それぞれの機能については manual.tjs を参照してください

●今後の予定

・シーン処理系の実装
・もろもろシーン制御処理
・各種プリミティブ操作の実装

・Irrlicht 管理下でのムービー再生処理の実装
　※できれば krmovie とうまく連携できるようなと理想だけど…

●ライセンス

Irrlicht は zlib/libpng スタイルのライセンスです。

  The Irrlicht Engine License
  ===========================

  Copyright (C) 2002-2009 Nikolaus Gebhardt

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgement in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be clearly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

このプラグイン自体のライセンスは吉里吉里本体に準拠してください。
