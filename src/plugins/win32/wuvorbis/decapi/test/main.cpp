//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdio.h>
#include "wuvorbisfile.h"



//---------------------------------------------------------------------------
// 各コールバック関数の準備
//---------------------------------------------------------------------------
static size_t _cdecl cb_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}
//---------------------------------------------------------------------------
static int _cdecl cb_seek_func(void *datasource, wu_ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}
//---------------------------------------------------------------------------
static int _cdecl cb_close_func(void *datasource)
{
	return fclose((FILE*)datasource);
}
//---------------------------------------------------------------------------
static long _cdecl cb_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// main 関数
//---------------------------------------------------------------------------
#pragma argsused
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	// 引数チェック
	if(argc < 3)
	{
		fprintf(stderr, "Usage: wu_decapi_test <input_vorbis_stream.ogg> <output_raw_pcm_file> [<path_to_wuvorbis.dll>]\n");
	}

	const char *wuvorbis_dll = argc >= 4 ? argv[3] : NULL;

	// WuVorbis を初期化
	// WuVorbisInit は wuvorbis.dll のファイル名で、NULL ならば
	// デフォルトの検索パス上のファイルが使われる (LoadLibrary("wuborbis.dll") )
	// WuvVorbisInit を呼び出さないと、wu_ が頭につく関数の呼び出しはすべて
	// 失敗するので注意。
	if(WuVorbisInit(wuvorbis_dll))
	{
		fprintf(stderr, "Cannot load or incorrect wuvorbis.dll\n");
		return 3;
	}

	// CPU チェックと CPU 使用機能の設定
	// wu_DetectCPU は CPU 機能を取得するために使う。
	// 通常、wu_DetectCPU の戻り値をそのまま wu_SetCPUType に渡して
	// 使う。wu_SetCPUType を使わないと、SSE や 3DNow! などの CPU 拡張機能は
	// まったく使われないので注意する。
	// wu_SetCPUType がみるビットは、wu_DetectCPU が返す値のうち
	// #define WU_CPU_HAS_MMX 0x000020000
	// #define WU_CPU_HAS_3DN 0x000040000
	// #define WU_CPU_HAS_SSE 0x000080000
	// の３つだけである。
	unsigned __int32 cputype = wu_DetectCPU();
	wu_SetCPUType(cputype);

	// ファイルを開く
	FILE *file = fopen(argv[1], "rb");
	if(!file)
	{
		// ファイルを開けない
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 3;
	}

	FILE *outfile = fopen(argv[2], "wb");
	if(!outfile)
	{
		// ファイルを開けない
		fclose(file);
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s\n", argv[2]);
		return 3;
	}

	// コールバック関数のセットアップ
	wu_ov_callbacks callbacks = {cb_read_func, cb_seek_func, cb_close_func, cb_tell_func};

	// wu_og_open_callbacks を使用して開く
	wu_OggVorbis_File ovfile;

	if(wu_ov_open_callbacks((void*)file, &ovfile, NULL, 0, callbacks) < 0)
	{
		// ファイルを正常に開けない
		fclose(file);
		fclose(outfile);
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s (in ov_open_callbacks)\n", argv[1]);
		return 3;
	}

	// 情報を表示
	wu_vorbis_info *info = wu_ov_info(&ovfile, -1);

	if(!info)
	{
		wu_ov_clear(&ovfile);
		fclose(outfile);
		WuVorbisUninit();
		fprintf(stderr, "ov_info failed\n", argv[1]);
		return 3;
	}

	fprintf(stderr,
		"version   : %d\n"
		"channels  : %d\n"
		"rate      : %d\n"
		"bitrate_upper   : %d\n"
		"bitrate_nominal : %d\n"
		"bitrate_lower   : %d\n"
		"bitrate_window  : %d\n",
		info->version, info->channels, info->rate, info->bitrate_upper,
		info->bitrate_nominal, info->bitrate_lower, info->bitrate_window);

	// デコードしてみる
	// このテストプログラムが出力するデコード結果は raw pcm なので注意
	int section = -1;
	while(true)
	{
		char buf[1024];
		// wu_ov_read の bigendianp は 0、
		// word は 2(16bit signed integer) か 4 (32bit float) 、
		// signed は 1 の必要がある
		int res = wu_ov_read(&ovfile, buf, 1024, 0, 2, 1, &section);
		// wu_ov_read は要求されたサイズを満たさずに帰る場合があるので注意。
		// 0 が戻ればデコード終了、正の数が戻れば
		// デコード成功(戻り値は書き込まれたバイト数)、
		// 負の数が戻った場合はパケット不足でデコードできない等。
		//  (ネットワークからのストリーミングとかでなくてもこの状況は発生する)
		// 負の数が戻った場合は、読み取れるようになるまで
		// そのまま wu_ov_read を繰り返せばよい

		if(res == 0) break; // デコード終了
		if(res > 0)	fwrite(buf, 1, res, outfile);
	}

	// wu_ov_clear で ovfile 構造体をクリア
	wu_ov_clear(&ovfile);
	// (入力ファイルは ov_clear が cb_close_func を呼び出して自動的に閉じるのでここでは閉じない)

	// 出力ファイルを閉じる
	fclose(outfile);

	// WuVorbis を終了
	// 通常 Windows はアプリケーション終了時に自動的に DLL を解放するので
	// これは必要ないかも。。。
	WuVorbisUninit();

	return 0;
}
//---------------------------------------------------------------------------
