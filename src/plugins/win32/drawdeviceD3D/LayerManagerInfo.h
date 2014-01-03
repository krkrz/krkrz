#ifndef LAYERMANAGERINFO_H
#define LAYERMANAGERINFO_H

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>

/**
 * レイヤマネージャ用付随情報
 */
class LayerManagerInfo {

protected:
	// 識別用ID
	int id;

	// 元レイヤサイズ
	int srcWidth;
	int srcHeight;

	// 割り当てテクスチャ
	IDirect3DTexture9 *texture;
	tjs_uint textureWidth; //< テクスチャの横幅
	tjs_uint textureHeight; //< テクスチャの縦幅

	void *textureBuffer; //< テクスチャのサーフェースへのメモリポインタ
	long texturePitch; //< テクスチャのピッチ

	bool lastOK;     //< 前回の処理は成功したか
	
public:
	// 表示対象かどうか
	bool visible;

public:
	/**
	 * コンストラクタ
	 * @param id レイヤID
	 * @param visible 初期表示状態
	 */
	LayerManagerInfo(int id, bool visible);
	virtual ~LayerManagerInfo();
	
	/**
	 * テクスチャ割り当て処理
	 */
	void alloc(iTVPLayerManager *manager, IDirect3DDevice9 *direct3DDevice);

	/*
	 * テクスチャ解放
	 */
	void free();
	
	// テクスチャ描画操作用
	void lock();
	void copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
			  const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	void unlock();

	/**
	 * 描画
	 */
	void draw(IDirect3DDevice9 *direct3DDevice9, const tTVPRect &destrect, const tTVPRect &cliprect);
};

#endif
