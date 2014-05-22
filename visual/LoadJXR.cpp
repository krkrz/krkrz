#include "tjsCommHead.h"

#include "GraphicsLoaderIntf.h"
#include "LayerBitmapIntf.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "tvpgl.h"

#include "DebugIntf.h"

static tjs_uint32 GetStride( const tjs_uint32 width, const tjs_uint32 bitCount) {
	const tjs_uint32 byteCount = bitCount / 8;
	const tjs_uint32 stride = (width * byteCount + 3) & ~3;
	return stride;
}

#if 0
//#ifdef WIN32
// Windows 組み込み機能で JPEG XR を開く場合はこちら
#include <wincodec.h>
#include <wincodecsdk.h>
#include <atlbase.h>
#include "StorageImpl.h"
#pragma comment(lib, "WindowsCodecs.lib")

void TVPLoadJXR(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	tTVPIStreamAdapter* s = new tTVPIStreamAdapter( src );
	CComPtr<tTVPIStreamAdapter> stream( s );
	s->Release();
	{
		CComPtr<IWICBitmapDecoder> decoder;
		HRESULT hr = decoder.CoCreateInstance(CLSID_WICWmpDecoder);
		hr = decoder->Initialize( stream, WICDecodeMetadataCacheOnDemand);
		UINT frameCount = 0;
		hr = decoder->GetFrameCount(&frameCount);
		for( UINT index = 0; index < frameCount; ++index ) {
			CComPtr<IWICBitmapFrameDecode> frame;
			hr = decoder->GetFrame(index, &frame);
			UINT width = 0;
			UINT height = 0;
			GUID pixelFormat = { 0 };
			hr = frame->GetPixelFormat(&pixelFormat);
			hr = frame->GetSize(&width, &height);
			const UINT stride = GetStride(width, 32);		
#ifdef _DEBUG
			std::vector<tjs_uint8> buff(stride*height*sizeof(tjs_uint8));
#else
			std::vector<tjs_uint8> buff;
			buff.reserve(stride*height*sizeof(tjs_uint8));
#endif
			sizecallback(callbackdata, width, height);
			WICRect rect = {0, 0, width, height};
			if( !IsEqualGUID( pixelFormat, GUID_WICPixelFormat32bppPBGRA) ) {
				CComPtr<IWICFormatConverter> converter;
				CComPtr<IWICImagingFactory> wicFactory;
				hr = wicFactory.CoCreateInstance( CLSID_WICImagingFactory );
				wicFactory->CreateFormatConverter(&converter);
				converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
				hr = converter->CopyPixels( &rect, stride, stride*height, (BYTE*)&buff[0] );
			} else {
				hr = frame->CopyPixels( &rect, stride, stride*height, (BYTE*)&buff[0] );
			}
			int offset = 0;
			for( UINT i = 0; i < height; i++) {
				void *scanline = scanlinecallback(callbackdata, i);
				memcpy( scanline, &buff[offset], width*sizeof(tjs_uint32));
				offset += stride;
				scanlinecallback(callbackdata, -1);
			} 
			break;
		}
	}
	if( stream ) stream->ClearStream();
}

#else

#include "JXRGlue.h"


static ERR JXR_close( WMPStream** ppWS ) {
	/* 何もしない */
	return WMP_errSuccess;
}
static Bool JXR_EOS(struct WMPStream* pWS) {
	tTJSBinaryStream* src = ((tTJSBinaryStream*)(pWS->state.pvObj));
	return src->GetPosition() >= src->GetSize();
}
static ERR JXR_read(struct WMPStream* pWS, void* pv, size_t cb) {
	tTJSBinaryStream* src = ((tTJSBinaryStream*)(pWS->state.pvObj));
	tjs_uint size = src->Read( pv, cb );
	return size == cb ? WMP_errSuccess : WMP_errFileIO;
}
static ERR JXR_write(struct WMPStream* pWS, const void* pv, size_t cb) {
	ERR err = WMP_errSuccess;
	if( 0 != cb ) {
		tTJSBinaryStream* src = ((tTJSBinaryStream*)(pWS->state.pvObj));
		tjs_uint size = src->Write( pv, cb );
		err = size == cb ? WMP_errSuccess : WMP_errFileIO;
	}
	return err;
}
static ERR JXR_set_pos( struct WMPStream* pWS, size_t offPos ) {
	tTJSBinaryStream* src = ((tTJSBinaryStream*)(pWS->state.pvObj));
	tjs_uint64 pos = src->Seek(  offPos, TJS_BS_SEEK_SET );
	return pos == offPos ? WMP_errSuccess : WMP_errFileIO;
}
static ERR JXR_get_pos( struct WMPStream* pWS, size_t* poffPos ) {
	tTJSBinaryStream* src = ((tTJSBinaryStream*)(pWS->state.pvObj));
	*poffPos = (size_t)src->GetPosition();
	return WMP_errSuccess;
}


#define SAFE_CALL( func ) if( Failed(err = (func)) ) { TVPThrowExceptionMessage( TJS_W("JPEG XR read error/%1"), err ); }
//---------------------------------------------------------------------------
void TVPLoadJXR(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int keyidx,  tTVPGraphicLoadMode mode)
{
	if( glmNormal != mode ) {
		// not supprted yet.
		TVPThrowExceptionMessage( TJS_W("JPEG XR read error/%1"), TJS_W("not supprted yet.") );
	}
	
	//PKFactory* pFactory = NULL;
	PKImageDecode* pDecoder = NULL;
	//PKFormatConverter* pConverter = NULL;
	WMPStream* pStream = NULL;
	//PKCodecFactory* pCodecFactory = NULL;
	try {
		ERR err;
		//SAFE_CALL( PKCreateFactory(&pFactory, PK_SDK_VERSION) );
		//SAFE_CALL( PKCreateCodecFactory(&pCodecFactory, WMP_SDK_VERSION) );
		//SAFE_CALL(pCodecFactory->CreateDecoderFromFile("test.jxr", &pDecoder));

		const PKIID* pIID = NULL;
		SAFE_CALL( GetImageDecodeIID(".jxr", &pIID) );
		// create stream
		pStream = (WMPStream*)calloc(1, sizeof(WMPStream));
		pStream->state.pvObj = (void*)src;
		pStream->Close = JXR_close;
		pStream->EOS = JXR_EOS;
		pStream->Read = JXR_read;
		pStream->Write = JXR_write;
		pStream->SetPos = JXR_set_pos;
		pStream->GetPos = JXR_get_pos;
		// Create decoder
		SAFE_CALL( PKCodecFactory_CreateCodec(pIID, (void **)&pDecoder) );
		// attach stream to decoder
		SAFE_CALL( pDecoder->Initialize(pDecoder, pStream) );
		pDecoder->fStreamOwner = !0;

		PKPixelFormatGUID srcFormat;
		pDecoder->GetPixelFormat( pDecoder, &srcFormat );
		PKPixelInfo PI;
		PI.pGUIDPixFmt = &srcFormat;
		PixelFormatLookup(&PI, LOOKUP_FORWARD);

		pDecoder->WMP.wmiSCP.bfBitstreamFormat = SPATIAL;
        if(!!(PI.grBit & PK_pixfmtHasAlpha))
            pDecoder->WMP.wmiSCP.uAlphaMode = 2;
        else
            pDecoder->WMP.wmiSCP.uAlphaMode = 0;
		pDecoder->WMP.wmiSCP.sbSubband = SB_ALL;
		pDecoder->WMP.bIgnoreOverlap = FALSE;
		pDecoder->WMP.wmiI.cfColorFormat = PI.cfColorFormat;
		pDecoder->WMP.wmiI.bdBitDepth = PI.bdBitDepth;
		pDecoder->WMP.wmiI.cBitsPerUnit = PI.cbitUnit;
		pDecoder->WMP.wmiI.cThumbnailWidth = pDecoder->WMP.wmiI.cWidth;
		pDecoder->WMP.wmiI.cThumbnailHeight = pDecoder->WMP.wmiI.cHeight;
		pDecoder->WMP.wmiI.bSkipFlexbits = FALSE;
		pDecoder->WMP.wmiI.cROILeftX = 0;
		pDecoder->WMP.wmiI.cROITopY = 0;
		pDecoder->WMP.wmiI.cROIWidth = pDecoder->WMP.wmiI.cThumbnailWidth;
		pDecoder->WMP.wmiI.cROIHeight = pDecoder->WMP.wmiI.cThumbnailHeight;
		pDecoder->WMP.wmiI.oOrientation = O_NONE;
		pDecoder->WMP.wmiI.cPostProcStrength = 0;
		pDecoder->WMP.wmiSCP.bVerbose = FALSE;

		int width = 0;
		int height = 0;
		pDecoder->GetSize( pDecoder, &width, &height );
		if( width == 0 || height == 0 ) {
			TVPThrowExceptionMessage( TJS_W("JPEG XR read error/%1"), TJS_W("width or height is zero.") );
		}
		sizecallback(callbackdata, width, height);
		const tjs_uint32 stride = GetStride( (tjs_uint32)width, (tjs_uint32)32 );
		PKRect rect = {0, 0, width, height};
#ifdef _DEBUG
		std::vector<tjs_uint8> buff(stride*height*sizeof(tjs_uint8));
#else
		std::vector<tjs_uint8> buff;
		buff.reserve(stride*height*sizeof(tjs_uint8));
#endif
		// rect で1ラインずつ指定してデコードする方法はjxrlibではうまくいかない様子
		int offset = 0;
		if( !IsEqualGUID( srcFormat, GUID_PKPixelFormat32bppPBGRA ) ) {
			if( IsEqualGUID( srcFormat, GUID_PKPixelFormat24bppRGB ) ) {
				pDecoder->Copy( pDecoder, &rect, (U8*)&buff[0], stride );
				for( int i = 0; i < height; i++) {
					void *scanline = scanlinecallback(callbackdata, i);
					tjs_uint8* d = (tjs_uint8*)scanline;
					tjs_uint8* s = (tjs_uint8*)&buff[offset];
					for( int x = 0; x < width; x++ ) {
						d[0] = s[2];
						d[1] = s[1];
						d[2] = s[0];
						d[3] = 0xff;
						d+=4;
						s+=3;
					}
					offset += stride;
					scanlinecallback(callbackdata, -1);
				}
			} else {
				TVPThrowExceptionMessage( TJS_W("JPEG XR read error/%1"), TJS_W("Not supported this file format yet.") );
			}
			/*
			Converter はどうもおかしいので使わない。
			SAFE_CALL( pCodecFactory->CreateFormatConverter(&pConverter) );
			SAFE_CALL( pConverter->Initialize(pConverter, pDecoder, NULL, GUID_PKPixelFormat32bppPBGRA) );
			pConverter->Copy( pConverter, &rect, (U8*)&buff[0], width*sizeof(int));
			*/
		} else {
			// アルファチャンネルが入っている時メモリリークしている(jxrlib直した)
			pDecoder->Copy( pDecoder, &rect, (U8*)&buff[0], stride );
			for( int i = 0; i < height; i++) {
				void *scanline = scanlinecallback(callbackdata, i);
				memcpy( scanline, &buff[offset], width*sizeof(tjs_uint32));
				offset += stride;
				scanlinecallback(callbackdata, -1);
			} 
		}
		//if( pConverter ) pConverter->Release(&pConverter);
		if( pDecoder ) pDecoder->Release(&pDecoder);
		//if( pCodecFactory ) pCodecFactory->Release(&pCodecFactory);
		if( pStream ) free( pStream );
	} catch(...) {
		//if( pConverter ) pConverter->Release(&pConverter);
		if( pDecoder ) pDecoder->Release(&pDecoder);
		//if( pCodecFactory ) pCodecFactory->Release(&pCodecFactory);
		if( pStream ) free( pStream );
		throw;
	}
}
#endif
