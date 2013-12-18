

#ifndef __WMV_ENCODER_H__
#define __WMV_ENCODER_H__

//#include <strsafe.h>

#include <windows.h>
#include <wmsdk.h>
#include <list>
#include <atlcomcli.h>
#include "TSimpleList.h"

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE( x )			\
		if( NULL != x ) {				\
			x->Release();				\
			x = NULL;					\
		}
#endif // SAFE_RELEASE

#ifndef SAFE_ARRAYDELETE
#	define SAFE_ARRAYDELETE( x )		\
		if( x ) {						\
			delete [] x;				\
			x = NULL;					\
       }
#endif //SAFE_ARRAYDELETE

#ifndef SAFE_CLOSEFILEHANDLE
#	define SAFE_CLOSEFILEHANDLE( h )		\
		if( INVALID_HANDLE_VALUE != h ) {	\
			CloseHandle( h );				\
			h = INVALID_HANDLE_VALUE;		\
		}
#endif //SAFE_CLOSEFILEHANDLE



class CWMInput
{
public:
	CWMInput();
	~CWMInput(){ Cleanup(); }

	bool operator<( const CWMInput & right ) const {
		return( m_qwPresentTime < right.m_qwPresentTime );
	}

	void Cleanup();

	void SetVideoSource( int width, int height, DWORD scale, DWORD rate );
	void SetAudioSource( WAVEFORMATEX* waveex, size_t samplesize  );

	DWORD			m_dwRate;
	DWORD			m_dwScale;

	QWORD			m_qwPresentTime;	// in 100 ns
	GUID			m_Type;
	DWORD			m_dwInput;
	WM_MEDIA_TYPE	m_Mt;
	WAVEFORMATEX	*m_pWFX;
//	DWORD			m_dwSamples;
	DWORD			m_dwCurrentSample;
	WCHAR			*m_pwszConnectionName;

	bool			m_Enable;
};

struct InputSample
{
	bool	is_video_;
	QWORD	tick_;		// 100ナノ秒単位
	void*	sample_;
	size_t	sample_size_;

	InputSample( QWORD tick, const void* sample, size_t size, bool video = true )
	: is_video_(video), tick_(tick), sample_size_(size)
	{
		sample_ = new char[size];
		memcpy( sample_, sample, size );
	}
	~InputSample() {
		if( sample_ ) {
			delete[] sample_;
		}
	}
};

class CProfileStreams;

////////////////////////////////////////////////////////////////////////////////
class CWMVEncoder
{
public:
	CWMVEncoder()
	: audio_enable_(false), encoder_running_(false), video_quality_(50), video_sec_per_key_(5)
	, video_width_(640), video_height_(480), video_scale_(1), video_rate_(30)
	{}

	~CWMVEncoder() { ReleaseAll(); }

	//! プロファイルをファイルから読み込む版
	HRESULT Initial( const WCHAR* pwszOutFile, LPCTSTR ptszProfileFile, int width, int height,
					DWORD scale, DWORD rate, WAVEFORMATEX* waveex, size_t samplesize  );

	//! プロファイルをパラメータから生成する版
	HRESULT Initial( const WCHAR* pwszOutFile );

	HRESULT Start();
	HRESULT Stop();

	void WriteVideoSample( void* sample, size_t sample_size, QWORD tick );
	void WriteAudioSample( void* sample, size_t sample_size, QWORD tick );

	void SetVideoQuality( DWORD q ) {
		if( encoder_running_ ) return;
		if( q <= 100 ) video_quality_ = q;
		}
	DWORD GetVideoQuality() const { return video_quality_; }

	void SetMaxKeyFrameSpacing( DWORD sec ) {
		if( encoder_running_ ) return;
		video_sec_per_key_ = sec;
	}
	DWORD GetMaxKeyFrameSpacing() const { return video_sec_per_key_; }

	void SetVideoWidth( int w ) {
		if( encoder_running_ ) return;
		video_width_ = w;
	}
	int GetVideoWidth() const { return video_width_; }

	void SetVideoHeight( int h ) {
		if( encoder_running_ ) return;
		video_height_ = h;
	}
	int GetVideoHeight() const { return video_height_; }

	void SetVideoScale( DWORD s ) {
		if( encoder_running_ ) return;
		if( s > 0 ) video_scale_ = s;
	}
	DWORD GetVideoScale() const { return video_scale_; }

	// レートを設定する
	void SetVideoRate( DWORD r ) {
		if( encoder_running_ ) return;
		if( r > 0 ) video_rate_ = r;
	}
	// レートを取得する
	DWORD GetVideoRate() const { return video_rate_; }

protected:
	HRESULT LoadCustomProfile( LPCTSTR ptszProfileFile );

	HRESULT UpdateWriterInputs();
	HRESULT WriteSample( CWMInput* pInput, void* sample, size_t sample_size );

	HRESULT UpdateProfile( IWMProfile* pProfile );
	HRESULT CreateProfileStreamList( IWMProfile* pProfile, CTSimpleList<CProfileStreams>* pProfStreamList );

	HRESULT AddVideoStream( IWMProfile* pIWMProfile, WMVIDEOINFOHEADER* pInputVIH, WORD* pwStreamNum, DWORD dwQuality, DWORD dwSecPerKey, WCHAR** pwszConnectionName );
	HRESULT AddAudioStream( IWMProfile* pIWMProfile, DWORD dwSampleRate, DWORD dwChannels, WORD wBitsPerSample, WORD* pwStreamNum, WCHAR** pwszConnectionName );
	HRESULT SetStreamBasics( IWMStreamConfig* pIWMStreamConfig, IWMProfile* pIWMProfile, LPWSTR pwszStreamName, LPWSTR pwszConnectionName, DWORD dwBitrate, WM_MEDIA_TYPE* pmt );
	HRESULT CreateEmptyProfile( IWMProfile** ppIWMProfile );

    HRESULT SaveProfile( LPCTSTR ptszFileName, IWMProfile* pIWMProfile );

	void ReleaseAll();

	CComPtr<IWMWriter>	m_WMWriter;
	CComPtr<IWMProfile>	m_WMProfile;

	bool			audio_enable_;		// オーディオの有効/無効
	bool			encoder_running_;	// エンコード実行中かどうか

	CWMInput		video_input_;
	CWMInput		audio_input_;

	DWORD			video_quality_;		// クオリティ、0 - 100 ( default 50 )
	DWORD			video_sec_per_key_;	// 最大キーフレーム間隔 ( default 5 )
	int				video_width_;		// 画像幅 ( default 640 )
	int				video_height_;		// 画像高さ ( default 480 )
	DWORD			video_scale_;		//
	DWORD			video_rate_;		//
};

////////////////////////////////////////////////////////////////////////////////
BOOL CompareMediaTypes( const WM_MEDIA_TYPE * pMedia1, const WM_MEDIA_TYPE * pMedia2);

////////////////////////////////////////////////////////////////////////////////
class CProfileStreams
{
public:
	CProfileStreams()
	 : m_wStreamNum( 0 ), m_pMt( NULL ), m_pwszConnectionName( NULL ) {
		ZeroMemory( &m_Type, sizeof( GUID ) );
	}

	CProfileStreams( GUID Type, WORD dwStreamNum, WM_MEDIA_TYPE * pMt, WCHAR * wszConnectionName )
	 : m_Type( Type ), m_wStreamNum( dwStreamNum ), m_pwszConnectionName( wszConnectionName ), m_pMt( pMt ) {
	}

	~CProfileStreams() { /*Cleanup();*/ }

	void Cleanup() {
		SAFE_ARRAYDELETE( m_pMt );
		SAFE_ARRAYDELETE( m_pwszConnectionName );
	}

	// Used by TSimpleList Find() to compare streams by stream type
	BOOL operator == ( const CWMInput & right ) {
		return( m_Type == right.m_Type && CompareMediaTypes( m_pMt, &right.m_Mt ) ); 
	}

	GUID			m_Type;
	WORD			m_wStreamNum;
	WM_MEDIA_TYPE*	m_pMt;
	WCHAR*			m_pwszConnectionName;
};

#endif // __WMV_ENCODER_H__

