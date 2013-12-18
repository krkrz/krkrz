#ifndef IRRLICHTBASE_H
#define IRRLICHTBASE_H

#include <windows.h>
#include "tp_stub.h"
#include <irrlicht.h>


/**
 * Irrlicht 処理のベース
 */
class IrrlichtBase : public irr::IEventReceiver, public tTVPContinuousEventCallbackIntf
{
public:
	enum EventMask {
		EMASK_ATTACH       = 1<<0, //< アタッチ時
		EMASK_DETACH       = 1<<1, //< デタッチ時
		EMASK_EVENT        = 1<<2, //< Irrlichtイベント
		EMASK_BEFORE_SCENE = 1<<3, //< シーンマネージャ描画前
		EMASK_AFTER_SCENE  = 1<<4, //< シーンマネージャ描画後
		EMASK_BEFORE_GUI   = 1<<5, //< GUI描画前
		EMASK_AFTER_GUI    = 1<<6, //< GUI描画後
	};
	
protected:
	/// TJSオブジェクト
	iTJSDispatch2 *objthis;
	/// デバイス
	irr::IrrlichtDevice *device;

	void showDriverInfo();

protected:
	// イベントマスク
	int eventMask;
	
	/**
	 * TJSイベント呼び出し。自己オブジェクトの該当メソッドを呼び出す。
	 * @param eventName イベント名
	 */
	void sendTJSEvent(const tjs_char *eventName);

protected:
	// デバイス割り当て済み
	bool attached;

	/**
	 * デバイスの割り当て
	 * @param hwnd 親ウインドウハンドル
	 * @param width バックバッファサイズ横幅
	 * @param height バックバッファサイズ縦幅
	 */
	void attach(HWND hwnd, int width=0, int height=0);

	// デバイスの割り当て後処理
	virtual void onAttach() {}
	
	/**
	 * デバイスの破棄
	 */
	void detach();

	// デバイスの破棄前処理
	virtual void onDetach() {};
	
public:
	IrrlichtBase(iTJSDispatch2 *objthis); //!< コンストラクタ
	virtual ~IrrlichtBase(); //!< デストラクタ

	// ------------------------------------------------------------
	// 更新処理
	// ------------------------------------------------------------
protected:
	/**
	 * クラス固有更新処理
	 * シーンマネージャの処理後、GUIの処理前に呼ばれる
	 */
	virtual void update(irr::video::IVideoDriver *driver) {};

protected:

	/**
	 * Irrlicht描画処理
	 * @param destRect 描画先領域。省略すると表示先全面
	 * @param srcRect 描画元領域。省略すると全面
	 * @param destDC 描画先DC。指定すると本来の表示先の代わりにこのDCに描画
	 * @return 描画されたら true
	 */
	bool show(irr::core::rect<irr::s32> *destRect=NULL, irr::core::rect<irr::s32> *srcRect=NULL, HDC destDC=0);

	// ------------------------------------------------------------
	// Irrlicht 共通プロパティ
	// ------------------------------------------------------------
public:

	void setEventMask(int mask) {
		eventMask = mask;
	}

	int getEventMask() {
		return eventMask;
	}

	/**
	 * @return ドライバ情報の取得
	 */
	irr::video::IVideoDriver *getVideoDriver() {
		return device ? device->getVideoDriver() : NULL;
	}
	
	/**
	 * @return シーンマネージャ情報の取得
	 */
	irr::scene::ISceneManager *getSceneManager() {
		return device ? device->getSceneManager() : NULL;
	}

	/**
	 * @return GUI環境情報の取得
	 */
	irr::gui::IGUIEnvironment *getGUIEnvironment() {
		return device ? device->getGUIEnvironment() : NULL;
	}

	/**
	 * @return ロガーの取得
	 */
	irr::ILogger *getLogger() {
		return device ? device->getLogger() : NULL;
	}

	/**
	 * @return ファイルシステムの取得
	 */
	irr::io::IFileSystem *getFileSystem() {
		return device ? device->getFileSystem() : NULL;
	}
	
	// ------------------------------------------------------------
	// Irrlicht イベント処理用
	// ------------------------------------------------------------
public:
	// Irrlicht にイベントを送る
	bool postEvent(irr::SEvent &ev);

	/**
	 * イベント受理
	 * GUI Environment からのイベントがここに送られてくる
	 * @param event イベント情報
	 * @return 処理したら true
	 */
	bool OnEvent(const irr::SEvent &event);

	// -----------------------------------------------------------------------
	// continuous handler
	// -----------------------------------------------------------------------
public:
	/**
	 * Irrlicht 呼び出し処理開始
	 */
	void start();

	/**
	 * Irrlicht 呼び出し処理中断
	 */
	void stop();
};

#endif