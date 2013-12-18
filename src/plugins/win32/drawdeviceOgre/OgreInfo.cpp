#include "OgreInfo.h"

/**
 * コンストラクタ
 */
OgreInfo::OgreInfo()
{
	// Ogre ルートクラス
	root = new Ogre::Root();
	
	// コンフィグ初期化
	ConfigFile cf;
	cf.load("resources.cfg");
	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();
	
	String secName, typeName, archName;
	while (seci.hasMoreElements()) {
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i) {
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}

/**
 * デストラクタ
 */
OgreInfo::~OgreInfo()
{
	stop();
	delete root;
}

/**
 * Ogre Config 呼び出し
 */
bool
OgreInfo::config()
{
	if (root->showConfigDialog()) {
		root->initialise(false);
		return true;
	}
	return false;
}

/**
 * Ogre 呼び出し処理開始
 */
void
OgreInfo::start()
{
	stop();
	TVPAddContinuousEventHook(this);
}

/**
 * Ogre 呼び出し処理停止
 */
void
OgreInfo::stop()
{
	TVPRemoveContinuousEventHook(this);
}

/**
 * Continuous コールバック
 * 吉里吉里が暇なときに常に呼ばれる
 */
void TJS_INTF_METHOD
OgreInfo::OnContinuousCallback(tjs_uint64 tick)
{
	root->renderOneFrame();
};
