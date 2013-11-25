
#ifndef __COMMNAD_BUFFER_H__
#define __COMMNAD_BUFFER_H__

struct CommandBuffer {
	size_t	size_;
	char*	data_;
	CommandBuffer( size_t size, char* data ) : size_(size), data_(data) {}
	CommandBuffer() : size_(0), data_(NULL) {}
};

#endif // __COMMNAD_BUFFER_H__
