#pragma comment(lib, "vfw32.lib")
#include "DrawDeviceD3D.h"
#include "LayerManagerInfo.h"

/**
 * コンストラクタ
 */
LayerManagerInfo::LayerManagerInfo(int id, bool visible)
	: id(id), visible(visible), srcWidth(0), srcHeight(0),
	  texture(NULL), textureWidth(0), textureHeight(0),
	  useDirectTransfer(false),
	  textureBuffer(NULL), texturePitch(0),
	  offScreenDC(NULL), drawDibHandle(NULL),
	  lastOK(true)
{
};

/**
 * デストラクタ
 */
LayerManagerInfo::~LayerManagerInfo()
{
	free();
}

/**
 * テクスチャ割り当て処理
 */
void
LayerManagerInfo::alloc(iTVPLayerManager *manager, IDirectDraw7 *directDraw, IDirect3DDevice7 *direct3DDevice)
{
	free();
	tjs_int w, h;
	if (manager->GetPrimaryLayerSize(w, h) && w > 0 && h > 0) {

		// retrieve device caps
		D3DDEVICEDESC7 caps;
		ZeroMemory(&caps, sizeof(caps));
		HRESULT hr;
		if (FAILED(hr = direct3DDevice->GetCaps(&caps))) {
			TVPThrowExceptionMessage(TJS_W("Failed to retrieve Direct3D device caps/HR=%1"),
									 TJSInt32ToHex(hr, 8));
		}

		// decide texture size
		textureWidth = w;
		textureHeight = h;
		if (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
			// only square textures are supported
			textureWidth = textureHeight > textureWidth ? textureHeight : textureWidth;
			textureHeight = textureWidth;
		}
			
		if (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) {
			// power of 2 size of texture dimentions are required
			tjs_uint sz;
			
			sz = 1; while(sz < textureWidth) sz <<= 1;
			textureWidth = sz;
			
			sz = 1; while(sz < textureHeight) sz <<= 1;
			textureHeight = sz;
		}
			
		if(caps.dwMinTextureWidth  > textureWidth) textureWidth = caps.dwMinTextureWidth;
		if(caps.dwMinTextureHeight > textureHeight) textureHeight = caps.dwMinTextureHeight;
		if(	caps.dwMaxTextureWidth  < textureWidth ||
			caps.dwMaxTextureHeight < textureHeight) {
			TVPThrowExceptionMessage(TJS_W("Could not allocate texture size of %1x%2"),
									 ttstr((int)textureWidth), ttstr((int)textureHeight));
		}

		// create Direct3D texture
		DDSURFACEDESC2 ddsd;
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		ddsd.dwWidth  = textureWidth;
		ddsd.dwHeight = textureHeight;
		ddsd.ddsCaps.dwCaps =
			/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_LOCALVIDMEM;
			
		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
		ddsd.ddpfPixelFormat.dwRGBBitCount	= 32;
		ddsd.ddpfPixelFormat.dwRBitMask		= 0x00FF0000;
		ddsd.ddpfPixelFormat.dwGBitMask		= 0x0000FF00;
		ddsd.ddpfPixelFormat.dwBBitMask		= 0x000000FF;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask		= 0xFF000000;
			
		hr = directDraw->CreateSurface(&ddsd, &texture, NULL);
		
		if (hr == DD_OK) {
			useDirectTransfer = true; // 直接のメモリ転送を有効にする
		} else /*if(hr != DD_OK) */	{
			// ピクセルフォーマットを指定せずに生成を試みる
			
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS/* | DDSD_PIXELFORMAT*/;
			ddsd.dwWidth = w;
			ddsd.dwHeight = h;
			ddsd.ddsCaps.dwCaps =
				/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_LOCALVIDMEM;
			
			hr = directDraw->CreateSurface(&ddsd, &texture, NULL);
			
			if(FAILED(hr)) {
				TVPThrowExceptionMessage(TJS_W("Cannot allocate D3D texture/HR=%1"),
										 TJSInt32ToHex(hr, 8));
			}
			
			TVPAddImportantLog("Passthrough: Using non 32bit ARGB texture format");
		}
		manager->RequestInvalidation(tTVPRect(0,0,w,h));
		srcWidth = w;
		srcHeight = h;
	}
}

/*
 * テクスチャ解放
 */
void
LayerManagerInfo::free()
{
	if(textureBuffer && texture) texture->Unlock(NULL), textureBuffer = NULL;
	if(offScreenDC && texture) texture->ReleaseDC(offScreenDC), offScreenDC = NULL;
	if(texture) texture->Release(), texture = NULL;
	if(drawDibHandle) DrawDibClose(drawDibHandle), drawDibHandle = NULL;
}

/**
 * テクスチャをロックして描画領域情報を取得する
 */
void
LayerManagerInfo::lock()
{
	if (texture) {
		if (useDirectTransfer) {
			if (textureBuffer) {
				TVPAddImportantLog(TJS_W("Passthrough: texture has already been locked (StartBitmapCompletion() has been called twice without EndBitmapCompletion()), unlocking the texture."));
				texture->Unlock(NULL), textureBuffer = NULL;
			}
			DDSURFACEDESC2 ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			HRESULT hr = texture->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL);
			if (lastOK = (hr == DD_OK)) {
				textureBuffer = ddsd.lpSurface;
				texturePitch = ddsd.lPitch;
			} else {
				textureBuffer = NULL;
				if (lastOK) {
					// display this message only once since last success
					TVPAddImportantLog(
						TJS_W("Passthrough: (inf) texture, IDirectDrawSurface::Lock failed/HR=") +
						TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
				}
			}
		} else {
			HDC dc = NULL;
			HRESULT hr = texture->GetDC(&dc);
			if (lastOK = (hr == DD_OK)) {
				offScreenDC = dc;
			} else {
				offScreenDC = NULL;
				if (lastOK) {
					// display this message only once since last success
					TVPAddImportantLog(
						TJS_W("Passthrough: (inf) texture, IDirectDrawSurface::GetDC failed/HR=") +
						TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
				}
			}
		}
	}
}

/**
 * ロックされたテクスチャにビットマップ描画を行う
 */
void
LayerManagerInfo::copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
					   const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo で表されるビットマップの cliprect の領域を、x, y に描画する。
	if (texture &&
		// 範囲外の転送は(一部だけ転送するのではなくて)無視してよい
		!(x < 0 || y < 0 ||
		  x + cliprect.get_width() > srcWidth ||
		  y + cliprect.get_height() > srcHeight) &&
		!(cliprect.left < 0 || cliprect.top < 0 ||
		  cliprect.right > bitmapinfo->bmiHeader.biWidth ||
		  cliprect.bottom > bitmapinfo->bmiHeader.biHeight)) {

		if (useDirectTransfer) {
			// 直接メモリ転送を用いて描画を行う
			if (textureBuffer) {

				long src_y       = cliprect.top;
				long src_y_limit = cliprect.bottom;
				long src_x       = cliprect.left;
				long width_bytes   = cliprect.get_width() * 4; // 32bit
				long dest_y      = y;
				long dest_x      = x;
				const tjs_uint8 * src_p = (const tjs_uint8 *)bits;
				long src_pitch;
				
				if (bitmapinfo->bmiHeader.biHeight < 0) {
					// bottom-down
					src_pitch = bitmapinfo->bmiHeader.biWidth * 4;
				} else {
					// bottom-up
					src_pitch = -bitmapinfo->bmiHeader.biWidth * 4;
					src_p += bitmapinfo->bmiHeader.biWidth * 4 * (bitmapinfo->bmiHeader.biHeight - 1);
				}
				
				for(; src_y < src_y_limit; src_y ++, dest_y ++) {
					const void *srcp = src_p + src_pitch * src_y + src_x * 4;
					void *destp = (tjs_uint8*)textureBuffer + texturePitch * dest_y + dest_x * 4;
					memcpy(destp, srcp, width_bytes);
				}
			}
		} else {
			// DrawDibDraw にて offScreenDC に描画を行う
			if (offScreenDC) {
				if (!drawDibHandle) {
					drawDibHandle = DrawDibOpen();
				}
				if (drawDibHandle) {
					DrawDibDraw(drawDibHandle,
								offScreenDC,
								x,
								y,
								cliprect.get_width(),
								cliprect.get_height(),
								const_cast<BITMAPINFOHEADER*>(reinterpret_cast<const BITMAPINFOHEADER*>(bitmapinfo)),
								const_cast<void*>(bits),
								cliprect.left,
								cliprect.top,
								cliprect.get_width(),
								cliprect.get_height(),
								0);
				}
			}
		}
	}
}

/**
 * テクスチャのロックの解除
 */
void
LayerManagerInfo::unlock()
{
	if (texture) {
		if (useDirectTransfer) {
			if (textureBuffer) {
				texture->Unlock(NULL), textureBuffer = NULL;
			}
		} else {
			if (offScreenDC) {
				texture->ReleaseDC(offScreenDC), offScreenDC = NULL;
			}
		}
	}
}

/**
 * 画面への描画
 */
void
LayerManagerInfo::draw(IDirect3DDevice7 * direct3DDevice, int destWidth, int destHeight)
{
	// Blt texture to surface

	if (texture && visible) {

		//- build vertex list
		struct tVertices {
			float x, y, z, rhw;
			float tu, tv;
		};
		
		float dw = (float)destWidth;
		float dh = (float)destHeight;
		
		float sw = (float)srcWidth  / (float)textureWidth;
		float sh = (float)srcHeight / (float)textureHeight;
		
		tVertices vertices[] = {
			{0.0f - 0.5f, 0.0f - 0.5f, 1.0f, 1.0f, 0.0f, 0.0f},
			{dw   - 0.5f, 0.0f - 0.5f, 1.0f, 1.0f, sw  , 0.0f},
			{0.0f - 0.5f, dh   - 0.5f, 1.0f, 1.0f, 0.0f, sh  },
			{dw   - 0.5f, dh   - 0.5f, 1.0f, 1.0f, sw  , sh  }
		};

		HRESULT hr;
		
		//- draw as triangles
		hr = direct3DDevice->SetTexture(0, texture);
		if (SUCCEEDED(hr)) {
			direct3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING			, FALSE);
			//	direct3DDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE		, FALSE);
			direct3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE	, TRUE); 
			direct3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
			direct3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
			direct3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE			, D3DCULL_NONE);
			
			direct3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
			direct3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
			direct3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
			direct3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS  , D3DTADDRESS_CLAMP);
			hr = direct3DDevice->BeginScene();
			if (SUCCEEDED(hr)) {
				hr = direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_XYZRHW | D3DFVF_TEX1,
												   vertices, 4, D3DDP_WAIT);
				direct3DDevice->EndScene();
				direct3DDevice->SetTexture(0, NULL);
			}
		}
		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY) {
			texture->Restore();
		} else if(hr == DDERR_INVALIDRECT) {
			// ignore this error
		} else if(hr != D3D_OK) {
			TVPAddImportantLog(
				TJS_W("Passthrough: (inf) Polygon drawing failed/HR=") +
				TJSInt32ToHex(hr, 8));
		}
	}
}
