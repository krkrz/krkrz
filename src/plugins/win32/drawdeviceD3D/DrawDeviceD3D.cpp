
#include "DrawDeviceD3D.h"
#include "LayerManagerInfo.h"

/**
 * コンストラクタ
 */
DrawDeviceD3D::DrawDeviceD3D(int width, int height)
	: width(width), height(height), destWidth(0), destHeight(0), defaultVisible(true),
	  direct3D(NULL), direct3DDevice(NULL)
{
	targetWindow = NULL;
	drawUpdateRectangle = false;
	backBufferDirty = true;

	direct3D = NULL;
	direct3DDevice = NULL;
	shouldShow = false;
	vsyncInterval = 16;
	ZeroMemory( &d3dPP, sizeof(d3dPP) );
	ZeroMemory( &dispMode, sizeof(dispMode) );

	TVPEnsureDirect3DObject();
}

/**
 * デストラクタ
 */
DrawDeviceD3D::~DrawDeviceD3D()
{
	detach();
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::DestroyD3DDevice() {
	if(direct3DDevice) direct3DDevice->Release(), direct3DDevice = NULL;
	if(direct3D) direct3D = NULL;
}
//---------------------------------------------------------------------------
bool DrawDeviceD3D::IsTargetWindowActive() const {
	if( targetWindow == NULL ) return false;
	return ::GetForegroundWindow() == targetWindow;
}
//---------------------------------------------------------------------------
UINT DrawDeviceD3D::GetMonitorNumber( HWND window )
{
	if( direct3D == NULL || window == NULL ) return D3DADAPTER_DEFAULT;
	HMONITOR windowMonitor = ::MonitorFromWindow( window, MONITOR_DEFAULTTOPRIMARY );
	UINT iCurrentMonitor = 0;
	UINT numOfMonitor = direct3D->GetAdapterCount();
	for( ; iCurrentMonitor < numOfMonitor; ++iCurrentMonitor ) 	{
		if( direct3D->GetAdapterMonitor(iCurrentMonitor) == windowMonitor )
			break;
	}
	if( iCurrentMonitor == numOfMonitor )
		iCurrentMonitor = D3DADAPTER_DEFAULT;
	return iCurrentMonitor;
}
//---------------------------------------------------------------------------
HRESULT DrawDeviceD3D::DecideD3DPresentParameters() {
	HRESULT			hr;
	UINT iCurrentMonitor = GetMonitorNumber(targetWindow);
	if( FAILED( hr = direct3D->GetAdapterDisplayMode( iCurrentMonitor, &dispMode ) ) )
		return hr;

	ZeroMemory( &d3dPP, sizeof(d3dPP) );
	d3dPP.Windowed = TRUE;
	d3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dPP.BackBufferHeight = dispMode.Height;
	d3dPP.BackBufferWidth = dispMode.Width;
	d3dPP.hDeviceWindow = targetWindow;
	d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::TryRecreateWhenDeviceLost()
{
	bool success = false;
	if( direct3DDevice ) {
		for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
			LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
			if (info) {
				info->free();
			}
		}
		HRESULT hr = direct3DDevice->TestCooperativeLevel();
		if( hr == D3DERR_DEVICENOTRESET ) {
			hr = direct3DDevice->Reset(&d3dPP);
		}
		if( FAILED(hr) ) {
			attach( targetWindow );
			success = direct3DDevice != NULL;
		}
	} else {
		attach( targetWindow );
		success = direct3DDevice != NULL;
	}
	if( success ) {
		if (direct3DDevice) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc( manager, direct3DDevice );
				}
			}
		}
		InvalidateAll();	// 画像の再描画(Layer Update)を要求する
	}
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::InvalidateAll()
{
	// レイヤ演算結果をすべてリクエストする
	// サーフェースが lost した際に内容を再構築する目的で用いる
	RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
}
/**
 * ウインドウの解除
 */
void
DrawDeviceD3D::attach(HWND hWnd)
{
	DestroyD3DDevice();
	this->hWnd = hWnd;
	try {
		// Direct3D デバイス、テクスチャなどを作成する
		HRESULT hr;
		// get Direct3D9 interface
		if( NULL == ( direct3D = TVPGetDirect3DObjectNoAddRef() ) )
			TVPThrowExceptionMessage( TJS_W("Direct3D9 not available") );
		// direct3D->AddRef();

		if( FAILED( hr = DecideD3DPresentParameters() ) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to decide backbuffer format.") );
			}
		}

		UINT iCurrentMonitor = GetMonitorNumber( targetWindow );
		DWORD	BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
		if( D3D_OK != ( hr = direct3D->CreateDevice( iCurrentMonitor, D3DDEVTYPE_HAL, targetWindow, BehaviorFlags, &d3dPP, &direct3DDevice ) ) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to create Direct3D9 Device.") );
			}
		}
		currentMonitor = iCurrentMonitor;
		backBufferDirty = true;

		D3DVIEWPORT9 vp;
		vp.X  = 0;
		vp.Y  = 0;
		vp.Width = d3dPP.BackBufferWidth;
		vp.Height = d3dPP.BackBufferHeight;
		vp.MinZ  = 0.0f;
		vp.MaxZ  = 1.0f;
		if( FAILED(hr = direct3DDevice->SetViewport(&vp)) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to set viewport.") );
			}
		}

		int refreshrate = dispMode.RefreshRate;
		if( refreshrate == 0 ) {
			HDC hdc;
			hdc = ::GetDC(targetWindow);
			refreshrate = GetDeviceCaps( hdc, VREFRESH );
			::ReleaseDC( targetWindow, hdc );
		}
		vsyncInterval = 1000 / refreshrate;
	} catch(...) {
		TVPAddImportantLog(TJS_W("D3DDrawDevice: Failed to create Direct3D devices: unknown reason"));
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
	DestroyD3DDevice();
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
	//manager->SetDesiredLayerType(ltAlpha); // ltAlpha な出力を受け取りたい
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
	targetWindow = wnd;
	isMainWindow = is_main;
	if (wnd != NULL) {
		attach(wnd);
		Window->NotifySrcResize(); // これを呼ぶことで GetSrcSize(), SetDestRectangle() の呼び返しが来る
		// マネージャに対するテクスチャの割り当て
		if (direct3DDevice) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc( manager, direct3DDevice );
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
	
	backBufferDirty = true;
	// 位置だけの変更の場合かどうかをチェックする
	if(dest.get_width() == DestRect.get_width() && dest.get_height() == DestRect.get_height()) {
		// 位置だけの変更だ
		inherited::SetDestRectangle(dest);
	} else {
		// サイズも違う
		inherited::SetDestRectangle(dest);
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
		if (direct3DDevice) {
			info->alloc(manager, direct3DDevice);
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
	if(!targetWindow) return;
	if(!direct3DDevice) return;
	if(!shouldShow) return;

	shouldShow = false;

	// 画面消去
	direct3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0 );
	// 個別レイヤマネージャの描画
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->draw(direct3DDevice, destWidth, destHeight);
		}
	}

	HRESULT hr = D3D_OK;
	RECT client;
	if( ::GetClientRect( targetWindow, &client ) ) {
		RECT drect;
		drect.left   = 0;
		drect.top    = 0;
		drect.right  = client.right - client.left;
		drect.bottom = client.bottom - client.top;

		RECT srect = drect;
		hr = direct3DDevice->Present( &srect, &drect, targetWindow, NULL );
	} else {
		shouldShow = true;
	}

	if(hr == D3DERR_DEVICELOST) {
		TryRecreateWhenDeviceLost();
	} else if(hr != D3D_OK) {
		TVPAddImportantLog( TVPFormatMessage( TJS_W("D3DDrawDevice: (inf) IDirect3DDevice::Present failed/HR=%1"),TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD DrawDeviceD3D::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
{
	if( direct3DDevice == NULL ) return false;

	bool inVsync = false;
	D3DRASTER_STATUS rs;
	if( D3D_OK == direct3DDevice->GetRasterStatus(0,&rs) ) {
		inVsync = rs.InVBlank == TRUE;
	}

	// VSync 待ちを行う
	bool isdelayed = false;
	if(!inVsync) {
		// vblank から抜けるまで待つ
		DWORD timeout_target_tick = ::timeGetTime() + 1;
		rs.InVBlank = FALSE;
		HRESULT hr = D3D_OK;
		do {
			hr = direct3DDevice->GetRasterStatus(0,&rs);
		} while( D3D_OK == hr && rs.InVBlank == TRUE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		// vblank に入るまで待つ
		rs.InVBlank = TRUE;
		do {
			hr = direct3DDevice->GetRasterStatus(0,&rs);
		} while( D3D_OK == hr && rs.InVBlank == FALSE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		if((int)(::timeGetTime() - timeout_target_tick) > 0) {
			// フレームスキップが発生したと考えてよい
			isdelayed  = true;
		}
		inVsync = rs.InVBlank == TRUE;
	}
	*delayed = isdelayed ? 1 : 0;
	*in_vblank = inVsync ? 1 : 0;
	return true;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD DrawDeviceD3D::SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	// フルスクリーン化の処理はなにも行わない、互換性のためにウィンドウを全画面化するのみで処理する
	// Direct3D9 でフルスクリーン化するとフォーカスを失うとデバイスをロストするので、そのたびにリセットor作り直しが必要になる。
	// モーダルウィンドウを使用するシステムでは、これは困るので常にウィンドウモードで行う。
	// モーダルウィンドウを使用しないシステムにするのなら、フルスクリーンを使用するDrawDeviceを作ると良い。
	backBufferDirty = true;
	shouldShow = true;
	//CheckMonitorMoved();
	return true;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD DrawDeviceD3D::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
	backBufferDirty = true;
	shouldShow = true;
	//CheckMonitorMoved();
}
//---------------------------------------------------------------------------



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
	shouldShow = true;
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
