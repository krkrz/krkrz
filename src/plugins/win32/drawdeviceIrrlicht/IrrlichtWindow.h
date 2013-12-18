#ifndef IRRLICHTWINDOW_H
#define IRRLICHTWINDOW_H

#include "IrrlichtBase.h"

extern void registerWindowClass();
extern void unregisterWindowClass();

/**
 * Irrlicht 描画が可能なウインドウ
 */
class IrrlichtWindow :	public IrrlichtBase
{
public:
	// ウインドウプロシージャ
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	HWND parent; //< 親窓(TScrollBox)のハンドル
	HWND hwnd;   //< 現在のハンドル
	iTJSDispatch2 *window; //< オブジェクト情報の参照

	// イベント処理
	static bool __stdcall messageHandler(void *userdata, tTVPWindowMessage *Message);

	// ユーザメッセージレシーバの登録/解除
	void setReceiver(tTVPWindowMessageReceiver receiver, bool enable);

	/**
	 * ウインドウを生成
	 * @param krkr 吉里吉里のウインドウ
	 */
	void createWindow(HWND krkr);

	/**
	 * ウインドウを破棄
	 */
	void destroyWindow();

	/**
	 * 吉里吉里窓にメッセージを送付
	 * @param message メッセージ
	 * @param wParam WPARAM
	 * @param lParam LPARAM
	 * @param convPosition lParam のマウス座標値を親のものに変換する
	 */
	void sendMessage(UINT message, WPARAM wParam, LPARAM lParam, bool convPosition=false);
	
public:
	bool transparentEvent; //< イベント透過
	

	/**
	 * コンストラクタ
	 */
	IrrlichtWindow(iTJSDispatch2 *objthis, iTJSDispatch2 *win, int left, int top, int width, int height);
		
	/**
	 * デストラクタ
	 */
	virtual ~IrrlichtWindow();

	// -----------------------------------------------------------------------
	// 生成ファクトリ
	// -----------------------------------------------------------------------

	static tjs_error Factory(IrrlichtWindow **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	// -----------------------------------------------------------------------
	// continuous handler
	// -----------------------------------------------------------------------
public:
	/**
	 * Continuous コールバック
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
	
	// -----------------------------------------------------------------------
	// 共通メソッド呼び出し用
	// -----------------------------------------------------------------------

public:
	void setEventMask(int mask) {
		IrrlichtBase::setEventMask(mask);
	}

	int getEventMask() {
		return IrrlichtBase::getEventMask();
	}

	irr::video::IVideoDriver *getVideoDriver() {
		return IrrlichtBase::getVideoDriver();
	}

	irr::scene::ISceneManager *getSceneManager() {
		return IrrlichtBase::getSceneManager();
	}

	irr::gui::IGUIEnvironment *getGUIEnvironment() {
		return IrrlichtBase::getGUIEnvironment();
	}

	irr::ILogger *getLogger() {
		return IrrlichtBase::getLogger();
	}
	
	irr::io::IFileSystem *getFileSystem() {
		return IrrlichtBase::getFileSystem();
	}

	// -----------------------------------------------------------------------
	// 固有メソッド
	// -----------------------------------------------------------------------
	
protected:
	bool visible;
	int left;
	int top;
	int width;
	int height;

	void _setPos();
	
public:
	void setVisible(bool v);
	bool getVisible();

	void setLeft(int l);
	int getLeft();

	void setTop(int t);
	int getTop();
	
	void setWidth(int w);
	int getWidth();

	void setHeight(int h);
	int getHeight();
	
	/**
	 * 窓場所指定
	 */	
	void setPos(int l, int t);

	/**
	 * 窓サイズ指定
	 */	
	void setSize(int w, int h);
};

#endif
