/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2003             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: CPU ID Check
 last mod: $Id: cpu.c,v 1.1 2006-06-09 00:00:00+09 blacksword Exp $

 ********************************************************************/

#if defined(__INTEL_COMPILER)&&defined(_WIN32)&&defined(_USRDLL)
extern int __intel_cpu_indicator;

void __intel_cpu_indicator_init(void)
{
	unsigned int t, u;
	_asm { 
		mov	eax,1 
		cpuid 
		mov	t, edx
		mov	u, ecx
	}
	/* SSE3 Check */
	if(u&0x0000001)
	{
		__intel_cpu_indicator = 0x800;
		return;
	}
	/* SSE2 Check */
	if(t&0x4000000)
	{
		__intel_cpu_indicator = 0x200;
		return;
	}
	/* SSE Check */
	if(t&0x2000000)
	{
		__intel_cpu_indicator = 0x100;
		return;
	}
	__intel_cpu_indicator = 1;
}
#endif
