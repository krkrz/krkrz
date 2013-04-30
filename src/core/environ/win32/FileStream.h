

#ifndef __T_FILE_STREAM_H__
#define __T_FILE_STREAM_H__

enum {
	fmCreate = 0x01,
	fmShareDenyWrite = 0x02,
};

class TFileStream {
public:
	TFileStream( const std::string& filename, int mode ) {
	};
	size_t WriteBuffer( const void* buffer, size_t length ) {
		return length;
	};
};


#endif // __T_FILE_STREAM_H__
