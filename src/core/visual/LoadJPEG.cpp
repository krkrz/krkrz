
#include "tjsCommHead.h"

#include "GraphicsLoaderIntf.h"
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

