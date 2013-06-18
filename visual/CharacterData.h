

#ifndef __CHARACTER_DATA_H__
#define __CHARACTER_DATA_H__

#include "tjsCommHead.h"
/**
 * １グリフのメトリックを表す構造体
 */
struct tGlyphMetrics
{
	tjs_int CellIncX;		//!< 一文字進めるの必要なX方向のピクセル数(64倍されている数値なので注意)
	tjs_int CellIncY;		//!< 一文字進めるの必要なY方向のピクセル数(64倍されている数値なので注意)
};

//---------------------------------------------------------------------------
/**
 * １グリフを表すクラス
 */
class tTVPCharacterData
{
	// character data holder for caching
private:
	tjs_uint8 * Data;
	tjs_int RefCount;

public:
	tjs_int OriginX;
	tjs_int OriginY;
	tGlyphMetrics	Metrics;	//!< メトリック
	//tjs_int CellIncX;
	//tjs_int CellIncY;
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
	tTVPCharacterData(tjs_uint8 * indata,
		tjs_int inpitch,
		tjs_int originx, tjs_int originy,
		tjs_uint blackboxw, tjs_uint blackboxh,
		const tGlyphMetrics & metrics);
	tTVPCharacterData(const tTVPCharacterData & ref);
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

#endif // __CHARACTER_DATA_H__
