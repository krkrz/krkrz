wuvorbisfile の使い方

■ 概要 -------------------------------------------------------------------

　wuvorbisfile は、吉里吉里用の OggVorbis デコーダ プラグインである
wuvorbis.dll を、一般のプログラムから使用可能にするためのインターフェース
ライブラリ (スタブライブラリ) です。wuvorbis.dll は SSE と 3DNow! 向けに最
適化された高性能の OggVorbis デコーダです。
　wuvorbis.dll では、通常の libvorbis (リファレンスライブラリ) を VC++ 6.0
で「実行速度」最適化においてコンパイルを行ったデコーダと比べ、
Pentium4 + SSE 使用時に 約 1.3 ～ 1.6 倍、K6-3 + 3DNow! 使用時に
約 1.4 ～ 1.7 倍の速度でデコードを行うことが出来ます。SSE や 3DNow! 非搭載
のプロセッサでもリファレンスライブラリよりは高速にデコードできます。


■ 動作環境 ---------------------------------------------------------------

　wuvorbis.dll は 486 未満の CPU (つまり i386) では動作しませんので注意が必
要ですが、現役の Windows マシンで i386 を使用しているコンピュータはまずない
と思います。wuvorbis.dll を使う場合は i386 を動作対象から外す必要がありま
す。


■ 対象開発環境 -----------------------------------------------------------

　wuvorbisfile は、Borland C++ Builder 5 および Visual C++ 6.0 で動作を確認
しています。おそらく 無償で提供されている Borland の C++ コンパイラや
Visual Studio .NET ではそのままつかえます。gcc でも多少の修正は必要かもしれ
ませんが、wuvorbisfile は wuvorbis.dll へのインターフェースを提供するだけで
すので、簡単に修正できると思います。


■ vorbisfile API ---------------------------------------------------------

　基本的に wuvorbisfile で使用可能になるのは本家の vorbisfile (Ogg Vorbis を
簡単に扱うためのライブラリ) の API と同じです。vorbisfile については

http://www.xiph.org/ogg/vorbis/doc/vorbisfile/

　などを参照してください。

　ただし、以下の注意点があります。

・定数や関数名、構造体名の前に "WU_" または "wu_" のプリフィクスがついてい
  ます。
・FILE 構造体を受け渡しするような API は使えません。wu_ov_open_callbacks を
  使用してファイルを開く必要があります。
・wu_ov_read は、bigendianp=0、sgned=1、word は 2 (16bit) あるいは
  4 (float) のみが使えます。float による読み出しは (ov_read の機能としては)
  本家のライブラリにはない機能です。

　wuvorbisfile は、本家の libvorbis/libogg ライブラリのヘッダファイルに依存
していません。つまり、本家のヘッダファイルを使用することなく、各機能を使用
できます。


■ WuVorbisInit -----------------------------------------------------------

　WuVorbisInit は、wuvorbis.dll を読み込み、各関数を使用可能にします。wu_*
各関数を使用する前に必ず呼び出す必要があります。

・定義

  int WuVorbisInit(const char *dll_file_name_can_be_null);

・引数
  dll_file_name_can_be_null
    　wuvorbis.dll のファイル名を表すゼロ終結文字列へのポインタです。NULL を
    指定するとデフォルトの検索パス上の wuvorbisfile.dll が使われます。

・戻り値
  0 : 正常
  1 : DLL が見つからないか読み込めない
  2 : DLL は読み込めたが関数のインポートに失敗した、あるいは読み込んだ DLL は
      wuvorbis.dll ではない
  3 : DLL と wuvorbisfile の間で不整合が発生した


■ WuVorbisUninit ---------------------------------------------------------

　WuVorbisUninit は、wuvorbis.dll を解放します。

・定義

  int WuVorbisUninit(void);

・戻り値
  0 : 正常
  1 : 異常

■ wu_DetectCPU -----------------------------------------------------------

　wu_DetectCPU は、CPU の種類や、CPU で利用可能な各機能を検出します。

・定義

  unsigned __int32 wu_DetectCPU(void)

・戻り値
  CPU 機能フラグ

  　CPU 機能フラグは以下の値のビットごとの論理和です。ただし例外として 586
  未満の CPU、あるいは 586 の名が付いていても中身が 486 相当の一部の CPU の
  場合は 0 が戻ります。

  #define WU_CPU_HAS_FPU  0x000010000
  #define WU_CPU_HAS_MMX  0x000020000
  #define WU_CPU_HAS_3DN  0x000040000
  #define WU_CPU_HAS_SSE  0x000080000
  #define WU_CPU_HAS_CMOV 0x000100000
  #define WU_CPU_HAS_E3DN 0x000200000
  #define WU_CPU_HAS_EMMX 0x000400000
  #define WU_CPU_HAS_SSE2 0x000800000
  #define WU_CPU_HAS_TSC  0x001000000

  　たとえば、CPU が SSE を持っている場合は、WU_CPU_HAS_SSE の表すビットが
  1 になっています。

  　WU_CPU_VENDOR_MASK とのビットごとの論理積をとると、CPU のベンダーを得る
  ことが出来ます。ベンダーは以下のように定義されています。

  #define WU_CPU_IS_INTEL     0x000000010
  #define WU_CPU_IS_AMD       0x000000020
  #define WU_CPU_IS_IDT       0x000000030
  #define WU_CPU_IS_CYRIX     0x000000040
  #define WU_CPU_IS_NEXGEN    0x000000050
  #define WU_CPU_IS_RISE      0x000000060
  #define WU_CPU_IS_UMC       0x000000070
  #define WU_CPU_IS_TRANSMETA 0x000000080
  #define WU_CPU_IS_UNKNOWN   0x000000000

  　たとえば、(戻り値 & WU_CPU_VENDOR_MASK) が WU_CPU_IS_INTEL ならば、その
  CPU は Intel 製です。


■ wu_SetCPUType ----------------------------------------------------------

　wu_SetCPUType は、wuvorbis.dll が使用する CPU 機能を指定します。この関数
を呼び出さない場合は SSE などの CPU 拡張機能を使わない動作になります。
　通常は、wu_DetectCPU の戻り値をそのままこの関数の引数に指定することで、
CPU に応じた機能を自動的に使用することが出来ます。
　この関数は、通常、WuVorbisInit で DLL を読み込んだ直後に呼び出します。

・定義

  void wu_SetCPUType(unsigned __int32 type);

・引数
  type
    　CPU 機能フラグを指定します。wu_setCPUType は、ここに指定された値のう
    ち、現バージョンでは以下のビットしか参照しません。他のビットは無視しま
    す (将来的には他のビットも参照するようになる可能性があります)。

    #define WU_CPU_HAS_MMX 0x000020000
    #define WU_CPU_HAS_3DN 0x000040000
    #define WU_CPU_HAS_SSE 0x000080000


■ wu_ScaleOutput ----------------------------------------------------------

　wu_ScaleOutput は、出力の増幅率を指定します。この関数で指定した増幅率が乗
算された出力を得ることが出来ます。
　この関数は２回以上呼び出した場合、２回目以降の増幅率の指定は、以前に指定
した増幅率に掛け合わされる形で設定されます。たとえば、

    wu_ScaleOutput(0.5f);
    wu_ScaleOutput(4.0f);

　とすると、最終的な増幅率は 0.5 * 4.0 つまり 2.0 になります。
　この関数を何回も呼び出すと、呼び出すごとに誤差が蓄積される可能性があり、
好ましくありません。

　wu_ov_read で float 形式の PCM を得るとき、そのままでは 上限が 32768.0
で、下限が -32768.0 の PCM が得られます。float の PCM として一般的な値の範
囲である -1.0 ～ 0 ～ 1.0 の範囲にするためには、この関数の引数に
1.0 / 32768.0 を指定する必要があります。以下のようになります。

    wu_ScaleOutput((float)(1.0 / 32768.0));

　この関数は、デコードを何も行っていない状態で呼び出してください。デコード
途中でこの関数を呼び出すと再生状態が異常になる可能性があります。従ってフェー
ドイン・フェードアウトの用途には使用しないでください。
　また、この関数はデコード中のすべてのインスタンスに影響します。

・定義

  void  wu_ScaleOutput(float scale);

・引数
  scale
    　出力の増幅率を倍数で指定します。dB やパーセント指定ではありません。
    1.0f を指定すると増幅率は以前のままとなります。


■ 使い方 -----------------------------------------------------------------

　まず、lib/wuvorbisfile.c をプロジェクトに追加してください。
　include/wuvorbisfile.h にはインクルード パスを通してください。

　DLL を使うためには WuVorbisInit を呼び出します。通常、その直後
wu_SetCPUType(wu_DetectCPU()); を呼び出します。

　使い方のサンプルは test ディレクトリにあります。このサンプルは、OggVorbis
ファイルをデコードし、 raw PCM ファイルに出力する物です。


■ 注意 -------------------------------------------------------------------

　スレッドセーフですが、同じ wu_OggVorbis_File オブジェクトに対して API を並
列して呼び出して良いというわけではありません。同じ wu_OggVorbis_File オブ
ジェクトに対しては、API の呼び出しは前の API の呼び出しが終わってからにする
必要があります。異なるオブジェクトに対して API を並列して呼び出すことは出来
ます。

　なるべく、wuvorbis.dll はそれと同時期の wuvorbisfile を使用してください。
さもないと、バージョン間の不整合が発生する可能性があります。

　wu_ov_read による float 形式の PCM はクリッピングを行いません。つまり、値
の範囲を -1.0 ～ 0 ～ 1.0に期待したとしても、波形のピークがこの範囲をはみ出
る可能性があります。


　最新の wuvorbis.dll は吉里吉里２ SDK の

/kirikiri2/plugin/

　wuvorbisfile は吉里吉里２のソース配布ファイルの

/environ/win32/wuvorbis/decapi/

　にあります。


■ ライセンス -------------------------------------------------------------

　wuvorbis.dll (デコーダ DLL) と wuvorbisfile (スタブライブラリ) は、
wuvorbis.dll のもととなっている libvorbis / libogg のライセンスと同じく、
BSD ライクなライセンスということにします。つまり、無保証・無責任です。
　wuvorbis.dll を使うソフトウェアが有償配布であれ無償配布であれ、また、オー
プンソースであれクローズドソースであれ、無償で使用できます。
wuvorbis.dll を使うソフトウェアのソースを公開する必要はありません。
　どの場合も、以下のライセンス表記をよくお読みになってご使用ください。

　使用報告の義務はありませんが、報告してくださっても結構です。

　libvorbis / libogg のライセンスは、以下のライセンス表記をドキュメント中な
ど配布物に記述することを要求しています。しかし、wuvobis.dll にはその
バージョン情報にこの表記を含めてあるので、改めてドキュメントなどでこの表記
を記述する必要はない物と思われます。心配な方は別途記述してください。


Copyright (c) 2002, Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


　追記で、wuvorbis.dll あるいは SSE/3DNow! パッチの著作権情報ですが、

SSE & 3DNow! patch Copyright (c) 2002-2005, W.Dee

　の表記をドキュメント中などの配布物に記述することが好ましいです。だたし、
これも wuvorbis.dll のバージョン情報に含まれているので、別途、ドキュメント
中などでこの表記を記述する必要はありません。

　wuvorbis.dll のソースや SSE/3DNow! パッチは吉里吉里２のソース配布ファイル
中に含まれています。


■ 更新履歴 ---------------------------------------------------------------

・2003/07/5

　Query_sizeof_OggVorbis_File のグローバル定義が正しく利用できなかったのを
　修正。

・2003/06/23

　wu_OggVorbis_File 構造体に後方互換性用の余裕を持たせた。
　wu_ScaleOutput 関数追加。


■ 作者連絡先 -------------------------------------------------------------

   W.Dee <dee@kikyou.info>
