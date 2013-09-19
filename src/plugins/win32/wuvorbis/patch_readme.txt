Vorbis decoder SSE/3DNow! optimization patch for VC++ 6.0
(needs VC++ 6.0 processor pack which is available at Microsoft site)

This pactch makes the decoder only runs under i486 or later.
(instruction "bswap", which is available on i486 or later, is used)

ogg.patch     - some trivial optimization patch
vorbis.patch  - SSE/3DNow! optimization patch


These can be applied on libogg 1.1.2 and libvorbis 1.1.0 by
patch (available from cygwin distribution):

> cd ogg-source-directory-which-includes-COPYING-file]
> patch -p1 < path-to-ogg.patch
> cd vorbis-source-directory-which-includes-COPYING-file]
> patch -p1 < path-to-vorbis.patch

All files may be rejected because the $Id$ cannot be replaced, but
you can ignore them.


You must declare following variables which indicate whether SSE/3DNow!/MMX
CPU features are to be used or not:

	int CPU_SSE = 0;  /* 0:not use   1:use */
	int CPU_3DN = 0;  /* 0:not use   1:use */
	int CPU_MMX = 0;  /* 0:not use   1:use */

These variables must be initialized before using the library.
(SSE/3DNow!/MMX detection code is not included in this patch nor
WuVorbis.dll's source)



And following memory allocation functions:

void * dee_ogg_malloc(size_t bytes)
void * dee_ogg_calloc(size_t num, size_t size)
void dee_ogg_free(void *ptr)
void * dee_ogg_realloc(void *block, size_t bytes)

These are implemented in WuVorbisUnit.cpp (you may refer or copy them),
to allocate paragraph-aligned memory blocks and margin to prevent
buffer-overrun in optimized code for faster SSE operation.


"fastcall" calling convension must be used, or else the program will
crash. (register save/restore related problems? not yet solved)



Note that applying this patch makes the library only able to output
16bit little-endian linear PCM or 32bit float(size=4) in ov_read.
Other PCM format abilities will be lost.


License: BSD-like license, the same as described in libvorbis license.



W.Dee <dee@kikyou.info>
