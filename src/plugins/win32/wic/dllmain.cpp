#include <windows.h>
#include "tp_stub.h"
#include <tchar.h>
#include <string.h>
#include <vector>

#include <wincodec.h>
#include <wincodecsdk.h>
#include <atlbase.h>
#include <comutil.h>
#pragma comment(lib, "WindowsCodecs.lib")
#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif

static tjs_uint32 GetStride( const tjs_uint32 width, const tjs_uint32 bitCount) {
	const tjs_uint32 byteCount = bitCount / 8;
	const tjs_uint32 stride = (width * byteCount + 3) & ~3;
	return stride;
}
// DDS のサポートはWin8.1以降なので、分岐してもいいがとりあえずは実装しない形に
// ICO 書き出しはサポートされない
void TVPLoadGIF(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode);
void TVPLoadICO(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode);
void TVPLoadTIFF(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode);
//void TVPLoadDDS(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode);
void TVPLoadHeaderGIF(void* formatdata, struct IStream* src, iTJSDispatch2** dic );
void TVPLoadHeaderICO(void* formatdata, struct IStream* src, iTJSDispatch2** dic );
void TVPLoadHeaderTIFF(void* formatdata, struct IStream* src, iTJSDispatch2** dic );
//void TVPLoadHeaderDDS(void* formatdata, struct IStream* src, iTJSDispatch2** dic );
void TVPSaveAsGIF(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta );
//void TVPSaveAsICO(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta );
void TVPSaveAsTIFF(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta );
void TVPSaveAsDDS(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta );
bool TVPAcceptSaveAsGIF(void* formatdata, const ttstr & type, iTJSDispatch2** dic );
//bool TVPAcceptSaveAsICO(void* formatdata, const ttstr & type, iTJSDispatch2** dic );
bool TVPAcceptSaveAsTIFF(void* formatdata, const ttstr & type, iTJSDispatch2** dic );
//bool TVPAcceptSaveAsDDS(void* formatdata, const ttstr & type, iTJSDispatch2** dic );

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	CoInitialize(NULL);
	TVPRegisterGraphicLoadingHandler( ttstr(TJS_W(".gif")), TVPLoadGIF, TVPLoadHeaderGIF, TVPSaveAsGIF, TVPAcceptSaveAsGIF, NULL );
	TVPRegisterGraphicLoadingHandler( ttstr(TJS_W(".ico")), TVPLoadICO, TVPLoadHeaderICO, NULL, NULL, NULL );
	TVPRegisterGraphicLoadingHandler( ttstr(TJS_W(".tiff")), TVPLoadTIFF, TVPLoadHeaderTIFF, TVPSaveAsTIFF, TVPAcceptSaveAsTIFF, NULL );
	
	// later win8.1
	// TVPRegisterGraphicLoadingHandler( ttstr(TJS_W(".dds")), TVPLoadDDS, TVPLoadHeaderDDS, TVPSaveAsDDS, TVPAcceptSaveAsDDS, NULL );
	
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	return S_OK;
}
extern "C" HRESULT _stdcall V2Unlink() {
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
	
	//TVPUnregisterGraphicLoadingHandler( ttstr(TJS_W(".gif")), TVPLoadGIF, TVPLoadHeaderGIF, TVPSaveAsGIF, TVPAcceptSaveAsGIF, NULL );
	TVPUnregisterGraphicLoadingHandler( ttstr(TJS_W(".gif")), TVPLoadGIF, TVPLoadHeaderGIF, NULL, NULL, NULL );
	TVPUnregisterGraphicLoadingHandler( ttstr(TJS_W(".ico")), TVPLoadICO, TVPLoadHeaderICO, NULL, NULL, NULL );
	TVPUnregisterGraphicLoadingHandler( ttstr(TJS_W(".tiff")), TVPLoadTIFF, TVPLoadHeaderTIFF, TVPSaveAsTIFF, TVPAcceptSaveAsTIFF, NULL );
	CoUninitialize();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();
	return S_OK;
}

bool TVPAcceptSaveAsGIF(void* formatdata, const ttstr & type, class iTJSDispatch2** dic ) {
	bool result = false;
	if( type.StartsWith(TJS_W("gif")) ) result = true;
	else if( type == TJS_W(".gif") ) result = true;
	if( result && dic ) {
		tTJSVariant result;
		TVPExecuteExpression(
			TJS_W("(const)%[")
			TJS_W("\"alpha\"=>(const)%[\"type\"=>\"boolean\",\"desc\"=>\"alpha channel\",\"default\"=>true]")
			TJS_W("]"),
			NULL, &result );
		if( result.Type() == tvtObject ) {
			*dic = result.AsObject();
		}
		// *dic = TJSCreateDictionaryObject();
	}
	return result;
}

bool TVPAcceptSaveAsTIFF(void* formatdata, const ttstr & type, class iTJSDispatch2** dic ) {
	bool result = false;
	if( type.StartsWith(TJS_W("tiff")) ) result = true;
	else if( type == TJS_W(".tiff") ) result = true;
	if( result && dic ) {
#if 0
		tTJSVariant result;
		TVPExecuteExpression(
			TJS_W("(const)%[")
			TJS_W("\"alpha\"=>(const)%[\"type\"=>\"boolean\",\"desc\"=>\"alpha channel\",\"default\"=>true]")
			TJS_W("]"),
			NULL, &result );
		if( result.Type() == tvtObject ) {
			*dic = result.AsObject();
		}
#endif
		*dic = TJSCreateDictionaryObject();
	}
	return result;
}
void TVPLoadWIC(const GUID& guid, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode) {
	if( mode == glmPalettized || mode == glmGrayscale ) {
		TVPThrowExceptionMessage( TJS_W("Unsupported color mode.") );
	}

	CComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = decoder.CoCreateInstance(guid);
	hr = decoder->Initialize( src, WICDecodeMetadataCacheOnDemand);
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
		tjs_uint8* buff = new tjs_uint8[stride*height];
		try {
			sizecallback(callbackdata, width, height);
			WICRect rect = {0, 0, width, height};
			if( !IsEqualGUID( pixelFormat, GUID_WICPixelFormat32bppBGRA) ) {
				CComPtr<IWICFormatConverter> converter;
				CComPtr<IWICImagingFactory> wicFactory;
				hr = wicFactory.CoCreateInstance( CLSID_WICImagingFactory );
				wicFactory->CreateFormatConverter(&converter);
				converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
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
		} catch(...) {
			delete[] buff;
		}
		delete[] buff;
		break;
	}
}

void TVPLoadGIF(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode) {
	TVPLoadWIC( CLSID_WICGifDecoder, callbackdata, sizecallback, scanlinecallback, metainfopushcallback, src, keyidx, mode );
}
void TVPLoadICO(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode) {
	TVPLoadWIC( CLSID_WICIcoDecoder, callbackdata, sizecallback, scanlinecallback, metainfopushcallback, src, keyidx, mode );
}
void TVPLoadTIFF(void* formatdata, void *callbackdata, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTVPMetaInfoPushCallback metainfopushcallback, IStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode) {
	TVPLoadWIC( CLSID_WICTiffDecoder, callbackdata, sizecallback, scanlinecallback, metainfopushcallback, src, keyidx, mode );
}

void TVPLoadHeaderWIC(const GUID& guid, struct IStream* stream, iTJSDispatch2** dic ) {
	CComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = decoder.CoCreateInstance(guid);
	if( SUCCEEDED(hr) ) hr = decoder->Initialize( stream, WICDecodeMetadataCacheOnDemand);
	UINT frameCount = 0;
	if( SUCCEEDED(hr) ) hr = decoder->GetFrameCount(&frameCount);
	if( SUCCEEDED(hr) ) {
		for( UINT index = 0; index < frameCount; ++index ) {
			UINT width = 0;
			UINT height = 0;
			//GUID pixelFormat = { 0 };
			CComPtr<IWICBitmapFrameDecode> frame;
			if( SUCCEEDED(hr) ) hr = decoder->GetFrame(index, &frame);
			//if( SUCCEEDED(hr) ) hr = frame->GetPixelFormat(&pixelFormat);
			if( SUCCEEDED(hr) ) hr = frame->GetSize(&width, &height);
			if( SUCCEEDED(hr) ) {
				*dic = TJSCreateDictionaryObject();
				tTJSVariant val((tjs_int64)width);
				(*dic)->PropSet(TJS_MEMBERENSURE, TJS_W("width"), 0, &val, (*dic) );
				val = tTJSVariant((tjs_int64)height);
				(*dic)->PropSet(TJS_MEMBERENSURE, TJS_W("height"), 0, &val, (*dic) );
				if( frameCount > 0 ) {
					val = tTJSVariant((tjs_int64)frameCount);
					(*dic)->PropSet(TJS_MEMBERENSURE, TJS_W("frames"), 0, &val, (*dic) );
				}
			}
			break;
		}
	}
}
void TVPLoadHeaderGIF(void* formatdata, struct IStream* src, iTJSDispatch2** dic ) {
	TVPLoadHeaderWIC( CLSID_WICGifDecoder, src, dic );
}
void TVPLoadHeaderICO(void* formatdata, struct IStream* src, iTJSDispatch2** dic ) {
	TVPLoadHeaderWIC( CLSID_WICIcoDecoder, src, dic );
}
void TVPLoadHeaderTIFF(void* formatdata, struct IStream* src, iTJSDispatch2** dic ) {
	TVPLoadHeaderWIC( CLSID_WICTiffDecoder, src, dic );
}
void TVPSaveAsWIC(const GUID& guid, void* callbackdata, IStream* stream, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta ) {
	CComPtr<IWICBitmapEncoder> encoder;
	HRESULT hr = encoder.CoCreateInstance(guid);
	if( SUCCEEDED(hr) ) hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
	CComPtr<IPropertyBag2> property;
	CComPtr<IWICBitmapFrameEncode> frame;
	if( SUCCEEDED(hr) ) hr = encoder->CreateNewFrame( &frame, &property );
	WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
	if( SUCCEEDED(hr) ) hr = frame->Initialize( property );
	if( SUCCEEDED(hr) ) hr = frame->SetSize( width, height );
	if( SUCCEEDED(hr) ) hr = frame->SetPixelFormat(&format);
	const UINT stride = width * sizeof(tjs_uint32);
	const UINT buffersize = stride * height;
	if( SUCCEEDED(hr) ) {
		tjs_uint8* buff = new tjs_uint8[buffersize];
		try {
			for( UINT i = 0; i < height; i++ ) {
				memcpy( &buff[i*stride], scanlinecallback(callbackdata,i), stride );
			}
			hr = frame->WritePixels( height, stride, buffersize, &buff[0] ); 
		} catch(...) {
			delete[] buff;
			throw;
		}
		delete[] buff;
	}
	if( SUCCEEDED(hr) ) hr = frame->Commit();
	if( SUCCEEDED(hr) ) hr = encoder->Commit();
}
void TVPSaveAsGIF(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta ) {
	// この方法ではうまく書き出せない様子
	TVPSaveAsWIC( CLSID_WICGifEncoder, callbackdata, dst, mode, width, height, scanlinecallback, meta );
}
void TVPSaveAsTIFF(void* formatdata, void* callbackdata, IStream* dst, const ttstr & mode, tjs_uint width, tjs_uint height, tTVPGraphicSaveScanLineCallback scanlinecallback, iTJSDispatch2* meta ) {
	TVPSaveAsWIC( CLSID_WICTiffEncoder, callbackdata, dst, mode, width, height, scanlinecallback, meta );
}
