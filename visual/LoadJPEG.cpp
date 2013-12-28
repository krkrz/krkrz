
#include "tjsCommHead.h"

#include "GraphicsLoaderIntf.h"
#include "LayerBitmapIntf.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "tvpgl.h"

extern "C"
{
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
		ttstr(TVPErrorCode) + ttstr(cinfo->err->msg_code));
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

//---------------------------------------------------------------------------
void TVPLoadJPEG(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	// JPEG loading handler

	// JPEG does not support palettized image
	if(mode == glmPalettized)
		TVPThrowExceptionMessage(TVPJPEGLoadError,
			ttstr(TVPUnsupportedJpegPalette));

	// prepare variables
	jpeg_decompress_struct cinfo;
	my_error_mgr jerr;
	JSAMPARRAY buffer;
	//tjs_int row_stride;

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
		//cinfo.dct_method = JDCT_IFAST;
		cinfo.dct_method = JDCT_ISLOW;
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
		sizecallback(callbackdata, cinfo.output_width, cinfo.output_height);
#if 1
		if( mode == glmNormal && cinfo.out_color_space == JCS_RGB ) {
			buffer = new JSAMPROW[cinfo.output_height];
			for( unsigned int i = 0; i < cinfo.output_height; i++ ) {
				buffer[i] = (JSAMPLE*)scanlinecallback(callbackdata, i);
			}
			while( cinfo.output_scanline < cinfo.output_height ) {
				jpeg_read_scanlines( &cinfo, buffer + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline );
			}
			delete[] buffer;
			for( unsigned int i = 0; i < cinfo.output_height; i++ ) {
				scanlinecallback(callbackdata, i);
				scanlinecallback(callbackdata, -1);
			}
		} else
#endif
		{
			buffer = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE,
					cinfo.output_width * cinfo.output_components + 3,
					cinfo.rec_outbuf_height);

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
#if 1
							memcpy(scanline,
								buffer[bufline], cinfo.output_width*sizeof(tjs_uint32));
#else
							// expand 24bits to 32bits
							TVPConvert24BitTo32Bit(
								(tjs_uint32*)scanline,
								(tjs_uint8*)buffer[bufline], cinfo.output_width);
#endif
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
struct stream_destination_mgr {
	struct jpeg_destination_mgr	pub;		/* public fields */
	tTJSBinaryStream*			stream;
	JOCTET*						buffer;		/* buffer start address */
	int							bufsize;	/* size of buffer */
	int							bufsizeinit;/* size of buffer */
	size_t						datasize;	/* final size of compressed data */
	int*						outsize;	/* user pointer to datasize */
};
typedef stream_destination_mgr* stream_dest_ptr;

METHODDEF(void) JPEG_write_init_destination( j_compress_ptr cinfo ) {
	stream_dest_ptr dest = (stream_dest_ptr)cinfo->dest;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->bufsize;
	dest->datasize = 0;	 /* reset output size */
}

METHODDEF(boolean) JPEG_write_empty_output_buffer( j_compress_ptr cinfo ) {
	stream_dest_ptr dest = (stream_dest_ptr)cinfo->dest;
	
	// 足りなくなったら途中書き込み
	size_t	wrotelen = dest->bufsizeinit - dest->pub.free_in_buffer;
	dest->stream->WriteBuffer( dest->buffer, wrotelen );

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->bufsize;
	return TRUE;
}

METHODDEF(void) JPEG_write_term_destination( j_compress_ptr cinfo ) {
	stream_dest_ptr dest = (stream_dest_ptr)cinfo->dest;
	dest->datasize = dest->bufsize - dest->pub.free_in_buffer;
	if( dest->outsize ) *dest->outsize += (int)dest->datasize;
}
METHODDEF(void) JPEG_write_stream( j_compress_ptr cinfo, JOCTET* buffer, int bufsize, int* outsize, tTJSBinaryStream* stream ) {
	stream_dest_ptr dest;

	/* first call for this instance - need to setup */
	if (cinfo->dest == 0) {
		cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(stream_destination_mgr) );
	}

	dest = (stream_dest_ptr) cinfo->dest;
	dest->stream = stream;
	dest->bufsize = bufsize;
	dest->bufsizeinit = bufsize;
	dest->buffer = buffer;
	dest->outsize = outsize;
	/* set method callbacks */
	dest->pub.init_destination = JPEG_write_init_destination;
	dest->pub.empty_output_buffer = JPEG_write_empty_output_buffer;
	dest->pub.term_destination = JPEG_write_term_destination;
}
//---------------------------------------------------------------------------
/**
 * JPG書き込み
 * フルカラーでの書き込みのみ対応
 * @param storagename : 出力ファイル名
 * @param mode : モード "jpg" と必要なら圧縮率を数値で後に付け足す
 * @param image : 書き出しイメージデータ
 */
void TVPSaveAsJPG( const ttstr & storagename, const ttstr & mode, const tTVPBaseBitmap* image )
{
	if(!image->Is32BPP())
		TVPThrowInternalError;

	if( !mode.StartsWith(TJS_W("jpg")) ) TVPThrowExceptionMessage(TVPInvalidImageSaveType, mode);
	tjs_uint quality = 90;
	if( mode.length() > 3 ) {
		quality = 0;
		for( tjs_int len = 3; len < mode.length(); len++ ) {
			tjs_char c = mode[len];
			if( c >= TJS_W('0') && c <= TJS_W('9') ) {
				quality *= 10;
				quality += c-TJS_W('0');
			}
		}
		if( quality <= 0 ) quality = 10;
		if( quality > 100 ) quality = 100;
	}

	tjs_uint height = image->GetHeight();
	tjs_uint width = image->GetWidth();
	if( height == 0 || width == 0 ) TVPThrowInternalError;

	// open stream
	tTJSBinaryStream *stream = TVPCreateStream(TVPNormalizeStorageName(storagename), TJS_BS_WRITE);
	struct jpeg_compress_struct cinfo;
	
	size_t buff_size = width*height*sizeof(tjs_uint32);
	tjs_uint8* dest_buf = new tjs_uint8[buff_size];
	try {
		TVPClearGraphicCache();

		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error( &jerr );
		jerr.error_exit = my_error_exit;
		jerr.output_message = my_output_message;
		jerr.reset_error_mgr = my_reset_error_mgr;
		// emit_message, format_message はデフォルトのを使う

		jpeg_create_compress( &cinfo );
		
		int num_write_bytes = 0; //size of jpeg after compression
		JOCTET *jpgbuff = (JOCTET*)dest_buf; //JOCTET pointer to buffer
		JPEG_write_stream( &cinfo, jpgbuff , buff_size, &num_write_bytes, stream );

		cinfo.image_width = width;
		cinfo.image_height = height;
		cinfo.input_components = 4;
		cinfo.in_color_space = JCS_RGB;
		cinfo.dct_method = JDCT_ISLOW;

		jpeg_set_defaults( &cinfo );
		jpeg_set_quality( &cinfo, quality, TRUE );
		jpeg_start_compress( &cinfo, TRUE );
		
		while( cinfo.next_scanline < cinfo.image_height ) {
			JSAMPROW row_pointer[1];
			int y = cinfo.next_scanline;
			row_pointer[0] = reinterpret_cast<JSAMPROW>(const_cast<void*>(image->GetScanLine(y)));
			jpeg_write_scanlines( &cinfo, row_pointer, 1 );
		}
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		stream->WriteBuffer( dest_buf, num_write_bytes );
	} catch(...) {
		jpeg_destroy_compress(&cinfo);
		delete[] dest_buf;
		delete stream;
		throw;
	}
	delete[] dest_buf;
	delete stream;
}
