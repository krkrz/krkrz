

#include "tjsCommHead.h"
#include "TextureNode.h"


void TextureNode::setSizeToImageSize() {
	width_ = texure_.getWidth();
	height_ = texure_.getHeight();
	updateRealSize();
}

void TextureNode::OnDraw( NodeDrawDevice* dd ) {
	if( texture_.getTexture() == NULL ) return;

	float texture_width = (float)texure_.getTextureWidth();
	float texture_height = (float)texure_.getTextureHeight();

	//- build vertex list
	struct tVertices {
		float x, y, z, rhw;
		float tu, tv;
	};
	tjs_int offsetx = dd->GetOffsetX();
	tjs_int offsety = dd->GetOffsetY();
	const tTVPRect& clip = dd->GetClipRect()

	// 転送先をクリッピング矩形に基づきクリッピング
	float dl = offsetx + real_x_;
	float dt = offsety + real_y_;
	float dr = dl + real_width_;
	float db = dt + real_height_;
	float dw = (float)real_width_;
	float dh = (float)real_height_;

	float tl = (float)( dl < clip.left ? clip.left : dl );
	float tt = (float)( dt < clip.top ? clip.top : dt );
	float tr = (float)( dr > clip.right ? clip.right : dr );
	float tb = (float)( db > clip.bottom ? clip.bottom : db );

	// はみ出している幅を求める
	float cl = tl - (float)dl;
	float ct = tt - (float)dt;
	float cr = (float)dr - tr;
	float cb = (float)dr - tb;

	// はみ出している幅を考慮して、転送元画像をクリッピング
	tjs_int w = texure_.getWidth();
	tjs_int h = texure_.getHeight();
	float sl = (float)(cl * w / dw ) / texture_width;
	float st = (float)(ct * h / dh ) / texture_height;
	float sr = (float)(w - (cr * w / dw) ) / texture_width;
	float sb = (float)(h - (cb * h / dh) ) / texture_height;

	tVertices vertices[] = {
		{dl - 0.5f, dt - 0.5f, 1.0f, 1.0f, sl, st },
		{dr - 0.5f, dt - 0.5f, 1.0f, 1.0f, sr, st },
		{dl - 0.5f, db - 0.5f, 1.0f, 1.0f, sl, sb },
		{dr - 0.5f, db - 0.5f, 1.0f, 1.0f, sr, sb }
	};
	IDirect3DDevice9* device = dd->GetDirect3DDevice();

	device->SetTexture(0, texture_.getTexture() );
	device->SetFVF( D3DFVF_XYZRHW|D3DFVF_TEX1 );
	device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(tVertices) );
	device->SetTexture( 0, NULL);
}

