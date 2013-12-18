#ifndef OGREINFO_H
#define OGREINFO_H

#include <windows.h>
#include "tp_stub.h"

#include "Ogre.h"
#include "OgreConfigFile.h"
using namespace Ogre;

/**
 * Ogre 基本情報クラス
 * 吉里吉里のメインループから常時よばれることを想定
 */
class OgreInfo : public tTVPContinuousEventCallbackIntf
{
public:
	// ルート情報
	Ogre::Root *root;

	/**
	 * コンストラクタ
	 */
	OgreInfo();

	/**
	 * デストラクタ
	 */
	virtual ~OgreInfo();

public:

	/**
	 * Ogre Config 呼び出し
	 */
	bool config();

	/**
	 * Ogre 呼び出し処理開始
	 */
	void start();

	/**
	 * Ogre 呼び出し処理中断
	 */
	void stop();
	
	/**
	 * Continuous コールバック
	 * 吉里吉里が暇なときに常に呼ばれる
	 * 塗り直し処理
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
};

#endif
