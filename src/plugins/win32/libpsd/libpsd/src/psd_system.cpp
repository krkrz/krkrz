// created: 2007-01-27
#include <stdio.h>
#include <stdlib.h>
#include "libpsd.h"
#include "psd_system.h"

// XXX support for kirikiri
#include <windows.h>
#include "../../../tp_stub.h"

void * psd_malloc(psd_int size)
{
	return TVP_malloc(size);
}

void * psd_realloc(void * block, psd_int size)
{
	return TVP_realloc(block, size);
}

void psd_free(void * block)
{
	TVP_free(block);
}

void psd_freeif(void * block)
{
	if (block != NULL)
		psd_free(block);
}

void * psd_fopen(psd_char * file_name)
{
	return (void *)TVPCreateIStream(file_name, TJS_BS_READ);
}

psd_int psd_fsize(void * file)
{
	IStream *is = (IStream *)file;
	if (is) {
		STATSTG stat;
		is->Stat(&stat, STATFLAG_NONAME);
		return (psd_int)stat.cbSize.QuadPart;
	}
	return 0;
}

psd_int psd_fread(psd_uchar * buffer, psd_int count, void * file)
{
	IStream *is = (IStream *)file;
	if (is) {
		ULONG len;
		if (is->Read(buffer,count,&len) == S_OK) {
			return len;
		} 
	}
	return 0;
}

psd_int psd_fseek(void * file, psd_int length)
{
	IStream *is = (IStream *)file;
	if (is) {
		LARGE_INTEGER move;
		move.QuadPart = length;
		ULARGE_INTEGER newposition;
		if (is->Seek(move, STREAM_SEEK_CUR, &newposition) == S_OK) {
			return (psd_int)newposition.QuadPart;
		}
	}
	return -1;
}

void psd_fclose(void * file)
{
	IStream *is = (IStream *)file;
	if (is) {
		is->Release();
	}
}

#include <stack>

void *createStack()
{
	return (void*)new std::stack<psd_layer_record*>;
}

void destroyStack(void *stack)
{
	std::stack<psd_layer_record*> *s = (std::stack<psd_layer_record*>*)stack;
	if (s) {
		delete s;
	}
}

void pushStack(void *stack, psd_layer_record *layer)
{
	std::stack<psd_layer_record*> *s = (std::stack<psd_layer_record*>*)stack;
	if (s) {
		s->push(layer);
	}
}

void popStack(void *stack)
{
	std::stack<psd_layer_record*> *s = (std::stack<psd_layer_record*>*)stack;
	if (s) {
		s->pop();
	}
}

psd_layer_record *getStackTop(void *stack)
{
	std::stack<psd_layer_record*> *s = (std::stack<psd_layer_record*>*)stack;
	return s && s->size() > 0 ? s->top() : NULL;
}

