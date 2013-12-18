●これはなに？

OGRE ベースの drawdevice 実装のテストコードです

●コンパイル方法

コンパイル時に設定しておく環境変数

 OGRE_HOME  OGRE の SDK が展開されてるフォルダ名

●テスト方法

(1) コンパイルすると drawdeviceirrlicht.dll が
    $(OGRE_HOME)\bin\$(ConfigurationName) に配置される

(2) $(OGRE_HOME)\bin\$(ConfigurationName)に以下を配置

  krkr.exe
  sample → data にリネーム

(3) 該当フォルダで krkr.exe を実行

　※構成プロパティのデバッグに以下を指定すれば良いです

  コマンド        : $(Outdir)\krkr.exe
  作業ディレクトリ: $(Outdir)

●注意点

　TJS 側にデータを読み込む口がまだないため、
　サンプルが強制的に dll 中に埋め込まれてます。
　データは SDK のファイルを相対パスで参照しています。
