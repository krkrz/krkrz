//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <memory>
#include <stdlib.h>
#include <math.h>

#include "LayerBitmapIntf.h"
#include "LayerBitmapImpl.h"
#include "MsgIntf.h"
#include "ComplexRect.h"
#include "tvpgl.h"
#include "tjsHashSearch.h"
#include "EventIntf.h"
#include "SysInitImpl.h"
#include "WideNativeFuncs.h"
#include "StorageIntf.h"
#include "DebugIntf.h"
#include "WindowFormUnit.h"
#include "UtilStreams.h"

#include "FontSelectFormUnit.h"



//---------------------------------------------------------------------------
// prototypes
//---------------------------------------------------------------------------
void TVPClearFontCache();
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// #define TVP_ALLOC_GBUF_MALLOC
	// for debugging
//---------------------------------------------------------------------------
#define TVP_CH_MAX_CACHE_COUNT 1300
#define TVP_CH_MAX_CACHE_COUNT_LOW 100
#define TVP_CH_MAX_CACHE_HASH_SIZE 512
//---------------------------------------------------------------------------

static bool TVPChUseResampling = false; // whether to use resampling anti-aliasing
enum tTVPChAntialiasMethod
{	camAPI,	camResample4, camResample8, camSubpixelRGB, camSubpixelBGR };
static tTVPChAntialiasMethod TVPChAntialiasMethod = camResample8;
static bool TVPChAntialiasMethodInit = false;
//---------------------------------------------------------------------------
static void TVPInitChAntialiasMethod()
{
	if(TVPChAntialiasMethodInit) return;

	TVPChAntialiasMethod = TVPSystemIsBasedOnNT ? camAPI : camResample8; // default

	tTJSVariant val;
	if(TVPGetCommandLine(TJS_W("-aamethod"), &val))
	{
		ttstr str(val);
		if(str == TJS_W("auto"))
			; // nothing to do
		if(str == TJS_W("res8"))
			TVPChAntialiasMethod = camResample8;
		else if(str == TJS_W("res4"))
			TVPChAntialiasMethod = camResample4;
		else if(str == TJS_W("api"))
			TVPChAntialiasMethod = camAPI;
		else if(str == TJS_W("rgb"))
			TVPChAntialiasMethod = camSubpixelRGB;
		else if(str == TJS_W("bgr"))
			TVPChAntialiasMethod = camSubpixelBGR;
	}

	TVPChAntialiasMethodInit = true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Pre-rendered font management
//---------------------------------------------------------------------------
static tjs_int TVPGlobalFontStateMagic = 0;
	// this is for checking global font status' change

class tTVPPrerenderedFont;
static tTJSHashTable<ttstr, tTVPPrerenderedFont *> TVPPrerenderedFonts;

#pragma pack(push, 1)
struct tTVPPrerenderedCharacterItem
{
	tjs_uint32 Offset;
	tjs_uint16 Width;
	tjs_uint16 Height;
	tjs_int16 OriginX;
	tjs_int16 OriginY;
	tjs_int16 IncX;
	tjs_int16 IncY;
	tjs_int16 Inc;
	tjs_uint16 Reserved;
};
#pragma pack(pop)

//---------------------------------------------------------------------------
// tTVPPrerenderedFont
//---------------------------------------------------------------------------
class tTVPPrerenderedFont
{
private:
	ttstr Storage;
	HANDLE FileHandle; // tft file handle
	HANDLE MappingHandle; // file mapping handle
	const tjs_uint8 * Image; // tft mapped memory
	tjs_uint RefCount;
	tTVPLocalTempStorageHolder LocalStorage;

	tjs_int Version; // data version
	const tjs_char * ChIndex;
	const tTVPPrerenderedCharacterItem * Index;
	tjs_uint IndexCount;

public:
	tTVPPrerenderedFont(const ttstr &storage);
	~tTVPPrerenderedFont();
	void AddRef();
	void Release();

	const tTVPPrerenderedCharacterItem *
		Find(tjs_char ch); // serch character
	void Retrieve(const tTVPPrerenderedCharacterItem * item, tjs_uint8 *buffer,
		tjs_int bufferpitch);

};
//---------------------------------------------------------------------------
tTVPPrerenderedFont::tTVPPrerenderedFont(const ttstr &storage) :
	LocalStorage(storage)
{
	RefCount = 1;

	Storage = storage;

	// open local storage via CreateFile
	if(procCreateFileW)
	{
		FileHandle = procCreateFileW(
			LocalStorage.GetLocalName().c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	else
	{
		tTJSNarrowStringHolder holder(LocalStorage.GetLocalName().c_str());
		FileHandle = CreateFile(
			holder,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}

	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		TVPThrowExceptionMessage(TVPCannotOpenStorage, storage);
	}

	MappingHandle = NULL;
	Image = NULL;
	try
	{
		// map the file into memory
		MappingHandle = CreateFileMapping(FileHandle, NULL, PAGE_READONLY,
			0, 0, NULL);
		if(MappingHandle == NULL)
		{
			CloseHandle(FileHandle);
			TVPThrowExceptionMessage(TVPPrerenderedFontMappingFailed,
				TJS_W("CreateFileMapping failed."));
		}

		Image = (const tjs_uint8*)MapViewOfFile(MappingHandle,
			FILE_MAP_READ, 0, 0, 0);
		if(Image == NULL)
		{
			CloseHandle(MappingHandle);
			CloseHandle(FileHandle);
			TVPThrowExceptionMessage(TVPPrerenderedFontMappingFailed,
				TJS_W("MapViewOfFile failed."));
		}

		// check header
		if(memcmp("TVP pre-rendered font\x1a", Image, 22))
		{
			TVPThrowExceptionMessage(TVPPrerenderedFontMappingFailed,
				TJS_W("Signature not found or invalid pre-rendered font file."));
		}

		if(Image[23] != 2)
		{
			TVPThrowExceptionMessage(TVPPrerenderedFontMappingFailed,
				TJS_W("Not a 16-bit UNICODE font file."));
		}

		Version = Image[22];
		if(Version != 0 && Version != 1)
		{
			TVPThrowExceptionMessage(TVPPrerenderedFontMappingFailed,
				TJS_W("Invalid header version."));
		}

		// read index offset
		IndexCount = *(const tjs_uint32*)(Image + 24);
		ChIndex = (const tjs_char*)(Image + *(const tjs_uint32*)(Image + 28));
		Index = (const tTVPPrerenderedCharacterItem*)
			(Image + *(const tjs_uint32*)(Image + 32));
	}
	catch(...)
	{
		if(Image) UnmapViewOfFile(Image);
		if(MappingHandle) CloseHandle(MappingHandle);
		CloseHandle(FileHandle);
		throw;
	}

	TVPPrerenderedFonts.Add(storage, this);
}
//---------------------------------------------------------------------------
tTVPPrerenderedFont::~tTVPPrerenderedFont()
{
	UnmapViewOfFile(Image);
	CloseHandle(MappingHandle);
	CloseHandle(FileHandle);

	TVPPrerenderedFonts.Delete(Storage);
}
//---------------------------------------------------------------------------
void tTVPPrerenderedFont::AddRef()
{
	RefCount ++;
}
//---------------------------------------------------------------------------
void tTVPPrerenderedFont::Release()
{
	if(RefCount == 1)
		delete this;
	else
		RefCount --;
}
//---------------------------------------------------------------------------
const tTVPPrerenderedCharacterItem *
		tTVPPrerenderedFont::Find(tjs_char ch)
{
	// search through ChIndex
	tjs_uint s = 0;
	tjs_uint e = IndexCount;
	const tjs_char *chindex = ChIndex;
	while(true)
	{
		tjs_int d = e-s;
		if(d <= 1)
		{
			if(chindex[s] == ch)
				return Index + s;
			else
				return NULL;
		}
		tjs_uint m = s + (d>>1);
		if(chindex[m] > ch) e = m; else s = m;
	}
}
//---------------------------------------------------------------------------
void tTVPPrerenderedFont::Retrieve(const tTVPPrerenderedCharacterItem * item,
	tjs_uint8 *buffer, tjs_int bufferpitch)
{
	// retrieve font data and store to buffer
	// bufferpitch must be larger then or equal to item->Width
	if(item->Width == 0 || item->Height == 0) return;

	const tjs_uint8 *ptr = item->Offset + Image;
	tjs_uint8 *dest = buffer;
	tjs_uint8 *destlim = dest + item->Width * item->Height;

	// expand compressed character bitmap data
	if(Version == 0)
	{
		// version 0 decompressor
		while(dest < destlim)
		{
			if(*ptr == 0x41) // running
			{
				ptr++;
				tjs_uint8 last = dest[-1];
				tjs_int len = *ptr;
				ptr++;
				while(len--) *(dest++) = (tjs_uint8)last;
			}
			else
			{
				*(dest++) = *(ptr++);
			}
		}
	}
	else if(Version >= 1)
	{
		// version 1+ decompressor
		while(dest < destlim)
		{
			if(*ptr >= 0x41) // running
			{
				tjs_int len = *ptr - 0x40;
				ptr++;
				tjs_uint8 last = dest[-1];
				while(len--) *(dest++) = (tjs_uint8)last;
			}
			else
			{
				*(dest++) = *(ptr++);
			}
		}
	}

	// expand to each pitch
	ptr = destlim - item->Width;
	dest = buffer + bufferpitch * item->Height - bufferpitch;
	while(buffer <= dest)
	{
		if(dest != ptr)
			memmove(dest, ptr, item->Width);
		dest -= bufferpitch;
		ptr -= item->Width;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPPrerenderedFontMap
//---------------------------------------------------------------------------
struct tTVPPrerenderedFontMap
{
	tTVPFont Font; // mapped font
	tTVPPrerenderedFont * Object; // prerendered font object
};
static std::vector<tTVPPrerenderedFontMap> TVPPrerenderedFontMapVector;
//---------------------------------------------------------------------------
void TVPMapPrerenderedFont(const tTVPFont & font, const ttstr & storage)
{
	// map specified font to specified prerendered font
	ttstr fn = TVPSearchPlacedPath(storage);

	// search or retrieve specified storage
	tTVPPrerenderedFont * object;

	tTVPPrerenderedFont ** found = TVPPrerenderedFonts.Find(fn);
	if(!found)
	{
		// not yet exist; create
		object = new tTVPPrerenderedFont(fn);
	}
	else
	{
		// already exist
		object = *found;
		object->AddRef();
	}

	// search existing mapped font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->Release();
			i->Object = object;
			break;
		}
	}
	if(i == TVPPrerenderedFontMapVector.end())
	{
		// not found
		tTVPPrerenderedFontMap map;
		map.Font = font;
		map.Object = object;
		TVPPrerenderedFontMapVector.push_back(map); // add
	}

	TVPGlobalFontStateMagic ++; // increase magic number

	TVPClearFontCache(); // clear font cache
}
//---------------------------------------------------------------------------
void TVPUnmapPrerenderedFont(const tTVPFont & font)
{
	// unmap specified font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->Release();
			TVPPrerenderedFontMapVector.erase(i);
			TVPGlobalFontStateMagic ++; // increase magic number
			TVPClearFontCache();
			return;
		}
	}
}
//---------------------------------------------------------------------------
static void TVPUnmapAllPrerenderedFonts()
{
	// unmap all prerendered fonts
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		i->Object->Release();
	}
	TVPPrerenderedFontMapVector.clear();
	TVPGlobalFontStateMagic ++; // increase magic number
}
//---------------------------------------------------------------------------
static tTVPAtExit TVPUnmapAllPrerenderedFontsAtExit
	(TVP_ATEXIT_PRI_PREPARE, TVPUnmapAllPrerenderedFonts);
//---------------------------------------------------------------------------
static tTVPPrerenderedFont * TVPGetPrerenderedMappedFont(const tTVPFont &font)
{
	// search mapped prerendered font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->AddRef();

			// note that the object is AddRefed
			return i->Object;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// Character Cache management
//---------------------------------------------------------------------------
struct tTVPFontAndCharacterData
{
	tTVPFont Font;
	tjs_uint32 FontHash;
	tjs_char Character;
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	bool Antialiased;
	bool Blured;
	bool operator == (const tTVPFontAndCharacterData &rhs) const
	{
		return Character == rhs.Character && Font == rhs.Font &&
			Antialiased == rhs.Antialiased && BlurLevel == rhs.BlurLevel &&
			BlurWidth == rhs.BlurWidth && Blured == rhs.Blured;
	}
};
//---------------------------------------------------------------------------
class tTVPFontHashFunc
{
public:
	static tjs_uint32 Make(const tTVPFontAndCharacterData &val)
	{
		tjs_uint32 v = val.FontHash;

		v ^= val.Antialiased;
		v ^= val.Character;
		v ^= val.Blured;
		v ^= val.BlurLevel ^ val.BlurWidth;
		return v;
	}
};
//---------------------------------------------------------------------------
class tTVPCharacterData
{
	// character data holder for caching
private:
	tjs_uint8 * Data;
	tjs_int RefCount;

public:
	tjs_int OriginX;
	tjs_int OriginY;
	tjs_int CellIncX;
	tjs_int CellIncY;
	tjs_int Pitch;
	tjs_uint BlackBoxX;
	tjs_uint BlackBoxY;
	tjs_int BlurLevel;
	tjs_int BlurWidth;

	bool Antialiased;
	bool Blured;
	bool FullColored;

public:
	tTVPCharacterData() { RefCount = 1; Data = NULL; }
	~tTVPCharacterData() { if(Data) delete [] Data; }

	void Alloc(tjs_int size)
	{
		if(Data) delete [] Data, Data = NULL;
		Data = new tjs_uint8[size];
	}

	tjs_uint8 * GetData() const { return Data; }

	void AddRef() { RefCount ++; }
	void Release()
	{
		if(RefCount == 1)
		{
			delete this;
		}
		else
		{
			RefCount--;
		}
	}

	void Expand();

	void Blur(tjs_int blurlevel, tjs_int blurwidth);
	void Blur();

	void Bold(tjs_int size);
	void Bold2(tjs_int size);

	void Resample4();
	void Resample8();
};
//---------------------------------------------------------------------------
void tTVPCharacterData::Expand()
{
	// expand the bitmap stored in 1bpp, to 8bpp
	tjs_int newpitch = (((BlackBoxX -1)>>2)+1)<<2;
	tjs_uint8 *nd;
	tjs_uint8 *newdata = nd = new tjs_uint8[newpitch * BlackBoxY];
	tjs_int h = BlackBoxY;
	tjs_uint8 *d = Data;

	tjs_int w = BlackBoxX;
	static tjs_uint32 pal[2] = {0, 64};
	while(h--)
	{
		TVPBLExpand1BitTo8BitPal(nd, d, w, pal);
		nd += newpitch, d += Pitch;
	}
	if(Data) delete [] Data;
	Data = newdata;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Blur(tjs_int blurlevel, tjs_int blurwidth)
{
	// blur the bitmap with given parameters
	// blur the bitmap
	if(!Data) return;
	if(blurlevel == 255 && blurwidth == 0) return; // no need to blur
	if(blurwidth == 0)
	{
		// no need to blur but must be transparent
		TVPChBlurMulCopy65(Data, Data, Pitch*BlackBoxY, BlurLevel<<10);
		return;
	}

	// simple blur ( need to optimize )
	tjs_int bw = std::abs(blurwidth);
	tjs_int newwidth = BlackBoxX + bw*2;
	tjs_int newheight = BlackBoxY + bw*2;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;

	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	TVPChBlurCopy65(newdata, newpitch, newwidth, newheight, Data, Pitch, BlackBoxX,
		BlackBoxY, bw, blurlevel);

	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	Pitch = newpitch;
	OriginX -= blurwidth;
	OriginY -= blurwidth;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Blur()
{
	// blur the bitmap
	Blur(BlurLevel, BlurWidth);
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Bold(tjs_int size)
{
	// enbold the bitmap for 65-level grayscale bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// apply bold
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		for(tjs_int i = 0; i<level; i++) destp[i] = srcp[i];
		destp[0] = srcp[0];
		for(tjs_int x = level; x<newwidth-level; x++)
		{
			tjs_uint largest = srcp[x];
			for(tjs_int xx = x-level; xx<x; xx++)
				if((tjs_uint)srcp[xx] > largest) largest = srcp[xx];
			destp[x] = largest;
		}
		for(tjs_int i = 0; i<level; i++) destp[newwidth-i-1] = srcp[BlackBoxX-1-i];

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Bold2(tjs_int size)
{
	// enbold the bitmap for black/white monochrome bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  (((newwidth -1)>>5)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// apply bold
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		memcpy(destp, srcp, Pitch);
		if(newpitch > Pitch) destp[Pitch] = 0;

		for(tjs_int i = 1; i<=level; i++)
		{
			tjs_uint8 bollow = 0;
			tjs_int bl = 8 - i;
			for(tjs_int x = 0; x < Pitch; x++)
			{
				destp[x] |= (srcp[x] >> i) + bollow;
				bollow = srcp[x] << bl;
			}
			if(newpitch > Pitch) destp[Pitch] |= bollow;
		}

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Resample4()
{
	// down-sampling 4x4

	static tjs_uint16 bitcounter[256] = {0xffff};
	if(bitcounter[0] == 0xffff)
	{
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++)
		{
			tjs_uint16 v;
			tjs_int n;
			n = i & 0x0f;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v = (n<<2);
			n = i >> 4;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v |= ((n<<2)) << 8;
			bitcounter[i] = v;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>2)+1;
	tjs_int newheight = ((BlackBoxY-1)>>2)+1;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// resampling
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0; y<newheight; y++)
	{
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &=
			((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*4;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 4 ? 4 : rem;

		tjs_uint8 *dp = destp;
		tjs_int lim = (newwidth+1) >> 1;
		for(tjs_int i = 0; i<lim; i++)
		{
			tjs_uint32 n = 0;
			tjs_uint8 *sp = srcp + i;
			switch(rem)
			{
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			dp[0] = n >> 8;
			dp[1] = n & 0xff;
			dp += 2;
		}

		srcp += Pitch * 4;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /4;
	OriginY = OriginY /4;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void tTVPCharacterData::Resample8()
{
	// down-sampling 8x8

	static tjs_uint8 bitcounter[256] = {0xff};
	if(bitcounter[0] == 0xff)
	{
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++)
		{
			tjs_int n;
			n = (i & 0x55) + ((i & 0xaa)>>1);
			n = (n & 0x33) + ((n & 0xcc)>>2);
			n = (n & 0x0f) + ((n & 0xf0)>>4);
			bitcounter[i] = (tjs_uint8)n;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>3)+1;
	tjs_int newheight = ((BlackBoxY-1)>>3)+1;
	tjs_int newpitch =  (((newwidth -1)>>2)+1)<<2;
	tjs_uint8 *newdata = new tjs_uint8[newpitch * newheight];

	// resampling
	tjs_uint8 * srcp = Data;
	tjs_uint8 * destp = newdata;
	for(tjs_int y = 0;;)
	{
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &=
			((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*8;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 8 ? 8 : rem;

		for(tjs_int x = 0; x<newwidth; x++)
		{
			tjs_uint n = 0;
			tjs_uint8 *sp = srcp + x;
			switch(rem)
			{
			case 8:	n += bitcounter[*sp]; sp += Pitch;
			case 7:	n += bitcounter[*sp]; sp += Pitch;
			case 6:	n += bitcounter[*sp]; sp += Pitch;
			case 5:	n += bitcounter[*sp]; sp += Pitch;
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			destp[x] = n;
		}

		y++;
		if(y >= newheight) break;
		srcp += Pitch * 8;
		destp += newpitch;
	}

	// replace old data
	delete [] Data;
	Data = newdata;
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /8;
	OriginY = OriginY /8;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
typedef tTJSRefHolder<tTVPCharacterData> tTVPCharacterDataHolder;

typedef
tTJSHashCache<tTVPFontAndCharacterData, tTVPCharacterDataHolder,
	tTVPFontHashFunc, TVP_CH_MAX_CACHE_HASH_SIZE> tTVPFontCache;
tTVPFontCache TVPFontCache(TVP_CH_MAX_CACHE_COUNT);
//---------------------------------------------------------------------------
void TVPSetFontCacheForLowMem()
{
	// set character cache limit
	TVPFontCache.SetMaxCount(TVP_CH_MAX_CACHE_COUNT_LOW);
}
//---------------------------------------------------------------------------
void TVPClearFontCache()
{
	TVPFontCache.Clear();
}
//---------------------------------------------------------------------------
struct tTVPClearFontCacheCallback : public tTVPCompactEventCallbackIntf
{
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level)
	{
		if(level >= TVP_COMPACT_LEVEL_MINIMIZE)
		{
			// clear the font cache on application minimize
			TVPClearFontCache();
		}
	}
} static TVPClearFontCacheCallback;
static bool TVPClearFontCacheCallbackInit = false;
//---------------------------------------------------------------------------
static tTVPCharacterData * TVPGetCharacter(const tTVPFontAndCharacterData & font,
	tTVPNativeBaseBitmap *bmp, tTVPPrerenderedFont *pfont, tjs_int aofsx, tjs_int aofsy)
{
	// returns specified character data.
	// draw a character if needed.

	TVPInitChAntialiasMethod();

	// compact interface initialization
	if(!TVPClearFontCacheCallbackInit)
	{
		TVPAddCompactEventHook(&TVPClearFontCacheCallback);
		TVPClearFontCacheCallbackInit = true;
	}

	// make hash and search over cache
	tjs_uint32 hash = tTVPFontCache::MakeHash(font);

	tTVPCharacterDataHolder * ptr = TVPFontCache.FindAndTouchWithHash(font, hash);
	if(ptr)
	{
		// found in the cache
		return ptr->GetObject();
	}

	// not found in the cache

	// look prerendered font
	const tTVPPrerenderedCharacterItem *pitem = NULL;
	if(pfont)
		pitem = pfont->Find(font.Character);

	if(pitem)
	{
		// prerendered font
		tTVPCharacterData *data = new tTVPCharacterData();
		data->BlackBoxX = pitem->Width;
		data->BlackBoxY = pitem->Height;
		data->CellIncX = pitem->IncX;
		data->CellIncY = pitem->IncY;
		data->OriginX = pitem->OriginX + aofsx;
		data->OriginY = -pitem->OriginY + aofsy;

		data->Antialiased = font.Antialiased;

		data->FullColored = false;

		data->Blured = font.Blured;
		data->BlurWidth = font.BlurWidth;
		data->BlurLevel = font.BlurLevel;

		try
		{
			if(data->BlackBoxX && data->BlackBoxY)
			{
				// render
				tjs_int newpitch =  (((pitem->Width -1)>>2)+1)<<2;
				data->Pitch = newpitch;

				data->Alloc(newpitch * data->BlackBoxY);

				pfont->Retrieve(pitem, data->GetData(), newpitch);

				// apply blur
				if(font.Blured) data->Blur(); // nasty ...

				// add to hash table
				tTVPCharacterDataHolder holder(data);
				TVPFontCache.AddWithHash(font, hash, holder);
			}
		}
		catch(...)
		{
			data->Release();
			throw;
		}

		return data;
	}
	else
	{
		// render font

		// setup GLYPHMETRICS and reveive buffer
		GLYPHMETRICS gm;
		ZeroMemory(&gm, sizeof(gm));
		static MAT2 no_transform_matrix = { {0,1}, {0,0}, {0,0}, {0,1} };
			// transformation matrix for intact conversion
			// | 1 0 |
			// | 0 1 |
		static MAT2 scale_4x_matrix = { {0,4}, {0,0}, {0,0}, {0,4} };
			// transformation matrix for 4x
			// | 4 0 |
			// | 0 4 |
		static MAT2 scale_8x_matrix = { {0,8}, {0,0}, {0,0}, {0,8} };
			// transformation matrix for 8x
			// | 8 0 |
			// | 0 8 |


		// determine format and transformation matrix
		tjs_int factor = 0;
		MAT2 *transmat;
		UINT format = font.Antialiased ? GGO_GRAY8_BITMAP : GGO_BITMAP;
		if(font.Antialiased)
		{
			switch(TVPChAntialiasMethod)
			{
			case camAPI:
				transmat = &no_transform_matrix;
				break;
			case camResample4:
				transmat = &scale_4x_matrix, factor = 2;
				format = GGO_BITMAP;
				break;
			case camSubpixelRGB:
			case camSubpixelBGR:
			case camResample8:
				transmat = &scale_8x_matrix, factor = 3;
				format = GGO_BITMAP;
				break;
			}
		}
		else
		{
			transmat = &no_transform_matrix;
		}


		// retrieve character code
		unsigned char pbuf[10+1];
		tjs_int pbuflen;
		WORD code;

		if(!procGetGlyphOutlineW)
		{
			// system supports ANSI
			pbuflen = TJS_wctomb((char*)pbuf, font.Character);
			if(pbuflen == -1) return NULL; // not drawable character

			pbuf[pbuflen] = 0;

			if(pbuf[1] == 0) // single byte
				code = pbuf[0];
			else // multi byte
				code = pbuf[1]+ (pbuf[0] << 8);
		}
		else
		{
			// system supports UNICODE
			code = font.Character;
		}


		// get buffer size and output dimensions
		int size;

		if(!procGetGlyphOutlineW)
			size = GetGlyphOutlineA(bmp->GetFontDC(), code, format, &gm, 0,
				NULL, transmat);
		else
			size = procGetGlyphOutlineW(bmp->GetFontDC(), code, format, &gm, 0,
				NULL, transmat);

		// set up structure's variables
		tTVPCharacterData * data = new tTVPCharacterData();

		{
			SIZE s;
			s.cx = 0;
			s.cy = 0;
			if(!procGetTextExtentPoint32W)
				GetTextExtentPoint32A(bmp->GetNonBoldFontDC(),
					(const char*)pbuf, pbuflen, &s);
			else
				procGetTextExtentPoint32W(bmp->GetNonBoldFontDC(),
					&font.Character, 1, &s);

			if(font.Font.Flags & TVP_TF_BOLD)
			{
				// calculate bold font width;
				// because sucking Win32 API returns different character size
				// between Win9x and WinNT, when we specified bold characters.
				// so we must alternatively calculate bold font size based on
				// non-bold font width.
				s.cx += (int)(s.cx / 50) + 1;
			}

			if(font.Font.Angle == 0)
			{
				data->CellIncX = s.cx;
				data->CellIncY = 0;
			}
			else if(font.Font.Angle == 2700)
			{
				data->CellIncX = 0;
				data->CellIncY = s.cx;
			}
			else
			{
				double angle = font.Font.Angle * (M_PI/1800);
				data->CellIncX =   cos(angle) * s.cx;
				data->CellIncY = - sin(angle) * s.cx;
			}
		}

		data->BlackBoxX = gm.gmBlackBoxX;
		data->BlackBoxY = gm.gmBlackBoxY;
		data->OriginX =
			(gm.gmptGlyphOrigin.x) +
			(aofsx<<factor);
		data->OriginY = -gm.gmptGlyphOrigin.y + (aofsy<<factor);

		data->Antialiased = font.Antialiased;

		data->FullColored = false;

		data->Blured = font.Blured;
		data->BlurWidth = font.BlurWidth;
		data->BlurLevel = font.BlurLevel;

		if(size == 0 || gm.gmBlackBoxY == 0)
		{
			data->BlackBoxX = 0;
			data->BlackBoxY = 0;

			// add to hash table
			tTVPCharacterDataHolder holder(data);
			TVPFontCache.AddWithHash(font, hash, holder);
		}
		else
		{

			if(format == GGO_GRAY8_BITMAP)
			{
				data->Pitch = (size / gm.gmBlackBoxY) & ~0x03;
					// data is aligned to DWORD
				/*
					data->Pitch = (((gm.gmBlackBoxX -1)>>2)+1)<<2 seems to be proper,
					but does not work with some characters.
				*/
			}
			else
			{
				data->Pitch = (((gm.gmBlackBoxX -1)>>5)+1)<<2;
					// data is aligned to DWORD
			}

			data->Alloc(size);

			try
			{
				// draw to buffer

				if(!procGetGlyphOutlineW)
					GetGlyphOutlineA(bmp->GetFontDC(), code, format, &gm, size,
						data->GetData(), transmat);
				else
					procGetGlyphOutlineW(bmp->GetFontDC(), code, format, &gm, size,
						data->GetData(), transmat);

				if((TVPChUseResampling || !TVPSystemIsBasedOnNT) &&
					(font.Font.Flags & TVP_TF_BOLD))
				{
					// our sucking win9x based OS cannot output bold characters
					// even if BOLD is specified.
					if(format == GGO_BITMAP)
						data->Bold2(font.Font.Height<<factor);
					else
						data->Bold(font.Font.Height<<factor);
						// so here we go a nasty way ...
				}

				if(!font.Antialiased)
				{
					// not antialiased
					data->Expand(); // nasty...
				}
				else
				{
					// antialiased
					switch(TVPChAntialiasMethod)
					{
					case camAPI:
						break;
					case camResample4:
						data->Resample4();
						break;
					case camResample8:
						data->Resample8();
						break;
					case camSubpixelRGB:
//						data->ResampleRGB();
						break;
					case camSubpixelBGR:
//						data->ResampleBGR();
						break;
					}
				}

				// apply blur
				if(font.Blured) data->Blur(); // nasty ...

				// add to hash table
				tTVPCharacterDataHolder holder(data);
				TVPFontCache.AddWithHash(font, hash, holder);

			}
			catch(...)
			{
				data->Release();
				throw;
			}
		}

		return data;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// heap allocation functions for bitmap bits
//---------------------------------------------------------------------------
typedef DWORD tTJSPointerSizedInteger;
	// this must be a integer type that has the same size with the normal
	// pointer ( void *)
//---------------------------------------------------------------------------
#pragma pack(push, 1)
struct tTVPLayerBitmapMemoryRecord
{
	void * alloc_ptr; // allocated pointer
	tjs_uint size; // original bmp bits size, in bytes
	tjs_uint32 sentinel_backup1; // sentinel value 1
	tjs_uint32 sentinel_backup2; // sentinel value 2
};
#pragma pack(pop)
//---------------------------------------------------------------------------
static void * TVPAllocBitmapBits(tjs_uint size, tjs_uint width, tjs_uint height)
{
	if(size == 0) return NULL;
	tjs_uint8 * ptrorg, * ptr;
	tjs_uint allocbytes = 16 + size + sizeof(tTVPLayerBitmapMemoryRecord) + sizeof(tjs_uint32)*2;
#ifdef TVP_ALLOC_GBUF_MALLOC
	ptr = ptrorg = (tjs_uint8*)malloc(allocbytes);
#else
	ptr = ptrorg = (tjs_uint8*)GlobalAlloc(GMEM_FIXED, allocbytes);
#endif
	if(!ptr) TVPThrowExceptionMessage(TVPCannotAllocateBitmapBits,
		TJS_W("at TVPAllocBitmapBits"), ttstr((tjs_int)allocbytes) + TJS_W("(") +
			ttstr((int)width) + TJS_W("x") + ttstr((int)height) + TJS_W(")"));
	// align to a paragraph ( 16-bytes )
	ptr += 16 + sizeof(tTVPLayerBitmapMemoryRecord);
	*reinterpret_cast<tTJSPointerSizedInteger*>(&ptr) >>= 4;
	*reinterpret_cast<tTJSPointerSizedInteger*>(&ptr) <<= 4;

	tTVPLayerBitmapMemoryRecord * record =
		(tTVPLayerBitmapMemoryRecord*)
		(ptr - sizeof(tTVPLayerBitmapMemoryRecord) - sizeof(tjs_uint32));

	// fill memory allocation record
	record->alloc_ptr = (void *)ptrorg;
	record->size = size;
	record->sentinel_backup1 = rand() + (rand() << 16);
	record->sentinel_backup2 = rand() + (rand() << 16);

	// set sentinel
	*(tjs_uint32*)(ptr - sizeof(tjs_uint32)) = ~record->sentinel_backup1;
	*(tjs_uint32*)(ptr + size              ) = ~record->sentinel_backup2;
		// Stored sentinels are nagated, to avoid that the sentinel backups in
		// tTVPLayerBitmapMemoryRecord becomes the same value as the sentinels.
		// This trick will make the detection of the memory corruption easier.
		// Because on some occasions, running memory writing will write the same
		// values at first sentinel and the tTVPLayerBitmapMemoryRecord.

	// return buffer pointer
	return ptr;
}
//---------------------------------------------------------------------------
static void TVPFreeBitmapBits(void *ptr)
{
	if(ptr)
	{
		// get memory allocation record pointer
		tjs_uint8 *bptr = (tjs_uint8*)ptr;
		tTVPLayerBitmapMemoryRecord * record =
			(tTVPLayerBitmapMemoryRecord*)
			(bptr - sizeof(tTVPLayerBitmapMemoryRecord) - sizeof(tjs_uint32));

		// check sentinel
		if(~(*(tjs_uint32*)(bptr - sizeof(tjs_uint32))) != record->sentinel_backup1)
			TVPThrowExceptionMessage(
				TJS_W("Layer bitmap: Buffer underrun detected. Check your drawing code!"));
		if(~(*(tjs_uint32*)(bptr + record->size      )) != record->sentinel_backup2)
			TVPThrowExceptionMessage(
				TJS_W("Layer bitmap: Buffer overrun detected. Check your drawing code!"));

#ifdef TVP_ALLOC_GBUF_MALLOC
		free((HGLOBAL)record->alloc_ptr);
#else
		GlobalFree((HGLOBAL)record->alloc_ptr);
#endif
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTVPBitmap : internal bitmap object
//---------------------------------------------------------------------------
bool TVPUseDIBSection = false; // always false since 2.29 2007.03.28
/*
	important:
	Note that each lines must be started at tjs_uint32 ( 4bytes ) aligned address.
	This is the default Windows bitmap allocate behavior.
*/
tTVPBitmap::tTVPBitmap(tjs_uint width, tjs_uint height, tjs_uint bpp)
{
	// tTVPBitmap constructor

	TVPInitWindowOptions(); // ensure window/bitmap usage options are initialized

	RefCount = 1;
	BitmapDC = BitmapHandle = OldBitmapHandle = NULL; // for DIBSection

	Allocate(width, height, bpp); // allocate initial bitmap
}
//---------------------------------------------------------------------------
tTVPBitmap::~tTVPBitmap()
{
	if(TVPUseDIBSection)
	{
		SelectObject(BitmapDC, OldBitmapHandle);
		DeleteObject(BitmapHandle);
		DeleteDC(BitmapDC);
	}
	else
	{
		TVPFreeBitmapBits(Bits);
	}
	GlobalFree((HGLOBAL)BitmapInfo);
}
//---------------------------------------------------------------------------
tTVPBitmap::tTVPBitmap(const tTVPBitmap & r)
{
	// constructor for cloning bitmap
	TVPInitWindowOptions(); // ensure window/bitmap usage options are initialized

	RefCount = 1;
	BitmapDC = BitmapHandle = OldBitmapHandle = NULL; // for DIBSection

	// allocate bitmap which has the same metrics to r
	Allocate(r.GetWidth(), r.GetHeight(), r.GetBPP());

	// copy BitmapInfo
	memcpy(BitmapInfo, r.BitmapInfo, BitmapInfoSize);

	// copy Bits
	if(r.Bits) memcpy(Bits, r.Bits, r.BitmapInfo->bmiHeader.biSizeImage);

	// copy pitch
	PitchBytes = r.PitchBytes;
	PitchStep = r.PitchStep;
}
//---------------------------------------------------------------------------
void tTVPBitmap::Allocate(tjs_uint width, tjs_uint height, tjs_uint bpp)
{
	// allocate bitmap bits
	// bpp must be 8 or 32

	// create BITMAPINFO
	BitmapInfoSize = sizeof(BITMAPINFOHEADER) +
			((bpp==8)?sizeof(RGBQUAD)*256 : 0);
	BitmapInfo = (BITMAPINFO*)
		GlobalAlloc(GPTR, BitmapInfoSize);
	if(!BitmapInfo) TVPThrowExceptionMessage(TVPCannotAllocateBitmapBits,
		TJS_W("allocating BITMAPINFOHEADER"), ttstr((tjs_int)BitmapInfoSize));

	Width = width;
	Height = height;

	tjs_uint bitmap_width = width;
		// note that the allocated bitmap size can be bigger than the
		// original size because the horizontal pitch of the bitmap
		// is aligned to a paragraph (16bytes)

	if(bpp == 8)
	{
		bitmap_width = (((bitmap_width-1) / 16)+1) *16; // align to a paragraph
		PitchBytes = (((bitmap_width-1) >> 2)+1) <<2;
	}
	else
	{
		bitmap_width = (((bitmap_width-1) / 4)+1) *4; // align to a paragraph
		PitchBytes = bitmap_width * 4;
	}

	PitchStep = -PitchBytes;


	BitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo->bmiHeader.biWidth = bitmap_width;
	BitmapInfo->bmiHeader.biHeight = height;
	BitmapInfo->bmiHeader.biPlanes = 1;
	BitmapInfo->bmiHeader.biBitCount = bpp;
	BitmapInfo->bmiHeader.biCompression = BI_RGB;
	BitmapInfo->bmiHeader.biSizeImage = PitchBytes * height;
	BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biClrUsed = 0;
	BitmapInfo->bmiHeader.biClrImportant = 0;

	// create grayscale palette
	if(bpp == 8)
	{
		RGBQUAD *pal = (RGBQUAD*)((tjs_uint8*)BitmapInfo + sizeof(BITMAPINFOHEADER));

		for(tjs_int i=0; i<256; i++)
		{
			pal[i].rgbBlue = pal[i].rgbGreen = pal[i].rgbRed = (BYTE)i;
			pal[i].rgbReserved = 0;
		}
	}

	// allocate bitmap bits
	try
	{
		if(TVPUseDIBSection)
		{
			HDC ref = GetDC(0);
			BitmapHandle = CreateDIBSection(ref, BitmapInfo, DIB_RGB_COLORS,
				&Bits, NULL, 0);
			if(!BitmapHandle)
			{
				ReleaseDC(0, ref);
				TVPThrowExceptionMessage(TVPCannotAllocateBitmapBits,
					TJS_W("CreateDIBSection failed"), TJS_W("(") +
				ttstr((int)width) + TJS_W("x") + ttstr((int)height) + TJS_W(")"));
			}
			BitmapDC = CreateCompatibleDC(ref);
			ReleaseDC(0, ref);
			OldBitmapHandle = SelectObject(BitmapDC, BitmapHandle);
		}
		else
		{
			Bits = TVPAllocBitmapBits(BitmapInfo->bmiHeader.biSizeImage,
				width, height);
		}
	}
	catch(...)
	{
		GlobalFree((HGLOBAL)BitmapInfo), BitmapInfo = NULL;
		throw;
	}
}
//---------------------------------------------------------------------------
void * tTVPBitmap::GetScanLine(tjs_uint l) const
{
	if((tjs_int)l>=BitmapInfo->bmiHeader.biHeight)
	{
		TVPThrowExceptionMessage(TVPScanLineRangeOver, ttstr((tjs_int)l),
			ttstr((tjs_int)BitmapInfo->bmiHeader.biHeight-1));
	}

	return (BitmapInfo->bmiHeader.biHeight - l -1 ) * PitchBytes + (tjs_uint8*)Bits;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// default LOGFONT retrieve function
//---------------------------------------------------------------------------
static const char * const TVPDefaultFontName = TJS_N("‚l‚r ‚oƒSƒVƒbƒN"); // TODO: i18n
static LOGFONT TVPDefaultLOGFONT;
static tTVPFont TVPDefaultFont;
static bool TVPDefaultLOGFONTCreated = false;
static void TVPConstructDefaultFont()
{
	if(!TVPDefaultLOGFONTCreated)
	{
		TVPDefaultLOGFONTCreated = true;
		LOGFONT &l = TVPDefaultLOGFONT;
		l.lfHeight = -12;
		l.lfWidth = 0;
		l.lfEscapement = 0;
		l.lfOrientation = 0;
		l.lfWeight = 400;
		l.lfItalic = FALSE;
		l.lfUnderline = FALSE;
		l.lfStrikeOut = FALSE;
		l.lfCharSet = SHIFTJIS_CHARSET; // TODO: i18n
		l.lfOutPrecision = OUT_DEFAULT_PRECIS;
		l.lfQuality = DEFAULT_QUALITY;
		l.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		strcpy(l.lfFaceName, TVPDefaultFontName); // TODO: i18n

		TVPDefaultFont.Height = l.lfHeight;
		TVPDefaultFont.Flags = 0;
		TVPDefaultFont.Angle = 0;
		TVPDefaultFont.Face = ttstr(l.lfFaceName);
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// font enumeration and existence check
//---------------------------------------------------------------------------
class tTVPAnsiStringHash
{
public:
	static tjs_uint32 Make(const AnsiString &val)
	{
		const char * ptr = val.c_str();
		if(*ptr == 0) return 0;
		tjs_uint32 v = 0;
		while(*ptr)
		{
			v += *ptr;
			v += (v << 10);
			v ^= (v >> 6);
			ptr++;
		}
		v += (v << 3);
		v ^= (v >> 11);
		v += (v << 15);
		if(!v) v = (tjs_uint32)-1;
		return v;
	}
};
static tTJSHashTable<AnsiString, tjs_int, tTVPAnsiStringHash>
	TVPFontNames;
bool TVPFontNamesInit = false;
//---------------------------------------------------------------------------
struct tTVPFontSearchStruct
{
	const char *Face;
	bool Found;
};
//---------------------------------------------------------------------------
static int CALLBACK TVPEnumFontsProc(LOGFONT *lplf, TEXTMETRIC *lptm,
	DWORD type, LPARAM data)
{
	TVPFontNames.Add(AnsiString(lplf->lfFaceName), 1);
	return 1;
}
//---------------------------------------------------------------------------
static void TVPInitFontNames()
{
	// enumlate all fonts
	if(TVPFontNamesInit) return;

	HDC dc = GetDC(NULL);
	EnumFonts(dc, NULL, (int (__stdcall *)())TVPEnumFontsProc,
			(LPARAM)NULL);
 	ReleaseDC(NULL, dc);

	TVPFontNamesInit = true;
}
//---------------------------------------------------------------------------
static bool TVPFontExists(const AnsiString &name)
{
	// check existence of font
	TVPInitFontNames();

	int * t = TVPFontNames.Find(name);

	return t != NULL;
}
//---------------------------------------------------------------------------
AnsiString TVPGetBeingFont(AnsiString fonts)
{
	// retrieve being font in the system.
	// font candidates are given by "fonts", separated by comma.

	bool vfont;

	if(fonts.c_str()[0] == '@')      // for vertical writing
	{
		fonts = fonts.c_str() + 1;
		vfont = true;
	}
	else
	{
		vfont = false;
	}

	bool prev_empty_name = false;

	while(fonts!="")
	{
		AnsiString fontname;
		int pos = fonts.AnsiPos(",");
		if(pos!=0)
		{
			fontname = Trim(fonts.SubString(1, pos-1));
			fonts = fonts.c_str()+pos;
		}
		else
		{
			fontname = Trim(fonts);
			fonts="";
		}

		// no existing check if previously specified font candidate is empty
		// eg. ",Fontname"

		if(fontname != "" && (prev_empty_name || TVPFontExists(fontname) ) )
		{
			if(vfont && fontname.c_str()[0] != '@')
			{
				return  "@" + fontname;
			}
			else
			{
				return fontname;
			}
		}

		prev_empty_name = (fontname == "");
	}

	if(vfont)
	{
		return AnsiString("@") + TVPDefaultFontName;
	}
	else
	{
		return TVPDefaultFontName;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// device context for font representation
//---------------------------------------------------------------------------
static tjs_int TVPFontDCRefCount = 0;
static Graphics::TBitmap * TVPBitmapForFontDC = NULL;
static Graphics::TBitmap * TVPBitmapForNonBoldFontDC = NULL;
static tTVPNativeBaseBitmap * TVPFontDCLastBitmap = NULL;
static LOGFONT TVPFontDCCurentLOGFONT;
//---------------------------------------------------------------------------
static void TVPFontDCAddRef()
{
	if(TVPFontDCRefCount == 0)
	{
		TVPBitmapForFontDC = new Graphics::TBitmap();
		TVPBitmapForFontDC->Width = 32;
		TVPBitmapForFontDC->Height = 32;
		TVPBitmapForFontDC->PixelFormat = pf32bit;
		TVPBitmapForNonBoldFontDC = new Graphics::TBitmap();
		TVPBitmapForNonBoldFontDC->Width = 32;
		TVPBitmapForNonBoldFontDC->Height = 32;
		TVPBitmapForNonBoldFontDC->PixelFormat = pf32bit;
	}
	TVPFontDCRefCount ++;
}
//---------------------------------------------------------------------------
static void TVPFontDCRelease()
{
	TVPFontDCRefCount --;
	TVPFontDCLastBitmap = NULL;
	if(TVPFontDCRefCount == 0)
	{
		delete TVPBitmapForFontDC;
		TVPBitmapForFontDC = NULL;
		delete TVPBitmapForNonBoldFontDC;
		TVPBitmapForNonBoldFontDC = NULL;
	}
}
//---------------------------------------------------------------------------
static void TVPFontDCApplyFont(tTVPNativeBaseBitmap *bmp, bool force)
{
	if(bmp != TVPFontDCLastBitmap || force)
	{
		TVPBitmapForFontDC->Canvas->Font->Handle =
			CreateFontIndirect(bmp->GetLOGFONT());
		TVPFontDCCurentLOGFONT = *(bmp->GetLOGFONT());
		int orgweight = TVPFontDCCurentLOGFONT.lfWeight;
		TVPFontDCCurentLOGFONT.lfWeight = 400;
		TVPBitmapForNonBoldFontDC->Canvas->Font->Handle =
			CreateFontIndirect(&TVPFontDCCurentLOGFONT);
		TVPFontDCCurentLOGFONT.lfWeight = orgweight;
		TVPFontDCLastBitmap = bmp;
	}
}
//---------------------------------------------------------------------------
static TCanvas * TVPFontDCGetCanvas() { return TVPBitmapForFontDC->Canvas; }
static TCanvas * TVPNonBoldFontDCGetCanvas() { return TVPBitmapForNonBoldFontDC->Canvas; }
//---------------------------------------------------------------------------
static tjs_int TVPFontDCGetAscentHeight()
{
  int otmSize = ::GetOutlineTextMetrics(TVPFontDCGetCanvas()->Handle, 0, NULL);
  char *otmBuf = new char[otmSize];
  OUTLINETEXTMETRIC *otm = (OUTLINETEXTMETRIC*)otmBuf;
  ::GetOutlineTextMetrics(TVPFontDCGetCanvas()->Handle, otmSize, otm);
  tjs_int result = otm->otmAscent;
  delete[] otmBuf;
  return result;
}
//---------------------------------------------------------------------------
static void TVPGetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h)
{
	SIZE s;
	s.cx = 0;
	s.cy = 0;
	if(!procGetTextExtentPoint32W)
	{
		unsigned char pbuf[10+1];
		tjs_int pbuflen = TJS_wctomb((char*)pbuf, ch);
		if(pbuflen != -1)
		{
			pbuf[pbuflen] = 0;
			GetTextExtentPoint32A(TVPNonBoldFontDCGetCanvas()->Handle,
				(const char*)pbuf, pbuflen, &s);
		}
	}
	else
	{
		procGetTextExtentPoint32W(TVPNonBoldFontDCGetCanvas()->Handle,
			&ch, 1, &s);
	}

	w = s.cx;
	h = s.cy;

	if(TVPFontDCCurentLOGFONT.lfWeight >= 700)
	{
		w += (int)(h / 50) + 1; // calculate bold font size
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// tTVPNativeBaseBitmap
//---------------------------------------------------------------------------
tTVPNativeBaseBitmap::tTVPNativeBaseBitmap(tjs_uint w, tjs_uint h, tjs_uint bpp)
{
	TVPFontDCAddRef();

	TVPConstructDefaultFont();
	Font = TVPDefaultFont;
	PrerenderedFont = NULL;
	LogFont = TVPDefaultLOGFONT;
    tTJSVariant val;
    if (TVPGetCommandLine(TJS_W("-fontcharset"), &val))
      LogFont.lfCharSet = (tjs_int)val;
	FontChanged = true;
	GlobalFontState = -1;
	TextWidth = TextHeight = 0;
	Bitmap = new tTVPBitmap(w, h, bpp);
}
//---------------------------------------------------------------------------
tTVPNativeBaseBitmap::tTVPNativeBaseBitmap(const tTVPNativeBaseBitmap & r)
{
	TVPFontDCAddRef();

	Bitmap = r.Bitmap;
	Bitmap->AddRef();

	Font = r.Font;
	PrerenderedFont = NULL;
	LogFont = TVPDefaultLOGFONT;
    tTJSVariant val;
    if (TVPGetCommandLine(TJS_W("-fontcharset"), &val))
      LogFont.lfCharSet = (tjs_int)val;
	FontChanged = true;
	TextWidth = TextHeight = 0;
}
//---------------------------------------------------------------------------
tTVPNativeBaseBitmap::~tTVPNativeBaseBitmap()
{
	Bitmap->Release();
	if(PrerenderedFont) PrerenderedFont->Release();

	TVPFontDCRelease();
}
//---------------------------------------------------------------------------
tjs_uint tTVPNativeBaseBitmap::GetWidth() const
{
	return Bitmap->GetWidth();
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::SetWidth(tjs_uint w)
{
	SetSize(w, Bitmap->GetHeight());
}
//---------------------------------------------------------------------------
tjs_uint tTVPNativeBaseBitmap::GetHeight() const
{
	return Bitmap->GetHeight();
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::SetHeight(tjs_uint h)
{
	SetSize(Bitmap->GetWidth(), h);
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::SetSize(tjs_uint w, tjs_uint h, bool keepimage)
{
	if(Bitmap->GetWidth() != w || Bitmap->GetHeight() != h)
	{
		// create a new bitmap and copy existing bitmap
		tTVPBitmap *newbitmap = new tTVPBitmap(w, h, Bitmap->GetBPP());

		if(keepimage)
		{
			tjs_int pixelsize = Bitmap->Is32bit() ? 4 : 1;
			tjs_int lh = h < Bitmap->GetHeight() ?
				h : Bitmap->GetHeight();
			tjs_int lw = w < Bitmap->GetWidth() ?
				w : Bitmap->GetWidth();
			tjs_int cs = lw * pixelsize;
			tjs_int i;
			for(i = 0; i < lh; i++)
			{
				void * ds = newbitmap->GetScanLine(i);
				void * ss = Bitmap->GetScanLine(i);

				memcpy(ds, ss, cs);
			}
		}

		Bitmap->Release();
		Bitmap = newbitmap;

		FontChanged = true;
	}
}
//---------------------------------------------------------------------------
tjs_uint tTVPNativeBaseBitmap::GetBPP() const
{
	return Bitmap->GetBPP();
}
//---------------------------------------------------------------------------
bool tTVPNativeBaseBitmap::Is32BPP() const
{
	return Bitmap->Is32bit();
}
//---------------------------------------------------------------------------
bool tTVPNativeBaseBitmap::Is8BPP() const
{
	return Bitmap->Is8bit();
}
//---------------------------------------------------------------------------
bool tTVPNativeBaseBitmap::Assign(const tTVPNativeBaseBitmap &rhs)
{
	if(this == &rhs || Bitmap == rhs.Bitmap) return false;

	Bitmap->Release();
	Bitmap = rhs.Bitmap;
	Bitmap->AddRef();

	Font = rhs.Font;
	FontChanged = true; // informs internal font information is invalidated


	return true; // changed
}
//---------------------------------------------------------------------------
bool tTVPNativeBaseBitmap::AssignBitmap(const tTVPNativeBaseBitmap &rhs)
{
	// assign only bitmap
	if(this == &rhs || Bitmap == rhs.Bitmap) return false;

	Bitmap->Release();
	Bitmap = rhs.Bitmap;
	Bitmap->AddRef();

	// font information are not copyed
	FontChanged = true; // informs internal font information is invalidated

	return true;
}
//---------------------------------------------------------------------------
const void * tTVPNativeBaseBitmap::GetScanLine(tjs_uint l) const
{
	return Bitmap->GetScanLine(l);
}
//---------------------------------------------------------------------------
void * tTVPNativeBaseBitmap::GetScanLineForWrite(tjs_uint l)
{
	Independ();
	return Bitmap->GetScanLine(l);
}
//---------------------------------------------------------------------------
tjs_int tTVPNativeBaseBitmap::GetPitchBytes() const
{
	return Bitmap->GetPitch();
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::Independ()
{
	// sever Bitmap's image sharing
	if(Bitmap->IsIndependent()) return;
	tTVPBitmap *newb = new tTVPBitmap(*Bitmap);
	Bitmap->Release();
	Bitmap = newb;
	FontChanged = true; // informs internal font information is invalidated
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::IndependNoCopy()
{
	// indepent the bitmap, but not to copy the original bitmap
	if(Bitmap->IsIndependent()) return;
	Recreate();
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::Recreate()
{
	Recreate(Bitmap->GetWidth(), Bitmap->GetHeight(), Bitmap->GetBPP());
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::Recreate(tjs_uint w, tjs_uint h, tjs_uint bpp)
{
	Bitmap->Release();
	Bitmap = new tTVPBitmap(w, h, bpp);
	FontChanged = true; // informs internal font information is invalidated
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::ApplyFont()
{
	// apply font
	if(FontChanged || GlobalFontState != TVPGlobalFontStateMagic)
	{
		Independ();

		FontChanged = false;
		GlobalFontState = TVPGlobalFontStateMagic;
		CachedText.Clear();
		TextWidth = TextHeight = 0;

		if(PrerenderedFont) PrerenderedFont->Release();
		PrerenderedFont = TVPGetPrerenderedMappedFont(Font);

		LogFont.lfHeight = -std::abs(Font.Height);
		LogFont.lfItalic = (Font.Flags & TVP_TF_ITALIC) ? TRUE:FALSE;
		LogFont.lfWeight = (Font.Flags & TVP_TF_BOLD) ? 700 : 400;
		LogFont.lfUnderline = (Font.Flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
		LogFont.lfStrikeOut = (Font.Flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
		LogFont.lfEscapement = LogFont.lfOrientation = Font.Angle;
		AnsiString face = TVPGetBeingFont(Font.Face.AsAnsiString());
		strncpy(LogFont.lfFaceName, face.c_str(), LF_FACESIZE -1);
		LogFont.lfFaceName[LF_FACESIZE-1] = 0;

		// compute ascent offset
		TVPFontDCApplyFont(this, true);
		tjs_int ascent = TVPFontDCGetAscentHeight();
		RadianAngle = Font.Angle * (M_PI/1800);
		double angle90 = RadianAngle + M_PI_2;
		AscentOfsX = -cos(angle90) * ascent;
		AscentOfsY = sin(angle90) * ascent;

		// compute font hash
		FontHash = tTJSHashFunc<ttstr>::Make(Font.Face);
		FontHash ^= Font.Height ^ Font.Flags ^ Font.Angle;
	}
	else
	{
		TVPFontDCApplyFont(this, false);
	}
}
//---------------------------------------------------------------------------
HDC tTVPNativeBaseBitmap::GetFontDC()
{
	ApplyFont();
	return TVPFontDCGetCanvas()->Handle;
}
//---------------------------------------------------------------------------
HDC tTVPNativeBaseBitmap::GetNonBoldFontDC()
{
	ApplyFont();
	return TVPNonBoldFontDCGetCanvas()->Handle;
}
//---------------------------------------------------------------------------
TCanvas * tTVPNativeBaseBitmap::GetFontCanvas()
{
	ApplyFont();
	return TVPFontDCGetCanvas();
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::SetFont(const tTVPFont &font)
{
	Font = font;
	FontChanged = true;
}
//---------------------------------------------------------------------------
bool tTVPNativeBaseBitmap::SelectFont(tjs_uint32 flags, const ttstr &caption,
	const ttstr &prompt, const ttstr &samplestring, ttstr &selectedfont)
{
	// show font selector dialog and let user select font
	ApplyFont();
	AnsiString newfont;
	TTVPFontSelectForm * form = new TTVPFontSelectForm(Application, GetFontCanvas(),
		flags, caption.AsAnsiString(), prompt.AsAnsiString(),
			samplestring.AsAnsiString());
	int mr = form->ShowModal();
	newfont = form->FontName;
	delete form;
	if(mr == mrOk)
	{
		selectedfont = ttstr(newfont.c_str());
		return true;
	}
	else
	{
		return false;
	}
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::GetFontList(tjs_uint32 flags, std::vector<ttstr> &list)
{
	ApplyFont();
	std::vector<AnsiString> ansilist;
	TVPGetFontList(ansilist, flags, GetFontCanvas());
	for(std::vector<AnsiString>::iterator i = ansilist.begin(); i != ansilist.end(); i++)
		list.push_back(i->c_str());
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::MapPrerenderedFont(const ttstr & storage)
{
	ApplyFont();
	TVPMapPrerenderedFont(Font, storage);
	FontChanged = true;
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::UnmapPrerenderedFont()
{
	ApplyFont();
	TVPUnmapPrerenderedFont(Font);
	FontChanged = true;
}
//---------------------------------------------------------------------------
struct tTVPDrawTextData
{
	tTVPRect rect;
	tjs_int bmppitch;
	tjs_int opa;
	bool holdalpha;
	tTVPBBBltMethod bltmode;
};
bool tTVPNativeBaseBitmap::InternalDrawText(tTVPCharacterData *data, tjs_int x,
	tjs_int y, tjs_uint32 color, tTVPDrawTextData *dtdata, tTVPRect &drect)
{
	tjs_uint8 *sl;
	tjs_int h;
	tjs_int w;
	tjs_uint8 *bp;
	tjs_int pitch;

	// setup destination and source rectangle
	drect.left = x + data->OriginX;
	drect.top = y + data->OriginY;
	drect.right = drect.left + data->BlackBoxX;
	drect.bottom = drect.top + data->BlackBoxY;

	tTVPRect srect;
	srect.left = srect.top = 0;
	srect.right = data->BlackBoxX;
	srect.bottom = data->BlackBoxY;

	// check boundary
	if(drect.left < dtdata->rect.left)
	{
		srect.left += (dtdata->rect.left - drect.left);
		drect.left = dtdata->rect.left;
	}

	if(drect.right > dtdata->rect.right)
	{
		srect.right -= (drect.right - dtdata->rect.right);
		drect.right = dtdata->rect.right;
	}

	if(srect.left >= srect.right) return false; // not drawable

	if(drect.top < dtdata->rect.top)
	{
		srect.top += (dtdata->rect.top - drect.top);
		drect.top = dtdata->rect.top;
	}

	if(drect.bottom > dtdata->rect.bottom)
	{
		srect.bottom -= (drect.bottom - dtdata->rect.bottom);
		drect.bottom = dtdata->rect.bottom;
	}

	if(srect.top >= srect.bottom) return false; // not drawable


	// blend to the bitmap
	pitch = data->Pitch;
	sl = (tjs_uint8*)GetScanLineForWrite(drect.top);
	h = drect.bottom - drect.top;
	w = drect.right - drect.left;
	bp = data->GetData() + pitch * srect.top;

	if(dtdata->bltmode == bmAlphaOnAlpha)
	{
		if(dtdata->opa > 0)
		{
			if(dtdata->opa == 255)
			{
				while(h--)
					TVPApplyColorMap65_d((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color), sl += dtdata->bmppitch,
						bp += pitch;
			}
			else
			{
				while(h--)
					TVPApplyColorMap65_do((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			}
		}
		else
		{
			// opacity removal
			if(dtdata->opa == -255)
			{
				while(h--)
					TVPRemoveOpacity65((tjs_uint32*)sl + drect.left,
						bp + srect.left, w), sl += dtdata->bmppitch,
						bp += pitch;
			}
			else
			{
				while(h--)
					TVPRemoveOpacity65_o((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, -dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			}
		}
	}
	else if(dtdata->bltmode == bmAlphaOnAddAlpha)
	{
		if(dtdata->opa == 255)
		{
			while(h--)
				TVPApplyColorMap65_a((tjs_uint32*)sl + drect.left,
					bp + srect.left, w, color), sl += dtdata->bmppitch,
					bp += pitch;
		}
		else
		{
			while(h--)
				TVPApplyColorMap65_ao((tjs_uint32*)sl + drect.left,
					bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
					bp += pitch;
		}
	}
	else
	{
		if(dtdata->opa == 255)
		{
			if(dtdata->holdalpha)
				while(h--)
					TVPApplyColorMap65_HDA((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color), sl += dtdata->bmppitch,
						bp += pitch;
			else
				while(h--)
					TVPApplyColorMap65((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color), sl += dtdata->bmppitch,
						bp += pitch;
		}
		else
		{
			if(dtdata->holdalpha)
				while(h--)
					TVPApplyColorMap65_HDA_o((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			else
				while(h--)
					TVPApplyColorMap65_o((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
		}
	}

	return true;
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::DrawTextSingle(const tTVPRect &destrect,
	tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa,
			bool holdalpha, bool aa, tjs_int shlevel,
			tjs_uint32 shadowcolor,
			tjs_int shwidth, tjs_int shofsx, tjs_int shofsy,
			tTVPComplexRect *updaterects)
{
	// text drawing function for single character

	if(!Is32BPP()) TVPThrowExceptionMessage(TVPInvalidOperationFor8BPP);

	if(bltmode == bmAlphaOnAlpha)
	{
		if(opa < -255) opa = -255;
		if(opa > 255) opa = 255;
	}
	else
	{
		if(opa < 0) opa = 0;
		if(opa > 255 ) opa = 255;
	}

	if(opa == 0) return; // nothing to do

	Independ();

	ApplyFont();

	const tjs_char *p = text.c_str();
	tTVPDrawTextData dtdata;
	dtdata.rect = destrect;
	dtdata.bmppitch = GetPitchBytes();
	dtdata.bltmode = bltmode;
	dtdata.opa = opa;
	dtdata.holdalpha = holdalpha;

	tTVPFontAndCharacterData font;
	font.Font = Font;
	font.Antialiased = aa;
	font.BlurLevel = shlevel;
	font.BlurWidth = shwidth;
	font.FontHash = FontHash;

	font.Character = *p;

	font.Blured = false;
	tTVPCharacterData * shadow = NULL;
	tTVPCharacterData * data = NULL;

	try
	{
		data = TVPGetCharacter(font, this, PrerenderedFont, AscentOfsX, AscentOfsY);

		if(shlevel != 0)
		{
			if(shlevel == 255 && shwidth == 0)
			{
				// normal shadow
				shadow = data;
				shadow->AddRef();
			}
			else
			{
				// blured shadow
				font.Blured = true;
				shadow =
					TVPGetCharacter(font, this, PrerenderedFont, AscentOfsX, AscentOfsY);
			}
		}


		if(data)
		{

			if(data->BlackBoxX != 0 && data->BlackBoxY != 0)
			{
				tTVPRect drect;
				tTVPRect shadowdrect;

				bool shadowdrawn = false;

				if(shadow)
				{
					shadowdrawn = InternalDrawText(shadow, x + shofsx, y + shofsy,
						shadowcolor, &dtdata, shadowdrect);
				}

				bool drawn = InternalDrawText(data, x, y, color, &dtdata, drect);
				if(updaterects)
				{
					if(!shadowdrawn)
					{
						if(drawn) updaterects->Or(drect);
					}
					else
					{
						if(drawn)
						{
							tTVPRect d;
							TVPUnionRect(&d, drect, shadowdrect);
							updaterects->Or(d);
						}
						else
						{
							updaterects->Or(shadowdrect);
						}
					}
				}
			}
		}
	}
	catch(...)
	{
		if(data) data->Release();
		if(shadow) shadow->Release();
		throw;
	}

	if(data) data->Release();
	if(shadow) shadow->Release();
}
//---------------------------------------------------------------------------
// structure for holding data for a character
struct tTVPCharacterDrawData
{
	tTVPCharacterData * Data; // main character data
	tTVPCharacterData * Shadow; // shadow character data
	tjs_int X, Y;
	tTVPRect ShadowRect;
	bool ShadowDrawn;

	tTVPCharacterDrawData(
		tTVPCharacterData * data,
		tTVPCharacterData * shadow,
		tjs_int x, tjs_int y)
	{
		Data = data;
		Shadow = shadow;
		X = x;
		Y = y;
		ShadowDrawn = false;

		if(Data) Data->AddRef();
		if(Shadow) Shadow->AddRef();
	}

	~tTVPCharacterDrawData()
	{
		if(Data) Data->Release();
		if(Shadow) Shadow->Release();
	}

	tTVPCharacterDrawData(const tTVPCharacterDrawData & rhs)
	{
		Data = Shadow = NULL;
		*this = rhs;
	}

	void operator = (const tTVPCharacterDrawData & rhs)
	{
		X = rhs.X;
		Y = rhs.Y;
		ShadowRect = rhs.ShadowRect;
		ShadowDrawn = rhs.ShadowDrawn;

		if(Data != rhs.Data)
		{
			if(Data) Data->Release();
			Data = rhs.Data;
			if(Data) Data->AddRef();
		}
		if(Shadow != rhs.Shadow)
		{
			if(Shadow) Shadow->Release();
			Shadow = rhs.Shadow;
			if(Shadow) Shadow->AddRef();
		}
	}
};
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::DrawTextMultiple(const tTVPRect &destrect,
	tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa,
			bool holdalpha, bool aa, tjs_int shlevel,
			tjs_uint32 shadowcolor,
			tjs_int shwidth, tjs_int shofsx, tjs_int shofsy,
			tTVPComplexRect *updaterects)
{
	// text drawing function for multiple characters

	if(!Is32BPP()) TVPThrowExceptionMessage(TVPInvalidOperationFor8BPP);

	if(bltmode == bmAlphaOnAlpha)
	{
		if(opa < -255) opa = -255;
		if(opa > 255) opa = 255;
	}
	else
	{
		if(opa < 0) opa = 0;
		if(opa > 255 ) opa = 255;
	}

	if(opa == 0) return; // nothing to do

	Independ();

	ApplyFont();

	const tjs_char *p = text.c_str();
	tTVPDrawTextData dtdata;
	dtdata.rect = destrect;
	dtdata.bmppitch = GetPitchBytes();
	dtdata.bltmode = bltmode;
	dtdata.opa = opa;
	dtdata.holdalpha = holdalpha;

	tTVPFontAndCharacterData font;
	font.Font = Font;
	font.Antialiased = aa;
	font.BlurLevel = shlevel;
	font.BlurWidth = shwidth;
	font.FontHash = FontHash;


	std::vector<tTVPCharacterDrawData> drawdata;
	drawdata.reserve(text.GetLen());

	// prepare all drawn characters
	while(*p) // while input string is remaining
	{
		font.Character = *p;

		font.Blured = false;
		tTVPCharacterData * data = NULL;
		tTVPCharacterData * shadow = NULL;
		try
		{
			data =
				TVPGetCharacter(font, this, PrerenderedFont, AscentOfsX, AscentOfsY);

			if(data)
			{
				if(shlevel != 0)
				{
					if(shlevel == 255 && shwidth == 0)
					{
						// normal shadow
						// shadow is the same as main character data
						shadow = data;
						shadow->AddRef();
					}
					else
					{
						// blured shadow
						font.Blured = true;
						shadow =
							TVPGetCharacter(font, this, PrerenderedFont, AscentOfsX, AscentOfsY);
					}
				}


				if(data->BlackBoxX != 0 && data->BlackBoxY != 0)
				{
					// append to array
					drawdata.push_back(tTVPCharacterDrawData(data, shadow, x, y));
				}

				// step to the next character position
				x += data->CellIncX;
				if(data->CellIncY != 0)
				{
					// Windows 9x returns negative CellIncY.
					// so we must verify whether CellIncY is proper.
					if(Font.Angle < 1800)
					{
						if(data->CellIncY > 0) data->CellIncY = - data->CellIncY;
					}
					else
					{
						if(data->CellIncY < 0) data->CellIncY = - data->CellIncY;
					}
					y += data->CellIncY;
				}
			}
		}
		catch(...)
		{
			 if(data) data->Release();
			 if(shadow) shadow->Release();
			 throw;
		}
		if(data) data->Release();
		if(shadow) shadow->Release();

		p++;
	}

	// draw shadows first
	if(shlevel != 0)
	{
		for(std::vector<tTVPCharacterDrawData>::iterator i = drawdata.begin();
			i != drawdata.end(); i++)
		{
			tTVPCharacterData * shadow = i->Shadow;

			if(shadow)
			{
				i->ShadowDrawn = InternalDrawText(shadow, i->X + shofsx, i->Y + shofsy,
					shadowcolor, &dtdata, i->ShadowRect);
			}
		}
	}

	// then draw main characters
	// and compute returning update rectangle
	for(std::vector<tTVPCharacterDrawData>::iterator i = drawdata.begin();
		i != drawdata.end(); i++)
	{
		tTVPCharacterData * data = i->Data;
		tTVPRect drect;

		bool drawn = InternalDrawText(data, i->X, i->Y, color, &dtdata, drect);
		if(updaterects)
		{
			if(!i->ShadowDrawn)
			{
				if(drawn) updaterects->Or(drect);
			}
			else
			{
				if(drawn)
				{
					tTVPRect d;
					TVPUnionRect(&d, drect, i->ShadowRect);
					updaterects->Or(d);
				}
				else
				{
					updaterects->Or(i->ShadowRect);
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::GetTextSize(const ttstr & text)
{
	ApplyFont();

	if(text != CachedText)
	{
		CachedText = text;

		if(PrerenderedFont)
		{
			tjs_uint width = 0;
			const tjs_char *buf = text.c_str();
			while(*buf)
			{
				const tTVPPrerenderedCharacterItem * item =
					PrerenderedFont->Find(*buf);
				if(item != NULL)
				{
					width += item->Inc;
				}
				else
				{
					tjs_int w, h;
					TVPGetTextExtent(*buf, w, h);
					width += w;
				}
				buf++;
			}
			TextWidth = width;
			TextHeight = std::abs(Font.Height);
		}
		else
		{
			tjs_uint width = 0;
			const tjs_char *buf = text.c_str();

			while(*buf)
			{
				tjs_int w, h;
				TVPGetTextExtent(*buf, w, h);
				width += w;
				buf++;
			}
			TextWidth = width;
			TextHeight = std::abs(Font.Height);
		}
	}
}
//---------------------------------------------------------------------------
tjs_int tTVPNativeBaseBitmap::GetTextWidth(const ttstr & text)
{
	GetTextSize(text);
	return TextWidth;
}
//---------------------------------------------------------------------------
tjs_int tTVPNativeBaseBitmap::GetTextHeight(const ttstr & text)
{
	GetTextSize(text);
	return TextHeight;
}
//---------------------------------------------------------------------------
double tTVPNativeBaseBitmap::GetEscWidthX(const ttstr & text)
{
	GetTextSize(text);
	return cos(RadianAngle) * TextWidth;
}
//---------------------------------------------------------------------------
double tTVPNativeBaseBitmap::GetEscWidthY(const ttstr & text)
{
	GetTextSize(text);
	return sin(RadianAngle) * (-TextWidth);
}
//---------------------------------------------------------------------------
double tTVPNativeBaseBitmap::GetEscHeightX(const ttstr & text)
{
	GetTextSize(text);
	return sin(RadianAngle) * TextHeight;
}
//---------------------------------------------------------------------------
double tTVPNativeBaseBitmap::GetEscHeightY(const ttstr & text)
{
	GetTextSize(text);
	return cos(RadianAngle) * TextHeight;
}
//---------------------------------------------------------------------------

