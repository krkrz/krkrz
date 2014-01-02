
#define NOMINMAX

#include "DrawDeviceD3D.h"
#include "LayerManagerInfo.h"
#include <algorithm>

/**
 * コンストラクタ
 */
LayerManagerInfo::LayerManagerInfo(int id, bool visible)
	: id(id), visible(visible), srcWidth(0), srcHeight(0),
	  texture(NULL), textureWidth(0), textureHeight(0),
	  textureBuffer(NULL), texturePitch(0),
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
LayerManagerInfo::alloc(iTVPLayerManager *manager, IDirect3DDevice9 *direct3DDevice)
{
	free();
	tjs_int w, h;
	if (manager->GetPrimaryLayerSize(w, h) && w > 0 && h > 0) {
		HRESULT hr = S_OK;
		D3DCAPS9 d3dcaps;
		direct3DDevice->GetDeviceCaps( &d3dcaps );

		textureWidth = w;
		textureHeight = h;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
			// only square textures are supported
			textureWidth = std::max(textureHeight, textureWidth);
			textureHeight = textureWidth;
		}

		DWORD dwWidth = 64;
		DWORD dwHeight = 64;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 ) {
			// 2の累乗のみ許可するかどうか判定
			while( dwWidth < textureWidth ) dwWidth = dwWidth << 1;
			while( dwHeight < textureHeight ) dwHeight = dwHeight << 1;
			textureWidth = dwWidth;
			textureHeight = dwHeight;

			if( dwWidth > d3dcaps.MaxTextureWidth || dwHeight > d3dcaps.MaxTextureHeight ) {
				TVPAddLog( TJS_W("warning : Image size too large. May be cannot create texture.") );
			}
			TVPAddLog( TJS_W("2の累乗サイズのサーフェイスを使用します。") );
		} else {
			dwWidth = textureWidth;
			dwHeight = textureHeight;
		}

		if( D3D_OK != ( hr = direct3DDevice->CreateTexture( dwWidth, dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL) ) ) {
			TVPThrowExceptionMessage(TJS_W("Cannot allocate D3D texture/HR=%1"),TJSInt32ToHex(hr, 8));
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
	if(textureBuffer && texture) texture->UnlockRect(0), textureBuffer = NULL;
	if(texture) texture->Release(), texture = NULL;
}

/**
 * テクスチャをロックして描画領域情報を取得する
 */
void
LayerManagerInfo::lock()
{
	if (texture) {
		if(textureBuffer) {
			TVPAddImportantLog( TJS_W("D3DDrawDevice: texture has already been locked (StartBitmapCompletion() has been called twice without EndBitmapCompletion()), unlocking the texture.") );
			texture->UnlockRect(0), textureBuffer = NULL;
		}

		D3DLOCKED_RECT rt;
		HRESULT hr = texture->LockRect( 0, &rt, NULL, D3DLOCK_NO_DIRTY_UPDATE );
		if (lastOK = (hr == DD_OK)) {
			textureBuffer = rt.pBits;
			texturePitch = rt.Pitch;
		} else {
			textureBuffer = NULL;
			if (lastOK) {
				// display this message only once since last success
				TVPAddImportantLog(
					TJS_W("D3DDrawDevice: (inf) texture, IDirect3DTexture9::LockRect failed/HR=") +
					TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
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
	}
}

/**
 * テクスチャのロックの解除
 */
void
LayerManagerInfo::unlock()
{
	if (texture) {
		if (textureBuffer) {
			texture->UnlockRect(0), textureBuffer = NULL;
		}
	}
}

/**
 * 画面への描画
 */
void
LayerManagerInfo::draw(IDirect3DDevice9 *direct3DDevice, int destWidth, int destHeight)
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
		hr = direct3DDevice->BeginScene();
		if (SUCCEEDED(hr)) {
			direct3DDevice->SetRenderState(D3DRS_LIGHTING			, FALSE);
			direct3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
			direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE	, TRUE); 
			direct3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			direct3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			direct3DDevice->SetRenderState(D3DRS_CULLMODE			, D3DCULL_NONE);
			
			direct3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			direct3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			direct3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU  , D3DTADDRESS_CLAMP);
			direct3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV  , D3DTADDRESS_CLAMP);
			direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			direct3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			hr = direct3DDevice->SetTexture(0, texture);
			if (SUCCEEDED(hr)) {
				direct3DDevice->SetFVF( D3DFVF_XYZRHW|D3DFVF_TEX1 );
				hr = direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(tVertices) );
				direct3DDevice->SetTexture(0, NULL);
				direct3DDevice->EndScene();
			}
		}
		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY) {
			// texture->Restore();
		} else if(hr == DDERR_INVALIDRECT) {
			// ignore this error
		} else if(hr != D3D_OK) {
			TVPAddImportantLog(
				TJS_W("D3DDrawDevice: (inf) Polygon drawing failed/HR=") +
				TJSInt32ToHex(hr, 8));
		}
	}
}
