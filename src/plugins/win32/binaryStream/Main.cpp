#include <windows.h>
#include <zlib.h>
#include "ncbind.hpp"

#define COPY_BUFFER_SIZE     (1024*1024)
#define COMPRESS_BUFFER_SIZE (1024*1024)

// XP3ArchiveAttractFilter_v2
typedef void (__stdcall *FilterProc)(tjs_uint32 hash, tjs_uint64 offset, void * buffer, long bufferlen);

class BinaryStream
{
	IStream *stream;
	ttstr storage;
	int mode;
	HMODULE    filterDLL;
	bool hasCallback;
	tTJSVariantClosure callback;

	template <typename T>
	void _swap(T *p, int cnt) {
		for (; cnt > 0; --cnt,p+=2) {
			T sw = p[0];
			p[0] = p[1];
			p[1] = sw;
		}
	}

	class CopyCommand {
		BinaryStream *owner;
		ttstr name;

		tjs_uint rbufsize;
		tjs_int64 rcount, wcount;
		tjs_uint32 adler;

		tjs_int64 inputOffset, inputMaxsize;
		FilterProc filterProc;
		tjs_uint32 filterValue;

		TVP_md5_state_t md5;

		bool noCopy, calcMD5;
	public:
		CopyCommand(BinaryStream *owner, tjs_char const *name)
			:   owner(owner), name(name),
				rbufsize(COPY_BUFFER_SIZE),
				rcount(0), wcount(0),
				inputOffset(0), inputMaxsize(0),
				filterProc(0), filterValue(0),
				noCopy(false), calcMD5(false)
		{
			adler = ::adler32(0L, Z_NULL, 0);
		}
		virtual ~CopyCommand() {}

		inline void calcSum(tjs_uint8 const *buf, tjs_uint len) {
			adler = ::adler32(adler, buf, len);
		}
		void filter(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			if (filterProc && len > 0) filterProc(filterValue, (tjs_uint64)ofs, buf, len);
		}

		tjs_error execute(tTJSVariant *r, tjs_int n, tTJSVariant **p) {
			if (n < 1) return TJS_E_BADPARAMCOUNT;
			ttstr storage = *p[0];
			bool proc = false;
			if (n >= 2) {
				ncbPropAccessor elm(*p[1]);
				if (elm.IsValid()) {
					detailOptionLoad(elm);

					proc = true;
					main(storage);

					detailOptionSave(elm);
				}
			}
			if (!proc) main(storage);

			if (r) *r = noCopy ? 0 : wcount;
			return TJS_S_OK;
		}
	private:
		void main(ttstr file) {
			IStream *in = TVPCreateIStream(file, TJS_BS_READ);
			if (!in) error(file, TJS_W(": storage not found"));
			try {
				if (inputOffset > 0) {
					LARGE_INTEGER set;
					set.QuadPart = inputOffset;
					in->Seek(set, STREAM_SEEK_SET, NULL);
				}
				tjs_uint8 *buf = new tjs_uint8[rbufsize];
				try {
					tjs_int64 remain = inputMaxsize;
					wcount += beforeCopy();
					while (true) {
						tjs_uint len = rbufsize;
						if (remain > 0 && remain < len) len = (tjs_uint)remain;
						len = owner->streamRead(in, buf, len);
						if (!len) break;

						wcount += doCopy(rcount, buf, len);
						rcount += len;
						if (owner->progress(file, rcount)) break;
						if (remain > 0) {
							if (remain <= len) break;
							remain -= len;
						}
					}
					wcount += afterCopy();
				} catch (...) {
					delete[] buf;
					throw;
				}
				delete[] buf;
			} catch (...) {
				in->Release();
				throw;
			}
			in->Release();
		}

	protected:
		void error(ttstr const &msg, tjs_char const *tag = 0) { owner->error((tag ? (name + tag) : name) + (msg.length() > 0 ? TJS_W(": ") + msg : TJS_W(""))); }
		virtual void detailOptionLoad(ncbPropAccessor &elm) {
			inputOffset  = elm.GetValue(TJS_W("offset"), ncbTypedefs::Tag<tjs_int64>());
			inputMaxsize = elm.GetValue(TJS_W("length"), ncbTypedefs::Tag<tjs_int64>());
			filterProc   = owner->getFilterProc(elm.getStrValue(TJS_W("filter")));
			filterValue  = elm.getIntValue(TJS_W("fparam"));
			noCopy       = !!elm.getIntValue(TJS_W("nocopy"));
			calcMD5      = !!elm.getIntValue(TJS_W("md5"));
			if (calcMD5) TVP_md5_init(&md5);
		}
		virtual void detailOptionSave(ncbPropAccessor &elm) {
			elm.SetValue(TJS_W("read"), rcount);
			elm.SetValue(TJS_W("hash"), adler);
			if (calcMD5) {
				tjs_uint8 digest[16];
				TVP_md5_finish(&md5, digest);

				tTJSVariantOctet *oct = TJSAllocVariantOctet(digest, 16);
				tTJSVariant voct;
				voct = oct;
				elm.SetValue(TJS_W("digest"), voct);
				oct->Release();
			}
		}

		tjs_int64 write(tjs_uint8 *buf, tjs_uint len) {
			onWrite(wcount, buf, len);
			return noCopy ? len : owner->streamWrite(owner->stream, buf, len);
		}
		virtual void     onWrite(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			if (calcMD5) TVP_md5_append(&md5, buf, len);
		}
		virtual tjs_int64 doCopy(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			calcSum(buf, len);
			filter(ofs, buf, len);
			return write(buf, len);
		}
		virtual tjs_int64 beforeCopy() { return 0; }
		virtual tjs_int64 afterCopy()  { return 0; }
	};

	class ZlibCommand : public CopyCommand {
		bool initialized;
		tjs_uint8 *zbuf;
		tjs_uint   zlen, wbufsize;
		z_stream z;
	public:
		ZlibCommand(BinaryStream *owner, tjs_char const *name) : CopyCommand(owner, name), initialized(false), zbuf(0), zlen(0), wbufsize(COMPRESS_BUFFER_SIZE) {}
		virtual ~ZlibCommand() { finish(true); }
	protected:
		void setup() {
			initialized = true;
			::ZeroMemory(&z, sizeof(z));
			if (zinit(&z) != Z_OK) error(z.msg, TJS_W(": setup"));
			zlen = wbufsize;
			zbuf = new tjs_uint8[zlen];
			oprepare();
		}
		void finish(bool ignoreError = false) {
			if (zbuf) delete[] zbuf;
			zbuf = 0;
			if (initialized && zdone(&z) != Z_OK && !ignoreError) error(z.msg, TJS_W(": finish"));
			initialized = false;
		}
		void iprepare(tjs_uint8 *buf, tjs_uint len) { z.next_in  =  buf; z.avail_in  =  len; }
		void oprepare()                             { z.next_out = zbuf; z.avail_out = zlen; }
		tjs_int64 flush() {
			tjs_int64 r = (zlen - z.avail_out > 0) ? write(zbuf, zlen - z.avail_out) : 0;
			oprepare();
			return r;
		}
		tjs_int64 procedure(int flag = Z_NO_FLUSH) {
			tjs_int64 out = 0;
			do {
				switch (zmain(&z, flag)) {
				case Z_OK:
					if (z.avail_out == 0) out += flush();
					break;
				case Z_STREAM_END:
					out += flush();
					break;
				default:
					error(z.msg, TJS_W(": procedure"));
					break;
				}
			} while (z.avail_in > 0);
			return out;
		}
		virtual int zinit(z_stream *z)           = 0;
		virtual int zmain(z_stream *z, int flag) = 0;
		virtual int zdone(z_stream *z)           = 0;
		virtual tjs_int64 beforeCopy() { setup(); return 0; }
		virtual tjs_int64 afterCopy()  { tjs_int64 r = procedure(Z_FINISH); finish(); return r;}
	};

	class CompressCommand   : public ZlibCommand {
		int complv;
	public:
		CompressCommand(BinaryStream *owner, tjs_char const *name) : ZlibCommand(owner, name), complv(Z_BEST_COMPRESSION) {}
	protected:
		virtual int zinit(z_stream *z)           { return ::deflateInit(z, complv); }
		virtual int zmain(z_stream *z, int flag) { return ::deflate(z, flag); }
		virtual int zdone(z_stream *z)           { return ::deflateEnd(z); }
		virtual tjs_int64 doCopy(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			calcSum(buf, len);
			filter(ofs, buf, len);
			iprepare(buf, len);
			return procedure();
		}
		virtual void detailOptionLoad(ncbPropAccessor &elm) {
			ZlibCommand::detailOptionLoad(elm);
			complv = elm.getIntValue(TJS_W("comp_lv"), complv);
		}
	};
	class DecompressCommand : public ZlibCommand {
	public:
		DecompressCommand(BinaryStream *owner, tjs_char const *name) : ZlibCommand(owner, name) {}
	protected:
		virtual int zinit(z_stream *z)           { return ::inflateInit(z); }
		virtual int zmain(z_stream *z, int flag) { return ::inflate(z, flag); }
		virtual int zdone(z_stream *z)           { return ::inflateEnd(z); }
		virtual tjs_int64 doCopy(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			iprepare(buf, len);
			return procedure();
		}
		virtual void onWrite(tjs_int64 ofs, tjs_uint8 *buf, tjs_uint len) {
			filter(ofs, buf, len);
			calcSum(buf, len);
			ZlibCommand::onWrite(ofs, buf, len);
		}
	};

	//--------------------------------------------------------------

public:
	BinaryStream() : stream(0), storage(), mode(-1), filterDLL(0), hasCallback(false), callback(0,0) {}
	virtual ~BinaryStream() {
		resetCopyFilter();
		close();
	}

	static tjs_error TJS_INTF_METHOD Factory(BinaryStream **inst, tjs_int n, tTJSVariant **p, iTJSDispatch2*) {
		if (inst) {
			BinaryStream *self = *inst = new BinaryStream();
			if (n >= 1) {
				tjs_int mode = (n >= 2) ? (tjs_int)*p[1] : TJS_BS_READ;
				try {
					self->open(p[0]->GetString(), mode);
				} catch (...) {
					delete self;
					*inst = 0;
					throw;
				}
			}
		}
		return TJS_S_OK;
	}

	/**
	 * ストレージを開く
	 * @param storage 対象ストレージ
	 * @param mode    モード指定(bsRead, bsWrite, bsAppend, bsUpdate)
	 */
	void open(tjs_char const *storage, int mode) {
		close();
		stream = TVPCreateIStream(storage, mode & TJS_BS_ACCESS_MASK);
		if (stream) {
			this->storage = storage;
			this->mode    = mode;
		} else {
			error(ttstr(TJS_W("cannot open : ")) + storage);
		}
	}

	/**
	 * オープン中のストレージを閉じる
	 */
	void close() {
		if (stream) {
			storage = TJS_W("");
			mode    = -1;
			stream->Release();
			stream  = 0;
		}
	}

	/**
	 * ストリームのポジションを変更する
	 * @param pos    位置
	 * @param whence 基準位置(bsSeekSet, bsSeekCur, bsSeekEnd)
	 * @return       移動後の位置
	 */
	tjs_int64 seek(tjs_int64 pos, int whence) {
		ULONG org;
		switch (whence) {
		case TJS_BS_SEEK_SET: org = STREAM_SEEK_SET; break;
		case TJS_BS_SEEK_CUR: org = STREAM_SEEK_CUR; break;
		case TJS_BS_SEEK_END: org = STREAM_SEEK_END; break;
		default:
			error(TJS_W("invalid whence value."));
			break;
		}
		LARGE_INTEGER  move;
		ULARGE_INTEGER newpos;

		move.QuadPart = pos;
		stream->Seek(move, org, &newpos);

		return newpos.QuadPart;
	}

	/**
	 * ストリームの現在のポジションを取得する
	 * @return 位置
	 */
	tjs_int64 tell() { return seek(0, STREAM_SEEK_CUR); }


	/**
	 * 現在開いているストレージ／モードを取得
	 */
	int   getMode()    const { return mode;    }
	ttstr getStorage() const { return storage; }


	//--------------------------------------------------------------

	/**
	 * 指定バイト読み込む
	 * @param size 読み込むサイズ（バイト）
	 * @return     読み込んだデータ（octet形式） 終端などで読めなかった場合はvoid（length=0のoctetではないので注意）
	 */
	static tjs_error TJS_INTF_METHOD read(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (!self) return TJS_E_NATIVECLASSCRASH;
		tTJSVariantOctet *oct = self->_read(p[0]->AsInteger());
		if (oct) {
			if (r) *r = oct;
			oct->Release();
		} else {
			if (r) r->Clear();
		}
		return TJS_S_OK;
	}
	tTJSVariantOctet* _read(tjs_int64 size) {
		tjs_uint32 sz   = (tjs_uint32)size;
		if (size <= 0) return 0;
		if (size != (tjs_int64)sz) error(TJS_W("too large read size."));

		tTJSVariantOctet *ret = 0;
		tjs_uint8 *data = 0;
		try {
			data = new tjs_uint8[sz];
			tjs_uint read = streamRead(stream, data, sz);
			if (read > 0) ret = TJSAllocVariantOctet(data, read);
		} catch (...) {
			if (data) delete[] data;
			if (ret) ret->Release();
			throw;
		}
		delete[] data;
		return ret;
	}

	/**
	 * {1,2,4,8}バイトを数値として読み込む
	 * @return     読み込んだ数値 / 終端などで読めなかった場合はvoid
	 * ※voidを返すのは1byteも読めなかった場合のみ
	 *   1byte以上指定byte未満しか読めなかった場合は足りない部分を0で埋めた値が返されるので注意
	 */
	static tjs_error TJS_INTF_METHOD readI8   (tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readByte(r)    : TJS_E_NATIVECLASSCRASH; }

	static tjs_error TJS_INTF_METHOD readI16LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,2,0) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD readI32LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,4,0) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD readI64LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,8,0) : TJS_E_NATIVECLASSCRASH; }

	static tjs_error TJS_INTF_METHOD readI16BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,2,1) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD readI32BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,4,1) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD readI64BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_readInt(r,8,1) : TJS_E_NATIVECLASSCRASH; }

	tjs_error _readByte(tTJSVariant *r) {
		tjs_uint8 num = 0;
		tjs_uint read = streamRead(stream, &num, 1);
		if (r) {
			if (read) *r = (tjs_int64)num;
			else       r->Clear();
		}
		return TJS_S_OK;
	}
	tjs_error _readInt(tTJSVariant *r, int byte, int bigendian) {
		tjs_int64 num = 0;
		tjs_uint8 buf[8] = { 0,0,0,0,0,0,0,0 };
		tjs_uint read = streamRead(stream, buf, byte);
		if (read) {
			if (bigendian) switch (byte) {
			case 8: _swap((tjs_uint32*)buf, byte>>3);
			case 4: _swap((tjs_uint16*)buf, byte>>2);
			case 2: _swap((tjs_uint8 *)buf, byte>>1);
			}
			if (r) *r = *(tjs_int64*)buf;
		} else {
			if (r) r->Clear();
		}
		return TJS_S_OK;
	}

	//--------------------------------------------------------------

	/**
	 * データを書き込む
	 * @param data 書き込むデータ（octet形式）
	 * @return     書き込んだバイト数
	 */
	static tjs_error TJS_INTF_METHOD write(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (!self) return TJS_E_NATIVECLASSCRASH;
		tjs_uint write = 0;
		switch (p[0]->Type()) {
		case tvtString: {
			tTJSVariantString *str = p[0]->AsStringNoAddRef();
			if (str) {
				tjs_int len = str->GetLength();
				if (len >= 0) {
					len = (len+1)*2;
					write = self->_write((tjs_uint8 const*)str->operator const tjs_char*(), len);
				}
			}
		} break;
		case tvtOctet: {
			tTJSVariantOctet *oct = p[0]->AsOctetNoAddRef();
			if (oct) write = self->_write(oct->GetData(), oct->GetLength());
		} break;
		default:
			error(TJS_W("invalid data type."));
		}
		if (r) *r = (tjs_int64)write;
		return TJS_S_OK;
	}
	tjs_uint _write(tjs_uint8 const *data, tjs_uint length) {
		return streamWrite(stream, data, length);
	}

	/**
	 * {1,2,4,8}バイトで数値を書き込む
	 * @param num 書き込む数値
	 * @return    書き込んだバイト数
	 */
	static tjs_error TJS_INTF_METHOD writeI8   (tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeByte(r,p[0])    : TJS_E_NATIVECLASSCRASH; }

	static tjs_error TJS_INTF_METHOD writeI16LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],2,0) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD writeI32LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],4,0) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD writeI64LE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],8,0) : TJS_E_NATIVECLASSCRASH; }

	static tjs_error TJS_INTF_METHOD writeI16BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],2,1) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD writeI32BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],4,1) : TJS_E_NATIVECLASSCRASH; }
	static tjs_error TJS_INTF_METHOD writeI64BE(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) { return self ? self->_writeInt(r,p[0],8,1) : TJS_E_NATIVECLASSCRASH; }

	tjs_error _writeByte(tTJSVariant *r, tTJSVariant *p) {
		tjs_uint8 num = (tjs_uint8)(p->AsInteger());
		tjs_uint write = streamWrite(stream, &num, 1);
		if (r) *r = (tjs_int64)write;
		return TJS_S_OK;
	}
	tjs_error _writeInt(tTJSVariant *r, tTJSVariant *p, int byte, int bigendian) {
		tjs_int64 num = p->AsInteger();
		if (bigendian) {
			tjs_uint8 buf[8];
			*(tjs_int64*)buf = num;
			switch (byte) {
			case 8: _swap((tjs_uint32*)buf, byte>>3);
			case 4: _swap((tjs_uint16*)buf, byte>>2);
			case 2: _swap((tjs_uint8 *)buf, byte>>1);
			}
			num = *(tjs_int64*)buf;
		}
		tjs_uint write = streamWrite(stream, (tjs_uint8*)&num, byte);
		if (r) *r = (tjs_int64)write;
		return TJS_S_OK;
	}

	//--------------------------------------------------------------

	static tjs_error TJS_INTF_METHOD copy(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) {
		if (!self) return TJS_E_NATIVECLASSCRASH;
		CopyCommand cmd(self, TJS_W("copy"));
		return cmd.execute(r, n, p);
	}
	static tjs_error TJS_INTF_METHOD compress(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) {
		if (!self) return TJS_E_NATIVECLASSCRASH;
		CompressCommand cmd(self, TJS_W("compress"));
		return cmd.execute(r, n, p);
	}
	static tjs_error TJS_INTF_METHOD decompress(tTJSVariant *r, tjs_int n, tTJSVariant **p, BinaryStream *self) {
		if (!self) return TJS_E_NATIVECLASSCRASH;
		DecompressCommand cmd(self, TJS_W("decompress"));
		return cmd.execute(r, n, p);
	}

	void setProgressCallback(tTJSVariant cb) {
		hasCallback = (cb.Type() == tvtObject && cb.AsObjectNoAddRef() != 0);
		callback = hasCallback ? cb.AsObjectClosureNoAddRef() : tTJSVariantClosure(0, 0);
	}
	bool progress(ttstr const &file, tjs_int64 read) {
		if (!hasCallback) return false;
		tTJSVariant result, vf = file, vr = read;
		tTJSVariant *p[] = { &vf, &vr };
		return TJS_SUCCEEDED(callback.FuncCall(0, NULL, NULL, &result, 2, p, NULL)) && result.operator bool();
	}

	void setFilter(tjs_char const *dll) {
		resetCopyFilter();
		if (!dll || dll[0] == 0) return;

		ttstr file = TVPGetPlacedPath(dll);
		TVPGetLocalName(file);
		filterDLL = ::LoadLibraryW(file.c_str());
		if (!filterDLL) error(ttstr(TJS_W("filter DLL not found: ")) + dll);
	}
	FilterProc getFilterProc(ttstr const &proc) {
		if (!filterDLL || !proc.length()) return 0;
		tjs_nchar tmp[256] = {0};
		proc.ToNarrowStr(tmp, sizeof(tmp));
		tmp[sizeof(tmp)-1] = 0;
		FilterProc r = (FilterProc)::GetProcAddress(filterDLL, tmp);
		if (!r) error(ttstr(TJS_W("filter proc not found: ")) + proc);
		return r;
	}

	static void error(ttstr const &message) {
		ttstr concat = ttstr(TJS_W("BinaryStream: ")) + message;
		TVPThrowExceptionMessage(concat.c_str());
	}
protected:
	void resetCopyFilter() {
		if (filterDLL) ::FreeLibrary(filterDLL);
		/**/filterDLL  = 0;
	}

	static inline tjs_uint streamRead (IStream *s, tjs_uint8       *buf, tjs_uint len) {
		if (!s) error(TJS_W("stream not opened."));
		if (!len) return 0;

		ULONG read = 0;
		if (s->Read(buf, len, &read) != S_OK) error("read failed.");
		return (tjs_uint)read;
	}
	static inline tjs_uint streamWrite(IStream *s, tjs_uint8 const *buf, tjs_uint len) {
		if (!s) error(TJS_W("stream not opened."));
		if (!len) return 0;

		ULONG write = 0;
		if (s->Write(buf, len, &write) != S_OK || write != len) error("write failed.");
		return (tjs_uint)write;
	}

};

NCB_REGISTER_CLASS(BinaryStream)
{
	Factory(&Class::Factory);

	Method(TJS_W("open"),   &Class::open);
	Method(TJS_W("close"),  &Class::close);
	Method(TJS_W("seek"),   &Class::seek);
	Method(TJS_W("tell"),   &Class::tell);

	Property(TJS_W("storage"),  &Class::getStorage,  (int)0);
	Property(TJS_W("mode"),     &Class::getMode,     (int)0);

	RawCallback(TJS_W("read"),  &Class::read,  0);
	RawCallback(TJS_W("write"), &Class::write, 0);

	RawCallback(TJS_W("copy"),       &Class::copy,       0);
	RawCallback(TJS_W("compress"),   &Class::compress,   0);
	RawCallback(TJS_W("decompress"), &Class::decompress, 0);
	Method(TJS_W("setProgressCallback"),  &Class::setProgressCallback);
	Method(TJS_W("setFilter"),            &Class::setFilter);

	RawCallback(TJS_W("readI8"),    &Class::readI8,     0);
	RawCallback(TJS_W("readI16LE"), &Class::readI16LE,  0);
	RawCallback(TJS_W("readI32LE"), &Class::readI32LE,  0);
	RawCallback(TJS_W("readI64LE"), &Class::readI64LE,  0);
	RawCallback(TJS_W("readI16BE"), &Class::readI16BE,  0);
	RawCallback(TJS_W("readI32BE"), &Class::readI32BE,  0);
	RawCallback(TJS_W("readI64BE"), &Class::readI64BE,  0);

	RawCallback(TJS_W("writeI8"),    &Class::writeI8,     0);
	RawCallback(TJS_W("writeI16LE"), &Class::writeI16LE,  0);
	RawCallback(TJS_W("writeI32LE"), &Class::writeI32LE,  0);
	RawCallback(TJS_W("writeI64LE"), &Class::writeI64LE,  0);
	RawCallback(TJS_W("writeI16BE"), &Class::writeI16BE,  0);
	RawCallback(TJS_W("writeI32BE"), &Class::writeI32BE,  0);
	RawCallback(TJS_W("writeI64BE"), &Class::writeI64BE,  0);

	Variant(TJS_W("bsRead"),    (tjs_int)TJS_BS_READ,     0);
	Variant(TJS_W("bsWrite"),   (tjs_int)TJS_BS_WRITE,    0);
	Variant(TJS_W("bsAppend"),  (tjs_int)TJS_BS_APPEND,   0);
	Variant(TJS_W("bsUpdate"),  (tjs_int)TJS_BS_UPDATE,   0);

	Variant(TJS_W("bsSeekSet"), (tjs_int)TJS_BS_SEEK_SET, 0);
	Variant(TJS_W("bsSeekCur"), (tjs_int)TJS_BS_SEEK_CUR, 0);
	Variant(TJS_W("bsSeekEnd"), (tjs_int)TJS_BS_SEEK_END, 0);
}
