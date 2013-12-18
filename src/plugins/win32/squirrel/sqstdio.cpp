/* see copyright notice in squirrel.h */
#include <new>
#include <stdio.h>
#include <squirrel.h>
#include <sqstdio.h>
#include "sqstdstream.h"

// XXX support for kirikiri
#include <windows.h>
#include "tp_stub.h"

#define SQSTD_FILE_TYPE_TAG (SQSTD_STREAM_TYPE_TAG | 0x00000001)
//basic API
SQFILE sqstd_fopen(const SQChar *filename ,const SQChar *mode)
{
	tjs_uint32 m;
	if (TJS_stricmp(mode, L"r") == 0 ||
		TJS_stricmp(mode, L"rb") == 0) {
		m = TJS_BS_READ;
	} else if (TJS_stricmp(mode, L"w") == 0 ||
			   TJS_stricmp(mode, L"wb") == 0) {
		m = TJS_BS_WRITE;
	} else if (TJS_stricmp(mode, L"r+") == 0 ||
			   TJS_stricmp(mode, L"rb+") == 0 ||			   
			   TJS_stricmp(mode, L"w+") == 0 ||
			   TJS_stricmp(mode, L"wb+") == 0) {
		m = TJS_BS_UPDATE;
	} else if (TJS_stricmp(mode, L"a") == 0 ||
			   TJS_stricmp(mode, L"ab") == 0) {
		m = TJS_BS_APPEND;
	} else {
		return NULL;
	}
	return TVPCreateIStream(filename, m);
}

SQInteger sqstd_fread(void* buffer, SQInteger size, SQInteger count, SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		ULONG len;
		if (is->Read(buffer,(ULONG)(size*count),&len) == S_OK) {
			return len / size;
		} 
	}
	return 0;
}

SQInteger sqstd_fwrite(const SQUserPointer buffer, SQInteger size, SQInteger count, SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		DWORD len;
		if (is->Write(buffer,(ULONG)(size*count),&len) == S_OK) {
			return len / size;
		}
	}
	return 0;
}

SQInteger sqstd_fseek(SQFILE file, SQInteger offset, SQInteger origin)
{
	DWORD dwOrigin;
	switch(origin) {
	case SQ_SEEK_CUR: dwOrigin = STREAM_SEEK_CUR; break;
	case SQ_SEEK_END: dwOrigin = STREAM_SEEK_END; break;
	case SQ_SEEK_SET: dwOrigin = STREAM_SEEK_SET; break;
	default: return -1; //failed
	}
	IStream *is = (IStream *)file;
	if (is) {
		LARGE_INTEGER move;
		move.QuadPart = offset;
		ULARGE_INTEGER newposition;
		if (is->Seek(move, (ULONG)origin, &newposition) == S_OK) {
			return (SQInteger)newposition.QuadPart;
		}
	}
	return -1;
}

SQInteger sqstd_ftell(SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		LARGE_INTEGER move = {0};
		ULARGE_INTEGER newposition;
		if (is->Seek(move, STREAM_SEEK_CUR, &newposition) == S_OK) {
			return (SQInteger)newposition.QuadPart;
		}
	}
	return -1;
}

SQInteger sqstd_fflush(SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		if (is->Commit(STGC_DEFAULT) == S_OK) {
			return 0;
		}
	}
	return EOF;
}

SQInteger sqstd_fclose(SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		is->Release();
		return 0;
	}
	return EOF;
}

SQInteger sqstd_flen(SQFILE file)
{
	IStream *is = (IStream *)file;
	if (is) {
		STATSTG stat;
		is->Stat(&stat, STATFLAG_NONAME);
		return (SQInteger)stat.cbSize.QuadPart;
	}
	return 0;
}

//File
struct SQFile : public SQStream {
	SQFile() { _handle = NULL; _owns = false;}
	SQFile(SQFILE file, bool owns) { _handle = file; _owns = owns;}
	virtual ~SQFile() { Close(); }
	bool Open(const SQChar *filename ,const SQChar *mode) {
		Close();
		if( (_handle = sqstd_fopen(filename,mode)) ) {
			_owns = true;
			return true;
		}
		return false;
	}
	void Close() {
		if(_handle && _owns) { 
			sqstd_fclose(_handle);
			_handle = NULL;
			_owns = false;
		}
	}
	SQInteger Read(void *buffer,SQInteger size) {
		return sqstd_fread(buffer,1,size,_handle);
	}
	SQInteger Write(void *buffer,SQInteger size) {
		return sqstd_fwrite(buffer,1,size,_handle);
	}
	SQInteger Flush() {
		return sqstd_fflush(_handle);
	}
	SQInteger Tell() {
		return sqstd_ftell(_handle);
	}
	SQInteger Len() {
		return sqstd_flen(_handle);
	}
	SQInteger Seek(SQInteger offset, SQInteger origin)	{
		return sqstd_fseek(_handle,offset,origin);
	}
	bool IsValid() { return _handle?true:false; }
	bool EOS() { return Tell()==Len()?true:false;}
	SQFILE GetHandle() {return _handle;}
private:
	SQFILE _handle;
	bool _owns;
};

static SQInteger _file__typeof(HSQUIRRELVM v)
{
	sq_pushstring(v,_SC("file"),-1);
	return 1;
}

static SQInteger _file_releasehook(SQUserPointer p, SQInteger size)
{
	SQFile *self = (SQFile*)p;
	delete self;
	return 1;
}

static SQInteger _file_constructor(HSQUIRRELVM v)
{
	const SQChar *filename,*mode;
	bool owns = true;
	SQFile *f;
	SQFILE newf;
	if(sq_gettype(v,2) == OT_STRING && sq_gettype(v,3) == OT_STRING) {
		sq_getstring(v, 2, &filename);
		sq_getstring(v, 3, &mode);
		newf = sqstd_fopen(filename, mode);
		if(!newf) return sq_throwerror(v, _SC("cannot open file"));
	} else if(sq_gettype(v,2) == OT_USERPOINTER) {
		owns = !(sq_gettype(v,3) == OT_NULL);
		sq_getuserpointer(v,2,&newf);
	} else {
		return sq_throwerror(v,_SC("wrong parameter"));
	}
	f = new SQFile(newf,owns);
	if(SQ_FAILED(sq_setinstanceup(v,1,f))) {
		delete f;
		return sq_throwerror(v, _SC("cannot create blob with negative size"));
	}
	sq_setreleasehook(v,1,_file_releasehook);
	return 0;
}

//bindings
#define _DECL_FILE_FUNC(name,nparams,typecheck) {_SC(#name),_file_##name,nparams,typecheck}
static SQRegFunction _file_methods[] = {
	_DECL_FILE_FUNC(constructor,3,_SC("x")),
	_DECL_FILE_FUNC(_typeof,1,_SC("x")),
	{0,0,0,0},
};



SQRESULT sqstd_createfile(HSQUIRRELVM v, SQFILE file,SQBool own)
{
	SQInteger top = sq_gettop(v);
	sq_pushregistrytable(v);
	sq_pushstring(v,_SC("std_file"),-1);
	if(SQ_SUCCEEDED(sq_get(v,-2))) {
		sq_remove(v,-2); //removes the registry
		sq_pushroottable(v); // push the this
		sq_pushuserpointer(v,file); //file
		if(own){
			sq_pushinteger(v,1); //true
		}
		else{
			sq_pushnull(v); //false
		}
		if(SQ_SUCCEEDED( sq_call(v,3,SQTrue,SQFalse) )) {
			sq_remove(v,-2);
			return SQ_OK;
		}
	}
	sq_settop(v,top);
	return SQ_OK;
}

SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQFILE *file)
{
	SQFile *fileobj = NULL;
	if(SQ_SUCCEEDED(sq_getinstanceup(v,idx,(SQUserPointer*)&fileobj,(SQUserPointer)SQSTD_FILE_TYPE_TAG))) {
		*file = fileobj->GetHandle();
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("not a file"));
}

SQInteger file_read(SQUserPointer file,SQUserPointer buf,SQInteger size)
{
	IStream *is = (IStream*)file;
	ULONG s;
	if (is->Read(buf, (ULONG)size, &s) == S_OK) {
		return (SQInteger)s;
	}
	return -1;
}

SQInteger file_write(SQUserPointer file,SQUserPointer p,SQInteger size)
{
	IStream *is = (IStream*)file;
	ULONG s;
	if (is->Write(p, (ULONG)size, &s) == S_OK) {
		return (SQInteger)s;
	}
	return -1;
}

SQRESULT sqstd_loadfile(HSQUIRRELVM v,const SQChar *filename,SQBool printerror)
{
	unsigned short us;

	IStream *is = TVPCreateIStream(filename, TJS_BS_UPDATE);
	if (is){
		DWORD len;
		us = 0;
		is->Read(&us, 2, &len);
		if(us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
			LARGE_INTEGER move = {0};
			is->Seek(move,STREAM_SEEK_SET,NULL);
			if (SQ_SUCCEEDED(sq_readclosure(v,file_read,is))) {
				is->Release();
				return SQ_OK;
			}
			is->Release();
		}
		else { //SCRIPT
			is->Release();
			// open for text
			iTJSTextReadStream *rs = TVPCreateTextStreamForRead(filename, L"");
			ttstr data;
			rs->Read(data, 0);
			rs->Destruct();
			if (SQ_SUCCEEDED(sq_compilebuffer(v, data.c_str(), data.length(), filename, printerror))) {
				return SQ_OK;
			}
		}
		return SQ_ERROR;
	}
	return sq_throwerror(v,_SC("cannot open the file"));
}

SQRESULT sqstd_dofile(HSQUIRRELVM v,const SQChar *filename,SQBool retval,SQBool printerror)
{
	if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror))) {
		sq_push(v,-2);
		if(SQ_SUCCEEDED(sq_call(v,1,retval,SQTrue))) {
			sq_remove(v,retval?-2:-1); //removes the closure
			return 1;
		}
		sq_pop(v,1); //removes the closure
	}
	return SQ_ERROR;
}

SQRESULT sqstd_writeclosuretofile(HSQUIRRELVM v,const SQChar *filename,SQInteger endian)
{
	IStream *is = TVPCreateIStream(filename, TJS_BS_WRITE);
	if(!is) return sq_throwerror(v,_SC("cannot open the file"));
	if(SQ_SUCCEEDED(sq_writeclosure(v,file_write,is,endian))) {
		is->Release();
		return SQ_OK;
	}
	is->Release();
	return SQ_ERROR; //forward the error
}

SQInteger _g_io_loadfile(HSQUIRRELVM v)
{
	const SQChar *filename;
	SQBool printerror = SQFalse;
	sq_getstring(v,2,&filename);
	if(sq_gettop(v) >= 3) {
		sq_getbool(v,3,&printerror);
	}
	if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror)))
		return 1;
	return SQ_ERROR; //propagates the error
}

SQInteger _g_io_writeclosuretofile(HSQUIRRELVM v)
{
	const SQChar *filename;
	sq_getstring(v,2,&filename);
	int endian = 0;
	if (sq_gettop(v) >= 3) {
		sq_getinteger(v,3,&endian);
	}
	if(SQ_SUCCEEDED(sqstd_writeclosuretofile(v,filename,endian)))
		return 1;
	return SQ_ERROR; //propagates the error
}

SQInteger _g_io_dofile(HSQUIRRELVM v)
{
	const SQChar *filename;
	SQBool printerror = SQFalse;
	sq_getstring(v,2,&filename);
	if(sq_gettop(v) >= 3) {
		sq_getbool(v,3,&printerror);
	}
	sq_push(v,1); //repush the this
	if(SQ_SUCCEEDED(sqstd_dofile(v,filename,SQTrue,printerror)))
		return 1;
	return SQ_ERROR; //propagates the error
}

#define _DECL_GLOBALIO_FUNC(name,nparams,typecheck) {_SC(#name),_g_io_##name,nparams,typecheck}
static SQRegFunction iolib_funcs[]={
	_DECL_GLOBALIO_FUNC(loadfile,-2,_SC(".sb")),
	_DECL_GLOBALIO_FUNC(dofile,-2,_SC(".sb")),
	_DECL_GLOBALIO_FUNC(writeclosuretofile,-3,_SC(".sc")),
	{0,0}
};

SQRESULT sqstd_register_iolib(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
	//create delegate
	declare_stream(v,_SC("file"),(SQUserPointer)SQSTD_FILE_TYPE_TAG,_SC("std_file"),_file_methods,iolib_funcs);
#if 0
	sq_pushstring(v,_SC("stdout"),-1);
	sqstd_createfile(v,stdout,SQFalse);
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("stdin"),-1);
	sqstd_createfile(v,stdin,SQFalse);
	sq_createslot(v,-3);
	sq_pushstring(v,_SC("stderr"),-1);
	sqstd_createfile(v,stderr,SQFalse);
	sq_createslot(v,-3);
#endif
	sq_settop(v,top);
	return SQ_OK;
}
