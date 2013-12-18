#include <squirrel.h>
#include <string.h>

/**
 * メモリ上のファイルからのロード処理もろもろの実装
 * sqstd_loadfile から機能コピー。
 * バイナリ/UTF/UCS 判定処理つき
 */

class MemFile
{
public:
	MemFile(const char *dataBuffer, int dataSize) : dataBuffer(dataBuffer), dataSize(dataSize), cur(0) {}
	~MemFile() {};

	// 読み込み
	// @return 読み込んだデータ数
	int read(void *buffer, int size, int cnt) {
		if (cur >= dataSize) {
			return 0;
		}
		int lcnt = (dataSize - cur) / size;
		if (cnt > lcnt) {
			cnt = lcnt;
		}
		size *= cnt;
		memcpy(buffer, dataBuffer+cur, size);
		cur += size;
		return cnt;
	}

	// シーク処理
	void seek(int offset) {
		cur = offset;
	}

	void close() {
		// nothing todo
	}

private:
	const char *dataBuffer;
	int dataSize;
	int cur;
};

// 通常のASCII
static SQInteger _io_file_lexfeed_ASCII(SQUserPointer file)
{
	MemFile *fp = (MemFile*)file;
	SQInteger ret;
	char c;
	if( ( ret= fp->read(&c,sizeof(c),1)>0) )
		return c;
	return 0;
}

// UTF-8 ファイルの読み込み
static SQInteger _io_file_lexfeed_UTF8(SQUserPointer file)
{
	MemFile *fp = (MemFile*)file;

#define READ() \
	if(fp->read(&inchar,sizeof(inchar),1) != 1) \
		return 0;

	static const SQInteger utf8_lengths[16] =
	{
		1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
		0,0,0,0,                /* 1000 to 1011 : not valid */
		2,2,                    /* 1100, 1101 : 2 bytes */
		3,                      /* 1110 : 3 bytes */
		4                       /* 1111 :4 bytes */
	};
	static unsigned char byte_masks[5] = {0,0,0x1f,0x0f,0x07};
	unsigned char inchar;
	SQInteger c = 0;
	READ();
	c = inchar;
	//
	if(c >= 0x80) {
		SQInteger tmp;
		SQInteger codelen = utf8_lengths[c>>4];
		if(codelen == 0) 
			return 0;
			//"invalid UTF-8 stream";
		tmp = c&byte_masks[codelen];
		for(SQInteger n = 0; n < codelen-1; n++) {
			tmp<<=6;
			READ();
			tmp |= inchar & 0x3F;
		}
		c = tmp;
	}
	return c;
}

// UCS2 little endian ファイルの読み込み
static SQInteger _io_file_lexfeed_UCS2_LE(SQUserPointer file)
{
	MemFile *fp = (MemFile*)file;
	SQInteger ret;
	wchar_t c;
	if( ( ret=fp->read(&c,sizeof(c),1)>0) )
		return (SQChar)c;
	return 0;
}

// UCS2 big endian ファイルの読み込み
static SQInteger _io_file_lexfeed_UCS2_BE(SQUserPointer file)
{
	MemFile *fp = (MemFile*)file;
	SQInteger ret;
	unsigned short c;
	if( ( ret=fp->read(&c,sizeof(c),1)>0) ) {
		c = ((c>>8)&0x00FF)| ((c<<8)&0xFF00);
		return (SQChar)c;
	}
	return 0;
}

// バイナリ読み込み用
static SQInteger file_read(SQUserPointer file,SQUserPointer buf,SQInteger size)
{
	MemFile *fp = (MemFile*)file;
	SQInteger ret;
	if( ( ret = (SQInteger)fp->read(buf,1,(int)size))!=0 )return ret;
	return -1;
}

/**
 * メモリ上のファイルからの読み込み処理
 * @param v squirrelVM
 * @param dataBuffer メモリバッファ
 * @param dataSize メモリバッファサイズ
 * @param filename ファイル名
 * @param printError エラーを表示するかどうか
 */
SQRESULT sqstd_loadmemory(HSQUIRRELVM v, const char *dataBuffer, int dataSize, const SQChar *filename, SQBool printerror)
{
	MemFile file(dataBuffer, dataSize);

	SQInteger ret;
	unsigned short us;
	unsigned char uc;
	SQLEXREADFUNC func = _io_file_lexfeed_ASCII;

	ret = file.read(&us,1,2);
	if(ret != 2) {
		//probably an empty file
		us = 0;
	}
	if(us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
		file.seek(0);
		if(SQ_SUCCEEDED(sq_readclosure(v,file_read,&file))) {
			file.close();
			return SQ_OK;
		}
	}
	else { //SCRIPT
		switch(us) {
			//gotta swap the next 2 lines on BIG endian machines
		case 0xFFFE: func = _io_file_lexfeed_UCS2_BE; break;//UTF-16 little endian;
		case 0xFEFF: func = _io_file_lexfeed_UCS2_LE; break;//UTF-16 big endian;
		case 0xBBEF: 
			if(file.read(&uc,1,sizeof(uc)) == 0) { 
				file.close();
				return sq_throwerror(v,_SC("io error")); 
			}
			if(uc != 0xBF) { 
				file.close();
				return sq_throwerror(v,_SC("Unrecognozed ecoding")); 
			}
			func = _io_file_lexfeed_UTF8;
			break;//UTF-8 ;
				default: file.seek(0); break; // ascii
			}
		if(SQ_SUCCEEDED(sq_compile(v,func,&file,filename,printerror))){
			file.close();
			return SQ_OK;
		}
	}
	file.close();
	return SQ_ERROR;
}
