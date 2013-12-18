#include <windows.h>
#include <stdio.h>
#include <string>
#include "ncbind/ncbind.hpp"

// ログ出力用
#if 0
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}
#endif

// xml.c
extern void showXMLCopyright();
extern bool createInvokeXML(std::wstring &xml, tjs_int numparams, tTJSVariant **params);
extern bool getVariantFromXML(tTJSVariant &var, tjs_char *xml);
extern bool invokeXML(std::wstring &result, iTJSDispatch2 *target, tjs_char *xml);

// variant.c
extern void storeVariant(tTJSVariant &result, VARIANT &variant);

//---------------------------------------------------------------------------

#import "c:\\windows\\system32\\macromed\\flash\\flash10a.ocx" named_guids

class FlashPlayer : public IOleClientSite,
					public IOleInPlaceSiteWindowless,
					public IOleInPlaceFrame,
					public IStorage,
					public ShockwaveFlashObjects::_IShockwaveFlashEvents
{

public:
	/**
	 * コンストラクタ
	 */
	FlashPlayer(iTJSDispatch2 *objthis)
		 : objthis(objthis), refCount(0), control(NULL), oleobj(NULL),
		   viewObjectEx(NULL), viewObject(NULL), windowless(NULL), conContainer(NULL),
		   conPoint(NULL), conPointId(0), updateFlag(false), hdc(NULL), bmp(NULL), pixels(NULL)
	{
	}

	/**
	 * デストラクタ
	 */
	virtual ~FlashPlayer() {
		if (conPoint) {
			if (conPointId) {
				conPoint->Unadvise(conPointId);
			}
			conPoint->Release();
		}
		if (conContainer) {
			conContainer->Release();
		}
		if (viewObjectEx) {
			viewObjectEx->Release();
		}
		if (viewObject){
			viewObject->Release();
		}
		if (windowless) {
			windowless->Release();
		}
		if (control) {
			control->Release();
		}
		if (oleobj) {
			oleobj->Close(OLECLOSE_NOSAVE);
			oleobj->Release();
		}
		if (hdc) {
			::DeleteDC(hdc);
		}
		if (bmp) {
			::DeleteObject(bmp);
		}
	}

	/**
	 * インスタンス生成ファクトリ
	 */
	static tjs_error factory(FlashPlayer **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		FlashPlayer *self = new FlashPlayer(objthis);
		if (self->Create(*params[0], *params[1])) {
			*result = self;
			return TJS_S_OK;
		} else {
			delete self;
			return TJS_E_FAIL;
		}
	}
	
public:

#define CHECK if(!control){TVPThrowExceptionMessage(L"not initialized");}
#define COMERROR TVPThrowExceptionMessage(L"flash com error:%1", ttstr((const wchar_t*)e.ErrorMessage()))
	
	long getReadyState() {
		CHECK;
		try {
			return control->GetReadyState();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	long getTotalFrames() {
		CHECK;
		try {
			return control->GetTotalFrames();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	
	bool getPlaying (){
		CHECK;
		try {
			return control->GetPlaying() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}

	void setPlaying (bool val) {
		CHECK;
		try {
			control->PutPlaying(val ? VARIANT_TRUE : VARIANT_FALSE);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	int getQuality () {
		CHECK;
		try {
			return control->GetQuality();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	void setQuality (int val) {
		CHECK;
		try {
			control->PutQuality(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	int getScaleMode() {
		CHECK;
		try {
			return control->GetScaleMode();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	void setScaleMode(int val) {
		CHECK;
		try {
			control->PutScaleMode(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	int getAlignMode() {
		CHECK;
		try {
			return control->GetAlignMode();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	void setAlignMode(int val) {
		CHECK;
		try {
			control->PutAlignMode(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	long getBackgroundColor() {
		CHECK;
		try {
			return control->GetBackgroundColor();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	void setBackgroundColor (int val) {
		CHECK;
		try {
			control->PutBackgroundColor(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getLoop() {
		CHECK;
		try {
			return control->GetLoop() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setLoop(bool val) {
		CHECK;
		try {
			control->PutLoop (val ? VARIANT_TRUE : VARIANT_FALSE);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getMovie() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetMovie());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setMovie(const tjs_char *val) {
		CHECK;
		try {
			control->PutMovie(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	int getFrameNum() {
		CHECK;
		try {
			return control->GetFrameNum();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	void setFrameNum (int val) {
		CHECK;
		try {
			control->PutFrameNum(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void setZoomRect(int left, int top, int right, int bottom) {
		CHECK;
		try {
			control->SetZoomRect(left, top, right, bottom);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void zoom(int factor) {
		CHECK;
		try {
			control->Zoom(factor);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void pan(int x, int y, int mode ) {
		CHECK;
		try {
			control->Pan(x,y,mode);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void play() {
		CHECK;
		try {
			control->Play();
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void stop() {
		CHECK;
		try {
			control->Stop();
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void back() {
		CHECK;
		try {
			control->Back();
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void forward() {
		CHECK;
		try {
			control->Forward();
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void rewind() {
		CHECK;
		try {
			control->Rewind();
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void stopPlay() {
		CHECK;
		try {
			control->StopPlay();
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void gotoFrame(int frameNum) {
		CHECK;
		try {
			control->GotoFrame(frameNum);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	long getCurrentFrame() {
		CHECK;
		try {
			return control->CurrentFrame();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	bool isPlaying() {
		CHECK;
		try {
			return control->IsPlaying() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}

	long getPercentLoaded() {
		CHECK;
		try {
			return control->PercentLoaded();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	
	bool getFrameLoaded(long frameNum) {
		CHECK;
		try {
			return control->FrameLoaded(frameNum) != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	long getFlashVersion() {
		CHECK;
		try {
			return control->FlashVersion();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	
	ttstr getSAlign() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetSAlign());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setSAlign(const tjs_char *val) {
		CHECK;
		try {
			control->PutSAlign(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getMenu() {
		CHECK;
		try {
			return control->GetMenu() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setMenu(bool val) {
		CHECK;
		try {
			control->PutMenu(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	
	ttstr getBase() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetBase());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setBase(const tjs_char *val) {
		CHECK;
		try {
			control->PutBase(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getScale() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetScale());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setScale(const tjs_char *val) {
		CHECK;
		try {
			control->PutScale(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getDeviceFont() {
		CHECK;
		try {
			return control->GetDeviceFont() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setDeviceFont(bool val) {
		CHECK;
		try {
			control->PutDeviceFont(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getEmbedMovie() {
		CHECK;
		try {
			return control->GetEmbedMovie() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setEmbedMovie(bool val) {
		CHECK;
		try {
			control->PutEmbedMovie(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getBgColor() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetBGColor());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setBgColor(const tjs_char *val) {
		CHECK;
		try {
			control->PutBGColor(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getQuality2() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetQuality2());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setQuality2(const tjs_char *val) {
		CHECK;
		try {
			control->PutQuality2(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	
	void loadMovie(int layer, const tjs_char *url) {
		CHECK;
		try {
			control->LoadMovie(layer, url);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void tGotoFrame(const tjs_char *target, long frameNum) {
		CHECK;
		try {
			control->TGotoFrame(target, frameNum);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void tGotoLabel(const tjs_char *target, const tjs_char *label) {
		CHECK;
		try {
			control->TGotoLabel(target, label);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	long tCurrentFrame(const tjs_char *target) {
		CHECK;
		try {
			return control->TCurrentFrame(target);
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}

	ttstr tCurrentLabel(const tjs_char *target) {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->TCurrentLabel(target));
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}

	void tPlay(const tjs_char *target) {
		CHECK;
		try {
			control->TPlay(target);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void tStopPlay(const tjs_char *target) {
		CHECK;
		try {
			control->TStopPlay(target);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void setVariable(const tjs_char *name, const tjs_char *value) {
		CHECK;
		try {
			control->SetVariable(name, value);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	ttstr getVariable(const tjs_char *name) {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetVariable(name));
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}

	void tSetProperty(const tjs_char *target, int property, const tjs_char *value) {
		CHECK;
		try {
			control->TSetProperty (target, property, value);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	ttstr tGetProperty(const tjs_char *target, int property) {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->TGetProperty(target, property));
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}

	void tCallFrame(const tjs_char *target, int frameNum) {
		CHECK;
		try {
			control->TCallFrame(target, frameNum);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	void tCallLabel(const tjs_char *target, const tjs_char *label) {
		CHECK;
		try {
			control->TCallLabel(target, label);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void tSetPropertyNum(const tjs_char *target, int property, double num) {
		CHECK;
		try {
			control->TSetPropertyNum (target, property, num);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	double tGetPropertyNum(const tjs_char *target, int property) {
		CHECK;
		try {
			return control->TGetPropertyNum(target, property);
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	
	ttstr getSWRemote() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetSWRemote());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setSWRemote(const tjs_char *val) {
		CHECK;
		try {
			control->PutSWRemote(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getFlashVars() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetFlashVars());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setFlashVars(const tjs_char *val) {
		CHECK;
		try {
			control->PutFlashVars(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getAllowScriptAccess() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetAllowScriptAccess());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setAllowScriptAccess(const tjs_char *val) {
		CHECK;
		try {
			control->PutAllowScriptAccess(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getMovieData() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetMovieData());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setMovieData(const tjs_char *val) {
		CHECK;
		try {
			control->PutMovieData(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getSeamlessTabbing() {
		CHECK;
		try {
			return control->GetSeamlessTabbing() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setSeamlessTabbing(bool val) {
		CHECK;
		try {
			control->PutSeamlessTabbing(val ? VARIANT_TRUE : VARIANT_FALSE);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	void enforceLocalSecurity() {
		CHECK;
		try {
			control->EnforceLocalSecurity();
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	bool getProfile() {
		CHECK;
		try {
			return control->GetProfile() != VARIANT_FALSE;
		} catch(_com_error &e) {
			COMERROR;
		}
		return false;
	}
	void setProfile(bool val) {
		CHECK;
		try {
			control->PutProfile(val ? VARIANT_TRUE : VARIANT_FALSE);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	ttstr getProfileAddress() {
		CHECK;
		try {
			return ttstr((const wchar_t*)control->GetProfileAddress());
		} catch(_com_error &e) {
			COMERROR;
		}
		return L"";
	}
	void setProfileAddress(const tjs_char *val) {
		CHECK;
		try {
			control->PutProfileAddress(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	int getProfilePort() {
		CHECK;
		try {
			return control->GetProfilePort();
		} catch(_com_error &e) {
			COMERROR;
		}
		return 0;
	}
	void setProfilePort(int val) {
		CHECK;
		try {
			control->PutProfilePort(val);
		} catch(_com_error &e) {
			COMERROR;
		}
	}
	
	void disableLocalSecurity() {
		CHECK;
		try {
			control->DisableLocalSecurity();
		} catch(_com_error &e) {
			COMERROR;
		}
	}

	/**
	 * メソッド呼び出し
	 */
	static tjs_error callFunction(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, FlashPlayer *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		std::wstring xml;
		if (!createInvokeXML(xml, numparams, params)) {
			TVPThrowExceptionMessage(L"failed to create invoke xml");
		}
		try {
			ttstr resultXML = (const tjs_char*)self->control->CallFunction(xml.c_str());
			if (result) {
				getVariantFromXML(*result, (tjs_char*)resultXML.c_str());
			}
		} catch(_com_error &e) {
			COMERROR;
		}
		return TJS_S_OK;
	}

	/**
	 * TJS呼び出し結果取得
	 */
	static tjs_error getLastTJSError(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, FlashPlayer *self) {
		if (result) {
			*result = self->lastTJSErrorMsg;
		}
		return TJS_S_OK;
	}
	
	// --------------------------------------------------
	// プレイヤー制御
	// --------------------------------------------------

	bool clearMovie() {
		bool ret = false;
		if (oleobj) {
			IPersistStreamInit * psStreamInit = 0;
			if (SUCCEEDED(oleobj->QueryInterface(::IID_IPersistStreamInit,  (LPVOID*)&psStreamInit))) {
				if (SUCCEEDED(psStreamInit->InitNew())) {
					HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, 8);
					if (hBuffer)	{
						unsigned char* pBuffer = (unsigned char*)::GlobalLock(hBuffer);
						if (pBuffer) {
							memcpy(pBuffer, "fUfU", 4);
							*(ULONG*)(pBuffer+4) = 0;
							IStream* pStream = NULL;
							if(::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK) 	{
								if (SUCCEEDED(psStreamInit->Load(pStream))) {
									ret = true;
								}
								pStream->Release();
							}
							::GlobalUnlock(hBuffer);
						}
						::GlobalFree(hBuffer);
					}
				}
				psStreamInit->Release();
			}
		}
		return ret;
	}
	
	/**
	 * 指定した吉里吉里のファイルを動画とみなして初期化する
	 * @param storage 吉里吉里のファイル
	 * @return 読み込みに成功したら true
	 */
	bool initMovie(const tjs_char *storage) {
		bool ret = false;
		if (oleobj) {
			IPersistStreamInit * psStreamInit = 0;
			if (SUCCEEDED(oleobj->QueryInterface(::IID_IPersistStreamInit,  (LPVOID*)&psStreamInit))) {
				if (SUCCEEDED(psStreamInit->InitNew())) {
					IStream *in = TVPCreateIStream(storage, TJS_BS_READ);
					if (in) {
						STATSTG stat;
						in->Stat(&stat, STATFLAG_NONAME);
						// サイズあふれ無視XXX
						ULONG size = (ULONG)stat.cbSize.QuadPart;
						HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, size + 8);
						if (hBuffer)	{
							unsigned char* pBuffer = (unsigned char*)::GlobalLock(hBuffer);
							if (pBuffer) {
								memcpy(pBuffer, "fUfU", 4);
								*(ULONG*)(pBuffer+4) = size;
								if (in->Read(pBuffer+8, size, &size) == S_OK) {
									IStream* pStream = NULL;
									if(::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK) 	{
										if (SUCCEEDED(psStreamInit->Load(pStream))) {
											ret = true;
										}
										pStream->Release();
									}
								}
								::GlobalUnlock(hBuffer);
							}
							::GlobalFree(hBuffer);
						}
						in->Release();
					}
				}
				psStreamInit->Release();
			}
		}
		return ret;
	}
	
	/**
	 * サイズを指定
	 * @param width 横幅
	 * @param height 縦幅
	 */
	void setSize(int width, int height) {
		RECT rect = {0,0,width,height};
		if (windowless) {
			windowless->SetObjectRects(&rect, &rect);
		}
		if (!hdc || !EqualRect(&rect, &size)) {
			if (hdc) {
				::DeleteDC(hdc);
			}
			if (bmp) {
				::DeleteObject(bmp);
			}
			this->width  = width;
			this->height = height;
			this->pitch  = (width * 4 + 3) / 4 * 4;
			updateRect = size = rect;
			updateFlag = true;
			BITMAPINFOHEADER bih = {0};
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biPlanes = 1;
			bih.biWidth  = width;
			bih.biHeight = -height;
			hdc = CreateCompatibleDC(NULL);
			bmp = CreateDIBSection(hdc, (BITMAPINFO *)&bih, DIB_RGB_COLORS, (void **)&pixels, NULL, 0x0);
			SelectObject(hdc, bmp);
		}
	}

	/**
	 * 入力判定
	 * @param x
	 * @param y
	 */
	bool hitTest(int x, int y) {
		if (viewObjectEx) {
			POINT p = {x, y};
			DWORD dwRes;
			HRESULT hr = viewObjectEx->QueryHitPoint(DVASPECT_CONTENT, &size, p, 1, &dwRes);
			if (hr == S_OK)	{
				return dwRes != HITRESULT_OUTSIDE;
			}
		}
		return false;
	}

	/**
	 * レイヤに対して内容を全描画
	 * @param layer レイヤ
	 * @param onlyUpdate 更新部のみ描画
	 */
	void _draw(iTJSDispatch2 *layer, bool onlyUpdate=false) {

		if (!hdc) {
			return;
		}

		if (updateFlag) {
			IViewObject *lpV = viewObjectEx ? (IViewObject *)viewObjectEx : viewObject;
			HRGN rgn = CreateRectRgn(updateRect.left, updateRect.top, updateRect.right+1, updateRect.bottom+1);
			SelectClipRgn(hdc, rgn);
			FillRect(hdc, &size, (HBRUSH)GetStockObject(BLACK_BRUSH));
			OleDraw(lpV, DVASPECT_TRANSPARENT, hdc, &size);
			DeleteObject(rgn);
		}
		
		ncbPropAccessor obj(layer);
		int iw = obj.getIntValue(L"imageWidth");
		int ih = obj.getIntValue(L"imageHeight");
		unsigned char *imageBuffer = (unsigned char*)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
		tjs_int imagePitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());

		//log(L"draw:%d %d", onlyUpdate, updateFlag);
		if (onlyUpdate) {
			if (updateFlag) {
				int l = updateRect.left;
				int t = updateRect.top;
				int w = updateRect.right - l;
				int h = updateRect.bottom - t;
				
				//log(L"before l:%d t:%d w:%d h:%d iw:%d ih:%d", l, t, w, h, iw, ih);
				if (l+w > iw) {
					w -= (l+w - iw);
				}
				if (t+h > ih) {
					h -= (t+h - ih);
				}
				//log(L"after l:%d t:%d w:%d h:%d", l, t, w, h);
				if (w >0 && h >0) {
					int line = w * 4;
					const tjs_uint8 *src = (tjs_uint8*)(pixels) + pitch * t + l*4;
					tjs_uint8 *dst = imageBuffer + imagePitch * t + l * 4;
					for (tjs_int y = 0; y < h; y++) {
						CopyMemory(dst, src, line);
						src += pitch;
						dst += imagePitch;
					}
					tTJSVariant  vars [4] = { l, t, w, h };
					tTJSVariant *varsp[4] = { vars, vars+1, vars+2, vars+3 };
					layer->FuncCall(0, L"update", NULL, NULL, 4, varsp, layer);
				}
			}
		} else {
			int w = width;
			int h = height;
			if (iw < w) {
				w = iw;
			}
			if (ih < h) {
				h = ih;
			}
			int line = w * 4;
			const tjs_uint8 *src = (tjs_uint8*)(pixels);
			tjs_uint8 *dst = imageBuffer;
			for (tjs_int y = 0; y < h; y++) {
				CopyMemory(dst, src, line);
				src += pitch;
				dst += imagePitch;
			}
			tTJSVariant  vars [4] = { 0, 0, w, h };
			tTJSVariant *varsp[4] = { vars, vars+1, vars+2, vars+3 };
			layer->FuncCall(0, L"update", NULL, NULL, 4, varsp, layer);
		}
		// 更新情報の抹消
		updateFlag = false;
		SetRect(&updateRect,0,0,0,0);
	}

	static tjs_error draw(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, FlashPlayer *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (params[0]->Type() != tvtObject || params[0]->AsObjectNoAddRef()->IsInstanceOf(0,NULL,NULL,L"Layer",NULL) == false) {
			return TJS_E_INVALIDTYPE;
		}
		self->_draw(*params[0], numparams > 1 && (int)*params[1] != 0);
		return TJS_S_OK;
	}
	
	/**
	 * キーダウンの通知
	 * @param key キーコード
	 * @return 処理されたら true
	 */
	bool doKeyDown(int key) {
		if (windowless) {
			LRESULT res;
			return SUCCEEDED(windowless->OnWindowMessage(WM_KEYDOWN, key, 0, &res));
		}
		return false;
	}

	/**
	 * キーアップの通知
	 * @param key キーコード
	 * @return 処理されたら true
	 */
	bool doKeyUp(int key) {
		if (windowless) {
			LRESULT res;
			return SUCCEEDED(windowless->OnWindowMessage(WM_KEYUP, key, 0, &res));
		}
		return false;
	}

	/**
	 * マウスが領域に入った通知
	 */
	void doMouseEnter() {
		mousemap = 0;
	}

	/**
	 * マウスが領域から出た通知
	 */
	void doMouseLeave() {
		mousemap = 0;
	}

	/**
	 * マウスキーおしさげ通知
	 * @param x 座標
	 * @param y 座標
	 * @param button 押し下げたボタン
	 * @param shift シフト状態
	 * @return 処理されたら true
	 */
	bool doMouseDown(int x, int y, int button, int shift) {
		if (windowless) {
			UINT uMsg;
			switch (button) {
			case mbLeft:   uMsg = WM_LBUTTONDOWN; mousemap |= MK_LBUTTON; break;
			case mbMiddle: uMsg = WM_MBUTTONDOWN; mousemap |= MK_MBUTTON; break;
			case mbRight:  uMsg = WM_RBUTTONDOWN; mousemap |= MK_RBUTTON; break;
			default: return false;
			}
			WPARAM wParam = getMouseShift(shift);
			LPARAM lParam = MAKELONG(x, y);
			LRESULT res;
			return SUCCEEDED(windowless->OnWindowMessage(uMsg, wParam, lParam, &res));
		}
		return false;
	}

	/**
	 * マウス移動通知
	 * @param x 座標
	 * @param y 座標
	 * @param shift シフト状態
	 * @return 処理されたら true
	 */
	bool doMouseMove(int x, int y, int shift) {
		if (windowless) {
			WPARAM wParam = getMouseShift(shift);
			LPARAM lParam = MAKELONG(x, y);
			LRESULT res;
			bool ret = SUCCEEDED(windowless->OnWindowMessage(WM_MOUSEMOVE, wParam, lParam, &res));
			return ret;
		}
		return false;
	}
	
	/**
	 * マウスキーおしあげ通知
	 * @param x 座標
	 * @param y 座標
	 * @param button 押し上げたボタン
	 * @param shift シフト状態
	 * @return 処理されたら true
	 */
	bool doMouseUp(int x, int y, int button, int shift) {
		if (windowless) {
			UINT uMsg;
			switch (button) {
			case mbLeft:   uMsg = WM_LBUTTONUP; mousemap &= ~MK_LBUTTON; break;
			case mbMiddle: uMsg = WM_MBUTTONUP; mousemap &= ~MK_MBUTTON; break;
			case mbRight:  uMsg = WM_RBUTTONUP; mousemap &= ~MK_RBUTTON; break;
			default: return false;
			}
			WPARAM wParam = getMouseShift(shift);
			LPARAM lParam = MAKELONG(x, y);
			LRESULT res;
			return SUCCEEDED(windowless->OnWindowMessage(uMsg, wParam, lParam, &res));
		}
		return false;
	}

	/**
	 * マウスホイール通知
	 * @param shift シフト状態
	 * @param delta ホイール回転量
	 * @param x 座標
	 * @param y 座標
	 * @return 処理されたら true
	 */
	bool doMouseWheel(int shift, int delta, int x, int y) {
		if (windowless) {
			WPARAM wParam = MAKEWORD(getMouseShift(shift), (WORD)delta);
			LPARAM lParam = MAKELONG(x, y);
			LRESULT res;
			return SUCCEEDED(windowless->OnWindowMessage(WM_MOUSEWHEEL, wParam, lParam, &res));
		}
		return false;
	}
	
public:
	//interface methods
	
	//IUnknown 
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) {
		if (IsEqualGUID(riid, IID_IUnknown))
			*ppvObject = (void*)(this);
		else if (IsEqualGUID(riid, IID_IOleInPlaceSite))
			*ppvObject = (void*)dynamic_cast<IOleInPlaceSite *>(this);
		else if (IsEqualGUID(riid, IID_IOleInPlaceSiteEx))
			*ppvObject = (void*)dynamic_cast<IOleInPlaceSiteEx *>(this);
		else if (IsEqualGUID(riid, IID_IOleInPlaceSiteWindowless))
			*ppvObject = (void*)dynamic_cast<IOleInPlaceSiteWindowless *>(this);
		else if (IsEqualGUID(riid, IID_IOleInPlaceFrame))
			*ppvObject = (void*)dynamic_cast<IOleInPlaceFrame *>(this);
		else if (IsEqualGUID(riid, IID_IStorage))
			*ppvObject = (void*)dynamic_cast<IStorage *>(this);
		else if (IsEqualGUID(riid, ShockwaveFlashObjects::DIID__IShockwaveFlashEvents))
			*ppvObject = (void*)dynamic_cast<ShockwaveFlashObjects::_IShockwaveFlashEvents *>(this);
		else
		{
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
		if (!(*ppvObject))
			return E_NOINTERFACE; //if dynamic_cast returned 0
		refCount++;
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef() {
		refCount++;
		return refCount;
	}

	ULONG STDMETHODCALLTYPE Release() {
		refCount--;
		return refCount;
	}

	//IOleClientSite
	virtual HRESULT STDMETHODCALLTYPE SaveObject() { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER FAR* ppContainer) { *ppContainer = 0; return E_NOINTERFACE; }
	virtual HRESULT STDMETHODCALLTYPE ShowObject() { return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout() { return E_NOTIMPL; }

	//IOleInPlaceSite
	virtual HRESULT STDMETHODCALLTYPE GetWindow(HWND FAR* lphwnd) {	return E_FAIL; }
	virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate() { return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate() { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE OnUIActivate() { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE GetWindowContext(LPOLEINPLACEFRAME FAR* lplpFrame,LPOLEINPLACEUIWINDOW FAR* lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtent) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate() { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE DiscardUndoState() { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo() { return E_NOTIMPL;} 
	virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect) { return S_OK; }

	//IOleInPlaceSiteEx
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivateEx(BOOL __RPC_FAR *pfNoRedraw, DWORD dwFlags) {
		if (pfNoRedraw)
			*pfNoRedraw = FALSE;
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivateEx(BOOL fNoRedraw) { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE RequestUIActivate(void) { return S_FALSE; }

	//IOleInPlaceSiteWindowless
	virtual HRESULT STDMETHODCALLTYPE CanWindowlessActivate(void) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE GetCapture(void) { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE SetCapture(/* [in] */ BOOL fCapture) { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE GetFocus(void) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE SetFocus(/* [in] */ BOOL fFocus) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE GetDC(/* [in] */ LPCRECT pRect, /* [in] */ DWORD grfFlags, /* [out] */ HDC __RPC_FAR *phDC) {	return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE ReleaseDC(/* [in] */ HDC hDC) { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE InvalidateRect(/* [in] */ LPCRECT pRect, /* [in] */ BOOL fErase) {
		if (pRect) {
			RECT u;
			updateFlag = UnionRect(&u, &updateRect, pRect) != 0;
			updateRect = u;
			static ttstr eventName(TJS_W("onFrameUpdate"));
			TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_IDLE | TVP_EPT_REMOVE_POST, 0, NULL);
		}
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE InvalidateRgn(/* [in] */ HRGN hRGN, /* [in] */ BOOL fErase) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE ScrollRect( 
		/* [in] */ INT dx,
		/* [in] */ INT dy,
		/* [in] */ LPCRECT pRectScroll,
		/* [in] */ LPCRECT pRectClip) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE AdjustRect( /* [out][in] */ LPRECT prc) { return S_FALSE; }
	virtual HRESULT STDMETHODCALLTYPE OnDefWindowMessage( 
		/* [in] */ UINT msg,
		/* [in] */ WPARAM wParam,
        /* [in] */ LPARAM lParam,
		/* [out] */ LRESULT __RPC_FAR *plResult) { return S_FALSE; }

	//IOleInPlaceFrame
//	virtual HRESULT STDMETHODCALLTYPE GetWindow(HWND FAR* lphwnd);
//	virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode);
	virtual HRESULT STDMETHODCALLTYPE GetBorder(LPRECT lprectBorder) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS pborderwidths) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS pborderwidths) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU hmenuShared) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR pszStatusText) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg, WORD wID) { return E_NOTIMPL; }

	//IStorage
	virtual HRESULT STDMETHODCALLTYPE CreateStream(const WCHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE OpenStream(const WCHAR * pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE CreateStorage(const WCHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage **ppstg) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE OpenStorage(const WCHAR * pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE CopyTo(DWORD ciidExclude, IID const *rgiidExclude, SNB snbExclude,IStorage *pstgDest) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE MoveElementTo(const OLECHAR *pwcsName,IStorage * pstgDest, const OLECHAR *pwcsNewName, DWORD grfFlags) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE Revert() { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE EnumElements(DWORD reserved1, void * reserved2, DWORD reserved3, IEnumSTATSTG ** ppenum) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE DestroyElement(const OLECHAR *pwcsName) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE RenameElement(const WCHAR *pwcsOldName, const WCHAR *pwcsNewName) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetElementTimes(const WCHAR *pwcsName, FILETIME const *pctime, FILETIME const *patime, FILETIME const *pmtime) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE SetClass(REFCLSID clsid) { return S_OK; }
	virtual HRESULT STDMETHODCALLTYPE SetStateBits(DWORD grfStateBits, DWORD grfMask)  { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG * pstatstg, DWORD grfStatFlag)  { return E_NOTIMPL; }

	//IDispatch
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
        /* [out] */ UINT __RPC_FAR *pctinfo) { return E_NOTIMPL; }
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
		/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
        /* [in] */ REFIID riid,
        /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
        /* [in] */ UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ DISPID __RPC_FAR *rgDispId) { return E_NOTIMPL; }
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( 
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
        /* [out] */ VARIANT __RPC_FAR *pVarResult,
        /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
		/* [out] */ UINT __RPC_FAR *puArgErr) { 

		// _IShockwaveFlashEvents用に実装

		int      argc = pDispParams->cArgs;
		VARIANT *rargv = pDispParams->rgvarg;

		switch (dispIdMember) {
		case DISPID_READYSTATECHANGE:
			if (argc >= 1) {
				tTJSVariant param;
				storeVariant(param, rargv[argc-1]);
				static ttstr eventName(TJS_W("onReadyStateChange"));
				TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
				return S_OK;
			}
			break;
		case 0x7a6:
			if (argc >= 1) {
				tTJSVariant param;
				storeVariant(param, rargv[argc-1]);
				static ttstr eventName(TJS_W("onProgress"));
				TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
				return S_OK;
			}
			break;
		case 0x96:
			if (argc >= 2) {
				tTJSVariant params[2];
				storeVariant(params[0], rargv[argc-1]);
				storeVariant(params[1], rargv[argc-2]);
				static ttstr eventName(TJS_W("onFSCommand"));
				TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 2, params);
				return S_OK;
			}
			break;
		case 0xc5:
			// External API
			if (argc >= 1) {
				tTJSVariant param;
				storeVariant(param, rargv[argc-1]);
				std::wstring result;
				ttstr xml = param;
				if (invokeXML(result, objthis, (tjs_char*)xml.c_str())) {
					control->SetReturnValue(result.c_str());
					lastTJSErrorMsg = "";
				} else {
					lastTJSErrorMsg = result.c_str();
				}
				return S_OK;
			}
			break;
		default:
			return DISP_E_MEMBERNOTFOUND;
		}
		return NOERROR;
	}

protected:

	/**
	 * コントロール生成
	 * @param width  コントロール横幅
	 * @param height コントロール縦幅
	 * @return コントロールの生成に成功したかどうか
	 */
	BOOL Create(int width, int height) {
		if (FAILED(OleCreate(ShockwaveFlashObjects::CLSID_ShockwaveFlash, IID_IOleObject, OLERENDER_DRAW,
							 0, (IOleClientSite *)this, (IStorage *)this, (void **)&oleobj))) {
			return FALSE;
		}
		if (FAILED(OleSetContainedObject(oleobj, TRUE))) {
			return FALSE;
		}
		if (FAILED(oleobj->QueryInterface(IID_IViewObjectEx, (void **)&viewObjectEx))) {
			viewObjectEx = NULL;
			if (FAILED(oleobj->QueryInterface(IID_IViewObject, (void **)&viewObject))) {
				return FALSE;
			}
		}
		if (FAILED(oleobj->QueryInterface(IID_IOleInPlaceObjectWindowless, (void **)&windowless))) {
			return FALSE;
		}
		if (FAILED(oleobj->QueryInterface(__uuidof(ShockwaveFlashObjects::IShockwaveFlash), (void **)&control))) {
			return FALSE;
		}
		if (FAILED(control->QueryInterface(IID_IConnectionPointContainer, (void**)&conContainer))) {
			return FALSE;
		}
		if (FAILED(conContainer->FindConnectionPoint(ShockwaveFlashObjects::DIID__IShockwaveFlashEvents, &conPoint))) {
			return FALSE;
		}
		if (FAILED(conPoint->Advise((ShockwaveFlashObjects::_IShockwaveFlashEvents *)this, &conPointId))) {
			return FALSE;
		}

		control->PutWMode(L"transparent");
		setSize(width, height);
		
		if (FAILED(oleobj->DoVerb(OLEIVERB_SHOW, NULL, (IOleClientSite *)this, 0, NULL, NULL))) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * シフト状態処理用
	 */
	WORD getMouseShift(int shift) {
		return mousemap | ((shift & TVP_SS_SHIFT) ? MK_SHIFT : 0) | ((shift & TVP_SS_CTRL) ? MK_CONTROL : 0);
	}
	
private:
	iTJSDispatch2 *objthis; ///< 自己オブジェクト情報の参照
	int refCount; ///< リファレンスカウント
	
	IOleObject *oleobj;
	ShockwaveFlashObjects::IShockwaveFlash *control;
	IViewObject *viewObject;
	IViewObjectEx *viewObjectEx;
	IOleInPlaceObjectWindowless *windowless;

	IConnectionPointContainer *conContainer;
	IConnectionPoint *conPoint;
	DWORD conPointId;

	int width;
	int height;
	int pitch;
	RECT size;
	RECT updateRect;
	bool updateFlag;
	HDC hdc;
	HBITMAP bmp;
	BYTE *pixels;

	WORD mousemap; //< マウス入力状態
	ttstr lastTJSErrorMsg;; //< 最後の吉里吉里呼び出しエラー
};

NCB_REGISTER_CLASS(FlashPlayer) {
	Factory(&ClassT::factory);
	NCB_METHOD(clearMovie);
	NCB_METHOD(initMovie);
	NCB_METHOD(setSize);
	NCB_METHOD(hitTest);
	RawCallback(TJS_W("draw"), &Class::draw, 0);
	NCB_METHOD(doKeyDown);
	NCB_METHOD(doKeyUp);
	NCB_METHOD(doMouseEnter);
	NCB_METHOD(doMouseLeave);
	NCB_METHOD(doMouseDown);
	NCB_METHOD(doMouseMove);
	NCB_METHOD(doMouseUp);
	NCB_METHOD(doMouseWheel);

	NCB_PROPERTY_RO(readyState, getReadyState);
	NCB_PROPERTY_RO(totalFrames, getTotalFrames);
	NCB_PROPERTY(playing, getPlaying, setPlaying);
	NCB_PROPERTY(quality, getQuality, setQuality);
	NCB_PROPERTY(scaleMode, getScaleMode, setScaleMode);
	NCB_PROPERTY(alighMode, getAlignMode, setAlignMode);
	NCB_PROPERTY(backgroundColor, getBackgroundColor, setBackgroundColor);
	NCB_PROPERTY(loop, getLoop, setLoop);
	NCB_PROPERTY(movie, getMovie, setMovie);
	NCB_PROPERTY(frameNum, getFrameNum, setFrameNum);

	NCB_METHOD(setZoomRect);
	NCB_METHOD(zoom);
	NCB_METHOD(pan);
	NCB_METHOD(play);
	NCB_METHOD(stop);
	NCB_METHOD(back);
	NCB_METHOD(forward);
	NCB_METHOD(rewind);
	NCB_METHOD(stopPlay);
	NCB_METHOD(gotoFrame);

	NCB_PROPERTY_RO(currentFrame, getCurrentFrame);
	NCB_METHOD(isPlaying);
	NCB_PROPERTY_RO(percentLoaded, getPercentLoaded);

	NCB_METHOD(getFrameLoaded);
	NCB_PROPERTY_RO(flashVersion, getFlashVersion);
	NCB_PROPERTY(sAlign, getSAlign, setSAlign);
		
	NCB_PROPERTY(menu, getMenu, setMenu);
	NCB_PROPERTY(base, getBase, setBase);
	NCB_PROPERTY(scale, getScale, setScale);
	NCB_PROPERTY(deviceFont, getDeviceFont, setDeviceFont);
	NCB_PROPERTY(embedMovie, getEmbedMovie, setEmbedMovie);
	NCB_PROPERTY(bgColor, getBgColor, setBgColor);
	NCB_PROPERTY(quality2, getQuality2, setQuality2);

	NCB_METHOD(loadMovie);

	NCB_METHOD(tGotoFrame);
	NCB_METHOD(tGotoLabel);
	NCB_METHOD(tCurrentFrame);
	NCB_METHOD(tCurrentLabel);
	NCB_METHOD(tPlay);
	NCB_METHOD(tStopPlay);
	NCB_METHOD(setVariable);
	NCB_METHOD(getVariable);
	NCB_METHOD(tSetProperty);
	NCB_METHOD(tGetProperty);
	NCB_METHOD(tCallFrame);
	NCB_METHOD(tCallLabel);
	NCB_METHOD(tSetPropertyNum);
	NCB_METHOD(tGetPropertyNum);

	NCB_PROPERTY(swRemote, getSWRemote, setSWRemote);
	NCB_PROPERTY(flashVars, getFlashVars, setFlashVars);
	NCB_PROPERTY(allowScriptAccess, getAllowScriptAccess, setAllowScriptAccess);
	
	NCB_PROPERTY(movieData, getMovieData, setMovieData);
	NCB_PROPERTY(seamlessTabbing, getSeamlessTabbing, setSeamlessTabbing);

	NCB_METHOD(enforceLocalSecurity);

	NCB_PROPERTY(profile, getProfile, setProfile);
	NCB_PROPERTY(profileAddress, getProfileAddress, setProfileAddress);
	NCB_PROPERTY(profilePort, getProfilePort, setProfilePort);

	NCB_METHOD(disableLocalSecurity);

	RawCallback("callFunction", &ClassT::callFunction, 0);
	RawCallback("getLastTJSError", &ClassT::getLastTJSError, 0);
};

//---------------------------------------------------------------------------

static BOOL gOLEInitialized = false;

/**
 * 登録処理前
 */
static void PreRegistCallback()
{
	showXMLCopyright();

	if (!gOLEInitialized) {
		if (SUCCEEDED(OleInitialize(NULL))) {
			gOLEInitialized = true;
		} else {
			TVPAddLog(L"OLE 初期化失敗");
		}
	}
}

/**
 * 開放処理後
 */
static void PostUnregistCallback()
{
	if (gOLEInitialized) {
		OleUninitialize();
		gOLEInitialized = false;
	}
}


NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
