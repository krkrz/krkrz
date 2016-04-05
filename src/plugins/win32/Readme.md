# プラグイン
windows 用のプラグインフォルダです。  
今のところ 32bit 専用のものが多いです。  
使い方などは各フォルダにある readme.txt や manual.tjs を参照してください。

## ビルドスクリプト
build.bat を実行すると添付されているプラグインを VisualC++ 2012 でまとめてビルドできます。  
plugins.msbuild は MSBuildスクリプトです。ここでビルドが難しいものは BAT ファイルにソリューションファイルで指定しています。  
postbuild.msbuild は、全てのビルドが終わった後に dll 以外の余計なものを plugin フォルダから削除します。  
ビルドするものを追加する場合は、基本的には plugins.msbuild に追加してください。  
ライブラリなどのパスが複雑でソリューションファイル位置を基準にしているものなどは、build.bat の postbuild.msbuild の前に追記してください。  

## extrans (32bit/64bit)
トランジションの種類を追加します。

## KAGParser (32bit/64bit)
KAG3 スクリプトパーサーです。

## menu (32bit/64bit)
メニューを追加します。

## opus (32bit/64bit)
音声 Codec に opus を追加します。

## theora (32bit/64bit)
動画 Codec に theora を追加します。

## wic (32bit/64bit)
画像の読み書きを Windows Imaging Component (WIC) を使って行います。
GIF/ICO/TIFF の読み込みと、TIFF の書き出しに対応しています。

## wuvorbis (32bit/64bit)
音声 Codec に vorbis を追加します。

