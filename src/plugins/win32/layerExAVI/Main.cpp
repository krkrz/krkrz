#include "ncbind/ncbind.hpp"
#include <vfw.h>
#pragma comment (lib, "vfw32.lib")
#pragma comment (lib, "winmm.lib")

#include "../layerExDraw/layerExBase.hpp"

/**
 * レイヤ画像/音声録画機構
 */
class LayerExAVI : public layerExBase {

protected:
	// 動画記録用の情報
	PAVIFILE      pavi; //< AVIファイル
	PAVISTREAM    pstm; //< AVIストリーム
        PAVISTREAM    ptmp; //< 圧縮ストリーム
        COMPVARS cv;
        bool hasCv;
        AVICOMPRESSOPTIONS opt;
	int aviWidth;  //< 開始時横幅
	int aviHeight; //< 開始時縦幅
	int lastFrame; //< 最終録画フレーム

protected:
	// 音声記録用の情報
	HWAVEIN hwi; //< 音声入力ハンドル
	WAVEHDR wvhdr; //< 保存用ヘッダ
	IStream *wvout; //< 出力先
	
	/**
	 * 音声入力コールバック処理
	 * @param hwi 音声入力ハンドル
	 * @param uMsg コマンド
	 * @param dwInstance インスタンス情報
	 * @param dwParam1 パラメータ1
	 * @param dwParam1 パラメータ2
	 */
	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
		LayerExAVI *owner = (LayerExAVI*)dwInstance;
		owner->waveIn(hwi, uMsg, dwParam1, dwParam2);
	}

	/**
	 * 音声入力コールバック
	 * @param hwi 音声入力ハンドル
	 * @param uMsg コマンド
	 * @param dwParam1 パラメータ1
	 * @param dwParam1 パラメータ2
	 */
	void waveIn(HWAVEIN hwi, UINT uMsg, DWORD dwParam1, DWORD dwParam2) {
		/* イベント処理 */
		switch(uMsg){
		case WIM_DATA:
			{
				LPWAVEHDR lpwvhdr = (LPWAVEHDR) dwParam1;
				ULONG s;
				wvout->Write(lpwvhdr->lpData, lpwvhdr->dwBufferLength, &s);
			}
			break;
		}
	}

public:
	/**
	 * コンストラクタ
	 */
	LayerExAVI(DispatchT obj) : layerExBase(obj), pavi(NULL), pstm(NULL), ptmp(NULL), hwi(NULL), wvout(NULL) {
		ZeroMemory(&wvhdr, sizeof wvhdr);
	}

	/**
	 * デストラクタ
	 */
	~LayerExAVI() {
		closeAVI();
	}

	/**
	 * AVIファイルを開く
	 * @param filename 保存ファイル名
	 * @param fps 秒間フレーム数
	 */
	void openAVI(const tjs_char *filename, int fps) {

		closeAVI();

		// 録画開始時のサイズを記録しておく
		aviWidth  = _width;
		aviHeight = _height;

		// AVIファイルを開く
		ttstr path = TVPNormalizeStorageName(ttstr(filename));
		TVPGetLocalName(path);
		if (AVIFileOpen(&pavi, path.c_str(), OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE,NULL) != 0)	{
			ttstr msg = filename;
			msg += ":can't open";
			TVPThrowExceptionMessage(msg.c_str());
		}

		// AVIストリームの生成
		AVISTREAMINFO si = {	
			streamtypeVIDEO, // Video Stream
			comptypeDIB,
			0,               // Stream Flag
			0,
			0,
			0,
			1,               // 時間単位 dwScale
			fps,             // フレーム dwRate
			0,
			0,         // ストリームの長さ XXX
			0,
			0,
			(DWORD)-1,       // -1: Default品質 [0-10000]
			0,
			// 表示する矩形サイズ
			{ 0, 0, aviWidth, aviHeight },
			0,
			0,
			L"KIRIKIRI" };

		if (AVIFileCreateStream(pavi, &pstm, &si) != 0) {
			closeAVI();
			TVPThrowExceptionMessage(L"AVIFileCreateStream");
                }


		// ストリームに投げ込むデータフォーマットを指定

		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(bih);
		bih.biWidth  = aviWidth;
		bih.biHeight = aviHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		if (AVIStreamSetFormat(pstm, 0, &bih, sizeof(bih)) != 0 ) {
			closeAVI();
			TVPThrowExceptionMessage(L"AVIFileCreateStream");
		}

		// 先頭フレーム
                lastFrame = -1;

                hasCv = false;
        }

	/**
	 * AVIファイルを圧縮フォーマットを指定して開く
	 * @param filename 保存ファイル名
	 * @param fps 秒間フレーム数
         * @param return 圧縮ダイアログでキャンセルを押した場合false。
	 */
	bool openCompressedAVI(const tjs_char *filename, int fps) {

		closeAVI();

		// 録画開始時のサイズを記録しておく
		aviWidth  = _width;
		aviHeight = _height;

		// ストリームに投げ込むデータフォーマットを指定

		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(bih);
		bih.biWidth  = aviWidth;
		bih.biHeight = aviHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

                // 圧縮オプションを取得
                memset(&cv,0,sizeof(COMPVARS));
                cv.cbSize=sizeof(COMPVARS);
                cv.dwFlags=ICMF_COMPVARS_VALID;
                cv.fccHandler=comptypeDIB;
                cv.lQ=ICQUALITY_DEFAULT;
                if (!ICCompressorChoose(NULL,ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
                                        &bih,NULL,&cv,NULL)) {
                  return false;
                }

                // オプションを指定
                opt.fccType=streamtypeVIDEO;
                opt.fccHandler=cv.fccHandler;
                opt.dwKeyFrameEvery=cv.lKey;
                opt.dwQuality=cv.lQ;
                opt.dwBytesPerSecond=cv.lDataRate;
                opt.dwFlags=(cv.lDataRate>0?AVICOMPRESSF_DATARATE:0)
                  |(cv.lKey>0?AVICOMPRESSF_KEYFRAMES:0);
                opt.lpFormat=NULL;
                opt.cbFormat=0;
                opt.lpParms=cv.lpState;
                opt.cbParms=cv.cbState;
                opt.dwInterleaveEvery=0;
                

		// AVIファイルを開く
		ttstr path = TVPNormalizeStorageName(ttstr(filename));
		TVPGetLocalName(path);
		if (AVIFileOpen(&pavi, path.c_str(), OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE,NULL) != 0)	{
			ttstr msg = filename;
			msg += ":can't open";
			TVPThrowExceptionMessage(msg.c_str());
		}

		// AVIストリームの生成
		AVISTREAMINFO si = {	
			streamtypeVIDEO, // Video Stream
			comptypeDIB,
			0,               // Stream Flag
			0,
			0,
			0,
			1,               // 時間単位 dwScale
			fps,             // フレーム dwRate
			0,
			10,         // ストリームの長さ XXX
			0,
			0,
			(DWORD)-1,       // -1: Default品質 [0-10000]
			0,
			// 表示する矩形サイズ
			{ 0, 0, aviWidth, aviHeight },
			0,
			0,
			L"KIRIKIRI" };
                si.fccHandler=cv.fccHandler;

		if (AVIFileCreateStream(pavi, &pstm, &si) != 0) {
			closeAVI();
			TVPThrowExceptionMessage(L"AVIFileCreateStream");
		}
                if (AVIMakeCompressedStream(&ptmp,pstm,&opt,NULL)!=AVIERR_OK) {
                  closeAVI();
                  TVPThrowExceptionMessage(L"AVIMakeCompressedStream");
                }

		if (AVIStreamSetFormat(ptmp, 0, &bih, sizeof(bih)) != 0 ) {
			closeAVI();
			TVPThrowExceptionMessage(L"AVIFileCreateStream");
		}

		// 先頭フレーム
		lastFrame = -1;

                hasCv = true;

                return true;
	}

	/**
	 * AVIファイルにデータを記録
	 */
	void recordAVI(int frame) {
		if (pavi && pstm) {
			if (frame > lastFrame) {
				// サイズが変わってたら例外
				if (aviWidth != _width ||
					aviHeight != _height) {
					TVPThrowExceptionMessage(L"layer size has changed");
				}
				// 吉里吉里のバッファは DIB と同じ構造なのでこの処理で通る
				int size = _height * -_pitch;
				const unsigned char *buffer = _buffer + (_height-1) * _pitch;
				if (AVIStreamWrite(hasCv ? ptmp : pstm, frame, 1, (void*)buffer, size, AVIIF_KEYFRAME, NULL, NULL ) != 0) {
					TVPThrowExceptionMessage(L"AVIStreamWrite");
				}
				lastFrame = frame;
			}
		} else {
			TVPThrowExceptionMessage(L"AVI file not opened");
		}
	}

	/**
	 * AVIファイルを閉じる
	 */
	void closeAVI() {
                if (ptmp) {
                   AVIStreamRelease(ptmp);
                   ptmp = NULL;
                }
		if (pstm) {
			AVIStreamRelease(pstm);
			pstm = NULL;
		}
		if (pavi) {
			AVIFileRelease(pavi);
			pavi = NULL;
		}
                if (hasCv) {
                  ICCompressorFree(&cv);
                }
	}

	/**
	 * WAV録音準備
	 * @param filename 保存ファイル名
	 * @param channel チャンネル
	 * @param rate レート
	 * @param bits ビット数
	 * @param interval 取得タイミング
	 */
	void openWAV(const tjs_char *filename, int channel, int rate, int bits, int interval) {

		closeWAV();
		
		// ファイルを開く
		wvout = TVPCreateIStream(filename, TJS_BS_WRITE);
		
		// フォーマットを指定
		WAVEFORMATEX waveForm;
		waveForm.wFormatTag      = WAVE_FORMAT_PCM;
		waveForm.nChannels       = channel;
		waveForm.nSamplesPerSec  = rate;
		waveForm.wBitsPerSample  = bits;
		waveForm.nBlockAlign     = waveForm.nChannels * waveForm.wBitsPerSample / 8;
		waveForm.nAvgBytesPerSec = waveForm.nSamplesPerSec * waveForm.nBlockAlign;
		
		// waveIn を開く
		if (waveInOpen(&hwi, WAVE_MAPPER, &waveForm, (DWORD)waveInProc, (DWORD)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
			TVPThrowExceptionMessage(L"waveInOpen");
		}
		
		/* キャプチャバッファ確保 */
		int length = waveForm.nAvgBytesPerSec * interval / 1000;
		wvhdr.lpData         = new char[length];
		wvhdr.dwBufferLength = length;
		wvhdr.dwFlags        = 0;
		wvhdr.reserved       = 0;

		// バッファを設定
		waveInPrepareHeader(hwi, &wvhdr, sizeof(wvhdr));
		waveInAddBuffer(hwi, &wvhdr, sizeof(wvhdr));
	}

	/**
	 * WAV録音開始
	 */
	void startWAV() {
		if (hwi) {
			if (waveInStart(hwi) != MMSYSERR_NOERROR) {
				TVPThrowExceptionMessage(L"waveInStart");
			}
		}
	}

	/**
	 * WAV録音停止
	 */
	void stopWAV() {
		if (hwi) {
			if(waveInStop(hwi) != MMSYSERR_NOERROR) {
				TVPThrowExceptionMessage(L"waveInStop");
			}
		}
	}

	/**
	 * WAV録音終了
	 */
	void closeWAV() {
		if (hwi) {
			waveInStop(hwi);
			waveInUnprepareHeader(hwi, &wvhdr, sizeof(wvhdr));
			waveInReset(hwi);
			waveInClose(hwi);
			hwi = NULL;
			// バッファクリア
			if (wvhdr.lpData) {
				delete[] wvhdr.lpData;
			}
			ZeroMemory(&wvhdr, sizeof wvhdr);
		}
		// ファイルクローズ
		if (wvout) {
			wvout->Release();
			wvout = NULL;
		}
	}

};

NCB_GET_INSTANCE_HOOK(LayerExAVI)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		obj->reset();
		return obj;
	}
	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};

NCB_ATTACH_CLASS_WITH_HOOK(LayerExAVI, Layer) {
	NCB_METHOD(openAVI);
	NCB_METHOD(openCompressedAVI);
	NCB_METHOD(closeAVI);
	NCB_METHOD(recordAVI);
	NCB_METHOD(openWAV);
	NCB_METHOD(startWAV);
	NCB_METHOD(stopWAV);
	NCB_METHOD(closeWAV);
}

// ----------------------------------- 起動・開放処理

/**
 * 登録処理前
 */
void PreRegistCallback()
{
	AVIFileInit();
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
	AVIFileExit();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
