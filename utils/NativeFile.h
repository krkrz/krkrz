/**
 * FILE ポインタを扱う極シンプルな実装
 */


#ifndef __NATIVE_FILE_H__
#define __NATIVE_FILE_H__

#include <stdio.h>
#ifdef ANDROID
#include "CharacterSet.h"
#endif

class NativeFile {
	FILE* fp_;

public:
	NativeFile() : fp_(NULL) {}

	~NativeFile() {
		Close();
	}

	bool Open( const wchar_t* filename, const wchar_t* mode ) {
		Close();
#ifndef ANDROID
		fp_ = _wfopen(filename, mode);
		return fp_ != NULL;
#else
		fp_ = NULL;
		std::wstring wname(filename);
		std::wstring wmode(mode);
		std::string nname, nmode;
		if( TVPUtf16ToUtf8(nname, wname) && TVPUtf16ToUtf8(nmode, wmode) ) {
			fp_ = fopen( nname.c_str(), nmode.c_str() );
		}
		return fp_ != NULL;
#endif
	}

	tjs_int Seek( tjs_int offset, int base ) {
		return fseek( fp_, offset, base );
	}

	tjs_int Tell() {
		return ftell( fp_ );
	}
	bool IsOpen() const {
		return fp_ != NULL;
	}

	void Close() {
		if( fp_ ) {
			fclose(fp_);
			fp_ = NULL;
		}
	}

	size_t Write( const void* buf, size_t size ) {
		return fwrite( buf, 1, size, fp_ );
	}

	size_t Read( void* buf, size_t size ) {
		return fread( buf, 1, size, fp_ );
	}

	void Flush() {
		fflush(fp_);
	}
};

#endif // __NATIVE_FILE_H__

