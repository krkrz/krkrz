Title: msgreceiver plugin
Author: わたなべごう

●これはなに？

※obsoluted plugin
  同等以上の機能をもつ messenger plugin が存在します
  そちらの利用を検討してください

吉里吉里に外部制御の口を追加します

Win32 API の SendMessage を使って WM_COPYDATA で
テキストを送ることができます。

●使い方

　(1) プラグインをロード

  (2) メッセージレシーバを登録

　　wmrStart(win);

　　指定したウインドウのウインドウメッセージ処理に割り込みして、
    WM_COPYDATA が送られた場合に、win.onCopyData(msg) {} が
　　呼び出されるようになります。
　　
　　制御用のハンドル情報は、実行ファイル名.hwnd というファイルが
　　生成されてそれが随時更新されるので、送信するプログラムは
　　このファイルを参照して SendMessage 用のハンドルを取得してください。

  (3) メッセージレシーバの解除 

    wmrStop(win)

    で受信処理が終了します。

　注意点：複数の窓を扱うことは配慮されてません

●サンプルの制御例（ruby から制御）

--------------------------------------------------------------------
exename = "krkr"
hwnd = open(exename + ".exe.hwnd").gets.to_i;

require 'dl/import'
require 'dl/struct'

module CopyData
	extend DL::Importable
	dlload 'user32'
	typealias "WPARAM", "UINT"
  	typealias "LPARAM", "UINT"
	WM_COPYDATA = 0x004A
	CopyData = struct [ 
		'ULONG dwData',
		'DWORD cbData',
		'PVOID lpData', 
	] 
	extern 'UINT SendMessage(HWND, UINT, WPARAM, LPARAM)'
end

msg = ARGV[0];

cd = CopyData::CopyData.malloc
cd.dwData = 0
cd.cbData = msg.size
cd.lpData = msg
CopyData::sendMessage(hwnd, CopyData::WM_COPYDATA, 0, cd.to_ptr.to_i)
--------------------------------------------------------------------

●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。
