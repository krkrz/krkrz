//---------------------------------------------------------------------------
/*
	このソースでは、xp3enc.dll を使って Releaser で暗号化した XP3 ファイルを
	扱えるようにする吉里吉里用プラグインを説明します。

	プラグインのファイル名は、拡張子が tpm ならば任意の物でかまいません。
	吉里吉里は拡張子が tpm のファイルを見つけると startup.tjs を読み込もうと
	する前、すなわち XP3 ファイルが最初に開かれる前に読み込むので、拡張子は
	tpm である必要があります。
	拡張子が tpm ならばファイル名は(プラグイン間の依存性がなければ)なんでも
	かまいませんが、ここでは便宜上、xp3dec.tpm とします。

	xp3enc.dll と異なり、xp3dec.tpm は吉里吉里本体のプラグインとして動作しま
	す。そのため、吉里吉里本体用のプラグインの作法に従って作成されなければな
	らなりませんが、tp_stub.h を介して利用可能になる、吉里吉里本体の各機能を
	利用することができます。ただし、TVPSetXP3ArchiveExtractionFilter は複数
	スレッドから同時に呼び出される可能性がありますが、tp_stub.h を介して利用
	可能になる関数はほとんどがメインスレッドからの呼び出しのみに対応している
	ので注意してください。

	暗号方式に関する諸注意について、 xp3enc.dll のサンプルのソース main.cpp
	も参照してください。
*/





//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
	// tp_stub.h にはインクルード・パスを指定しておきます。
	// tp_stub.cpp および tp_stub.h のバージョンは吉里吉里本体と
	// 同時期の物であることが好ましいです。
	// (もちろん本体側のインターフェースが変わらなければ、古い
	// xp3dec.dll でも継続して利用できます)
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void TVP_tTVPXP3ArchiveExtractionFilter_CONVENTION
	TVPXP3ArchiveExtractionFilter(tTVPXP3ExtractionFilterInfo *info)
{
	// TVPXP3ArchiveExtractionFilter 関数は本体側から呼び出される
	// コールバック関数です。
	// 引数を一つ取り、それは tTVPXP3ExtractionFilterInfo 構造体へのポインタ
	// です。

	// TVPXP3ArchiveExtractionFilter は、後述の V2Link 関数内で
	// TVPSetXP3ArchiveExtractionFilter により設定されます。

	// ここでは単純に、xp3enc.dll のサンプルで作成された XP3 アーカイブを
	// 復号すべく、データをすべて FileHash の最下位バイトで XOR
	// することにします。

	// この関数は複数のスレッドから同時に呼び出される可能性があるので
	// 注意してください。

	/*
		tTVPXP3ExtractionFilterInfo のメンバは以下の通り
		* SizeOfSelf        : 自分自身の構造体のサイズ
		* Offset            : "Buffer" メンバが指し示すデータが、
		*                   : アーカイブに格納されているそのファイルの先頭からの
		*                   : どのオフセット位置からのものか、を表す
		* Buffer            : データ本体
		* BufferSize        : "Buffer" メンバの指し示すデータのサイズ(バイト単位)
		* FileHash          : ファイルの暗号化解除状態でのファイル内容の32bitハッシュ
	*/

	// 一応構造体のサイズをチェックする
	if(info->SizeOfSelf != sizeof(tTVPXP3ExtractionFilterInfo))
	{
		// 構造体のサイズが違う場合はここでエラーにした方がよい
		TVPThrowExceptionMessage(TJS_W("Incompatible tTVPXP3ExtractionFilterInfo size"));
			// TVPThrowExceptionMessage は例外メッセージを投げる関数
			// この関数は戻らない ( もっと呼び出し元をさかのぼった位置で
			// 例外が補足されるため )
	}

	// 復号
	tjs_uint i;
	for(i = 0; i < info->BufferSize; i++)
		((unsigned char *)info->Buffer)[i] ^= info->FileHash;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// V2Link は、吉里吉里本体とリンクされるときに呼び出される関数です
	
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// TVPSetXP3ArchiveExtractionFilter 関数に
	// TVPXP3ArchiveExtractionFilter を指定し、コールバック関数を設定する
	TVPSetXP3ArchiveExtractionFilter(TVPXP3ArchiveExtractionFilter);

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall V2Unlink()
{
	// V2Unlink は、吉里吉里本体から切り離されるときに呼び出される関数です

	// 一応 TVPSetXP3ArchiveExtractionFilter に NULL を渡し、
	// コールバック関数の登録を解除する
	TVPSetXP3ArchiveExtractionFilter(NULL);

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK; 
}
//---------------------------------------------------------------------------



