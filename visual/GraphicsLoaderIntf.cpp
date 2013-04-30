//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Graphics Loader ( loads graphic format from storage )
//---------------------------------------------------------------------------

#include "tjsCommHead.h"

#include <stdlib.h>
#include "GraphicsLoaderIntf.h"
#include "LayerBitmapIntf.h"
#include "LayerIntf.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "tjsHashSearch.h"
#include "EventIntf.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"
#include "tvpgl.h"
#include "TickCount.h"
#include "DetectCPU.h"
#include "UtilStreams.h"
#include "LoadTLG.h"
#include "tjsDictionary.h"

//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Graphics Format Management
//---------------------------------------------------------------------------
struct tTVPGraphicHandlerType
{
	ttstr Extension;
	tTVPGraphicLoadingHandler Handler;
	void * FormatData;

	tTVPGraphicHandlerType(const ttstr &ext,
		tTVPGraphicLoadingHandler handler, void * data)
	{ Extension = ext, Handler = handler, FormatData = data; }

	tTVPGraphicHandlerType(const tTVPGraphicHandlerType & ref)
	{
		Extension = ref.Extension;
		Handler = ref.Handler;
		FormatData = ref.FormatData;
	}

	bool operator == (const tTVPGraphicHandlerType & ref) const
	{
		return FormatData == ref.FormatData &&
			Handler == ref.Handler &&
			Extension == ref.Extension;
	}
};
class tTVPGraphicType
{
public:
	tTJSHashTable<ttstr, tTVPGraphicHandlerType> Hash;
	std::vector<tTVPGraphicHandlerType> Handlers;

	static bool Avail;

	tTVPGraphicType()
	{
		// register some native-supported formats
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".bmp"), TVPLoadBMP, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".dib"), TVPLoadBMP, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".jpeg"), TVPLoadJPEG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".jpg"), TVPLoadJPEG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".jif"), TVPLoadJPEG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".png"), TVPLoadPNG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".eri"), TVPLoadERI, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".tlg"), TVPLoadTLG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".tlg5"), TVPLoadTLG, NULL));
		Handlers.push_back(tTVPGraphicHandlerType(
			TJS_W(".tlg6"), TVPLoadTLG, NULL));
		ReCreateHash();
		Avail = true;
	}

	~tTVPGraphicType()
	{
		Avail = false;
	}

	void ReCreateHash()
	{
		// re-create hash table for faster search

		std::vector<tTVPGraphicHandlerType>::iterator i;
		for(i = Handlers.begin();
			i!= Handlers.end(); i++)
		{
			Hash.Add(i->Extension, *i);
		}
	}

	void Register(const ttstr &name, tTVPGraphicLoadingHandler handler,
		void * formatdata)
	{
		// register graphic format to the table.
		Handlers.push_back(tTVPGraphicHandlerType(name, handler, formatdata));
		ReCreateHash();
	}

	void Unregister(const ttstr &name, tTVPGraphicLoadingHandler handler,
		void * formatdata)
	{
		// unregister format from table.

		tTVPGraphicHandlerType type(name, handler, formatdata);
		std::vector<tTVPGraphicHandlerType>::iterator i;

		if(Handlers.size() > 0)
		{
			for(i = Handlers.end() -1; i >= Handlers.begin(); i--)
			{
				if(type == *i)
				{
					Handlers.erase(i);
					break;
				}
			}
		}

		ReCreateHash();
	}

} static TVPGraphicType;
bool tTVPGraphicType::Avail = false;
//---------------------------------------------------------------------------
void TVPRegisterGraphicLoadingHandler(const ttstr & name,
	tTVPGraphicLoadingHandler handler, void * formatdata)
{
	// name must be un-capitalized
	if(TVPGraphicType.Avail)
	{
		TVPGraphicType.Register(name, handler, formatdata);
	}
}
//---------------------------------------------------------------------------
void TVPUnregisterGraphicLoadingHandler(const ttstr & name,
	tTVPGraphicLoadingHandler handler, void * formatdata)
{
	// name must be un-capitalized
	if(TVPGraphicType.Avail)
	{
		TVPGraphicType.Unregister(name, handler, formatdata);
	}
}
//---------------------------------------------------------------------------




/*
	loading handlers return whether the image contains an alpha channel.
*/


//---------------------------------------------------------------------------
// BMP loading handler
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define TVP_BMP_READ_LINE_MAX 8
void TVPInternalLoadBMP(void *callbackdata,
	tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback,
	TVP_WIN_BITMAPINFOHEADER &bi,
	const tjs_uint8 *palsrc,
	tTJSBinaryStream * src,
	tjs_int keyidx,
	tTVPBMPAlphaType alphatype,
	tTVPGraphicLoadMode mode)
{
	// mostly taken ( but totally re-written ) from SDL,
	// http://www.libsdl.org/

	// TODO: only checked on Win32 platform


	if(bi.biSize == 12)
	{
		// OS/2
		bi.biCompression = BI_RGB;
		bi.biClrUsed = 1 << bi.biBitCount;
	}

	tjs_uint16 orgbitcount = bi.biBitCount;
	if(bi.biBitCount == 1 || bi.biBitCount == 4)
	{
		bi.biBitCount = 8;
	}

	switch(bi.biCompression)
	{
	case BI_RGB:
		// if there are no masks, use the defaults
		break; // use default
/*
		if( bf.bfOffBits == ( 14 + bi.biSize) )
		{
		}
		// fall through -- read the RGB masks
*/
	case BI_BITFIELDS:
		TVPThrowExceptionMessage(TVPImageLoadError,
			TJS_W("BITFIELDS not supported."));

	default:
		TVPThrowExceptionMessage(TVPImageLoadError,
			TJS_W("Compressed BMP not supported."));
	}

	// load palette
	tjs_uint32 palette[256];   // (msb) argb (lsb)
	if(orgbitcount <= 8)
	{
		if(bi.biClrUsed == 0) bi.biClrUsed = 1 << orgbitcount ;
		if(bi.biSize == 12)
		{
			// read OS/2 palette
			for(tjs_uint i = 0; i < bi.biClrUsed; i++)
			{
				palette[i] = palsrc[0] + (palsrc[1]<<8) + (palsrc[2]<<16) +
					0xff000000;
				palsrc += 3;
			}
		}
		else
		{
			// read Windows palette
			for(tjs_uint i = 0; i<bi.biClrUsed; i++)
			{
				palette[i] = palsrc[0] + (palsrc[1]<<8) + (palsrc[2]<<16) +
					0xff000000;
					// we assume here that the palette's unused segment is useless.
					// fill it with 0xff ( = completely opaque )
				palsrc += 4;
			}
		}

		if(mode == glmGrayscale)
		{
			TVPDoGrayScale(palette, 256);
		}

		if(keyidx != -1)
		{
			// if color key by palette index is specified
			palette[keyidx&0xff] &= 0x00ffffff; // make keyidx transparent
		}
	}
	else
	{
		if(mode == glmPalettized)
			TVPThrowExceptionMessage(TVPImageLoadError,
				TJS_W("Unsupported color mode for palettized image."));
	}

	tjs_int height;
	height = bi.biHeight<0?-bi.biHeight:bi.biHeight;
		// positive value of bi.biHeight indicates top-down DIB

	sizecallback(callbackdata, bi.biWidth, height);

	tjs_int pitch;
	pitch = (((bi.biWidth * orgbitcount) + 31) & ~31) /8;
	tjs_uint8 *readbuf = (tjs_uint8 *)TJSAlignedAlloc(pitch * TVP_BMP_READ_LINE_MAX, 4);
	tjs_uint8 *buf;
	tjs_int bufremain = 0;
	try
	{
		// process per a line
		tjs_int src_y = 0;
		tjs_int dest_y;
		if(bi.biHeight>0) dest_y = bi.biHeight-1; else dest_y = 0;

		for(; src_y < height; src_y++)
		{
			if(bufremain == 0)
			{
				tjs_int remain = height - src_y;
				tjs_int read_lines = remain > TVP_BMP_READ_LINE_MAX ?
					TVP_BMP_READ_LINE_MAX : remain;
				src->ReadBuffer(readbuf, pitch * read_lines);
				bufremain = read_lines;
				buf = readbuf;
			}

			void *scanline = scanlinecallback(callbackdata, dest_y);
			if(!scanline) break;

			switch(orgbitcount)
			{
				// convert pixel format
			case 1:
				if(mode == glmPalettized)
				{
					TVPBLExpand1BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth);
				}
				else if(mode == glmGrayscale)
				{
					TVPBLExpand1BitTo8BitPal(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				else
				{
					TVPBLExpand1BitTo32BitPal(
						(tjs_uint32*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				break;

			case 4:
				if(mode == glmPalettized)
				{
					TVPBLExpand4BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth);
				}
				else if(mode == glmGrayscale)
				{
					TVPBLExpand4BitTo8BitPal(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				else
				{
					TVPBLExpand4BitTo32BitPal(
						(tjs_uint32*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				break;

			case 8:
				if(mode == glmPalettized)
				{
					// intact copy
					memcpy(scanline, buf, bi.biWidth);
				}
				else
				if(mode == glmGrayscale)
				{
					// convert to grayscale
					TVPBLExpand8BitTo8BitPal(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				else
				{
					TVPBLExpand8BitTo32BitPal(
						(tjs_uint32*)scanline,
						(tjs_uint8*)buf, bi.biWidth, palette);
				}
				break;

			case 15:
			case 16:
				if(mode == glmGrayscale)
				{
					TVPBLConvert15BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint16*)buf, bi.biWidth);
				}
				else
				{
					TVPBLConvert15BitTo32Bit(
						(tjs_uint32*)scanline,
						(tjs_uint16*)buf, bi.biWidth);
				}
				break;

			case 24:
				if(mode == glmGrayscale)
				{
					TVPBLConvert24BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint8*)buf, bi.biWidth);
				}
				else
				{
					TVPBLConvert24BitTo32Bit(
						(tjs_uint32*)scanline,
						(tjs_uint8*)buf, bi.biWidth);
				}
				break;

			case 32:
				if(mode == glmGrayscale)
				{
					TVPBLConvert32BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint32*)buf, bi.biWidth);
				}
				else
				{
					if(alphatype == batNone)
					{
						// alpha channel is not given by the bitmap.
						// destination alpha is filled with 255.
						TVPBLConvert32BitTo32Bit_NoneAlpha(
							(tjs_uint32*)scanline,
							(tjs_uint32*)buf, bi.biWidth);
					}
					else if(alphatype == batMulAlpha)
					{
						// this is the TVP native representation of the alpha channel.
						// simply copy from the buffer.
						TVPBLConvert32BitTo32Bit_MulAddAlpha(
							(tjs_uint32*)scanline,
							(tjs_uint32*)buf, bi.biWidth);
					}
					else if(alphatype == batAddAlpha)
					{
						// this is alternate representation of the alpha channel,
						// this must be converted to TVP native representation.
						TVPBLConvert32BitTo32Bit_AddAlpha(
							(tjs_uint32*)scanline,
							(tjs_uint32*)buf, bi.biWidth);

					}
				}
				break;
			}

			scanlinecallback(callbackdata, -1); // image was written

			if(bi.biHeight>0) dest_y--; else dest_y++;
			buf += pitch;
			bufremain--;
		}


	}
	catch(...)
	{
		TJSAlignedDealloc(readbuf);
		throw;
	}

	TJSAlignedDealloc(readbuf);
}
//---------------------------------------------------------------------------
void TVPLoadBMP(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	// Windows BMP Loader
	// mostly taken ( but totally re-written ) from SDL,
	// http://www.libsdl.org/

	// TODO: only checked in Win32 platform



	tjs_uint64 firstpos = src->GetPosition();

	// check the magic
	tjs_uint8 magic[2];
	src->ReadBuffer(magic, 2);
	if(magic[0] != TJS_N('B') || magic[1] != TJS_N('M'))
		TVPThrowExceptionMessage(TVPImageLoadError,
			TJS_W("This is not a Windows BMP file or an OS/2 BMP file."));

	// read the BITMAPFILEHEADER
	TVP_WIN_BITMAPFILEHEADER bf;
	bf.bfSize = src->ReadI32LE();
	bf.bfReserved1 = src->ReadI16LE();
	bf.bfReserved2 = src->ReadI16LE();
	bf.bfOffBits = src->ReadI32LE();

	// read the BITMAPINFOHEADER
	TVP_WIN_BITMAPINFOHEADER bi;
	bi.biSize = src->ReadI32LE();
	if(bi.biSize == 12)
	{
		// OS/2 Bitmap
		memset(&bi, 0, sizeof(bi));
		bi.biWidth = (tjs_uint32)src->ReadI16LE();
		bi.biHeight = (tjs_uint32)src->ReadI16LE();
		bi.biPlanes = src->ReadI16LE();
		bi.biBitCount = src->ReadI16LE();
		bi.biClrUsed = 1 << bi.biBitCount;
	}
	else if(bi.biSize == 40)
	{
		// Windows Bitmap
		bi.biWidth = src->ReadI32LE();
		bi.biHeight = src->ReadI32LE();
		bi.biPlanes = src->ReadI16LE();
		bi.biBitCount = src->ReadI16LE();
		bi.biCompression = src->ReadI32LE();
		bi.biSizeImage = src->ReadI32LE();
		bi.biXPelsPerMeter = src->ReadI32LE();
		bi.biYPelsPerMeter = src->ReadI32LE();
		bi.biClrUsed = src->ReadI32LE();
		bi.biClrImportant = src->ReadI32LE();
	}
	else
	{
		TVPThrowExceptionMessage(TVPImageLoadError,
			TJS_W("Non-supported header version."));
	}


	// load palette
	tjs_int palsize = (bi.biBitCount <= 8) ?
		((bi.biClrUsed == 0 ? (1<<bi.biBitCount) : bi.biClrUsed) *
		((bi.biSize == 12) ? 3:4)) : 0;  // bi.biSize == 12 ( OS/2 palette )
	tjs_uint8 *palette = NULL;

	if(palsize) palette = new tjs_uint8 [palsize];

	try
	{
		src->ReadBuffer(palette, palsize);
		src->SetPosition(firstpos + bf.bfOffBits);

		TVPInternalLoadBMP(callbackdata, sizecallback, scanlinecallback,
			bi, palette, src, keyidx, batMulAlpha, mode);
	}
	catch(...)
	{
		if(palette) delete [] palette;
		throw;
	}
	if(palette) delete [] palette;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// BMP saving handler
//---------------------------------------------------------------------------
static void TVPWriteLE16(tTJSBinaryStream * stream, tjs_uint16 number)
{
	tjs_uint8 data[2];
	data[0] = number & 0xff;
	data[1] = (number >> 8) & 0xff;
	stream->WriteBuffer(data, 2);
}
//---------------------------------------------------------------------------
static void TVPWriteLE32(tTJSBinaryStream * stream, tjs_uint32 number)
{
	tjs_uint8 data[4];
	data[0] = number & 0xff;
	data[1] = (number >> 8) & 0xff;
	data[2] = (number >> 16) & 0xff;
	data[3] = (number >> 24) & 0xff;
	stream->WriteBuffer(data, 4);
}
//---------------------------------------------------------------------------
void TVPSaveAsBMP(const ttstr & storagename, const ttstr & mode, tTVPBaseBitmap *bmp)
{
	if(!bmp->Is32BPP())
		TVPThrowInternalError;

	tjs_int pixelbytes;

	if(mode == TJS_W("bmp32") || mode == TJS_W("bmp"))
		pixelbytes = 4;
	else if(mode == TJS_W("bmp24"))
		pixelbytes = 3;
	else if(mode == TJS_W("bmp8"))
		pixelbytes = 1;
	else TVPThrowExceptionMessage(TVPInvalidImageSaveType, mode);

	// open stream
	tTJSBinaryStream *stream =
		TVPCreateStream(TVPNormalizeStorageName(storagename), TJS_BS_WRITE);
	tjs_uint8 * buf = NULL;

	try
	{
		TVPClearGraphicCache();

		// prepare header
		tjs_uint bmppitch = bmp->GetWidth() * pixelbytes;
		bmppitch = (((bmppitch - 1) >> 2) + 1) << 2;

		TVPWriteLE16(stream, 0x4d42);  /* bfType */
		TVPWriteLE32(stream, sizeof(TVP_WIN_BITMAPFILEHEADER) +
				sizeof(TVP_WIN_BITMAPINFOHEADER) + bmppitch * bmp->GetHeight() +
				(pixelbytes == 1 ? 1024 : 0)); /* bfSize */
		TVPWriteLE16(stream, 0); /* bfReserved1 */
		TVPWriteLE16(stream, 0); /* bfReserved2 */
		TVPWriteLE32(stream, sizeof(TVP_WIN_BITMAPFILEHEADER) +
				sizeof(TVP_WIN_BITMAPINFOHEADER)+
				(pixelbytes == 1 ? 1024 : 0)); /* bfOffBits */

		TVPWriteLE32(stream, sizeof(TVP_WIN_BITMAPINFOHEADER)); /* biSize */
		TVPWriteLE32(stream, bmp->GetWidth()); /* biWidth */
		TVPWriteLE32(stream, bmp->GetHeight()); /* biHeight */
		TVPWriteLE16(stream, 1); /* biPlanes */
		TVPWriteLE16(stream, pixelbytes * 8); /* biBitCount */
		TVPWriteLE32(stream, BI_RGB); /* biCompression */
		TVPWriteLE32(stream, 0); /* biSizeImage */
		TVPWriteLE32(stream, 0); /* biXPelsPerMeter */
		TVPWriteLE32(stream, 0); /* biYPelsPerMeter */
		TVPWriteLE32(stream, 0); /* biClrUsed */
		TVPWriteLE32(stream, 0); /* biClrImportant */

		// write palette
		if(pixelbytes == 1)
		{
			tjs_uint8 palette[1024];
			tjs_uint8 * p = palette;
			for(tjs_int i = 0; i < 256; i++)
			{
				p[0] = TVP252DitherPalette[0][i];
				p[1] = TVP252DitherPalette[1][i];
				p[2] = TVP252DitherPalette[2][i];
				p[3] = 0;
				p += 4;
			}
			stream->WriteBuffer(palette, 1024);
		}

		// write bitmap body
		for(tjs_int y = bmp->GetHeight() - 1; y >= 0; y --)
		{
			if(!buf) buf = new tjs_uint8[bmppitch];
			if(pixelbytes == 4)
			{
				memcpy(buf, bmp->GetScanLine(y), bmppitch);
			}
			else if(pixelbytes == 1)
			{
				TVPDither32BitTo8Bit(buf, (const tjs_uint32*)bmp->GetScanLine(y),
					bmp->GetWidth(), 0, y);  
			}
			else
			{
				const tjs_uint8 *src = (const tjs_uint8 *)bmp->GetScanLine(y);
				tjs_uint8 *dest = buf;
				tjs_int w = bmp->GetWidth();
				for(tjs_int x = 0; x < w; x++)
				{
					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
					dest += 3;
					src += 4;
				}
			}
			stream->WriteBuffer(buf, bmppitch);
		}
	}
	catch(...)
	{
		if(buf) delete [] buf;
		delete stream;
		throw;
	}
	if(buf) delete [] buf;
	delete stream;
}
//---------------------------------------------------------------------------





#pragma option push -pr
extern "C"
{
	// fix me: need platform independ one
#define XMD_H
#include <jinclude.h>
#include <jpeglib.h>
#include <jerror.h>
}


//---------------------------------------------------------------------------
// JPEG loading handler
//---------------------------------------------------------------------------
tTVPJPEGLoadPrecision TVPJPEGLoadPrecision = jlpMedium;
//---------------------------------------------------------------------------
struct my_error_mgr
{
	struct jpeg_error_mgr pub;
};
//---------------------------------------------------------------------------
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	TVPThrowExceptionMessage(TVPJPEGLoadError,
		ttstr(TJS_W("error code : ")) + ttstr(cinfo->err->msg_code));
}
//---------------------------------------------------------------------------
METHODDEF(void)
my_emit_message(j_common_ptr c, int n)
{
}
//---------------------------------------------------------------------------
METHODDEF(void)
my_output_message(j_common_ptr c)
{
}
//---------------------------------------------------------------------------
METHODDEF(void)
my_format_message(j_common_ptr c, char* m)
{
}
//---------------------------------------------------------------------------
METHODDEF(void)
my_reset_error_mgr(j_common_ptr c)
{
	c->err->num_warnings = 0;
	c->err->msg_code = 0;
}
//---------------------------------------------------------------------------
#define BUFFER_SIZE 8192
struct my_source_mgr
{
	jpeg_source_mgr pub;
	JOCTET * buffer;
	tTJSBinaryStream * stream;
	boolean start_of_file;
} ;
//---------------------------------------------------------------------------
METHODDEF(void)
init_source(j_decompress_ptr cinfo)
{
	// ??
  my_source_mgr * src = (my_source_mgr*) cinfo->src;

  src->start_of_file = TRUE;
}
//---------------------------------------------------------------------------
METHODDEF(boolean)
fill_input_buffer(j_decompress_ptr cinfo)
{
  my_source_mgr * src = (my_source_mgr*) cinfo->src;

  int nbytes = src->stream->Read(src->buffer, BUFFER_SIZE);

  if(nbytes <= 0)
  {
	if(src->start_of_file)
		ERREXIT(cinfo, JERR_INPUT_EMPTY);
	WARNMS(cinfo, JWRN_JPEG_EOF);

    src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;

  src->start_of_file = FALSE;

  return TRUE;
}
//---------------------------------------------------------------------------
METHODDEF(void)
skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  my_source_mgr * src = (my_source_mgr*) cinfo->src;

  if (num_bytes > 0) {
	while (num_bytes > (long) src->pub.bytes_in_buffer) {
	  num_bytes -= (long) src->pub.bytes_in_buffer;
	  fill_input_buffer(cinfo);
	  /* note that we assume that fill_input_buffer will never return FALSE,
	   * so suspension need not be handled.
	   */
	}
	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}
//---------------------------------------------------------------------------
METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}
//---------------------------------------------------------------------------
GLOBAL(void)
jpeg_TStream_src (j_decompress_ptr cinfo, tTJSBinaryStream * infile)
{
  my_source_mgr * src;

  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
	cinfo->src = (struct jpeg_source_mgr *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_mgr));
	src = (my_source_mgr * ) cinfo->src;
	src->buffer = (JOCTET *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  BUFFER_SIZE * SIZEOF(JOCTET));
  }

  src = (my_source_mgr *) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->stream = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}
#pragma option pop
//---------------------------------------------------------------------------
void TVPLoadJPEG(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	// JPEG loading handler

	// JPEG does not support palettized image
	if(mode == glmPalettized)
		TVPThrowExceptionMessage(TVPJPEGLoadError,
			ttstr(TJS_W("JPEG does not support palettized image")));

	// prepare variables
	jpeg_decompress_struct cinfo;
	my_error_mgr jerr;
	JSAMPARRAY buffer;
	tjs_int row_stride;

	// error handling
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.emit_message = my_emit_message;
	jerr.pub.output_message = my_output_message;
	jerr.pub.format_message = my_format_message;
	jerr.pub.reset_error_mgr = my_reset_error_mgr;

	// create decompress object
	jpeg_create_decompress(&cinfo);

	// set data source
	jpeg_TStream_src(&cinfo, src);

	// read the header
	jpeg_read_header(&cinfo, TRUE);

	// decompress option
	switch(TVPJPEGLoadPrecision)
	{
	case jlpLow:
		cinfo.dct_method = JDCT_IFAST;
		cinfo.do_fancy_upsampling = FALSE;
		break;
	case jlpMedium:
		cinfo.dct_method = JDCT_IFAST;
		cinfo.do_fancy_upsampling = TRUE;
		break;
	case jlpHigh:
		cinfo.dct_method = JDCT_FLOAT;
		cinfo.do_fancy_upsampling = TRUE;
		break;
	}

	if(mode == glmGrayscale) cinfo.out_color_space =  JCS_GRAYSCALE;

	// start decompression
	jpeg_start_decompress(&cinfo);

	try
	{
		buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE,
				cinfo.output_width * cinfo.output_components + 3,
				cinfo.rec_outbuf_height);

		sizecallback(callbackdata, cinfo.output_width, cinfo.output_height);

		while(cinfo.output_scanline < cinfo.output_height)
		{
			tjs_int startline = cinfo.output_scanline;

			jpeg_read_scanlines(&cinfo, buffer, cinfo.rec_outbuf_height);

			tjs_int endline = cinfo.output_scanline;
			tjs_int bufline;
			tjs_int line;

			for(line = startline, bufline = 0; line < endline; line++, bufline++)
			{
				void *scanline =
					scanlinecallback(callbackdata, line);
				if(!scanline) break;

				// color conversion
				if(mode == glmGrayscale)
				{
					// write through
					memcpy(scanline,
						buffer[bufline], cinfo.output_width);
				}
				else
				{
					if(cinfo.out_color_space == JCS_RGB)
					{
						// expand 24bits to 32bits
						TVPConvert24BitTo32Bit(
							(tjs_uint32*)scanline,
							(tjs_uint8*)buffer[bufline], cinfo.output_width);
					}
					else
					{
						// expand 8bits to 32bits
						TVPExpand8BitTo32BitGray(
							(tjs_uint32*)scanline,
							(tjs_uint8*)buffer[bufline], cinfo.output_width);
					}
				}

				scanlinecallback(callbackdata, -1);
			}
			if(line != endline) break; // interrupted by !scanline
		}
	}
	catch(...)
	{
		jpeg_destroy_decompress(&cinfo);
		throw;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}
//---------------------------------------------------------------------------





#pragma option push -pr
#include <png.h>
#pragma option pop


//---------------------------------------------------------------------------
// PNG loading handler
//---------------------------------------------------------------------------
static ttstr PNG_tag_offs_x(TJS_W("offs_x"));
static ttstr PNG_tag_offs_y(TJS_W("offs_y"));
static ttstr PNG_tag_offs_unit(TJS_W("offs_unit"));
static ttstr PNG_tag_reso_x(TJS_W("reso_x"));
static ttstr PNG_tag_reso_y(TJS_W("reso_y"));
static ttstr PNG_tag_reso_unit(TJS_W("reso_unit"));
static ttstr PNG_tag_vpag_w(TJS_W("vpag_w"));
static ttstr PNG_tag_vpag_h(TJS_W("vpag_h"));
static ttstr PNG_tag_vpag_unit(TJS_W("vpag_unit"));
static ttstr PNG_tag_pixel(TJS_W("pixel"));
static ttstr PNG_tag_micrometer(TJS_W("micrometer"));
static ttstr PNG_tag_meter(TJS_W("meter"));
static ttstr PNG_tag_unknown(TJS_W("unknown"));
//---------------------------------------------------------------------------
// meta callback information structure used by  PNG_read_chunk_callback
struct PNG_read_chunk_callback_user_struct
{
	void * callbackdata;
    tTVPMetaInfoPushCallback metainfopushcallback;
};
//---------------------------------------------------------------------------
// user_malloc_fn
static png_voidp __fastcall PNG_malloc(png_structp ps, png_size_t size)
{
	return malloc(size);
}
//---------------------------------------------------------------------------
// user_free_fn
static void __fastcall PNG_free (png_structp ps,void*/* png_structp*/ mem)
{
	free(mem);
}
//---------------------------------------------------------------------------
// user_error_fn
static void __fastcall PNG_error (png_structp ps, png_const_charp msg)
{
	TVPThrowExceptionMessage(TVPPNGLoadError, msg);
}
//---------------------------------------------------------------------------
// user_warning_fn
static void __fastcall PNG_warning (png_structp ps, png_const_charp msg)
{
	// do nothing
}
//---------------------------------------------------------------------------
// user_read_data
static void __fastcall PNG_read_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
	((tTJSBinaryStream *)png_get_io_ptr(png_ptr))->ReadBuffer((void*)data, length);
}
//---------------------------------------------------------------------------
// read_row_callback
static void __fastcall PNG_read_row_callback(png_structp png_ptr,png_uint_32 row,int pass)
{

}
//---------------------------------------------------------------------------
// read_chunk_callback
static int __fastcall PNG_read_chunk_callback(png_structp png_ptr,png_unknown_chunkp chunk)
{
	// handle vpAg chunk (this will contain the virtual page size of the image)
	// vpAg chunk can be embeded by ImageMagick -trim option etc.
	// we don't care about how the chunk bit properties are being provided.
	if(	(chunk->name[0] == 0x76/*'v'*/ || chunk->name[0] == 0x56/*'V'*/) &&
		(chunk->name[1] == 0x70/*'p'*/ || chunk->name[1] == 0x50/*'P'*/) &&
		(chunk->name[2] == 0x61/*'a'*/ || chunk->name[2] == 0x41/*'A'*/) &&
		(chunk->name[3] == 0x67/*'g'*/ || chunk->name[3] == 0x47/*'G'*/) && chunk->size >= 9)
	{
		PNG_read_chunk_callback_user_struct * user_struct =
			reinterpret_cast<PNG_read_chunk_callback_user_struct *>(png_get_user_chunk_ptr(png_ptr));
		// vpAg found
		/*
			uint32 width
			uint32 height
			uchar unit
		*/
		// be careful because the integers are stored in network byte order
		#define PNG_read_be32(a) (((tjs_uint32)(a)[0]<<24)+\
			((tjs_uint32)(a)[1]<<16)+((tjs_uint32)(a)[2]<<8)+\
			((tjs_uint32)(a)[3]))
		tjs_uint32 width  = PNG_read_be32(chunk->data+0);
		tjs_uint32 height = PNG_read_be32(chunk->data+4);
		tjs_uint8  unit   = chunk->data[8];

		// push information into meta-info
		user_struct->metainfopushcallback(user_struct->callbackdata, PNG_tag_vpag_w, ttstr((tjs_int)width));
		user_struct->metainfopushcallback(user_struct->callbackdata, PNG_tag_vpag_h, ttstr((tjs_int)height));
		switch(unit)
		{
		case PNG_OFFSET_PIXEL:
			user_struct->metainfopushcallback(user_struct->callbackdata, PNG_tag_vpag_unit, PNG_tag_pixel);
			break;
		case PNG_OFFSET_MICROMETER:
			user_struct->metainfopushcallback(user_struct->callbackdata, PNG_tag_vpag_unit, PNG_tag_micrometer);
			break;
		default:
			user_struct->metainfopushcallback(user_struct->callbackdata, PNG_tag_vpag_unit, PNG_tag_unknown);
			break;
		}
		return 1; // chunk read success
	}
	return 0; // did not recognize
}
//---------------------------------------------------------------------------
void TVPLoadPNG(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	png_structp png_ptr=NULL;
	png_infop info_ptr=NULL;
	png_infop end_info=NULL;

	png_uint_32 i;

	png_bytep *row_pointers=NULL;
	tjs_uint8 *image=NULL;


	try
	{
		// create png_struct
		png_ptr=png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
			(png_voidp)NULL, PNG_error, PNG_warning,
			(png_voidp)NULL, PNG_malloc, PNG_free);

		// set read_chunk_callback
		PNG_read_chunk_callback_user_struct read_chunk_callback_user_struct;
		read_chunk_callback_user_struct.callbackdata = callbackdata;
		read_chunk_callback_user_struct.metainfopushcallback = metainfopushcallback;
		png_set_read_user_chunk_fn(png_ptr,
			reinterpret_cast<void*>(&read_chunk_callback_user_struct),
			PNG_read_chunk_callback);
		png_set_keep_unknown_chunks(png_ptr, 2, NULL, 0);
			// keep only if safe-to-copy chunks, for all unknown chunks

		// create png_info
		info_ptr=png_create_info_struct(png_ptr);

		// create end_info
		end_info=png_create_info_struct(png_ptr);

		// set stream interface
		png_set_read_fn(png_ptr,(voidp)src, PNG_read_data);

		// set read_row_callback
		png_set_read_status_fn(png_ptr, PNG_read_row_callback);

		// set png_read_info
		png_read_info(png_ptr,info_ptr);

		// retrieve IHDR
		png_uint_32 width,height;
		int bit_depth,color_type,interlace_type,compression_type,filter_type;
		png_get_IHDR(png_ptr,info_ptr,&width,&height,&bit_depth,&color_type,
			&interlace_type,&compression_type,&filter_type);

		if(bit_depth==16) png_set_strip_16(png_ptr);

		// retrieve offset information
		png_int_32 offset_x, offset_y;
		int offset_unit_type;
		if(metainfopushcallback &&
			png_get_oFFs(png_ptr, info_ptr, &offset_x, &offset_y, &offset_unit_type))
		{
			// push offset information into metainfo data
			metainfopushcallback(callbackdata, PNG_tag_offs_x, ttstr((tjs_int)offset_x));
			metainfopushcallback(callbackdata, PNG_tag_offs_y, ttstr((tjs_int)offset_y));
			switch(offset_unit_type)
			{
			case PNG_OFFSET_PIXEL:
				metainfopushcallback(callbackdata, PNG_tag_offs_unit, PNG_tag_pixel);
				break;
			case PNG_OFFSET_MICROMETER:
				metainfopushcallback(callbackdata, PNG_tag_offs_unit, PNG_tag_micrometer);
				break;
			default:
				metainfopushcallback(callbackdata, PNG_tag_offs_unit, PNG_tag_unknown);
				break;
			}
		}
		
		png_uint_32 reso_x, reso_y;
		int reso_unit_type;
		if(metainfopushcallback &&
			png_get_pHYs(png_ptr, info_ptr, &reso_x, &reso_y, &reso_unit_type))
		{
			// push offset information into metainfo data
			metainfopushcallback(callbackdata, PNG_tag_reso_x, ttstr((tjs_int)reso_x));
			metainfopushcallback(callbackdata, PNG_tag_reso_y, ttstr((tjs_int)reso_y));
			switch(reso_unit_type)
			{
			case PNG_RESOLUTION_METER:
				metainfopushcallback(callbackdata, PNG_tag_reso_unit, PNG_tag_meter);
				break;
			default:
				metainfopushcallback(callbackdata, PNG_tag_reso_unit, PNG_tag_unknown);
				break;
			}
		}


		bool do_convert_rgb_gray = false;

		if(mode == glmPalettized)
		{
			// convert the image to palettized one if needed
			if(bit_depth > 8)
				TVPThrowExceptionMessage(
					TVPPNGLoadError, TJS_W("Unsupported color type for palattized image"));

			if(color_type == PNG_COLOR_TYPE_PALETTE)
			{
				png_set_packing(png_ptr);
			}

			if(color_type == PNG_COLOR_TYPE_GRAY) png_set_gray_1_2_4_to_8(png_ptr);
		}
		else if(mode == glmGrayscale)
		{
			// convert the image to grayscale
			if(color_type == PNG_COLOR_TYPE_PALETTE)
			{
				png_set_palette_to_rgb(png_ptr);
				png_set_bgr(png_ptr);
				if (bit_depth < 8)
					png_set_packing(png_ptr);
				do_convert_rgb_gray = true; // manual conversion
			}
			if(color_type == PNG_COLOR_TYPE_GRAY &&
				bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
			if(color_type == PNG_COLOR_TYPE_RGB ||
				color_type == PNG_COLOR_TYPE_RGB_ALPHA)
				png_set_rgb_to_gray_fixed(png_ptr, 1, 0, 0);
			if(color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
				color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
				png_set_strip_alpha(png_ptr);
		}
		else
		{
			// glmNormal
			// convert the image to full color ( 32bits ) one if needed

			if(color_type == PNG_COLOR_TYPE_PALETTE)
			{
				if(keyidx == -1)
				{
					if(png_get_valid(png_ptr, info_ptr,PNG_INFO_tRNS))
					{
						// set expansion with palettized picture
						png_set_palette_to_rgb(png_ptr);
						png_set_tRNS_to_alpha(png_ptr);
						color_type=PNG_COLOR_TYPE_RGB_ALPHA;
					}
					else
					{
						png_set_palette_to_rgb(png_ptr);
						color_type=PNG_COLOR_TYPE_RGB;
					}
				}
				else
				{
					png_byte trans = (png_byte) keyidx;
					png_set_tRNS(png_ptr, info_ptr, &trans, 1, 0);
						// make keyidx transparent color.
					png_set_palette_to_rgb(png_ptr);
					png_set_tRNS_to_alpha(png_ptr);
					color_type=PNG_COLOR_TYPE_RGB_ALPHA;

				}
			}

			switch(color_type)
			{
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				png_set_gray_to_rgb(png_ptr);
				color_type=PNG_COLOR_TYPE_RGB_ALPHA;
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
//				alpha_valid = true;
				break;
			case PNG_COLOR_TYPE_GRAY:
				png_set_expand(png_ptr);
				png_set_gray_to_rgb(png_ptr);
				color_type=PNG_COLOR_TYPE_RGB;
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
//				alpha_valid=true;
				png_set_bgr(png_ptr);
				break;
			case PNG_COLOR_TYPE_RGB:
				png_set_bgr(png_ptr);
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
				break;
			default:
				TVPThrowExceptionMessage(
					TVPPNGLoadError, TJS_W("Unsupported color type"));
			}
		}


		// size checking
		if(width>=65536 || height>=65536)
		{
			// too large image to handle
			TVPThrowExceptionMessage(
					TVPPNGLoadError, TJS_W("Too large image"));
		}


		// call png_read_update_info
		png_read_update_info(png_ptr,info_ptr);

		// set size
		sizecallback(callbackdata, width, height);

		// load image
		if(info_ptr->interlace_type == PNG_INTERLACE_NONE)
		{
			// non-interlace
			if(do_convert_rgb_gray)
			{
				png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
				image = new tjs_uint8[rowbytes];
			}

			for(i=0; i<height; i++)
			{
				void *scanline = scanlinecallback(callbackdata, i);
				if(!scanline) break;
				if(!do_convert_rgb_gray)
				{
					png_read_row(png_ptr, (png_bytep)scanline, NULL);
				}
				else
				{
					png_read_row(png_ptr, (png_bytep)image, NULL);
					TVPBLConvert24BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint8*)image, width);
				}
				scanlinecallback(callbackdata, -1);
			}

			// finish loading
			png_read_end(png_ptr,info_ptr);
		}
		else
		{
			// interlace handling
			// load the image at once

			row_pointers = new png_bytep[height];
			png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
			image = new tjs_uint8[rowbytes * height];
			for(i=0; i<height; i++)
			{
				row_pointers[i] = image+ i*rowbytes;
			}

			// loads image
			png_read_image(png_ptr, row_pointers);

			// finish loading
			png_read_end(png_ptr, info_ptr);

			// set the pixel data
			for(i=0; i<height; i++)
			{
				void *scanline = scanlinecallback(callbackdata, i);
				if(!scanline) break;
				if(!do_convert_rgb_gray)
				{
					memcpy(scanline, row_pointers[i], rowbytes);
				}
				else
				{
					TVPBLConvert24BitTo8Bit(
						(tjs_uint8*)scanline,
						(tjs_uint8*)row_pointers[i], width);
				}
				scanlinecallback(callbackdata, -1);
			}
		}
	}
	catch(...)
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
		if(row_pointers) delete [] row_pointers;
		if(image) delete [] image;
		throw;
	}

	png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
	if(row_pointers) delete [] row_pointers;
	if(image) delete [] image;

}
//---------------------------------------------------------------------------






#ifdef TVP_SUPPORT_ERI

//---------------------------------------------------------------------------
// ERI loading handler
//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option push -pc -Vms
	// fix me: need platform independ one
#pragma pack(push, 1)
#endif

#include <eritypes.h>
#include <erinalib.h>

#ifdef __BORLANDC__
#pragma pack(pop)
#endif

//---------------------------------------------------------------------------
// eriAllocateMemory and eriFreeMemory
//---------------------------------------------------------------------------
PVOID eriAllocateMemory( DWORD dwBytes )
{
	return TJSAlignedAlloc(dwBytes, 4);
}
//---------------------------------------------------------------------------
void eriFreeMemory( PVOID ptrMem )
{
	TJSAlignedDealloc(ptrMem);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTVPEFileObject : class for storage access
//---------------------------------------------------------------------------
class tTVPEFileObject : public ::EFileObject
{
	tTJSBinaryStream * Stream;
public:
	tTVPEFileObject(tTJSBinaryStream *ref);
	~tTVPEFileObject();

	virtual DWORD Read( void * ptrBuf, DWORD dwByte );
	virtual DWORD GetLength( void );
	virtual DWORD GetPointer( void );
	virtual void Seek( DWORD dwPointer );
};
//---------------------------------------------------------------------------
tTVPEFileObject::tTVPEFileObject(tTJSBinaryStream *ref)
{
	// tTVPEFileObject constructor

	// this object does not own "ref", so user routine still has responsibility
	// for "ref".

	Stream = ref;
//	if(ref->GetSize() & TJS_UI64_VAL(0xffffffff00000000))
	// if-statement above may cause a bcc i64 comparison-to-zero bug
	if(((tjs_uint64)ref->GetSize()) >> 32) // this is still buggy, but works
	{
		// too large to handle with tEFlieObject
		TVPThrowExceptionMessage(TVPERILoadError, TJS_W("Too large storage"));
	}
}
//---------------------------------------------------------------------------
tTVPEFileObject::~tTVPEFileObject()
{
	// tTVPEFileObject destructor
	// nothing to do
}
//---------------------------------------------------------------------------
DWORD tTVPEFileObject::Read( void * ptrBuf, DWORD dwByte )
{
	return (DWORD)Stream->Read(ptrBuf, dwByte);
}
//---------------------------------------------------------------------------
DWORD tTVPEFileObject::GetLength( void )
{
	return (DWORD)Stream->GetSize();
}
//---------------------------------------------------------------------------
DWORD tTVPEFileObject::GetPointer( void )
{
	return (DWORD)Stream->GetPosition();
}
//---------------------------------------------------------------------------
void tTVPEFileObject::Seek( DWORD dwPointer )
{
	Stream->SetPosition(dwPointer);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTVPRLHDecodeContext
//---------------------------------------------------------------------------
class tTVPRLHDecodeContext : public ::RLHDecodeContext
{
	::ERIFile & ERIFileObject;

public:
	tTVPRLHDecodeContext(::ERIFile & erifile, ULONG nBufferingSize)
		: ERIFileObject(erifile), RLHDecodeContext(nBufferingSize) {};
	virtual ULONG ReadNextData(PBYTE ptrBuffer, ULONG nBytes )
		{ return ERIFileObject.Read(ptrBuffer, nBytes); }
};
//---------------------------------------------------------------------------


#ifdef __BORLANDC__
#pragma option pop
#endif

#ifdef _M_IX86
#include "tvpgl_ia32_intf.h"
#endif

//---------------------------------------------------------------------------
static bool TVPERINAInit = false;
static void TVPInitERINA()
{
	if(TVPERINAInit) return;
	::eriInitializeLibrary();

	// sync simd features with the library.
#ifdef _M_IX86
	DWORD flags;

	flags = (TVPCPUType & TVP_CPU_HAS_MMX) ? ERI_USE_MMX_PENTIUM : 0 +
			(TVPCPUType & TVP_CPU_HAS_SSE) ? ERI_USE_XMM_P3 : 0;
	if(flags) ::eriEnableMMX(flags);

	flags = !(TVPCPUType & TVP_CPU_HAS_MMX) ? ERI_USE_MMX_PENTIUM : 0 +
			!(TVPCPUType & TVP_CPU_HAS_SSE) ? ERI_USE_XMM_P3 : 0;
	if(flags) ::eriDisableMMX(flags);
#endif

	TVPERINAInit = true;
}
//---------------------------------------------------------------------------
static void TVPUninitERINA()
{
	if(TVPERINAInit)
		::eriCloseLibrary();
}
//---------------------------------------------------------------------------
static tTVPAtExit TVPUninitERINAAtExit
	(TVP_ATEXIT_PRI_CLEANUP, TVPUninitERINA);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void TVPLoadERI(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	// ERI loading handler
	TVPInitERINA();

	// ERI-chan ( stands for "Entis Rasterized Image" ) will be
	// available at http://www.entis.jp/eri/

	// currently TVP's ERI handler does not support palettized ERI loading
	if(mode == glmPalettized)
		TVPThrowExceptionMessage(TVPERILoadError,
			TJS_W("currently Kirikiri does not support loading palettized ERI image."));

	// create wrapper object for EFileObject
	tTVPEFileObject fileobject(src);

	// open with ERIFile
	::ERIFile erifile;
	if(!erifile.Open(&fileobject))
	{
		TVPThrowExceptionMessage(TVPERILoadError, TJS_W("ERIFile::Open failed."));
	}

	// set size
	sizecallback(callbackdata,
		erifile.m_InfoHeader.nImageWidth,
		std::abs(static_cast<int>(erifile.m_InfoHeader.nImageHeight)));

	// set RASTER_IMAGE_INFO up
	bool has_alpha;
	::RASTER_IMAGE_INFO rii;
	if(erifile.m_InfoHeader.dwBitsPerPixel == 8 &&
		!(erifile.m_InfoHeader.fdwFormatType & ERI_WITH_ALPHA))
	{
		// grayscaled or paletted image
		rii.fdwFormatType = erifile.m_InfoHeader.fdwFormatType;
		rii.dwBitsPerPixel = 8;
		has_alpha = false;
	}
	else if(erifile.m_InfoHeader.dwBitsPerPixel >= 24)
	{
		rii.fdwFormatType = ERI_RGBA_IMAGE;
		rii.dwBitsPerPixel = 32;
		has_alpha = erifile.m_InfoHeader.fdwFormatType & ERI_WITH_ALPHA;
	}
	else
	{
		// ??
		rii.fdwFormatType = (mode == glmGrayscale) ? ERI_GRAY_IMAGE : ERI_RGBA_IMAGE;
		rii.dwBitsPerPixel = (mode == glmGrayscale) ? 8 : 32;
		has_alpha = erifile.m_InfoHeader.fdwFormatType & ERI_WITH_ALPHA;
	}
	
	tjs_int h;
	tjs_int w;
	rii.nImageWidth = w = erifile.m_InfoHeader.nImageWidth;
	tjs_uint imagesize;
	rii.nImageHeight = h = std::abs(static_cast<int>(erifile.m_InfoHeader.nImageHeight));

	rii.BytesPerLine = ((rii.nImageWidth * rii.dwBitsPerPixel /8)+0x07)& ~0x07;

	rii.ptrImageArray = (PBYTE)malloc(imagesize = h * rii.BytesPerLine);
	if(!rii.ptrImageArray)
		TVPThrowExceptionMessage(TVPERILoadError, TJS_W("Insufficient memory."));


	tjs_uint32 *palette = NULL;
	try
	{
		// create decode context
		tTVPRLHDecodeContext dc(erifile, 0x10000);

		// create decoder
		::ERIDecoder ed;
		if(ed.Initialize(erifile.m_InfoHeader))
			TVPThrowExceptionMessage(TVPERILoadError,
				TJS_W("ERIDecoder::Initialize failed."));

		// decode
		if(ed.DecodeImage(rii, dc, false))
			TVPThrowExceptionMessage(TVPERILoadError,
				TJS_W("ERIDecoder::DecodeImage failed."));

		// prepare the palette
		if(rii.dwBitsPerPixel == 8 &&
			(erifile.m_InfoHeader.fdwFormatType&ERI_TYPE_MASK) != ERI_GRAY_IMAGE)
		{
			palette = new tjs_uint32 [256];
			for(tjs_int i = 0; i<256; i++)
			{
				// CHECK: endian-ness should be considered ??
				tjs_uint32 val = *(tjs_uint32*)&(erifile.m_PaletteTable[i]);
				val |= 0xff000000;
				palette[i] = val;
			}

			// convert palette to monochrome
			if(mode == glmGrayscale)
				TVPDoGrayScale(palette, 256);

			if(keyidx != -1)
			{
				// if color key by palette index is specified
				palette[keyidx&0xff] &= 0x00ffffff; // make keyidx transparent
			}
		}

		// put image back to caller

		// TODO: more efficient implementation
		// (including to write more store functions )
		tjs_int pitch = rii.BytesPerLine;
		const tjs_uint8 *in = ((const tjs_uint8 *)rii.ptrImageArray) +
			imagesize - pitch;
		for(tjs_int i=0; i<h; i++)
		{
			void *scanline = scanlinecallback(callbackdata, i);
			if(!scanline) break;
			if(mode == glmNormal)
			{
				// destination is RGBA
				if(rii.dwBitsPerPixel == 8)
				{
					if((erifile.m_InfoHeader.fdwFormatType&ERI_TYPE_MASK) ==
						ERI_GRAY_IMAGE)
					{
						// source is gray scale
						TVPExpand8BitTo32BitGray((tjs_uint32*)scanline,
							(const tjs_uint8 *)in, w);
					}
					else
					{
						// source is paletted image
						TVPBLExpand8BitTo32BitPal((tjs_uint32*)scanline,
							(const tjs_uint8 *)in, w, palette);
					}

				}
				else
				{
					if(!has_alpha)
					{
						// source does not have alpha
						TVPCopyOpaqueImage((tjs_uint32*)scanline,
							(const tjs_uint32*)in, w);
					}
					else
					{
						TVPBLConvert32BitTo32Bit_AddAlpha(
							(tjs_uint32*)scanline,
							(tjs_uint32*)in, w);
					}
				}
			}
			else
			{
				// destination is grayscale
				if(rii.dwBitsPerPixel == 8)
				{
					if((erifile.m_InfoHeader.fdwFormatType&ERI_TYPE_MASK) ==
						ERI_GRAY_IMAGE)
					{
						// source is gray scale
						memcpy(scanline, in, w);
					}
					else
					{
						// source is paletted image
						TVPBLExpand8BitTo8BitPal((tjs_uint8*)scanline,
							(const tjs_uint8 *)in, w, palette);
					}
				}
				else
				{
					TVPBLConvert32BitTo8Bit((tjs_uint8*)scanline,
						(const tjs_uint32 *)in, w);

				}
			}

			scanlinecallback(callbackdata, -1);
			in -= pitch;
		}
	}
	catch(...)
	{
		if(palette) delete [] palette;
		free(rii.ptrImageArray);
		throw;
	}

	if(palette) delete [] palette;
	free(rii.ptrImageArray);
}
//---------------------------------------------------------------------------
#else // #ifdef TVP_SUPPORT_ERI
void TVPLoadERI(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	TVPThrowExceptionMessage(TVPNotImplemented);
}
//---------------------------------------------------------------------------
#endif // #ifdef TVP_SUPPORT_ERI























//---------------------------------------------------------------------------
// TVPLoadGraphic related
//---------------------------------------------------------------------------
enum tTVPLoadGraphicType
{
	lgtFullColor, // full 32bit color
	lgtPalGray, // palettized or grayscale
	lgtMask // mask
};
struct tTVPGraphicMetaInfoPair
{
	ttstr Name;
	ttstr Value;
	tTVPGraphicMetaInfoPair(const ttstr &name, const ttstr &value) :
		Name(name), Value(value) {;}
};
struct tTVPLoadGraphicData
{
	ttstr Name;
	tTVPBaseBitmap *Dest;
	tTVPLoadGraphicType Type;
	tjs_int ColorKey;
	tjs_uint8 *Buffer;
	tjs_uint ScanLineNum;
	tjs_uint DesW;
	tjs_uint DesH;
	tjs_uint OrgW;
	tjs_uint OrgH;
	tjs_uint BufW;
	tjs_uint BufH;
	bool NeedMetaInfo;
	std::vector<tTVPGraphicMetaInfoPair> * MetaInfo;
};
//---------------------------------------------------------------------------
static void TVPLoadGraphic_SizeCallback(void *callbackdata, tjs_uint w,
	tjs_uint h)
{
	tTVPLoadGraphicData * data = (tTVPLoadGraphicData *)callbackdata;

	// check size
	data->OrgW = w;
	data->OrgH = h;
	if(data->DesW && w < data->DesW) w = data->DesW;
	if(data->DesH && h < data->DesH) h = data->DesH;
	data->BufW = w;
	data->BufH = h;

	// create buffer
	if(data->Type == lgtMask)
	{
		// mask ( _m ) load

		// check the image previously loaded
		if(data->Dest->GetWidth() != w || data->Dest->GetHeight() != h)
			TVPThrowExceptionMessage(TVPMaskSizeMismatch);

		// allocate line buffer
		data->Buffer = new tjs_uint8 [w];
	}
	else
	{
		// normal load or province load
		data->Dest->Recreate(w, h, data->Type!=lgtFullColor?8:32);
	}
}
//---------------------------------------------------------------------------
static void * TVPLoadGraphic_ScanLineCallback(void *callbackdata, tjs_int y)
{
	tTVPLoadGraphicData * data = (tTVPLoadGraphicData *)callbackdata;

	if(y >= 0)
	{
		// query of line buffer

		data->ScanLineNum = y;
		if(data->Type == lgtMask)
		{
			// mask
			return data->Buffer;
		}
		else
		{
			// return the scanline for writing
			return data->Dest->GetScanLineForWrite(y);
		}
	}
	else
	{
		// y==-1 indicates the buffer previously returned was written

		if(data->Type == lgtMask)
		{
			// mask

			// tile for horizontal direction
			tjs_uint i;
			for(i = data->OrgW; i<data->BufW; i+=data->OrgW)
			{
				tjs_uint w = data->BufW - i;
				w = w > data->OrgW ? data->OrgW : w;
				memcpy(data->Buffer + i, data->Buffer, w);
			}

			// bind mask buffer to main image buffer ( and tile for vertical )
			for(i = data->ScanLineNum; i<data->BufH; i+=data->OrgH)
			{
				TVPBindMaskToMain(
					(tjs_uint32*)data->Dest->GetScanLineForWrite(i),
					data->Buffer, data->BufW);
			}
			return NULL;
		}
		else if(data->Type == lgtFullColor)
		{
			tjs_uint32 * sl =
				(tjs_uint32*)data->Dest->GetScanLineForWrite(data->ScanLineNum);
			if((data->ColorKey & 0xff000000) == 0x00000000)
			{
				// make alpha from color key
				TVPMakeAlphaFromKey(
					sl,
					data->BufW,
					data->ColorKey);
			}

			// tile for horizontal direction
			tjs_uint i;
			for(i = data->OrgW; i<data->BufW; i+=data->OrgW)
			{
				tjs_uint w = data->BufW - i;
				w = w > data->OrgW ? data->OrgW : w;
				memcpy(sl + i, sl, w * sizeof(tjs_uint32));
			}

			// tile for vertical direction
			for(i = data->ScanLineNum + data->OrgH; i<data->BufH; i+=data->OrgH)
			{
				memcpy(
					(tjs_uint32*)data->Dest->GetScanLineForWrite(i),
					sl,
					data->BufW * sizeof(tjs_uint32) );
			}

			return NULL;
		}
		else if(data->Type == lgtPalGray)
		{
			// nothing to do
			if(data->OrgW < data->BufW || data->OrgH < data->BufH)
			{
				tjs_uint8 * sl =
					(tjs_uint8*)data->Dest->GetScanLineForWrite(data->ScanLineNum);
				tjs_uint i;

				// tile for horizontal direction
				for(i = data->OrgW; i<data->BufW; i+=data->OrgW)
				{
					tjs_uint w = data->BufW - i;
					w = w > data->OrgW ? data->OrgW : w;
					memcpy(sl + i, sl, w * sizeof(tjs_uint8));
				}

				// tile for vertical direction
				for(i = data->ScanLineNum + data->OrgH; i<data->BufH; i+=data->OrgH)
				{
					memcpy(
						(tjs_uint8*)data->Dest->GetScanLineForWrite(i),
						sl,
						data->BufW * sizeof(tjs_uint8));
				}
			}

			return NULL;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------
static void TVPLoadGraphic_MetaInfoPushCallback(void *callbackdata,
	const ttstr & name, const ttstr & value)
{
	tTVPLoadGraphicData * data = (tTVPLoadGraphicData *)callbackdata;

	if(data->NeedMetaInfo)
	{
		if(!data->MetaInfo) data->MetaInfo = new std::vector<tTVPGraphicMetaInfoPair>();
		data->MetaInfo->push_back(tTVPGraphicMetaInfoPair(name, value));
	}
}
//---------------------------------------------------------------------------
static int _USERENTRY TVPColorCompareFunc(const void *_a, const void *_b)
{
	tjs_uint32 a = *(const tjs_uint32*)_a;
	tjs_uint32 b = *(const tjs_uint32*)_b;

	if(a<b) return -1;
	if(a==b) return 0;
	return 1;
}
//---------------------------------------------------------------------------
static void TVPMakeAlphaFromAdaptiveColor(tTVPBaseBitmap *dest)
{
	// make adaptive color key and make alpha from it.
	// adaptive color key is most used(popular) color at first line of the
	// graphic.
	if(!dest->Is32BPP()) return;

	// copy first line to buffer
	tjs_int w = dest->GetWidth();
	tjs_int pitch =  std::abs(dest->GetPitchBytes());
	tjs_uint32 * buffer = new tjs_uint32[pitch];

	try
	{
		const void *d = dest->GetScanLine(0);

		memcpy(buffer, d, pitch);
		tjs_int i;
		for(i = w -1; i>=0; i--) buffer[i] &= 0xffffff;
		buffer[w] = (tjs_uint32)-1; // a sentinel

		// sort by color
		qsort(buffer, dest->GetWidth(), sizeof(tjs_uint32), TVPColorCompareFunc);

		// find most used color
		tjs_int maxlen = 0;
		tjs_uint32 maxlencolor = 0;
		tjs_uint32 pcolor = (tjs_uint32)-1;
		tjs_int l = 0;
		for(i = 0; i< w+1; i++)
		{
			if(buffer[i] != pcolor)
			{
				if(maxlen < l)
				{
					maxlen = l;
					maxlencolor = pcolor;
					l = 0;
				}
			}
			else
			{
				l++;
			}
			pcolor = buffer[i];
		}

		if(maxlencolor == (tjs_uint32)-1)
		{
			// may color be not found...
			maxlencolor = 0; // black is a default colorkey
		}

		// make alpha from maxlencolor
		tjs_int h;
		for(h = dest->GetHeight()-1; h>=0; h--)
		{
			TVPMakeAlphaFromKey((tjs_uint32*)dest->GetScanLineForWrite(h),
				w, maxlencolor);

		}

	}
	catch(...)
	{
		delete [] buffer;
		throw;
	}

	delete [] buffer;
}
//---------------------------------------------------------------------------
static void TVPDoAlphaColorMat(tTVPBaseBitmap *dest, tjs_uint32 color)
{
	// Do alpha matting.
	// 'mat' means underlying color of the image. This function piles
	// specified color under the image, then blend. The output image
	// will be totally opaque. This function always assumes the image
	// has pixel value for alpha blend mode, not additive alpha blend mode.
	if(!dest->Is32BPP()) return;

	tjs_int w = dest->GetWidth();
	tjs_int h = dest->GetHeight();

	for(tjs_int y = 0; y < h; y++)
	{
		tjs_uint32 * buffer = (tjs_uint32*)dest->GetScanLineForWrite(y);
		TVPAlphaColorMat(buffer, color, w);
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static iTJSDispatch2 * TVPMetaInfoPairsToDictionary(
	std::vector<tTVPGraphicMetaInfoPair> *vec)
{
	if(!vec) return NULL;
	std::vector<tTVPGraphicMetaInfoPair>::iterator i;
	iTJSDispatch2 *dic = TJSCreateDictionaryObject();
	try
	{
		for(i = vec->begin(); i != vec->end(); i++)
		{
			tTJSVariant val(i->Value);
			dic->PropSet(TJS_MEMBERENSURE, i->Name.c_str(), 0,
				&val, dic);
		}
	}
	catch(...)
	{
		dic->Release();
		throw;
	}
	return dic;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Graphics Cache Management
//---------------------------------------------------------------------------
bool TVPAllocGraphicCacheOnHeap = false;
	// this allocates graphic cache's store memory on heap, rather than
	// sharing bitmap object. ( since sucking win9x cannot have so many bitmap
	// object at once, WinNT/2000 is ok. )
	// this will take more time for memory copying.
//---------------------------------------------------------------------------
struct tTVPGraphicsSearchData
{
	ttstr Name;
	tjs_int32 KeyIdx; // color key index
	tTVPGraphicLoadMode Mode; // image mode
	tjs_uint DesW; // desired width ( 0 for original size )
	tjs_uint DesH; // desired height ( 0 for original size )

	bool operator == (const tTVPGraphicsSearchData &rhs) const
	{
		return KeyIdx == rhs.KeyIdx && Mode == rhs.Mode &&
			Name == rhs.Name && DesW == rhs.DesW && DesH == rhs.DesH;
	}
};
//---------------------------------------------------------------------------
class tTVPGraphicsSearchHashFunc
{
public:
	static tjs_uint32 Make(const tTVPGraphicsSearchData &val)
	{
		tjs_uint32 v = tTJSHashFunc<ttstr>::Make(val.Name);

		v ^= val.KeyIdx + (val.KeyIdx >> 23);
		v ^= (val.Mode << 30);
		v ^= val.DesW + (val.DesW >> 8);
		v ^= val.DesH + (val.DesH >> 8);
		return v;
	}
};
//---------------------------------------------------------------------------
class tTVPGraphicImageData
{
private:
	tTVPBaseBitmap *Bitmap;
	tjs_uint8 * RawData;
	tjs_int Width;
	tjs_int Height;
	tjs_int PixelSize;

public:
	ttstr ProvinceName;

	std::vector<tTVPGraphicMetaInfoPair> * MetaInfo;

private:
	tjs_int RefCount;
	tjs_uint Size;

public:
	tTVPGraphicImageData()
	{
		RefCount = 1; Size = 0; Bitmap = NULL; RawData = NULL;
		MetaInfo = NULL;
	}
	~tTVPGraphicImageData()
	{
		if(Bitmap) delete Bitmap;
		if(RawData) delete [] RawData;
		if(MetaInfo) delete MetaInfo;
	}

	void AssignBitmap(const tTVPBaseBitmap *bmp)
	{
		if(Bitmap) delete Bitmap, Bitmap = NULL;
		if(RawData) delete [] RawData, RawData = NULL;

		Width = bmp->GetWidth();
		Height = bmp->GetHeight();
		PixelSize = bmp->Is32BPP()?4:1;
		Size =  Width*Height*PixelSize;

		if(!TVPAllocGraphicCacheOnHeap)
		{
			// simply assin to Bitmap
			Bitmap = new tTVPBaseBitmap(*bmp);
		}
		else
		{
			// allocate heap and copy to it
			tjs_int h = Height;
			RawData = new tjs_uint8 [ Size ];
			tjs_uint8 *p = RawData;
			tjs_int rawpitch = Width * PixelSize;
			for(h--; h>=0; h--)
			{
				memcpy(p, bmp->GetScanLine(h), rawpitch);
				p += rawpitch;
			}
		}
	}

	void AssignToBitmap(tTVPBaseBitmap *bmp) const
	{
		if(!TVPAllocGraphicCacheOnHeap)
		{
			// simply assign to Bitmap
			if(Bitmap) bmp->AssignBitmap(*Bitmap);
		}
		else
		{
			// copy from the rawdata heap
			if(RawData)
			{
				bmp->Recreate(Width, Height, PixelSize==4?32:8);
				tjs_int h = Height;
				tjs_uint8 *p = RawData;
				tjs_int rawpitch = Width * PixelSize;
				for(h--; h>=0; h--)
				{
					memcpy(bmp->GetScanLineForWrite(h), p, rawpitch);
					p += rawpitch;
				}
			}
		}
	}

	tjs_uint GetSize() const { return Size; }

	void AddRef() { RefCount ++; }
	void Release()
	{
		if(RefCount == 1)
		{
			delete this;
		}
		else
		{
			RefCount--;
		}
	}
};
//---------------------------------------------------------------------------
typedef tTJSRefHolder<tTVPGraphicImageData> tTVPGraphicImageHolder;

typedef
tTJSHashTable<tTVPGraphicsSearchData, tTVPGraphicImageHolder, tTVPGraphicsSearchHashFunc>
	tTVPGraphicCache;
tTVPGraphicCache TVPGraphicCache;
static bool TVPGraphicCacheEnabled = false;
static tjs_uint TVPGraphicCacheLimit = 0;
static tjs_uint TVPGraphicCacheTotalBytes = 0;
tjs_uint TVPGraphicCacheSystemLimit = 0; // maximum possible value of  TVPGraphicCacheLimit
//---------------------------------------------------------------------------
static void TVPCheckGraphicCacheLimit()
{
	while(TVPGraphicCacheTotalBytes > TVPGraphicCacheLimit)
	{
		// chop last graphics
		tTVPGraphicCache::tIterator i;
		i = TVPGraphicCache.GetLast();
		if(!i.IsNull())
		{
			tjs_uint size = i.GetValue().GetObjectNoAddRef()->GetSize();
			TVPGraphicCacheTotalBytes -= size;
			TVPGraphicCache.ChopLast(1);
		}
		else
		{
			break;
		}
	}
}
//---------------------------------------------------------------------------
void TVPClearGraphicCache()
{
	TVPGraphicCache.Clear();
	TVPGraphicCacheTotalBytes = 0;
}
//---------------------------------------------------------------------------
struct tTVPClearGraphicCacheCallback : public tTVPCompactEventCallbackIntf
{
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level)
	{
		if(level >= TVP_COMPACT_LEVEL_MINIMIZE)
		{
			// clear the font cache on application minimize
			TVPClearGraphicCache();
		}
	}
} static TVPClearGraphicCacheCallback;
static bool TVPClearGraphicCacheCallbackInit = false;
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
static bool TVPInternalLoadGraphic(tTVPBaseBitmap *dest, const ttstr &_name,
	tjs_uint32 keyidx, tjs_uint desw, tjs_int desh, std::vector<tTVPGraphicMetaInfoPair> * * MetaInfo,
		tTVPGraphicLoadMode mode, ttstr *provincename)
{
	// name must be normalized.
	// if "provincename" is non-null, this function set it to province storage
	// name ( with _p suffix ) for convinience.
	// desw and desh are desired size. if the actual picture is smaller than
	// the given size, the graphic is to be tiled. give 0,0 to obtain default
	// size graphic.


	// graphic compact initialization
	if(!TVPClearGraphicCacheCallbackInit)
	{
		TVPAddCompactEventHook(&TVPClearGraphicCacheCallback);
		TVPClearGraphicCacheCallbackInit = true;
	}


	// search according with its extension
	tjs_int namelen = _name.GetLen();
	ttstr name(_name);

	ttstr ext = TVPExtractStorageExt(name);
	int extlen = ext.GetLen();
	tTVPGraphicHandlerType * handler;

	if(ext == TJS_W(""))
	{
		// missing extension
		// suggest registered extensions
		tTJSHashTable<ttstr, tTVPGraphicHandlerType>::tIterator i;
		for(i = TVPGraphicType.Hash.GetFirst(); !i.IsNull(); /*i++*/)
		{
			ttstr newname = name + i.GetKey();
			if(TVPIsExistentStorage(newname))
			{
				// file found
				name = newname;
				break;
			}
			i++;
		}
		if(i.IsNull())
		{
			// not found
			TVPThrowExceptionMessage(TVPCannotSuggestGraphicExtension, name);
		}

		handler = & i.GetValue();
	}
	else
	{
		handler = TVPGraphicType.Hash.Find(ext);
	}

	if(!handler) TVPThrowExceptionMessage(TVPUnknownGraphicFormat, name);


	tTVPStreamHolder holder(name); // open a storage named "name"

	// load the image
	tTVPLoadGraphicData data;
	data.Dest = dest;
	data.ColorKey = keyidx;
	data.Type = mode == glmNormal? lgtFullColor : lgtPalGray;
	data.Name = name;
	data.DesW = desw;
	data.DesH = desh;
	data.NeedMetaInfo = true;
	data.MetaInfo = NULL;

	bool keyadapt = (keyidx == TVP_clAdapt);
	bool doalphacolormat = TVP_Is_clAlphaMat(keyidx);
	tjs_uint32 alphamatcolor = TVP_get_clAlphaMat(keyidx);

	if(TVP_Is_clPalIdx(keyidx))
	{
		// pass the palette index number to the handler.
		// ( since only Graphic Loading Handler can process the palette information )
		keyidx = TVP_get_clPalIdx(keyidx);
	}
	else
	{
		keyidx = -1;
	}

	(handler->Handler)(handler->FormatData, (void*)&data, TVPLoadGraphic_SizeCallback,
		TVPLoadGraphic_ScanLineCallback, TVPLoadGraphic_MetaInfoPushCallback,
		holder.Get(), keyidx, mode);

	*MetaInfo = data.MetaInfo;

	if(keyadapt && mode == glmNormal)
	{
		// adaptive color key
		TVPMakeAlphaFromAdaptiveColor(dest);
	}


	if(mode != glmNormal) return true;

	if(provincename)
	{
		// set province name
		*provincename = ttstr(_name, namelen-extlen) + TJS_W("_p");

		// search extensions
		tTJSHashTable<ttstr, tTVPGraphicHandlerType>::tIterator i;
		for(i = TVPGraphicType.Hash.GetFirst(); !i.IsNull(); /*i++*/)
		{
			ttstr newname = *provincename + i.GetKey();
			if(TVPIsExistentStorage(newname))
			{
				// file found
				*provincename = newname;
				break;
			}
			i++;
		}
		if(i.IsNull())
		{
			// not found
			provincename->Clear();
		}
	}

	// mask image handling ( addding _m suffix with the filename )
	while(true)
	{
		name = ttstr(_name, namelen-extlen) + TJS_W("_m") + ext;
		if(ext.IsEmpty())
		{
			// missing extension
			// suggest registered extensions
			tTJSHashTable<ttstr, tTVPGraphicHandlerType>::tIterator i;
			for(i = TVPGraphicType.Hash.GetFirst(); !i.IsNull(); /*i++*/)
			{
				ttstr newname = name;
				newname += i.GetKey();
				if(TVPIsExistentStorage(newname))
				{
					// file found
					name = newname;
					break;
				}
				i++;
			}
			if(i.IsNull())
			{
				// not found
				handler = NULL;
				break;
			}

			handler = & i.GetValue();
			break;
		}
		else
		{
			if(!TVPIsExistentStorage(name))
			{
				// not found
				ext.Clear();
				continue; // retry searching
			}
			handler = TVPGraphicType.Hash.Find(ext);
			break;
		}
	}

	if(handler)
	{
		// open the mask file
		holder.Open(name);

		// fill "data"'s member
	    data.Type = lgtMask;
	    data.Name = name;
		data.Buffer = NULL;
		data.DesW = desw;
		data.DesH = desh;
		data.NeedMetaInfo = false;

	    try
	    {
			// load image via handler
			(handler->Handler)(handler->FormatData, (void*)&data,
				TVPLoadGraphic_SizeCallback, TVPLoadGraphic_ScanLineCallback,
				NULL,
				holder.Get(), -1, glmGrayscale);
	    }
		catch(...)
	    {
			if(data.Buffer) delete [] data.Buffer;
			throw;
		}

	    if(data.Buffer) delete [] data.Buffer;
	}

	// do color matting
	if(doalphacolormat)
	{
		// alpha color mat
		TVPDoAlphaColorMat(dest, alphamatcolor);
	}

	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPLoadGraphic
//---------------------------------------------------------------------------
void TVPLoadGraphic(tTVPBaseBitmap *dest, const ttstr &name, tjs_int32 keyidx,
	tjs_uint desw, tjs_uint desh,
	tTVPGraphicLoadMode mode, ttstr *provincename, iTJSDispatch2 ** metainfo)
{
	// loading with cache management
	ttstr nname = TVPNormalizeStorageName(name);
	tjs_uint32 hash;
	tTVPGraphicsSearchData searchdata;

	if(TVPGraphicCacheEnabled)
	{
		searchdata.Name = nname;
		searchdata.KeyIdx = keyidx;
		searchdata.Mode = mode;
		searchdata.DesW = desw;
		searchdata.DesH = desh;

		hash = tTVPGraphicCache::MakeHash(searchdata);

		tTVPGraphicImageHolder * ptr =
			TVPGraphicCache.FindAndTouchWithHash(searchdata, hash);
		if(ptr)
		{
			// found in cache
			ptr->GetObjectNoAddRef()->AssignToBitmap(dest);
			if(provincename) *provincename = ptr->GetObjectNoAddRef()->ProvinceName;
			if(metainfo)
				*metainfo = TVPMetaInfoPairsToDictionary(ptr->GetObjectNoAddRef()->MetaInfo);
			return;
		}
	}

	// not found

	// load into dest
	tTVPGraphicImageData * data = NULL;

	ttstr pn;
	std::vector<tTVPGraphicMetaInfoPair> * mi = NULL;
	try
	{
		TVPInternalLoadGraphic(dest, nname, keyidx, desw, desh, &mi, mode, &pn);

		if(provincename) *provincename = pn;
		if(metainfo)
			*metainfo = TVPMetaInfoPairsToDictionary(mi);

		if(TVPGraphicCacheEnabled)
		{
			data = new tTVPGraphicImageData();
			data->AssignBitmap(dest);
			data->ProvinceName = pn;
			data->MetaInfo = mi; // now mi is managed under tTVPGraphicImageData
			mi = NULL;

			// check size limit
			TVPCheckGraphicCacheLimit();

			// push into hash table
			tjs_uint datasize = data->GetSize();
//			if(datasize < TVPGraphicCacheLimit)
//			{
				TVPGraphicCacheTotalBytes += datasize;
				tTVPGraphicImageHolder holder(data);
				TVPGraphicCache.AddWithHash(searchdata, hash, holder);
//			}
		}
	}
	catch(...)
	{
		if(mi) delete mi;
		if(data) data->Release();
		throw;
	}

	if(mi) delete mi;
	if(data) data->Release();
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// TVPTouchImages
//---------------------------------------------------------------------------
void TVPTouchImages(const std::vector<ttstr> & storages, tjs_int limit,
	tjs_uint64 timeout)
{
	// preload graphic files into the cache.
	// "limit" is a limit memory for preload, in bytes.
	// this function gives up when "timeout" (in ms) expired.
	// currently this function only loads normal graphics.
	// (univ.trans rule graphics nor province image may not work properly)

	if(!TVPGraphicCacheLimit) return;

	tjs_uint limitbytes;
	if(limit >= 0)
	{
		if((tjs_uint)limit > TVPGraphicCacheLimit || limit == 0)
			limitbytes = TVPGraphicCacheLimit;
		else
			limitbytes = limit;
	}
	else
	{
		// negative value of limit indicates remaining bytes after loading
		if((tjs_uint)-limit >= TVPGraphicCacheLimit) return;
		limitbytes = TVPGraphicCacheLimit + limit;
	}

	tjs_int count = 0;
	tjs_uint bytes = 0;
	tjs_uint64 starttime = TVPGetTickCount();
	tjs_uint64 limittime = starttime + timeout;
	tTVPBaseBitmap tmp(32, 32, 32);
	ttstr statusstr(TJS_W("(info) Touching "));
	bool first = true;
	while((tjs_uint)count < storages.size())
	{
		if(timeout && TVPGetTickCount() >= limittime)
		{
			statusstr += TJS_W(" ... aborted [timed out]");
			break;
		}
		if(bytes >= limitbytes)
		{
			statusstr += TJS_W(" ... aborted [limit bytes exceeded]");
			break;
		}

		try
		{
			if(!first) statusstr += TJS_W(", ");
			first = false;
			statusstr += storages[count];

			TVPLoadGraphic(&tmp, storages[count++], TVP_clNone,
				0, 0, glmNormal, NULL); // load image

			// get image size
			tTVPGraphicImageData * data = new tTVPGraphicImageData();
			try
			{
				data->AssignBitmap(&tmp);
				bytes += data->GetSize();
			}
			catch(...)
			{
				data->Release();
				throw;
			}
			data->Release();
		}
		catch(eTJS &e)
		{
			statusstr += TJS_W("(error!:");
			statusstr += e.GetMessage();
			statusstr += TJS_W(")");
		}
		catch(...)
		{
			// ignore all errors
		}
	}

	// re-touch graphic cache to ensure that more earlier graphics in storages
	// array can get more priority in cache order.
	count--;
	for(;count >= 0; count--)
	{
		tTVPGraphicsSearchData searchdata;
		searchdata.Name = TVPNormalizeStorageName(storages[count]);
		searchdata.KeyIdx = TVP_clNone;
		searchdata.Mode = glmNormal;
		searchdata.DesW = 0;
		searchdata.DesH = 0;

		tjs_uint32 hash = tTVPGraphicCache::MakeHash(searchdata);

		TVPGraphicCache.FindAndTouchWithHash(searchdata, hash);
	}

	statusstr += TJS_W(" (elapsed "); // Fixed spelling by W.Dee, 20031105
	statusstr += ttstr((tjs_int)(TVPGetTickCount() - starttime));
	statusstr += TJS_W("ms)");

	TVPAddLog(statusstr);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPSetGraphicCacheLimit
//---------------------------------------------------------------------------
void TVPSetGraphicCacheLimit(tjs_uint limit)
{
	// set limit of graphic cache by total bytes.
	if(limit == 0 )
	{
		TVPGraphicCacheLimit = limit;
		TVPGraphicCacheEnabled = false;
	}
	else if(limit == -1)
	{
		TVPGraphicCacheLimit = TVPGraphicCacheSystemLimit;
		TVPGraphicCacheEnabled = true;
	}
	else
	{
		if(limit > TVPGraphicCacheSystemLimit)
			limit = TVPGraphicCacheSystemLimit;
		TVPGraphicCacheLimit = limit;
		TVPGraphicCacheEnabled = true;
	}

	TVPCheckGraphicCacheLimit();
}
//---------------------------------------------------------------------------
tjs_uint TVPGetGraphicCacheLimit()
{
	return TVPGraphicCacheLimit;
}
//---------------------------------------------------------------------------




