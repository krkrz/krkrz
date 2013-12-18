Title: layerExBTOA
Author: わたなべごう

●これはなに？

レイヤのα領域や Province画像をいじるメソッドを集めたものです

●使い方

各メソッドについては manual.tjs 参照

α動画用に使う場合は、VideoOverlay クラスを使ってレイヤに動画(右半分にα画像)
を描画したあと、onFrameUpdate() で copyRightBlueToLeftAlpha() を
呼び出してください。処理は内部画像データ(imageWidthのサイズ)に対して行われます。

描画先レイヤの width は VideoOverlay クラスによって動画のサイズに
拡張されてるので、このタイミングで半分に再調整してください。

例１
class AlphaVideo extends VideoOverlay
{
  function AlphaVideo(window) {
    super.VideoOverlay(window);
    mode = vomLayer;
  }

  function onFrameUpdate(frame) {
    if (layer1) {
      layer1.width = layer1.imageWidth / 2;
      layer1.copyRightBlueToLeftAlpha();
    }
  }
}

例２
Movie.tjs をアルファムービー対応に改造したサンプルです。
変更点は Movie.patch を参照してください。

video タグに alphatype のオプションが拡張されます。

alphatype=0 で アルファを使用しない通常のモード
alphatype=1 で copyRightBlueToLeftAlpha を使用（右側にアルファ）
alphatype=2 で copyBottomBlueToTopAlpha を使用（下側にアルファ）

アルファムービーは mode=layer でしか機能しないことにご注意ください。
また、image タグの mode 属性で、あらかじめ対象のレイヤの透過モードを
変更しておかないと、正しくアルファが出ない場合があります。


●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。
