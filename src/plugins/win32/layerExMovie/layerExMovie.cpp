#pragma comment(lib, "strmiids.lib")
#include "layerExMovie.hpp"

#ifndef FILEBASE
void ParseVideoType( CMediaType &mt, const wchar_t *type )
{
	// note: audio-less mpeg stream must have an extension of
	// ".mpv" .
	if      (wcsicmp(type, L".mpg") == 0)
		mt.subtype = MEDIASUBTYPE_MPEG1System;
	else if (wcsicmp(type, L".mpeg") == 0)
		mt.subtype = MEDIASUBTYPE_MPEG1System;
	else if (wcsicmp(type, L".mpv") == 0) 
		mt.subtype = MEDIASUBTYPE_MPEG1Video;
//		mt.subtype = MEDIASUBTYPE_MPEG1System;
	else if (wcsicmp(type, L".m1v") == 0) 
		mt.subtype = MEDIASUBTYPE_MPEG1Video;
	else if (wcsicmp(type, L".dat") == 0)
		mt.subtype = MEDIASUBTYPE_MPEG1VideoCD;
	else if (wcsicmp(type, L".avi") == 0)
		mt.subtype = MEDIASUBTYPE_Avi;
	else if (wcsicmp(type, L".mov") == 0)
		mt.subtype = MEDIASUBTYPE_QTMovie;
//	else if (wcsicmp(type, L".mp4") == 0)
//		mt.subtype = MEDIASUBTYPE_QTMovie;
//	else if (wcsicmp(type, L".wmv") == 0)
//		mt.subtype = SubTypeGUID_WMV3;
	else
		TVPThrowExceptionMessage(L"Unknown video format extension."); // unknown format
}
#endif

/**
 * コンストラクタ
 */
layerExMovie::layerExMovie(DispatchT obj) : _pType(obj, TJS_W("type")), layerExBase(obj)
{
	pAMStream         = NULL;
	pPrimaryVidStream = NULL;
	pDDStream         = NULL;
	pSample           = NULL; 
	pSurface          = NULL;
	loop = false;
	alpha = false;
	movieWidth = 0;
	movieHeight = 0;
#ifdef FILEBASE
	tempFile = "";
#else
	m_Proxy = NULL;
	m_Reader = NULL;
#endif
	in = NULL;
	{
		tTJSVariant var;
		if (TJS_SUCCEEDED(obj->PropGet(TJS_IGNOREPROP, L"onStartMovie", NULL, &var, obj))) onStartMovie = var;
		else onStartMovie = NULL;
		if (TJS_SUCCEEDED(obj->PropGet(TJS_IGNOREPROP, L"onStopMovie", NULL, &var, obj))) onStopMovie = var;
		else onStopMovie = NULL;
		if (TJS_SUCCEEDED(obj->PropGet(TJS_IGNOREPROP, L"onUpdateMovie", NULL, &var, obj))) onUpdateMovie = var;
		else onUpdateMovie = NULL;
	}
	playing = false;
}

/**
 * デストラクタ
 */
layerExMovie::~layerExMovie()
{
	stopMovie();
}

void
layerExMovie::clearMovie()
{ 
	if (pAMStream) {
		pAMStream->SetState(STREAMSTATE_STOP);
	}
	if (pSurface) {
		pSurface->Release();
		pSurface = NULL;
	}
	if (pSample) {
		pSample->Release();
		pSample = NULL;
	}
	if (pDDStream) {
		pDDStream->Release();
		pDDStream = NULL;
	}
	if (pPrimaryVidStream) {
		pPrimaryVidStream->Release();
		pPrimaryVidStream = NULL;
	}
	if (pAMStream) {
		pAMStream->Release();
		pAMStream = NULL;
	}
#ifdef FILEBASE
	if (tempFile != "") {
		DeleteFileW(tempFile.c_str());
		tempFile = "";
	}
#else
	if (m_Reader) {
		m_Reader->Release();
		m_Reader = NULL;
	}
	if( m_Proxy ) {
		delete m_Proxy;
		m_Proxy = NULL;
	}
#endif
	if (in) {
		in->Release();
		in = NULL;
	}
}

/**
 * ムービーファイルを開いて準備する
 * @param filename ファイル名
 * @param alpha アルファ指定（半分のサイズでα処理する）
 */
void
layerExMovie::openMovie(const tjs_char* filename, bool alpha)
{ 
	clearMovie();
	this->alpha = alpha;
	movieWidth = 0;
	movieHeight = 0;

#ifndef FILEBASE
	ttstr ext = TVPExtractStorageExt(filename);
	CMediaType mt;
	mt.majortype = MEDIATYPE_Stream;
	ParseVideoType(mt, ext.c_str()); // may throw an exception
#endif

	// ファイルをテンポラリにコピーして対応
	if ((in = TVPCreateIStream(filename, GENERIC_READ)) == NULL) {
		ttstr error = filename;
		error += ":ファイルが開けません";
		TVPAddLog(error);
		return;
	}
#ifdef FILEBASE
	tempFile = TVPGetTemporaryName();
	HANDLE hFile;
	if ((hFile = CreateFileW(tempFile.c_str(), GENERIC_WRITE, 0, NULL,
							CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL)) == INVALID_HANDLE_VALUE) {
		TVPAddLog("テンポラリファイルが開けません");
		tempFile = "";
		in->Release();
		in = NULL;
		return;
	}
	// ファイルをコピー
	BYTE buffer[1024*16];
	DWORD size;
	while (in->Read(buffer, sizeof buffer, &size) == S_OK && size > 0) {			
		WriteFile(hFile, buffer, size, &size, NULL);
	}
	CloseHandle(hFile);
	in->Release();
	in = NULL;
#else
	DWORD size;
	STATSTG stat;
	in->Stat(&stat, STATFLAG_NONAME);
	size = stat.cbSize.LowPart;
#endif
	
	if (SUCCEEDED(CoCreateInstance(CLSID_AMMultiMediaStream,0,1,IID_IAMMultiMediaStream,(void**)&pAMStream))) {
		if (SUCCEEDED(pAMStream->Initialize(STREAMTYPE_READ, 0, NULL)) &&
			SUCCEEDED(pAMStream->AddMediaStream(0, &MSPID_PrimaryVideo, 0, NULL))) {

#ifdef FILEBASE
			if (!SUCCEEDED(pAMStream->OpenFile(tempFile.c_str(), 0))) {
				pAMStream->Release();
				pAMStream = NULL;
				return;
			}
#else
			IGraphBuilder *builder;
			if (SUCCEEDED(pAMStream->GetFilterGraph(&builder))) {
				m_Proxy = new CIStreamProxy(in, size);
				HRESULT hr;
				m_Reader = new CIStreamReader(m_Proxy, &mt, &hr);
				m_Reader->AddRef();
				builder->AddFilter(m_Reader, L"SourceFilter");
				builder->Render(m_Reader->GetPin(0));
				builder->Release();
			} else {
				pAMStream->Release();
				pAMStream = NULL;
				return;
			}
#endif
			
			DWORD flag;
			pAMStream->GetInformation(&flag, NULL);
			if ((flag & MMSSF_ASYNCHRONOUS)) {
				TVPAddLog("ASYNC更新サポート");
				supportAsync = true;
			} else {
				supportAsync = false;
			}
			if ((flag & MMSSF_HASCLOCK)) {
				TVPAddLog("CLOCK あり");
			}
			if ((flag & MMSSF_SUPPORTSEEK)) {
				TVPAddLog("SEEKをサポート");
				supportSeek = true;
			} else {
				supportSeek = false;
			}

			if (SUCCEEDED(pAMStream->GetMediaStream(MSPID_PrimaryVideo, &pPrimaryVidStream))) {
				if (SUCCEEDED(pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream,(void**)&pDDStream))) {

					// フォーマットの指定 ARGB32
					DDSURFACEDESC desc;
					ZeroMemory(&desc, sizeof DDSURFACEDESC);
					desc.dwSize = sizeof(DDSURFACEDESC);
					desc.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
					desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
					desc.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
					desc.ddpfPixelFormat.dwFlags = DDPF_RGB; 
					desc.ddpfPixelFormat.dwRGBBitCount = 32;
					desc.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
					desc.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
					desc.ddpfPixelFormat.dwBBitMask = 0x000000FF;
					//						desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
					HRESULT hr;
					if (SUCCEEDED(hr = pDDStream->SetFormat(&desc, NULL))) {
						if (SUCCEEDED(pDDStream->CreateSample(0,0,0,&pSample))) {
							RECT rect; 
							if (SUCCEEDED(pSample->GetSurface(&pSurface,&rect))) {
								movieWidth  = rect.right - rect.left;
								movieHeight = rect.bottom - rect.top;
								if (alpha) {
									movieWidth /= 2;
								}
								// 画像サイズをムービーのサイズにあわせる
								_pWidth.SetValue(movieWidth);
								_pHeight.SetValue(movieHeight);
								_pType.SetValue(alpha ? ltAlpha : ltOpaque);
							} else {
								// サーフェース取得失敗
								pSample->Release();
								pSample = NULL;
								pDDStream->Release();
								pDDStream = NULL;
								pPrimaryVidStream->Release();
								pPrimaryVidStream = NULL;
								pAMStream->Release();
								pAMStream = NULL;
							}
						}
					} else {
						// サンプル作製失敗
						pDDStream->Release();
						pDDStream = NULL;
						pPrimaryVidStream->Release();
						pPrimaryVidStream = NULL;
						pAMStream->Release();
						pAMStream = NULL;
					}
				} else {
					// DirectDraw ストリーム取得失敗
					pPrimaryVidStream->Release();
					pPrimaryVidStream = NULL;
					pAMStream->Release();
					pAMStream = NULL;
				}
			} else {
				// ビデオストリーム取得失敗
				pAMStream->Release();
				pAMStream = NULL;
			}
		} else {
			pAMStream->Release();
			pAMStream = NULL;
		}
	}
}

/**
 * ムービーの開始
 */
void
layerExMovie::startMovie(bool loop)
{
	if (pSample) {
		// 再生開始
		this->loop = loop;
		pAMStream->SetState(STREAMSTATE_RUN);
		start();
		if (onStartMovie != NULL) {
			onStartMovie->FuncCall(0, NULL, NULL, NULL, 0, NULL, _obj);
		}
		if (supportAsync) {
			pSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);
		}
	}
}

/**
 * ムービーの停止
 */
void
layerExMovie::stopMovie()
{
	bool p = playing;
	stop();
	clearMovie();
	if (p) {
		if (onStopMovie != NULL) {
			onStopMovie->FuncCall(0, NULL, NULL, NULL, 0, NULL, _obj);
		}
	}
}

bool
layerExMovie::isPlayingMovie()
{
	return playing;
}

void
layerExMovie::start()
{
	stop();
	TVPAddContinuousEventHook(this);
	playing = true;
}

/**
 * Irrlicht 呼び出し処理停止
 */
void
layerExMovie::stop()
{
	TVPRemoveContinuousEventHook(this);
	playing = false;
}

void TJS_INTF_METHOD
layerExMovie::OnContinuousCallback(tjs_uint64 tick)
{
	if (pSample) {
		// 更新
		HRESULT hr;
		if (supportAsync) {
			hr = pSample->CompletionStatus(0,0);
		} else {
			hr = pSample->Update(0, NULL, NULL, 0);
		}
		if (hr == S_OK) {
			// 更新完了
			// サーフェースからレイヤに画面コピー
			reset();
			if (_buffer != NULL) {
				DDSURFACEDESC  ddsd; 
				ddsd.dwSize=sizeof(DDSURFACEDESC); 
				if (SUCCEEDED(pSurface->Lock( NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT , NULL))) {
					if (alpha) {
						int w = movieWidth*4;
						for (int y=0; y<ddsd.dwHeight && y<_height; y++) {
							BYTE *dst  = _buffer+(y*_pitch);
							BYTE *src1 = (BYTE*)ddsd.lpSurface+y*ddsd.lPitch;
							BYTE *src2 = src1 + w;
							for (int x=0; x<_width;x++) {
								*dst++ = *src1++;
								*dst++ = *src1++;
								*dst++ = *src1++;
								*dst++ = *src2; src2+=4; src1++;
							}
						}
					} else {
						int w = _width < movieWidth ? _width * 4 : movieWidth * 4;
						for (int y=0; y<_height; y++) {
							memcpy(_buffer+(y*_pitch), (BYTE*)ddsd.lpSurface+y*ddsd.lPitch, w);
						}
					}
					pSurface->Unlock(NULL); 
				}
				//redraw();
				if (onUpdateMovie != NULL) {
					onUpdateMovie->FuncCall(0, NULL, NULL, NULL, 0, NULL, _obj);
				}
			}
			if (supportAsync) {
				pSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);
			}
		} else if (hr == MS_S_ENDOFSTREAM) {
			// 更新終了
			if (loop) {
				if (supportSeek) {
					pAMStream->Seek(0);
				} else {
					pAMStream->SetState(STREAMSTATE_STOP);
					pAMStream->SetState(STREAMSTATE_RUN);
				}
				if (supportAsync) {
					pSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);
				}
			} else {
				stopMovie();
			}
		} else if (hr == MS_S_PENDING) {
			//TVPAddLog("更新待ち");
		} else {
			stopMovie();
		}
	} else {
		stop();
	}
}
