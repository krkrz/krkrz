#include <windows.h>

#include "OgreDrawDevice.h"

/**
 * コンストラクタ
 */
tTVPOgreDrawDevice::tTVPOgreDrawDevice(OgreInfo *info)
{
	ogreInfo = info;
	_renderWindow = NULL;
	_sceneManager = ogreInfo->root->createSceneManager(ST_GENERIC, "ExampleSMInstance");
}

/**
 * デストラクタ
 */
tTVPOgreDrawDevice::~tTVPOgreDrawDevice()
{
	detach();
}

/**
 * ウインドウの再設定
 * @param hwnd ハンドル
 */
void
tTVPOgreDrawDevice::attach(HWND hwnd)
{
	// サイズ情報
	RECT rect;
	GetClientRect(hwnd, &rect);
	width  = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// ハンドルを文字列化
	char hwndName[100];
	snprintf(hwndName, sizeof hwndName, "%d", hwnd);

	// ウインドウ名称
	windowName = "window";
	windowName += hwndName;
	
	// 初期化パラメータ
	NameValuePairList params;
	params["parentWindowHandle"] = hwndName;
	params["left"] = "0";
	params["top"] = "0";
	
	// ウインドウ生成
	_renderWindow = ogreInfo->root->createRenderWindow(windowName,
													   width,
													   height,
													   false,
													   &params);

	// XX テスト用
	init();
	
	// ogre 駆動開始
	ogreInfo->start();
}

/**
 * ウインドウの解除
 */
void
tTVPOgreDrawDevice::detach()
{
	ogreInfo->stop();
	if (_renderWindow) {
		_sceneManager->destroyAllCameras();
		_renderWindow->removeAllViewports();
		_renderWindow->removeAllListeners();
		ogreInfo->root->getRenderSystem()->destroyRenderWindow(windowName);
		_renderWindow = NULL;
	}
}

/***
 * ウインドウの指定
 * @param wnd ウインドウハンドラ
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::SetTargetWindow(HWND wnd)
{
	detach();
	if (wnd != NULL) {
		attach(wnd);
	}
}

/**
 * ビットマップコピー処理開始
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	// bitmap処理開始
}

/**
 * ビットマップコピー処理
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo で表されるビットマップの cliprect の領域を、x, y に描画する。
}

/**
 * ビットマップコピー処理終了
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	// bitmap 処理終了
}

//---------------------------------------------------------------------------

/**
 * テスト用初期化処理
 */
void
tTVPOgreDrawDevice::init()
{
	// カメラ初期化
	Camera *camera = _sceneManager->createCamera("Player");
	camera->setPosition(Vector3(0,0,500));
	camera->lookAt(Vector3(0,0,-300));
	camera->setNearClipDistance(5);

	// ビューポート初期化
	Viewport* vp = _renderWindow->addViewport(camera);
	vp->setBackgroundColour(ColourValue(0,0,0));

	// リソース初期化
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
	// テスト用にオブジェクトを配置してみる
	_sceneManager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	Entity *ent = _sceneManager->createEntity("head", "ogrehead.mesh");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	
	// Green nimbus around Ogre
	ParticleSystem* pSys1 = _sceneManager->createParticleSystem("Nimbus", 
																"Examples/GreenyNimbus");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(pSys1);
	
	// Create a rainstorm 
	ParticleSystem* pSys4 = _sceneManager->createParticleSystem("rain", 
																"Examples/Rain");
	SceneNode* rNode = _sceneManager->getRootSceneNode()->createChildSceneNode();
	rNode->translate(0,1000,0);
	rNode->attachObject(pSys4);
	// Fast-forward the rain so it looks more natural
	pSys4->fastForward(5);
	
	// Aureola around Ogre perpendicular to the ground
	ParticleSystem* pSys5 = _sceneManager->createParticleSystem("Aureola", 
																"Examples/Aureola");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(pSys5);
	
	// Set nonvisible timeout
	ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);
}
