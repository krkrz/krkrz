Win/Mac は大文字小文字区別しない
Linux/Android はする



自動検索パスへの追加
Storages.addAutoPath -> TVPAddAutoPath
正規化後 TVPAutoPathList 追加する。
ディレクトリ名が正規化されると困るので、ペアを保持する必要がある

ストレージ名の拡張子の切り落とし
Storages.chopStorageExt

ストレージ名の拡張子の抽出(ファイル拡張子)
Storages.extractStorageExt

ストレージ名の抽出(ファイル名)
Storages.extractStorageName

ストレージ名のパスの抽出(ディレクトリ名)
Storages.extractStoragePath

完全な統一ストレージ名の取得
Storages.getFullPath

ローカルファイル名の取得
Storages.getLocalName

ストレージの検索
Storages.getPlacedPath

ストレージの存在確認
Storages.isExistentStorage -> TVPIsExistentStorage
1. TVPAutoPathCache から検索
2. 正規化後、アーカイブならその中から、それ以外はTVPStorageMediaManager.CheckExistentStorageで調べる
(2.5 Win/Mac以外の場合は、正規化せずに検索する必要がある、見付かったらそのままの名前を保持)
3. ファイル名で TVPAutoPathTable から検索

自動検索パスの削除
Storages.removeAutoPath

CD の検索 (ドライブレターを返す実装なのでWindows固有)
Storages.searchCD

ファイル選択ダイアログボックスを表示
Storages.selectFile


各種ファイル読み込み
Scripts.evalStorage/execStorage/Layer.loadImages/WaveSoundBuffer.open/VideoOverlay.open/
画像、音声、動画


自動検索パスに追加されたディレクトリは、tTVPFileMedia::GetListAt によって一覧取得されるので、
この中で正規化前の名前と正規化後の名前のペアをtTVPFileMediaに持たせる必要がある。

TVP_NATIVE_STORAGE_IS_CASE_SENSITIVE
