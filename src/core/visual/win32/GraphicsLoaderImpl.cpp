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

#include "GraphicsLoaderImpl.h"
#include "GraphicsLoaderIntf.h"
#include "tjsHashSearch.h"
#include "StorageImpl.h"
#include "MsgIntf.h"
#include "tjsUtils.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"

/*
	support of SPI for archive files is in StorageImpl.cpp
*/

//---------------------------------------------------------------------------
// tTVPSusiePlugin
//---------------------------------------------------------------------------
tTVPSusiePlugin::tTVPSusiePlugin(HINSTANCE inst, const char *api)
{
	ModuleInstance = inst;

	// get functions
	*(FARPROC*)&GetPluginInfo = GetProcAddress(inst, "GetPluginInfo");
	*(FARPROC*)&IsSupported = GetProcAddress(inst, "IsSupported");

	*(FARPROC*)&GetPicture = GetProcAddress(inst, "GetPicture");


	*(FARPROC*)&GetArchiveInfo = GetProcAddress(inst, "GetArchiveInfo");
	*(FARPROC*)&GetFile = GetProcAddress(inst, "GetFile");

	if(!memcmp(api, "00IN", 4))
	{
		if(!GetPluginInfo || !IsSupported || !GetPicture)
			TVPThrowExceptionMessage(TVPNotSusiePlugin);
	}
	else if(!memcmp(api, "00AM", 4))
	{
		if(!GetPluginInfo || !IsSupported || !GetArchiveInfo || !GetFile)
			TVPThrowExceptionMessage(TVPNotSusiePlugin);
	}
	else
	{
		TVPThrowInternalError;
	}

	// check API version and dump copyright information
	char buffer[256];
	char buffer2[256];

	if(GetPluginInfo(0, buffer, 255) >= 4)
	{
		if(memcmp(buffer, api, 4))
			TVPThrowExceptionMessage(TVPNotSusiePlugin);
	}
	else
	{
		TVPThrowExceptionMessage(TVPNotSusiePlugin);
	}

	memset(buffer, 0, 256);
	GetPluginInfo(1, buffer, 255);
	if(buffer[0]) TVPAddImportantLog(TJS_W("(info) Susie plugin info : ") + ttstr(buffer));

	// retrieve format information
	tjs_int i;
	for(i = 0; ; i++)
	{
		int r = GetPluginInfo(2*i + 2, buffer, 255);
		if(r == 0) break;
		buffer[255] = 0;

		// here buffer contains exetension information such as "*.JPG" "*.RGB;*.Q0"
		strlwr(buffer);

		// split buffer to each extensions
		char *p = buffer;
		while((p = strstr(p, "*.")) != NULL)
		{
			p++;
			char *b2 = buffer2;
			while(*p && *p != ' ' && *p != ';')
			{
				*b2 = *p;
				b2++;
				p++;
			}
			*b2 = 0;
			Extensions.push_back(ttstr(buffer2));
		}
	}
};
//---------------------------------------------------------------------------
tTVPSusiePlugin::~tTVPSusiePlugin()
{
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTVPSusiePicturePlugin
//---------------------------------------------------------------------------
class tTVPSusiePicturePlugin : public tTVPSusiePlugin
{
	tTVPBMPAlphaType AlphaType;
public:
	tTVPSusiePicturePlugin(HINSTANCE inst, tTVPBMPAlphaType alphatype);
	~tTVPSusiePicturePlugin();

	tTVPBMPAlphaType GetAlphaType() const { return AlphaType; }

	void Load(void *callbackdata,
		tTVPGraphicSizeCallback sizecallback,
		tTVPGraphicScanLineCallback scanlinecallback,
		tTJSBinaryStream *src,
		tjs_int keyidx,
		tTVPGraphicLoadMode mode);

};
//---------------------------------------------------------------------------
tTVPSusiePicturePlugin::tTVPSusiePicturePlugin(HINSTANCE inst,
	tTVPBMPAlphaType alphatype) : tTVPSusiePlugin(inst, "00IN")
{
	// member setup
	AlphaType = alphatype;
}
//---------------------------------------------------------------------------
tTVPSusiePicturePlugin::~tTVPSusiePicturePlugin()
{
}
//---------------------------------------------------------------------------
void tTVPSusiePicturePlugin::Load(void *callbackdata,
		tTVPGraphicSizeCallback sizecallback,
		tTVPGraphicScanLineCallback scanlinecallback,
		tTJSBinaryStream *src,
		tjs_int keyidx,
		tTVPGraphicLoadMode mode)
{
	bool bitmaplocked = false;
	HLOCAL bitmap = NULL;
	bool infolocked = false;
	HLOCAL info = NULL;

	// load source to memory
	tjs_uint64 size = src->GetSize();
	tjs_uint8 * source = new tjs_uint8[(tjs_int)size];

	try
	{
		src->ReadBuffer(source, size);

		// call GetPicture
		int r = GetPicture((LPSTR)source, (long)size, 0x01, &info, &bitmap,
			(FARPROC)ProgressCallback, 0);
		if((r&0xff) != 0) TVPThrowExceptionMessage(TVPSusiePluginError, ttstr(r));

		// setup bitmapinfoheader
		TVP_WIN_BITMAPINFOHEADER bi;
		memset(&bi, 0, sizeof(bi));
		BITMAPINFOHEADER *srcbi = (BITMAPINFOHEADER *)LocalLock(info);
		infolocked = true;
		tjs_int datasize = LocalSize(bitmap);
		void * data = (void*) LocalLock(bitmap);
		bitmaplocked = true;

		if(srcbi->biSize == 12)
		{
			// OS/2 bitmap header
			bi.biSize = srcbi->biSize;
			bi.biWidth = srcbi->biWidth;
			bi.biHeight = srcbi->biHeight;
			bi.biPlanes = srcbi->biPlanes;
			bi.biBitCount = srcbi->biBitCount;
			bi.biClrUsed = 0;
			bi.biCompression = BI_RGB;
		}
		else if(srcbi->biSize == 40)
		{
			// Windows bitmap header
			bi.biSize = srcbi->biSize;
			bi.biWidth = srcbi->biWidth;
			bi.biHeight = srcbi->biHeight;
			bi.biPlanes = srcbi->biPlanes;
			bi.biBitCount = srcbi->biBitCount;
			bi.biCompression = srcbi->biCompression;
			bi.biSizeImage = srcbi->biSizeImage;
			bi.biXPelsPerMeter = srcbi->biXPelsPerMeter;
			bi.biYPelsPerMeter = srcbi->biYPelsPerMeter;
			bi.biClrUsed = srcbi->biClrUsed;
			bi.biClrImportant = srcbi->biClrImportant;
		}
		else
		{
			// not supported bitmap format
			TVPThrowExceptionMessage(TVPImageLoadError,
				TJS_W("Non-supported bitmap header was given from susie plug-in."));

		}

		// create reference memory stream for bitmap pixel data
		tTVPMemoryStream memstream(data, datasize);

		if(bi.biClrUsed == 0 && bi.biBitCount <= 8)
			bi.biClrUsed = 1 << bi.biBitCount;

		// pass information to TVPInternalLoadBMP
		TVPInternalLoadBMP(callbackdata, sizecallback, scanlinecallback,
			bi, ((tjs_uint8*)srcbi) + bi.biSize, &memstream, keyidx, AlphaType,
				mode);

	}
	catch(...)
	{
		delete [] source;
		if(bitmaplocked) LocalUnlock(bitmap);
		if(bitmap) LocalFree(bitmap);
		if(infolocked) LocalUnlock(info);
		if(info) LocalFree(info);
		throw;
	}

	delete [] source;
	if(bitmaplocked) LocalUnlock(bitmap);
	if(bitmap) LocalFree(bitmap);
	if(infolocked) LocalUnlock(info);
	if(info) LocalFree(info);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Global/static data
//---------------------------------------------------------------------------
typedef tTJSHashTable<HINSTANCE, tTVPSusiePicturePlugin*> tTVPSusiePluginList;
static tTVPSusiePluginList TVPSusiePluginList;

static void TVPDestroySusiePluginList()
{
	tTVPSusiePluginList::tIterator i;
	for(i = TVPSusiePluginList.GetFirst(); !i.IsNull(); i++)
	{
		delete i.GetValue();
	}
}
static tTVPAtExit TVPDestroySusiePluginListAtExit
	(TVP_ATEXIT_PRI_CLEANUP, TVPDestroySusiePluginList);

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPLoadViaSusiePlugin
//---------------------------------------------------------------------------
static void TVPLoadViaSusiePlugin(void* formatdata, void *callbackdata,
	tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback,
	tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src,
	tjs_int keyidx,
	tTVPGraphicLoadMode mode)
{
	tTVPSusiePicturePlugin * plugin = (tTVPSusiePicturePlugin*)formatdata;
	plugin->Load(callbackdata, sizecallback, scanlinecallback, src, keyidx,
		mode);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPLoadPictureSPI/TVPUnloadPictureSPI : load/unload spi
//---------------------------------------------------------------------------
void TVPLoadPictureSPI(HINSTANCE inst, tTVPBMPAlphaType alphatype)
{
	// load specified Picture Susie plug-in.
	tTVPSusiePicturePlugin *spi = new tTVPSusiePicturePlugin(inst, alphatype);

	TVPSusiePluginList.Add(inst, spi);

	const std::vector<ttstr> & exts = spi->GetExtensions();
	std::vector<ttstr>::const_iterator i;
	for(i = exts.begin(); i != exts.end(); i++)
	{
		TVPRegisterGraphicLoadingHandler(*i, TVPLoadViaSusiePlugin, (void*)spi);
	}
}
//---------------------------------------------------------------------------
void TVPUnloadPictureSPI(HINSTANCE inst)
{
	// unload specified Picture Susie plug-in from System.
	tTVPSusiePicturePlugin** p = TVPSusiePluginList.Find(inst);

	if(!p)
		TVPThrowExceptionMessage(TVPNotLoadedPlugin);

	const std::vector<ttstr> & exts = (*p)->GetExtensions();
	std::vector<ttstr>::const_iterator i;
	for(i = exts.begin(); i != exts.end(); i++)
	{
		TVPUnregisterGraphicLoadingHandler(*i, TVPLoadViaSusiePlugin, (void*)*p);
	}

	TVPSusiePluginList.Delete(inst);

	delete *p;
}
//---------------------------------------------------------------------------

