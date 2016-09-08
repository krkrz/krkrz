//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Universal Storage System
//---------------------------------------------------------------------------
#ifndef StorageImplH
#define StorageImplH
//---------------------------------------------------------------------------
#include "StorageIntf.h"
#include "UtilStreams.h"

//---------------------------------------------------------------------------
// tTVPLocalFileStream
//---------------------------------------------------------------------------
class tTVPLocalFileStream : public tTJSBinaryStream
{
private:
	HANDLE Handle;

public:
	tTVPLocalFileStream(const ttstr &origname, const ttstr & localname,
		tjs_uint32 flag);
	~tTVPLocalFileStream();

	tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence);

	tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size);
	tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint write_size);

	void TJS_INTF_METHOD SetEndOfStorage();

	tjs_uint64 TJS_INTF_METHOD GetSize();

	HANDLE GetHandle() const { return Handle; }
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
TJS_EXP_FUNC_DEF(bool, TVPCheckExistentLocalFolder, (const ttstr &name));
	/* name must be an OS's NATIVE folder name */

TJS_EXP_FUNC_DEF(bool, TVPCheckExistentLocalFile, (const ttstr &name));
	/* name must be an OS's NATIVE file name */

TJS_EXP_FUNC_DEF(bool, TVPCreateFolders, (const ttstr &folder));
	/* make folders recursively, like mkdir -p. folder must be OS NATIVE folder name */
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTVPPluginHolder
//---------------------------------------------------------------------------
/*
	tTVPPluginHolder holds plug-in. if the plug-in is not a local storage,
	plug-in is to be extracted to temporary directory and be held until
	the program done using it.
*/
class tTVPPluginHolder
{
private:
	ttstr LocalName;
	tTVPLocalTempStorageHolder * LocalTempStorageHolder;

public:
	tTVPPluginHolder(const ttstr &aname);
	~tTVPPluginHolder();

	const ttstr & GetLocalName() const;
};
//---------------------------------------------------------------------------

#endif
