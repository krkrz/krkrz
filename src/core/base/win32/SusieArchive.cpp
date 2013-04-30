//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Archive eXtractor Susie plug-in support
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <time.h>
#include "StorageImpl.h"
#include "SusieArchive.h"
#include "WideNativeFuncs.h"
#include "GraphicsLoaderImpl.h"
#include "DebugIntf.h"
#include "XP3Archive.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"
#include <algorithm>

//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPSusieArchivePlugin
//---------------------------------------------------------------------------
#pragma pack(push, 1)
struct tTVPSusieFileInfo
{
	unsigned char method[8];	// compression method
	unsigned long position;		// position in archive file
	unsigned long compsize;		// compressed size
	unsigned long filesize;		// original size
	time_t timestamp;			// update timestamp
	char path[200];				// relative path
	char filename[200];			// filename
	unsigned long crc;			// CRC
};
#pragma pack(pop)
struct tTVPSusieFileRecord
{
	ttstr Name;
	unsigned long Position;
	unsigned long Size;
	bool operator < (const tTVPSusieFileRecord & rhs) const
	{
		return this->Name < rhs.Name;
	}
};
//---------------------------------------------------------------------------
// tTVPSusiePlugin is defined in GraphicLoaderImpl.h
class tTVPSusieArchivePlugin : public tTVPSusiePlugin
{
	tTJSCriticalSection CS;
	tjs_int LockCount;

public:
	tTVPSusieArchivePlugin(HINSTANCE inst);
	~tTVPSusieArchivePlugin();

	void Lock();
	void Unlock();

	bool CanRelease() { return LockCount <= 0; }

	bool CheckSupported(tTVPLocalFileStream * stream, AnsiString localname);

	void GetFileList(AnsiString localname, std::vector<tTVPSusieFileRecord> &dest);

	tTJSBinaryStream * CreateStream(AnsiString localname,
		unsigned long pos, unsigned long sise);

	tTJSCriticalSection & GetCS() { return CS; }
};
//---------------------------------------------------------------------------
tTVPSusieArchivePlugin::tTVPSusieArchivePlugin(HINSTANCE inst) :
	tTVPSusiePlugin(inst, "00AM")
{
	LockCount = 0;
}
//---------------------------------------------------------------------------
tTVPSusieArchivePlugin::~tTVPSusieArchivePlugin()
{
}
//---------------------------------------------------------------------------
void tTVPSusieArchivePlugin::Lock()
{
	tTJSCriticalSectionHolder holder(CS);

	LockCount++;
}
//---------------------------------------------------------------------------
void tTVPSusieArchivePlugin::Unlock()
{
	tTJSCriticalSectionHolder holder(CS);

	LockCount--;
}
//---------------------------------------------------------------------------
bool tTVPSusieArchivePlugin::CheckSupported(tTVPLocalFileStream * stream,
	AnsiString localname)
{
	int res = IsSupported(localname.c_str(), (DWORD)stream->GetHandle());

	return (bool)res;
}
//---------------------------------------------------------------------------
void tTVPSusieArchivePlugin::GetFileList(AnsiString localname,
	std::vector<tTVPSusieFileRecord> &dest)
{
	// retrieve file list
	TVPAddLog(TJS_W("(info) Listing files in ") + ttstr(localname.c_str()) + TJS_W(" ..."));

	HLOCAL infohandle = NULL;
	int errorcode = 0xff&GetArchiveInfo(localname.c_str(), 0, 0x00, &infohandle);
	if(infohandle == NULL)
	{
		TVPThrowExceptionMessage(TVPSusiePluginError,
			ttstr(TJS_W("tTVPSusieArchivePlugin::GetArchiveInfo failed, errorcode = ")) +
			ttstr((tjs_int)errorcode));
	}
	else
	{
		if(errorcode != 0)
		{
			TVPAddLog(TJS_W("Warning : invalid errorcode ") + ttstr((tjs_int)errorcode) +
				TJS_W(" was returned from tTVPSusieArchivePlugin::GetArchiveInfo, "
					"continuing anyway."));
		}
	}


	const tTVPSusieFileInfo *info = (const tTVPSusieFileInfo*)LocalLock(infohandle);
	if(info == NULL)
	{
		TVPThrowExceptionMessage(TVPSusiePluginError,
			ttstr(TJS_W("tTVPSusieArchivePlugin::GetArchiveInfo failed : invalid memory block.")));
	}

	try
	{
		while(info->method[0])
		{
			if(info->filename[0])
			{
				char buf[401];
				strcpy(buf, info->path);
				strcat(buf, "/");
				strcat(buf, info->filename);

				tTVPSusieFileRecord record;
				record.Name = buf;
				tTVPArchive::NormalizeInArchiveStorageName(record.Name);
				record.Position = info->position;
				record.Size = info->filesize;

				dest.push_back(record);
			}

			info++;
		}

		// sort item vector by its name (required for tTVPArchive specification)
		std::stable_sort(dest.begin(), dest.end());
	}
	catch(...)
	{
		LocalUnlock(infohandle);
		LocalFree(infohandle);
		throw;
	}

	LocalUnlock(infohandle);
	LocalFree(infohandle);

	TVPAddLog(TJS_W("(info) ") + ttstr((tjs_int)dest.size()) + TJS_W(" files found."));
}
//---------------------------------------------------------------------------
tTJSBinaryStream * tTVPSusieArchivePlugin::CreateStream(AnsiString localname,
	unsigned long pos, unsigned long size)
{
	HLOCAL memhandle = NULL;
	int errorcode = 0xff & GetFile(localname.c_str(), pos, (LPSTR)(void*)&memhandle,
		0x0100, (FARPROC)ProgressCallback, 0);
	if(errorcode || memhandle == NULL)
	{
		TVPThrowExceptionMessage(TVPSusiePluginError,
			ttstr(TJS_W("tTVPSusieArchivePlugin::GetFile failed, errorcode = ")) +
			ttstr((tjs_int)errorcode));
	}

	tTVPMemoryStream *memstream = new tTVPMemoryStream;
	void *memblock = NULL;

	try
	{
		memblock = LocalLock(memhandle);
		if(memblock == NULL)
		{
			TVPThrowExceptionMessage(TVPSusiePluginError,
				ttstr(TJS_W("tTVPSusieArchivePlugin::GetFile failed : invalid memory block.")));
		}

		// write to on-memory stream
		memstream->WriteBuffer(memblock, size);
		memstream->SetPosition(0);
	}
	catch(...)
	{
		if(memblock) LocalUnlock(memhandle);
		LocalFree(memhandle);
		delete memstream;
		throw;
	}

	if(memblock) LocalUnlock(memhandle);
	LocalFree(memhandle);
	return memstream;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static std::vector<tTVPSusieArchivePlugin*> TVPSusiePluginVector;

static void TVPDestroySusiePluginList()
{
	std::vector<tTVPSusieArchivePlugin*>::iterator i;
	for(i = TVPSusiePluginVector.begin(); i != TVPSusiePluginVector.end(); i++)
	{
		delete *i;
	}
}
static tTVPAtExit TVPDestroySusiePluginListAtExit
	(TVP_ATEXIT_PRI_CLEANUP, TVPDestroySusiePluginList);
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPLoadArchiveSPI/TVPUnloadArchiveSPI : load/unload archive spi
//---------------------------------------------------------------------------
void TVPLoadArchiveSPI(HINSTANCE inst)
{
	// load specified Susie plug-in.
	std::vector<tTVPSusieArchivePlugin*>::iterator i;
	for(i = TVPSusiePluginVector.begin(); i != TVPSusiePluginVector.end(); i++)
	{
		if((*i)->GetModuleInstance() == inst)
			TVPThrowInternalError;
	}

	tTVPSusieArchivePlugin *spi = new tTVPSusieArchivePlugin(inst);

	TVPSusiePluginVector.push_back(spi);
}
//---------------------------------------------------------------------------
void TVPUnloadArchiveSPI(HINSTANCE inst)
{
	// unload specified Susie plug-in from system.
	std::vector<tTVPSusieArchivePlugin*>::iterator i;
	for(i = TVPSusiePluginVector.begin(); i != TVPSusiePluginVector.end(); i++)
	{
		if((*i)->GetModuleInstance() == inst) break;
	}

	if(i == TVPSusiePluginVector.end())
		TVPThrowExceptionMessage(TVPNotLoadedPlugin);

	if(!(*i)->CanRelease())
		TVPThrowExceptionMessage(TVPCannotReleasePlugin);

	for(i = TVPSusiePluginVector.begin(); i != TVPSusiePluginVector.end();)
	{
		if((*i)->GetModuleInstance() == inst)
			i = TVPSusiePluginVector.erase(i);
		else
			i++;
	}

	delete *i;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCheckSusieSupport : checks which plugin supports specified archive
//---------------------------------------------------------------------------
tTVPSusieArchivePlugin * TVPCheckSusieSupport(const ttstr &name, AnsiString &a_localname)
{
	if(TVPSusiePluginVector.size() == 0) return NULL;

	ttstr localname = TVPGetLocallyAccessibleName(name);
	tTVPLocalFileStream stream(name, localname, TJS_BS_READ);

	if(localname.GetLen() == 0) return NULL; // only local filesystem stream is supported

	a_localname = localname.AsAnsiString();

	std::vector<tTVPSusieArchivePlugin*>::iterator i;
	for(i = TVPSusiePluginVector.end() - 1; i >= TVPSusiePluginVector.begin(); i--)
	{
		stream.SetPosition(0);
		if((*i)->CheckSupported(&stream, a_localname))
			return *i;
	}

	return NULL;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTVPSusieArchive
//---------------------------------------------------------------------------
class tTVPSusieArchive : public tTVPArchive
{
	tTVPSusieArchivePlugin *Plugin;
	AnsiString LocalName;
	std::vector<tTVPSusieFileRecord> FileRecords;

public:
	tTVPSusieArchive(tTVPSusieArchivePlugin *plugin, const ttstr & name,
		AnsiString localname);
	~tTVPSusieArchive();

	tjs_uint GetCount();
	ttstr GetName(tjs_uint idx);

	tTJSBinaryStream * CreateStreamByIndex(tjs_uint idx);
};
//---------------------------------------------------------------------------
tTVPSusieArchive::tTVPSusieArchive(tTVPSusieArchivePlugin *plugin,
	const ttstr &name, AnsiString localname) : tTVPArchive(name)
{
	LocalName = localname;
	Plugin = plugin;

	tTJSCriticalSectionHolder(Plugin->GetCS());

	Plugin->Lock();

	Plugin->GetFileList(LocalName, FileRecords);

}
//---------------------------------------------------------------------------
tTVPSusieArchive::~tTVPSusieArchive()
{
	Plugin->Unlock();
}
//---------------------------------------------------------------------------
tjs_uint tTVPSusieArchive::GetCount()
{
	return (tjs_uint)FileRecords.size();
}
//---------------------------------------------------------------------------
ttstr tTVPSusieArchive::GetName(tjs_uint idx)
{
	return FileRecords[idx].Name;
}
//---------------------------------------------------------------------------
tTJSBinaryStream * tTVPSusieArchive::CreateStreamByIndex(tjs_uint idx)
{
	tTJSCriticalSectionHolder(Plugin->GetCS());

	return Plugin->CreateStream(LocalName, FileRecords[idx].Position,
		FileRecords[idx].Size);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPOpenSusieArchive
//---------------------------------------------------------------------------
tTVPArchive * TVPOpenSusieArchive(const ttstr & name)
{
	AnsiString localname;
	tTVPSusieArchivePlugin *plugin = TVPCheckSusieSupport(name, localname);
	if(plugin)
	{
		return new tTVPSusieArchive(plugin, name, localname);
	}
	else
	{
		return NULL;
	}
}
//---------------------------------------------------------------------------


