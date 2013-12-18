//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// BMP での画像保存
//---------------------------------------------------------------------------
void SaveAsBMP(const ttstr & name, tjs_int width, tjs_int height,
	const tjs_uint8 * bufferptr, tjs_int bufferpitch)
{
	// BMP で画像を保存する

	// ストリームを開く
	IStream * out = TVPCreateIStream(name, TJS_BS_WRITE);
	if(!out)
		TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) +
			ttstr(name)).c_str());
	tjs_uint8 * buf = NULL;

	try
	{
		// 吉里吉里内部の画像キャッシュをクリア
		// これを行わないと、画像をファイルに書き込んでも
		// 吉里吉里内部のキャッシュを参照してしまう
		TVPClearGraphicCache();


		// ヘッダを準備
		tjs_uint pitch = width*4;
		tjs_uint srcpitch = bufferpitch < 0 ? -bufferpitch : bufferpitch;

		BITMAPFILEHEADER bfh;
		bfh.bfType = 'B' + ('M' << 8);
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			pitch * height;
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		unsigned long written;
		out->Write(&bfh, sizeof(bfh), &written);
		if(written != sizeof(bfh))
			TVPThrowExceptionMessage((ttstr(TJS_W("write failed : ")) +
				ttstr(name)).c_str());

		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(bih);
		bih.biWidth = width;
		bih.biHeight = height;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		out->Write(&bih, sizeof(bih), &written);
		if(written != sizeof(bih))
			TVPThrowExceptionMessage((ttstr(TJS_W("write failed : ")) +
				ttstr(name)).c_str());


		// ビットマップを出力
		buf = new tjs_uint8 [pitch];
		bufferptr += bufferpitch * (height-1);

		for(tjs_int y = 0; y < height; y++)
		{
			memcpy(buf, bufferptr, srcpitch);
			out->Write(buf, pitch, &written);
			if(written != pitch)
				TVPThrowExceptionMessage((ttstr(TJS_W("write failed : ")) +
					ttstr(name)).c_str());
			bufferptr -= bufferpitch; // 上下を逆に保存するので
		}

	}
	catch(...)
	{
		out->Release();
		if(buf) delete [] buf;
		throw;
	}

	out->Release();
	if(buf) delete [] buf;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// レイヤの画像を保存するための関数 ( TJS 関数 )
//---------------------------------------------------------------------------
class tSaveLayerImageFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(membername) return TJS_E_MEMBERNOTFOUND;

		// 引数 : レイヤ, ファイル名, 画像形式 [, 画像固有オプション ...]
		if(numparams < 3) return TJS_E_BADPARAMCOUNT;

		const tjs_uint8 * bufferptr;
		tjs_int bufferpitch;
		tjs_int width;
		tjs_int height;

		// 画像のサイズ、ピクセルバッファへのポインタ、ピッチを得る
		iTJSDispatch2 * layerobj = param[0]->AsObjectNoAddRef();

		tTJSVariant val;
		if(TJS_FAILED(layerobj->PropGet(0, TJS_W("imageWidth"), NULL, &val, layerobj)))
			TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageWidth failed."));
		width = val;

		if(TJS_FAILED(layerobj->PropGet(0, TJS_W("imageHeight"), NULL, &val, layerobj)))
			TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageHeight failed."));
		height = val;

		if(TJS_FAILED(layerobj->PropGet(0, TJS_W("mainImageBuffer"), NULL, &val, layerobj)))
			TVPThrowExceptionMessage(TJS_W("invoking of Layer.mainImageBuffer failed."));
		bufferptr = (const tjs_uint8 *)(tjs_int)val;

		if(TJS_FAILED(layerobj->PropGet(0, TJS_W("mainImageBufferPitch"), NULL, &val, layerobj)))
			TVPThrowExceptionMessage(TJS_W("invoking of Layer.mainImageBufferPitch failed."));
		bufferpitch = val;

		// 画像形式をチェック
		ttstr format = *param[2];
		if(format == TJS_W("bmp"))
		{
			// BMP で保存
			SaveAsBMP(*param[1], width, height, bufferptr, bufferpitch);
		}
		else
		{
			TVPThrowExceptionMessage(TJS_W("Not supported format."));
		}

		// 戻り値は void
		if(result) result->Clear();

		// 戻る
		return TJS_S_OK;
	}
} * SaveLayerImageFunction;
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	// SaveLayerImageFunction の作成と登録
	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 まずオブジェクトを作成
	SaveLayerImageFunction = new tSaveLayerImageFunction();

	// 2 SaveLayerImageFunction を tTJSVariant 型に変換
	val = tTJSVariant(SaveLayerImageFunction);

	// 3 すでに val が SaveLayerImageFunction を保持しているので、
	// SaveLayerImageFunction は Release する
	SaveLayerImageFunction->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("saveLayerImage"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);


	// - global を Release する
	global->Release();

	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	TVPAddLog(TVPPluginGlobalRefCount);
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で
	// 管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、
	// プラグインは解放できないと言うことになる。

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	TVPAddLog(TVPPluginGlobalRefCount);
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	// TJS のグローバルオブジェクトに登録した saveLayerImage 関数を削除する

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("saveLayerImage"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
			// 登録した関数が複数ある場合は これを繰り返す
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------



