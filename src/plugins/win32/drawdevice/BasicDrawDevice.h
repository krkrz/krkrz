//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//!@file 描画デバイス管理
//---------------------------------------------------------------------------
#ifndef DRAWDEVICE_H
#define DRAWDEVICE_H

#include "tp_stub.h"
#include <vector>


//---------------------------------------------------------------------------
//! @brief		描画デバイスインターフェースの基本的な実装
//---------------------------------------------------------------------------
class tTVPDrawDevice : public iTVPDrawDevice
{
protected:
	iTVPWindow * Window;
	size_t PrimaryLayerManagerIndex; //!< プライマリレイヤマネージャ
	std::vector<iTVPLayerManager *> Managers; //!< レイヤマネージャの配列
	tTVPRect DestRect; //!< 描画先位置

protected:
	tTVPDrawDevice(); //!< コンストラクタ
protected:
	virtual ~tTVPDrawDevice(); //!< デストラクタ

public:
	//! @brief		指定位置にあるレイヤマネージャを得る
	//! @param		index		インデックス(0～)
	//! @return		指定位置にあるレイヤマネージャ(AddRefされないので注意)。
	//!				指定位置にレイヤマネージャがなければNULLが返る
	iTVPLayerManager * GetLayerManagerAt(size_t index)
	{
		if(Managers.size() <= index) return NULL;
		return Managers[index];
	}

	//! @brief		Device→LayerManager方向の座標の変換を行う
	//! @param		x		X位置
	//! @param		y		Y位置
	//! @return		変換に成功すれば真。さもなければ偽。PrimaryLayerManagerIndexに該当する
	//!				レイヤマネージャがなければ偽が返る
	//! @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
	bool TransformToPrimaryLayerManager(tjs_int &x, tjs_int &y);

	//! @brief		LayerManager→Device方向の座標の変換を行う
	//! @param		x		X位置
	//! @param		y		Y位置
	//! @return		変換に成功すれば真。さもなければ偽。PrimaryLayerManagerIndexに該当する
	//!				レイヤマネージャがなければ偽が返る
	//! @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
	bool TransformFromPrimaryLayerManager(tjs_int &x, tjs_int &y);

//---- オブジェクト生存期間制御
	virtual void TJS_INTF_METHOD Destruct();

//---- window interface 関連
	virtual void TJS_INTF_METHOD SetWindowInterface(iTVPWindow * window);

//---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager);

//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager);

//---- ユーザーインターフェース関連
	// window → drawdevice
	virtual void TJS_INTF_METHOD OnClick(tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD OnDoubleClick(tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnReleaseCapture();
	virtual void TJS_INTF_METHOD OnMouseOutOfWindow();
	virtual void TJS_INTF_METHOD OnKeyDown(tjs_uint key, tjs_uint32 shift);
	virtual void TJS_INTF_METHOD OnKeyUp(tjs_uint key, tjs_uint32 shift);
	virtual void TJS_INTF_METHOD OnKeyPress(tjs_char key);
	virtual void TJS_INTF_METHOD OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD RecheckInputState();

	// layer manager → drawdevice
	virtual void TJS_INTF_METHOD SetDefaultMouseCursor(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD SetMouseCursor(iTVPLayerManager * manager, tjs_int cursor);
	virtual void TJS_INTF_METHOD GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD SetHintText(iTVPLayerManager * manager, const ttstr & text);
	virtual void TJS_INTF_METHOD WindowReleaseCapture(iTVPLayerManager * manager);

	virtual void TJS_INTF_METHOD SetAttentionPoint(iTVPLayerManager * manager, tTJSNI_BaseLayer *layer,
							tjs_int l, tjs_int t);
	virtual void TJS_INTF_METHOD DisableAttentionPoint(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD SetImeMode(iTVPLayerManager * manager, tTVPImeMode mode);
	virtual void TJS_INTF_METHOD ResetImeMode(iTVPLayerManager * manager);

//---- プライマリレイヤ関連
	virtual tTJSNI_BaseLayer * TJS_INTF_METHOD GetPrimaryLayer();
	virtual tTJSNI_BaseLayer * TJS_INTF_METHOD GetFocusedLayer();
	virtual void TJS_INTF_METHOD SetFocusedLayer(tTJSNI_BaseLayer * layer);

//---- 再描画関連
	virtual void TJS_INTF_METHOD RequestInvalidation(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD Update();
	virtual void TJS_INTF_METHOD Show() = 0;

//---- デバッグ支援
	virtual void TJS_INTF_METHOD DumpLayerStructure();
	virtual void TJS_INTF_METHOD SetShowUpdateRect(bool b);

// ほかのメソッドについては実装しない
};
//---------------------------------------------------------------------------
#endif
