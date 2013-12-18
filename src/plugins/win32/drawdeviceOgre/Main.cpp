#include "ncbind/ncbind.hpp"
#include "OgreDrawDevice.h"

// Ogre 基本情報
static OgreInfo *ogreInfo = NULL;

/**
 * Ogre ベース DrawDevice のクラス
 */
class OgreDrawDevice {
public:
	// デバイス情報
	iTVPDrawDevice *device;
public:
	/**
	 * コンストラクタ
	 */
	OgreDrawDevice() {
		device = new tTVPOgreDrawDevice(ogreInfo);
	}

	/**
	 * デストラクタ
	 */
	~OgreDrawDevice() {
		if (device) {
			device->Destruct();
			device = NULL;
		}
	}
	
	/**
	 * @return デバイス情報
	 */
	tjs_int64 GetDevice() {
		return reinterpret_cast<tjs_int64>(device);
	}

	// ---------------------------------------------
	// 以下 Ogre を制御するための機能を順次追加予定
	// ---------------------------------------------
	
};


NCB_REGISTER_CLASS(OgreDrawDevice) {
	NCB_CONSTRUCTOR(());
	NCB_PROPERTY_RO(interface, GetDevice);
}

/**
 * 登録処理前
 */
static void
PreRegistCallback()
{
	// OGRE の基本情報生成
	ogreInfo = new OgreInfo();
	if (!ogreInfo->config()) {
		delete ogreInfo;
		ogreInfo = NULL;
		TVPThrowExceptionMessage(TJS_W("can't init OGRE."));
	}
}

/**
 * 開放処理後
 */
static void PostUnregistCallback()
{
	// ogre 終了
	delete ogreInfo;
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
