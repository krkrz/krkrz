//===========================================================================
//Copyright (C) 2003, 2004, 2005 Zentaro Kavanagh
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//- Neither the name of Zentaro Kavanagh nor the names of contributors 
//  may be used to endorse or promote products derived from this software 
//  without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================
#pragma once

#ifndef INC_OGGDLLSTUFF
#define INC_OGGDLLSTUFF



////Ignore the wanrings in the directshow base classses
//#pragma warning( push )
//#pragma warning( disable : 4312 )


#include <streams.h>
#include <pullpin.h>

//#pragma warning( pop )

#ifndef OGGDEMUX_DLL
    #define LIBOOOGG_API
#else
    #ifdef LIBOOOGG_EXPORTS
        #define LIBOOOGG_API __declspec(dllexport)
    #else
        #define LIBOOOGG_API __declspec(dllimport)
    #endif
#endif

#ifndef OGGDEMUX_DLL
    #define OGG_DEMUX2_API
#else
    #ifdef DSFOGGDEMUX2_EXPORTS
        #pragma message("----> Exporting from Ogg Demux...")
        #define OGG_DEMUX2_API __declspec(dllexport)
    #else
        #pragma message("<---- Importing from Ogg Demux...")
        #define OGG_DEMUX2_API __declspec(dllimport)
    #endif
#endif

#endif