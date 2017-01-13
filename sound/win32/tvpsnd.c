/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Oct 03 00:37:04 2000
 */
/* Compiler settings for tvpsnd.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID LIBID_TVPSndSysLib = {0xF09B2E87,0x740A,0x4AEA,{0x90,0xCB,0xAE,0x1C,0x4E,0xFD,0x35,0xF1}};


const IID IID_ITSSMediaBaseInfo = {0xB4C4239B,0x7D27,0x43CC,{0x85,0x23,0x66,0x95,0x5B,0xDF,0x59,0xDF}};


const IID IID_ITSSStorageProvider = {0x7DD61993,0x615D,0x481D,{0xB0,0x60,0xA9,0xFD,0x48,0x39,0x44,0x30}};


const IID IID_ITSSModule = {0xA938D1A5,0x2980,0x498B,{0xB0,0x51,0x99,0x93,0x1D,0x41,0x89,0x5D}};


const IID IID_ITSSWaveDecoder = {0x313864E2,0x910E,0x496F,{0x8A,0x6D,0x43,0x46,0x5C,0x10,0x5B,0x58}};


#ifdef __cplusplus
}
#endif

