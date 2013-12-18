#ifndef OGREDRAWDEVICE_H
#define OGREDRAWDEVICE_H

#include "OgreInfo.h"
#include <string>
using namespace std;

#include "BasicDrawDevice.h"

/**
 * Ogre ベースの DrawDevice
 */
class tTVPOgreDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

	/// OGRE 関係情報
	OgreInfo *ogreInfo;
	string windowName;
	RenderWindow* _renderWindow;
	SceneManager* _sceneManager;
	
public:
	tTVPOgreDrawDevice(OgreInfo *info); //!< コンストラクタ
private:
	virtual ~tTVPOgreDrawDevice(); //!< デストラクタ

	void attach(HWND hwnd);
	void detach();

	int width;
	int height;

	unsigned char *destBuffer;
	int destWidth;
	int destHeight;
	int destPitch;
	
public:

	//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd);

	//---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

	//---------------------------------------------------------------------------

	// テスト用処理
	void init();
};

#endif
