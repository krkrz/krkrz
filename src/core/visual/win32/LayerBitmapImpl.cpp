//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#define _USE_MATH_DEFINES
#include "tjsCommHead.h"

#include <memory>
#include <stdlib.h>
#include <math.h>
#include "tstring.h"

#include "LayerBitmapIntf.h"
#include "LayerBitmapImpl.h"
#include "MsgIntf.h"
#include "ComplexRect.h"
#include "tvpgl.h"
#include "tjsHashSearch.h"
#include "EventIntf.h"
#include "SysInitImpl.h"
#include "StorageIntf.h"
#include "DebugIntf.h"
#include "WindowFormUnit.h"
#include "UtilStreams.h"

//#include "FontSelectFormUnit.h"

#include "StringUtil.h"
#include "TFont.h"
#include "CharacterData.h"
#include "PrerenderedFont.h"

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
#if 0 // まったく意味のないコード？
		if(str == TJS_W("auto"))
			; // nothing to do
#endif
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
tTJSHashTable<ttstr, tTVPPrerenderedFont *> TVPPrerenderedFonts;





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

		v ^= val.Antialiased?1:0;
		v ^= val.Character;
		v ^= val.Blured?1:0;
		v ^= val.BlurLevel ^ val.BlurWidth;
		return v;
	}
};






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
		data->Metrics.CellIncX = pitem->IncX;
		data->Metrics.CellIncY = pitem->IncY;
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
		WORD code;
		// system supports UNICODE
		code = font.Character;

		// get buffer size and output dimensions
		int size = ::GetGlyphOutline(bmp->GetFontDC(), code, format, &gm, 0, NULL, transmat);

		// set up structure's variables
		tTVPCharacterData * data = new tTVPCharacterData();

		{
			SIZE s;
			s.cx = 0;
			s.cy = 0;
			GetTextExtentPoint32(bmp->GetNonBoldFontDC(), &font.Character, 1, &s);

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
				data->Metrics.CellIncX = s.cx;
				data->Metrics.CellIncY = 0;
			}
			else if(font.Font.Angle == 2700)
			{
				data->Metrics.CellIncX = 0;
				data->Metrics.CellIncY = s.cx;
			}
			else
			{
				double angle = font.Font.Angle * (M_PI/1800);
				data->Metrics.CellIncX = static_cast<tjs_int>(  cos(angle) * s.cx);
				data->Metrics.CellIncY = static_cast<tjs_int>(- sin(angle) * s.cx);
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
				::GetGlyphOutline(bmp->GetFontDC(), code, format, &gm, size, data->GetData(), transmat);

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

	Allocate(width, height, bpp); // allocate initial bitmap
}
//---------------------------------------------------------------------------
tTVPBitmap::~tTVPBitmap()
{
	TVPFreeBitmapBits(Bits);
	delete BitmapInfo;
}
//---------------------------------------------------------------------------
tTVPBitmap::tTVPBitmap(const tTVPBitmap & r)
{
	// constructor for cloning bitmap
	TVPInitWindowOptions(); // ensure window/bitmap usage options are initialized

	RefCount = 1;

	// allocate bitmap which has the same metrics to r
	Allocate(r.GetWidth(), r.GetHeight(), r.GetBPP());

	// copy BitmapInfo
	*BitmapInfo = *r.BitmapInfo;

	// copy Bits
	if(r.Bits) memcpy(Bits, r.Bits, r.BitmapInfo->GetImageSize() );

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
	BitmapInfo = new BitmapInfomation( width, height, bpp );

	Width = width;
	Height = height;
	PitchBytes = BitmapInfo->GetPitchBytes();
	PitchStep = -PitchBytes;

	// allocate bitmap bits
	try
	{
		Bits = TVPAllocBitmapBits(BitmapInfo->GetImageSize(), width, height);
	}
	catch(...)
	{
		delete BitmapInfo;
		BitmapInfo = NULL;
		throw;
	}
}
//---------------------------------------------------------------------------
void * tTVPBitmap::GetScanLine(tjs_uint l) const
{
	if((tjs_int)l>=BitmapInfo->GetHeight() )
	{
		TVPThrowExceptionMessage(TVPScanLineRangeOver, ttstr((tjs_int)l),
			ttstr((tjs_int)BitmapInfo->GetHeight()-1));
	}

	return (BitmapInfo->GetHeight() - l -1 ) * PitchBytes + (tjs_uint8*)Bits;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// default LOGFONT retrieve function
//---------------------------------------------------------------------------
static const TCHAR * const TVPDefaultFontName = _T("ＭＳ Ｐゴシック"); // TODO: i18n
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
		_tcscpy(l.lfFaceName, TVPDefaultFontName); // TODO: i18n

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
	static tjs_uint32 Make(const tstring &val)
	{
		const TCHAR * ptr = val.c_str();
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
static tTJSHashTable<tstring, tjs_int, tTVPAnsiStringHash> TVPFontNames;
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
	TVPFontNames.Add(tstring(lplf->lfFaceName), 1);
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
static bool TVPFontExists(const tstring &name)
{
	// check existence of font
	TVPInitFontNames();

	int * t = TVPFontNames.Find(name);

	return t != NULL;
}
//---------------------------------------------------------------------------
tstring TVPGetBeingFont(tstring fonts)
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

	while(fonts!=_T(""))
	{
		tstring fontname;
		int pos = fonts.find_first_of(_T(","));
		if( pos != std::string::npos )
		{
			fontname = Trim( fonts.substr( 0, pos) );
			fonts = fonts.c_str()+pos;
		}
		else
		{
			fontname = Trim(fonts);
			fonts=_T("");
		}

		// no existing check if previously specified font candidate is empty
		// eg. ",Fontname"

		if(fontname != _T("") && (prev_empty_name || TVPFontExists(fontname) ) )
		{
			if(vfont && fontname.c_str()[0] != '@')
			{
				return  _T("@") + fontname;
			}
			else
			{
				return fontname;
			}
		}

		prev_empty_name = (fontname == _T(""));
	}

	if(vfont)
	{
		return tstring(_T("@")) + TVPDefaultFontName;
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
/*
static TBitmap * TVPBitmapForFontDC = NULL;
static TBitmap * TVPBitmapForNonBoldFontDC = NULL;
*/
static TFont * TVPBitmapForFontDC = NULL;
static TFont * TVPBitmapForNonBoldFontDC = NULL;
static tTVPNativeBaseBitmap * TVPFontDCLastBitmap = NULL;
static LOGFONT TVPFontDCCurentLOGFONT;
//---------------------------------------------------------------------------
static void TVPFontDCAddRef()
{
	if(TVPFontDCRefCount == 0)
	{/*
		TVPBitmapForFontDC = new TBitmap();
		TVPBitmapForFontDC->SetWidth( 32 );
		TVPBitmapForFontDC->SetHeight( 32 );
		TVPBitmapForFontDC->SetPixelFormat( pf32bit );
		TVPBitmapForNonBoldFontDC = new TBitmap();
		TVPBitmapForNonBoldFontDC->SetWidth( 32 );
		TVPBitmapForNonBoldFontDC->SetHeight( 32 );
		TVPBitmapForNonBoldFontDC->SetPixelFormat( pf32bit );
		*/
		TVPBitmapForFontDC = new TFont();
		TVPBitmapForNonBoldFontDC = new TFont();
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
		//TVPBitmapForFontDC->GetCanvas()->GetFont()->SetHandle(
		//	CreateFontIndirect(bmp->GetLOGFONT()) );
		TVPBitmapForFontDC->ApplyFont( bmp->GetLOGFONT() );
		TVPFontDCCurentLOGFONT = *(bmp->GetLOGFONT());
		int orgweight = TVPFontDCCurentLOGFONT.lfWeight;
		TVPFontDCCurentLOGFONT.lfWeight = 400;
		//TVPBitmapForNonBoldFontDC->GetCanvas()->GetFont()->SetHandle(
		//	CreateFontIndirect(&TVPFontDCCurentLOGFONT) );
		TVPBitmapForNonBoldFontDC->ApplyFont( &TVPFontDCCurentLOGFONT );
		TVPFontDCCurentLOGFONT.lfWeight = orgweight;
		TVPFontDCLastBitmap = bmp;
	}
}
//---------------------------------------------------------------------------
/*
static TCanvas * TVPFontDCGetCanvas() { return TVPBitmapForFontDC->GetCanvas(); }
static TCanvas * TVPNonBoldFontDCGetCanvas() { return TVPBitmapForNonBoldFontDC->GetCanvas(); }
*/
//---------------------------------------------------------------------------
static tjs_int TVPFontDCGetAscentHeight()
{
	/*
  int otmSize = ::GetOutlineTextMetrics(TVPFontDCGetCanvas()->GetHandle(), 0, NULL);
  char *otmBuf = new char[otmSize];
  OUTLINETEXTMETRIC *otm = (OUTLINETEXTMETRIC*)otmBuf;
  ::GetOutlineTextMetrics(TVPFontDCGetCanvas()->GetHandle(), otmSize, otm);
  tjs_int result = otm->otmAscent;
  delete[] otmBuf;
  return result;
  */
  return TVPBitmapForFontDC->GetAscentHeight();
}
//---------------------------------------------------------------------------
static void TVPGetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h)
{
	SIZE s;
	s.cx = 0;
	s.cy = 0;
	::GetTextExtentPoint32(TVPBitmapForNonBoldFontDC->GetDC(), &ch, 1, &s);

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
		tstring face = TVPGetBeingFont(Font.Face.AsStdString());
		_tcsncpy(LogFont.lfFaceName, face.c_str(), LF_FACESIZE -1);
		LogFont.lfFaceName[LF_FACESIZE-1] = 0;

		// compute ascent offset
		TVPFontDCApplyFont(this, true);
		tjs_int ascent = TVPFontDCGetAscentHeight();
		RadianAngle = Font.Angle * (M_PI/1800);
		double angle90 = RadianAngle + M_PI_2;
		AscentOfsX = static_cast<tjs_int>(-cos(angle90) * ascent);
		AscentOfsY = static_cast<tjs_int>(sin(angle90) * ascent);

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
	//return TVPFontDCGetCanvas()->GetHandle();
	return TVPBitmapForFontDC->GetDC();
}
//---------------------------------------------------------------------------
HDC tTVPNativeBaseBitmap::GetNonBoldFontDC()
{
	ApplyFont();
	//return TVPNonBoldFontDCGetCanvas()->GetHandle();
	return TVPBitmapForNonBoldFontDC->GetDC();
}
//---------------------------------------------------------------------------
//TCanvas * tTVPNativeBaseBitmap::GetFontCanvas()
TFont * tTVPNativeBaseBitmap::GetFontCanvas()
{
	ApplyFont();
	//return TVPFontDCGetCanvas();
	return TVPBitmapForFontDC;
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
#if 0 // TODO フォント選択ダイアログ
	std::string newfont;
	TTVPFontSelectForm * form = new TTVPFontSelectForm(Application, GetFontCanvas(),
		flags, caption.AsStdString(), prompt.AsStdString(),
			samplestring.AsStdString());
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
#else
	return false;
#endif
}
//---------------------------------------------------------------------------
void tTVPNativeBaseBitmap::GetFontList(tjs_uint32 flags, std::vector<ttstr> &list)
{
	ApplyFont();
	std::vector<std::wstring> ansilist;
	TVPGetFontList(ansilist, flags, GetFontCanvas());
	for(std::vector<std::wstring>::iterator i = ansilist.begin(); i != ansilist.end(); i++)
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
				x += data->Metrics.CellIncX;
				if(data->Metrics.CellIncY != 0)
				{
					// Windows 9x returns negative CellIncY.
					// so we must verify whether CellIncY is proper.
					if(Font.Angle < 1800)
					{
						if(data->Metrics.CellIncY > 0) data->Metrics.CellIncY = - data->Metrics.CellIncY;
					}
					else
					{
						if(data->Metrics.CellIncY < 0) data->Metrics.CellIncY = - data->Metrics.CellIncY;
					}
					y += data->Metrics.CellIncY;
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

