Title: messenger Plugin
Author: わたなべごう

●これはなに？

同一マシン上で起動している吉里吉里間での相互通信機能を提供します。
window message を経由して同報メッセージを送信します。
 
●使用方法

manual.tjs 参照

●外部アプリケーションからの制御例

storeHWND に "hwnd" を指定して、外部アプリから HWND を検知し、
そのウインドウに対して WM_COPYDATA を通知します。

ruby での制御例
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

ライセンスは吉里吉里本体に準拠してください。
