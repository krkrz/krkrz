// sqthread用ファイル読み込み処理サンプル
#include <stdio.h>
#include <squirrel.h>
#include <sqstdio.h>

/**
 * ファイル読み込み用データ構造
 */
class SQFileInfo {

public:
#ifdef SQOBJHEAP
    SQHEAPDEFINE;
#endif
	/// コンストラクタ
	SQFileInfo(const SQChar *filename, bool binary) : file(NULL), buffer(NULL), size(0), binary(binary) {
		file = sqstd_fopen(filename, binary ? _SC("rb") : _SC("r"));
		if (file) {
			if (sqstd_fseek(file, 0, SQ_SEEK_END) == 0) {
				size = sqstd_ftell(file);
				sqstd_fseek(file, 0, SQ_SEEK_SET);
				if (size > 0) {
					buffer = sq_malloc(size);
					size = sqstd_fread(buffer, 1, size, file);
				}
			}
		}
	}

	/// デストラクタ
	~SQFileInfo() {
		if (buffer) {
			sq_free(buffer,size);
		}
		if (file) {
			sqstd_fclose(file);
		}
	}

	/// @return 読み込み完了したら true
	bool check() {
		if (buffer) {
			return true;
		} else {
			return true;
		}
	}

	/// @return バッファ
	const char *getBuffer() {
		return (const char*)buffer;
	}

	/// @return サイズ
	int getSize() {
		return (int)size;
	}

private:
	SQFILE file;  ///< 入力ストリーム
	void *buffer; ///< 入力データのバッファ
	SQInteger size;   ///< 読み込みサイズ
	bool binary;
};

/**
 * ファイルを非同期に開く
 * @param filename スクリプトファイル名
 * @param binary バイナリ指定で開く
 * @return ファイルハンドラ
 */
void *sqobjOpenFile(const SQChar *filename, bool binary)
{
	return (void*) new SQFileInfo(filename, binary);
}

/**
 * ファイルが開かれたかどうかのチェック
 * @param handler ファイルハンドラ
 * @param dataPtr データ格納先アドレス(出力) (エラー時はNULL)
 * @param dataSize データサイズ(出力)
 * @return ロード完了していたら true
 */
bool sqobjCheckFile(void *handler, const char **dataAddr, int *dataSize)
{
	SQFileInfo *file = (SQFileInfo*)handler;
	if (file) {
		if (file->check()) {
			*dataAddr = file->getBuffer();
			*dataSize = file->getSize();
			return true;
		} else {
			return false;
		}
	} else {
		*dataAddr = NULL;
		*dataSize = 0;
		return true;
	}
}

/**
 * ファイルを閉じる
 * @param handler ファイルハンドラ
 */
void sqobjCloseFile(void *handler)
{
	SQFileInfo *file = (SQFileInfo*)handler;
	if (file) {
		delete file;
	}
}
