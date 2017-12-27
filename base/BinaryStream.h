//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Text read/write stream
//---------------------------------------------------------------------------
#ifndef BinaryStreamH
#define BinaryStreamH


#include "StorageIntf.h"

//---------------------------------------------------------------------------
// BinaryStream Functions
//---------------------------------------------------------------------------
extern tTJSBinaryStream *TVPCreateBinaryStreamForRead(const ttstr &name, const ttstr &modestr);
extern tTJSBinaryStream *TVPCreateBinaryStreamForWrite(const ttstr &name, const ttstr &modestr);
//---------------------------------------------------------------------------
TJS_EXP_FUNC_DEF(iTJSBinaryStream *, TVPCreateBinaryStreamInterfaceForWrite, (const ttstr &name, const ttstr &modestr) );
TJS_EXP_FUNC_DEF(iTJSBinaryStream *, TVPCreateBinaryStreamInterfaceForRead, (const ttstr &name, const ttstr &modestr) );
//---------------------------------------------------------------------------

#endif
