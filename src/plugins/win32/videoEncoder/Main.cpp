
#include <tchar.h>
#include "ncbind/ncbind.hpp"

#pragma comment (lib, "quartz.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "wmvcore.lib")
//#pragma comment (lib, "dsound.lib")
//#pragma comment (lib, "dxguid.lib")
// comctl32.lib dxerr.lib winmm.lib d3dx9d.lib dsound.lib dxguid.lib odbc32.lib odbccp32.lib

#include "DShowException.h"
#include "WMVEncoder.h"
#include <vector>

#define SAFE_DELETE( obj ) if( obj ) { delete obj; obj = NULL; }


/**
 * ビデオエンコーダー
 */
class videoEncoder {
	iTJSDispatch2*	objthis_;		///< 自己オブジェクト情報の参照
	CWMVEncoder*	wmv_encoder_;	///< WMV エンコーダー

	DWORD			video_quality_;		///< クオリティ、0 - 100 ( default 50 )
	DWORD			video_sec_per_key_;	///< 最大キーフレーム間隔 ( default 5 )
	long			video_width_;		///< 画像幅 ( default 640 )
	long			video_height_;		///< 画像高さ ( default 480 )
	DWORD			video_scale_;		///< 時間スケール
	DWORD			video_rate_;		///< 時間レート
	QWORD			video_tick_per_frame_;	///< 1フレームの tick
	QWORD			video_tick_;		///< 現在のフレームの tick

public:
	/**
	 * コンストラクタ
	 */
	videoEncoder( iTJSDispatch2 *objthis )
	 : objthis_(objthis), wmv_encoder_(NULL)
	 , video_quality_(50), video_sec_per_key_(5), video_width_(640), video_height_(480), video_scale_(1), video_rate_(30)
	 , video_tick_(0), video_tick_per_frame_(0)
	 {}
	/**
	 * デストラクタ
	 */
	virtual ~videoEncoder() { close(); }

	/**
	 * インスタンス生成ファクトリ
	 */
	static tjs_error factory(videoEncoder **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		videoEncoder *self = new videoEncoder(objthis);
		if( self ) {
			*result = self;
			return TJS_S_OK;
		} else {
			return TJS_E_FAIL;
		}
	}
	long getVideoQuality() const { return video_quality_; }
	void setVideoQuality( long q ) { video_quality_ = q; }
	long getSecondPerKey() const { return video_sec_per_key_; }
	void setSecondPerKey( long s ) { video_sec_per_key_ = s; }
	long getVideoTimeScale() const { return video_scale_; }
	void setVideoTimeScale( long s ) { video_scale_ = s; }
	long getVideoTimeRate() const { return video_rate_; }
	void setVideoTimeRate( long r ) { video_rate_ = r; }
	long getVideoWidth() const { return video_width_; }
	void setVideoWidth( long w ) { video_width_ = w; }
	long getVideoHeight() const { return video_height_; }
	void setVideoHeight( long h ) { video_height_ = h; }


#define CHECK_ENCODER if(!wmv_encoder_){TVPThrowExceptionMessage(L"not opened");}

	bool open( const tjs_char *filename ) {
		if( wmv_encoder_ ) delete wmv_encoder_;
		wmv_encoder_ = new CWMVEncoder();

		video_tick_ = 0;
		video_tick_per_frame_ = 10000000LL * (QWORD)getVideoTimeScale()/ (QWORD)getVideoTimeRate();
		wmv_encoder_->SetVideoQuality( getVideoQuality() );
		wmv_encoder_->SetMaxKeyFrameSpacing( getSecondPerKey() );
		wmv_encoder_->SetVideoWidth( getVideoWidth() );
		wmv_encoder_->SetVideoHeight( getVideoHeight() );
		wmv_encoder_->SetVideoScale( getVideoTimeScale() );
		wmv_encoder_->SetVideoRate( getVideoTimeRate() );
		HRESULT hr = S_OK;
		if( FAILED( hr = wmv_encoder_->Initial( filename ) ) ) {
			SAFE_DELETE( wmv_encoder_ );
			return false;
		}
		if( FAILED( hr = wmv_encoder_->Start() ) ) {
			SAFE_DELETE( wmv_encoder_ );
			return false;
		}
		return true;
	}
	void close() {
		if( wmv_encoder_ ) {
			wmv_encoder_->Stop();

			SAFE_DELETE( wmv_encoder_ );
		}
	}
	/**
	 * レイヤをエンコード
	 * @param layer レイヤ
	 */
	void EncodeVideoSample( iTJSDispatch2 *layer ) {
		CHECK_ENCODER

		ncbPropAccessor obj(layer);
		const int imageWidth = obj.getIntValue(L"imageWidth");
		const int imageHeight= obj.getIntValue(L"imageHeight");
		// レイヤーサイズチェック
		if( imageWidth != video_width_ || imageHeight != video_height_ ) {
			TVPThrowExceptionMessage( L"invalid layer size");
		}

		const unsigned char* imageBuffer = (const unsigned char*)obj.GetValue(L"mainImageBuffer", ncbTypedefs::Tag<tjs_int>());
		if( imageBuffer == NULL ) {
			TVPThrowExceptionMessage( L"layer image is NULL");
		}
		const tjs_int imagePitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());

		// 吉里吉里のバッファは DIB と同じ構造なのでこの処理で通る
		int size = imageHeight * -imagePitch;
		const unsigned char* buffer = imageBuffer + (imageHeight-1) * imagePitch;

		// tick は 100ナノ秒単位に
		wmv_encoder_->WriteVideoSample( (void*)buffer, size, video_tick_ );

		video_tick_ += video_tick_per_frame_;
	}

	static tjs_error encodeVideoSample( tTJSVariant *result, tjs_int numparams, tTJSVariant **params, videoEncoder *self ) {
		if( numparams < 1 ) {
			return TJS_E_BADPARAMCOUNT;
		}
		if( params[0]->Type() != tvtObject || params[0]->AsObjectNoAddRef()->IsInstanceOf(0,NULL,NULL,L"Layer",NULL) == false ) {
			return TJS_E_INVALIDTYPE;
		}
		self->EncodeVideoSample( *params[0] );
		return TJS_S_OK;
	}
};

NCB_REGISTER_CLASS(videoEncoder) {
	Factory(&ClassT::factory);

	NCB_METHOD(open);
	NCB_METHOD(close);
	RawCallback(TJS_W("encodeVideoSample"), &Class::encodeVideoSample, 0);

	NCB_PROPERTY(videoQuality, getVideoQuality, setVideoQuality);
	NCB_PROPERTY(secondPerKey, getSecondPerKey, setSecondPerKey);
	NCB_PROPERTY(videoTimeScale, getVideoTimeScale, setVideoTimeScale);
	NCB_PROPERTY(videoTimeRate, getVideoTimeRate, setVideoTimeRate);
	NCB_PROPERTY(videoWidth, getVideoWidth, setVideoWidth);
	NCB_PROPERTY(videoHeight, getVideoHeight, setVideoHeight);
}

// ----------------------------------- 起動・開放処理

/**
 * 登録処理前
 */
void PreRegistCallback()
{
	CoInitialize( NULL );
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
	CoUninitialize();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
