Title: SystemEx Plugin
Author: わたなべごう/miahmie/Kiyobee

●これはなに？

System に細かい機能を足すプラグインです

※注意事項
writeRegValue は registory プラグインのそれと同一です
readEnvValue/expandEnvString は windowEx プラグインのそれと同一です

writeEnvValueはプロセス環境変数を変更するだけです。
ユーザー環境変数やシステム環境変数を変更したい場合はwriteRegValueで適切なレジストリに値を設定してください。
TODO: WM_SETTINGCHANGE メッセージをブロードキャストするような手段を提供する必要あり


●使用方法

manual.tjs 参照

●ライセンス

ライセンスは吉里吉里本体に準拠してください。
