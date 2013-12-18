#ifndef LAYERMANAGERINFO_H
#define LAYERMANAGERINFO_H

/**
 * レイヤマネージャ用付随情報
 */
class LayerManagerInfo {

protected:
	// 識別用ID
	int id;

	// 元レイヤサイズ
	irr::core::rect<irr::s32> srcRect;
	
	// コピー処理用一時変数
	unsigned char *destBuffer;
	int destWidth;
	int destHeight;
	int destPitch;

	// テクスチャ割り当てに使ったドライバ
	irr::video::IVideoDriver *driver;
	// 割り当てテクスチャ
	irr::video::ITexture *texture;
	
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
	
	// テクスチャ割り当て処理
	void alloc(iTVPLayerManager *manager, irr::video::IVideoDriver *driver);
	// テクスチャ解放
	void free();
	
	// テクスチャ描画操作用
	void lock();
	void copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
			  const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	void unlock();

	// 描画
	void draw(irr::video::IVideoDriver *driver, irr::core::rect<irr::s32> destRect);
};

#endif
