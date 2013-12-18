#pragma comment(lib, "d3dx.lib")
#include "DrawDeviceD3D.h"
#include "LayerManagerInfo.h"

/**
 * コンストラクタ
 */
DrawDeviceD3D::DrawDeviceD3D(int width, int height)
	: width(width), height(height), destWidth(0), destHeight(0), defaultVisible(true),
	  DirectDraw7(NULL), Direct3D7(NULL), Direct3DDevice7(NULL), Surface(NULL), Clipper(NULL)
{
	TVPEnsureDirectDrawObject();
}

/**
 * デストラクタ
 */
DrawDeviceD3D::~DrawDeviceD3D()
{
	detach();
}

/**
 * ウインドウの解除
 */
void
DrawDeviceD3D::attach(HWND hWnd)
{
	this->hWnd = hWnd;
	try {
		// Direct3D デバイス、テクスチャなどを作成する
		HRESULT hr;
		// get DirectDraw7/Direct3D7 interface
		if ((DirectDraw7 = TVPGetDirectDraw7ObjectNoAddRef())) {
			DirectDraw7->AddRef();
		} else {
			TVPThrowExceptionMessage(TJS_W("DirectDraw7 not available"));
		}
		if (FAILED(DirectDraw7->QueryInterface(IID_IDirect3D7, (void**)&Direct3D7))) {
			TVPThrowExceptionMessage(TJS_W("Direct3D7 not available"));
		}
		
		// check display mode
		DDSURFACEDESC2 ddsd;
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		hr = DirectDraw7->GetDisplayMode(&ddsd);
		if (FAILED(hr) || ddsd.ddpfPixelFormat.dwRGBBitCount <= 8) {
			TVPThrowExceptionMessage(TJS_W("Too less display color depth"));
		}
		
		// create clipper object
		hr = DirectDraw7->CreateClipper(0, &Clipper, NULL);
		if (hr != DD_OK) {
			TVPThrowExceptionMessage(TJS_W("Cannot create a clipper object/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}
		hr = Clipper->SetHWnd(0, hWnd);
		if (hr != DD_OK) {
			TVPThrowExceptionMessage(TJS_W("Cannot set the window handle to the clipper object/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}
		
	} catch(...) {
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: unknown reason"));
	}
}


/**
 * ウインドウの解除
 */
void
DrawDeviceD3D::detach()
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->free();
		}
	}
	// 解放処理
	if (Direct3DDevice7) Direct3DDevice7->Release(), Direct3DDevice7 = NULL;
	if (Surface) Surface->Release(), Surface = NULL;
	if (Direct3D7) Direct3D7->Release(), Direct3D7 = NULL;
	if (DirectDraw7) DirectDraw7->Release(), DirectDraw7 = NULL;
	TVPReleaseDDPrimarySurface();
	if (Clipper) Clipper->Release(), Clipper = NULL;
}

/**
 * Device→プライマリレイヤの座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
 */
void
DrawDeviceD3D::transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = destWidth  ? (x * pl_w / destWidth) : 0;
	y = destHeight ? (y * pl_h / destHeight) : 0;
}

/** プライマリレイヤ→Device方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
 */
void
DrawDeviceD3D::transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = pl_w ? (x * destWidth  / pl_w) : 0;
	y = pl_h ? (y * destHeight / pl_h) : 0;
}

/**
 * Device→標準画面の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
 */
void
DrawDeviceD3D::transformTo(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * width / destWidth) : 0;
	y = destHeight ? (y * height / destHeight) : 0;
}

/** 標準画面→Device方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
 */
void
DrawDeviceD3D::transformFrom(tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	x = width ? (x * destWidth  / width) : 0;
	y = height ? (y * destHeight / height) : 0;
}

/**
 * レイヤマネージャの登録
 * @param manager レイヤマネージャ
 */
void TJS_INTF_METHOD
DrawDeviceD3D::AddLayerManager(iTVPLayerManager * manager)
{
	int id = (int)Managers.size();
	tTVPDrawDevice::AddLayerManager(manager);
	LayerManagerInfo *info = new LayerManagerInfo(id, defaultVisible);
	manager->SetDrawDeviceData((void*)info);
}

/**
 * レイヤマネージャの削除
 * @param manager レイヤマネージャ
 */
void TJS_INTF_METHOD
DrawDeviceD3D::RemoveLayerManager(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		manager->SetDrawDeviceData(NULL);
		delete info;
	}
	tTVPDrawDevice::RemoveLayerManager(manager);
}

/***
 * ウインドウの指定
 * @param wnd ウインドウハンドラ
 */
void TJS_INTF_METHOD
DrawDeviceD3D::SetTargetWindow(HWND wnd, bool is_main)
{
	detach();
	if (wnd != NULL) {
		attach(wnd);
		Window->NotifySrcResize(); // これを呼ぶことで GetSrcSize(), SetDestRectangle() の呼び返しが来る
		// マネージャに対するテクスチャの割り当て
		if (Surface) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc(manager, DirectDraw7, Direct3DDevice7);
				}
			}
		}
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::SetDestRectangle(const tTVPRect &dest)
{
	destLeft = dest.Left;
	destTop  = dest.Top;
	destWidth = dest.get_width();
	destHeight = dest.get_height();

	try {
		if (Direct3DDevice7) Direct3DDevice7->Release(), Direct3DDevice7 = NULL;
		if (Surface) Surface->Release(), Surface = NULL;
		
		DDSURFACEDESC2 ddsd;
		// allocate secondary off-screen buffer
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth  = destWidth;
		ddsd.dwHeight = destHeight;
		ddsd.ddsCaps.dwCaps =
			/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY /*| DDSCAPS_LOCALVIDMEM*/ | DDSCAPS_3DDEVICE;
		
		HRESULT hr = DirectDraw7->CreateSurface(&ddsd, &Surface, NULL);
		
		if (hr != DD_OK) {
			TVPThrowExceptionMessage(TJS_W("Cannot allocate D3D off-screen surface/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}
		
		// check whether the surface is on video memory
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		
		hr = Surface->GetSurfaceDesc(&ddsd);
		if (hr != DD_OK) {
			TVPThrowExceptionMessage(TJS_W("Cannot get D3D surface description/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}
			
		if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY &&
		   ddsd.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM){
			// ok
		} else {
			TVPThrowExceptionMessage(TJS_W("Cannot allocate the D3D surface on the local video memory"),
									 TJSInt32ToHex(hr, 8));
		}

		// create Direct3D Device
		hr = Direct3D7->CreateDevice(IID_IDirect3DHALDevice, Surface, &Direct3DDevice7);
		if (FAILED(hr)) {
			TVPThrowExceptionMessage(TJS_W("Cannot create Direct3D device/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}

	} catch(...) {
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: unknown reason"));
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::GetSrcSize(tjs_int &w, tjs_int &h)
{
	w = width;
	h = height;
}

void TJS_INTF_METHOD
DrawDeviceD3D::NotifyLayerResize(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		info->free();
		if (Surface) {
			info->alloc(manager, DirectDraw7, Direct3DDevice7);
		}
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::NotifyLayerImageChange(iTVPLayerManager * manager)
{
	Window->RequestUpdate();
}

// -------------------------------------------------------------------------------------
// 入力イベント処理用
// -------------------------------------------------------------------------------------

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseDown(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseUp(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags)
{
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseMove(x, y, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y)
{
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseWheel(shift, delta, x, y);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	Window->GetCursorPos(x, y);
	transformToManager(manager, x, y);
}

void TJS_INTF_METHOD
DrawDeviceD3D::SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y)
{
	transformFromManager(manager, x, y);
	Window->SetCursorPos(x, y);
}

void TJS_INTF_METHOD
DrawDeviceD3D::RequestInvalidation(const tTVPRect & rect)
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		iTVPLayerManager *manager = *i;
		LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
		if (info && info->visible) {
			tjs_int l = rect.left, t = rect.top, r = rect.right, b = rect.bottom;
			transformToManager(manager, l, t);
			transformToManager(manager, r, b);
			r ++; // 誤差の吸収(本当はもうちょっと厳密にやらないとならないがそれが問題になることはない)
			b ++;
			manager->RequestInvalidation(tTVPRect(l, t, r, b));
		}
	}
}


// -------------------------------------------------------------------------------------
// 再描画処理用
// -------------------------------------------------------------------------------------

void
DrawDeviceD3D::Show()
{
	// Blt to the primary surface
	if (!Surface) return;

	// 画面消去
	Direct3DDevice7->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 0.0, 0);
	// 個別レイヤマネージャの描画
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->draw(Direct3DDevice7, destWidth, destHeight);
		}
	}
	
	// retrieve the primary surface
	IDirectDrawSurface *pri = TVPGetDDPrimarySurfaceNoAddRef();
	if (!pri) {
		TVPThrowExceptionMessage(TJS_W("Cannot retrieve primary surface object"));
	}
		
	// set clipper
	TVPSetDDPrimaryClipper(Clipper);

	// get PaintBox's origin
	POINT origin; origin.x = destLeft, origin.y = destTop;
	ClientToScreen(hWnd, &origin);
	// entire of the bitmap is to be transfered (this is not optimal. FIX ME!)
	RECT drect;
	drect.left   = origin.x;
	drect.top    = origin.y;
	drect.right  = origin.x + destWidth;
	drect.bottom = origin.y + destHeight;

	RECT srect;
	srect.left   = 0;
	srect.top    = 0;
	srect.right  = destWidth;
	srect.bottom = destHeight;
	
	HRESULT hr = pri->Blt(&drect, (IDirectDrawSurface*)Surface, &srect, DDBLT_WAIT, NULL);
	
	if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY) {
		pri->Restore();
		Surface->Restore();
		RequestInvalidation(tTVPRect(0, 0, destWidth, destHeight));
	} else if(hr == DDERR_INVALIDRECT) {
		// ignore this error
	} else if(hr != DD_OK) {
		TVPAddImportantLog(
			TJS_W("Passthrough: (inf) Primary surface, IDirectDrawSurface::Blt failed/HR=") +
			TJSInt32ToHex(hr, 8));
	}
}

// -------------------------------------------------------------------------------------
// LayerManagerからの画像うけわたし
// -------------------------------------------------------------------------------------

/**
 * ビットマップコピー処理開始
 */
void TJS_INTF_METHOD
DrawDeviceD3D::StartBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->lock();
	}
}

/**
 * ビットマップコピー処理
 */
void TJS_INTF_METHOD
DrawDeviceD3D::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->copy(x, y, bits, bitmapinfo, cliprect, type, opacity);
	}
}

/**
 * ビットマップコピー処理終了
 */
void TJS_INTF_METHOD
DrawDeviceD3D::EndBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->unlock();
	}
}

/**
 * プライマリレイヤの表示状態の指定
 * @param id プライマリレイヤの登録ID
 * @param visible 表示状態
 */
void
DrawDeviceD3D::setVisible(int id, bool visible)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			info->visible = visible;
			Window->RequestUpdate();
		}
	}
}

/**
 * プライマリレイヤの表示状態の指定
 * @param id プライマリレイヤの登録ID
 * @return visible 表示状態
 */
bool
DrawDeviceD3D::getVisible(int id)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			return info->visible;
		}
	}
	return false;
}
