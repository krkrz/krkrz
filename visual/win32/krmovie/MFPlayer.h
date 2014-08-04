
#ifndef __MF_PLAYER_H__
#define __MF_PLAYER_H__

// Media Foundation Player

class tTVPPlayerCallback : public IMFPMediaPlayerCallback, public IMFAsyncCallback, public CUnknown {
	class tTVPMFPlayer* owner_;
public:
	tTVPPlayerCallback( class tTVPMFPlayer* owner ) : CUnknown(L"PlayerCallback",NULL), owner_(owner) {}

	// IUnknown
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

	void STDMETHODCALLTYPE OnMediaPlayerEvent( MFP_EVENT_HEADER *pEventHeader );

	STDMETHODIMP GetParameters( DWORD *pdwFlags, DWORD *pdwQueue );
	STDMETHODIMP Invoke( IMFAsyncResult *pAsyncResult );
};

class tTVPMFPlayer : public iTVPVideoOverlay
{
protected:
	ULONG		RefCount;
	HWND		OwnerWindow;
	bool		Visible;
	bool		Shutdown;
	RECT		Rect;
	HWND		CallbackWindow;
	bool		HasVideo;
	float		PlayRate;
	float		ZoomLevel;	// (1.0 == 100%)

	UINT32		FPSNumerator;
	UINT32		FPSDenominator;

	MFP_MEDIAITEM_CHARACTERISTICS	MediaIteamCap;

	tTVPPlayerCallback* PlayerCallback;
    IMFPMediaPlayer*	MediaPlayer;
	IMFByteStream*		ByteStream;
	IMFPMediaItem*		MediaItem;
	IMFVideoDisplayControl*	VideoDisplayControl;

	IMFMediaSession*	MediaSession;
	IMFTopology*		Topology;

	//tTVPAudioSessionVolume*	AudioVolume;
	//tTVPMFVideoMediaSink*	VideoStreamSink;

	tTVPVideoStatus		VideoStatue;
	std::wstring		StreamName;
	IStream*			Stream;

protected:
	void __stdcall ReleaseAll();

	HRESULT AddSourceNode( IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, IMFStreamDescriptor *pSD, IMFTopologyNode **ppNode );
	//HRESULT AddOutputNode( IMFTopology *pTopology, IMFStreamSink *pStreamSink, IMFTopologyNode **ppNode );
	HRESULT AddOutputNode( IMFTopology *pTopology, IMFActivate *pActivate, DWORD dwId, IMFTopologyNode **ppNode );

	HRESULT AddBranchToPartialTopology( IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, DWORD iStream, HWND hVideoWnd );
	HRESULT CreateMediaSinkActivate( IMFStreamDescriptor *pSourceSD, HWND hVideoWindow, IMFActivate **ppActivate );

	HRESULT CreateVideoPlayer( HWND hWnd );
public:
	IMFMediaSession* GetMediaSession() { return MediaSession; }

	// from IMFPMediaPlayerCallback
	void NotifyError( HRESULT hr );
	void NotifyState( MFP_MEDIAPLAYER_STATE state );

	void OnMediaItemCreated( MFP_MEDIAITEM_CREATED_EVENT* event );
	void OnMediaItemSet( MFP_MEDIAITEM_SET_EVENT* event );
	void OnRateSet( MFP_RATE_SET_EVENT* event );
	void OnPlayBackEnded( MFP_PLAYBACK_ENDED_EVENT* event );
	void OnStop( MFP_STOP_EVENT* event );
	void OnPlay( MFP_PLAY_EVENT* event );
	void OnPause( MFP_PAUSE_EVENT* event );
	void OnPositionSet( MFP_POSITION_SET_EVENT* event );
	void OnFremeStep( MFP_FRAME_STEP_EVENT* event );
	void OnMediaItemCleared( MFP_MEDIAITEM_CLEARED_EVENT* event );
	void OnMF( MFP_MF_EVENT* event );
	void OnError( MFP_ERROR_EVENT* event );
	void OnAcquireUserCredential( MFP_ACQUIRE_USER_CREDENTIAL_EVENT* event );

	void OnTopologyStatus(UINT32 status);
public:
	tTVPMFPlayer();
	virtual ~tTVPMFPlayer();

	virtual void __stdcall BuildGraph( HWND callbackwin, IStream *stream,
		const wchar_t * streamname, const wchar_t *type, unsigned __int64 size );

	virtual void __stdcall AddRef();
	virtual void __stdcall Release();

	virtual void __stdcall SetWindow(HWND window);
	virtual void __stdcall SetMessageDrainWindow(HWND window);
	virtual void __stdcall SetRect(RECT *rect);
	virtual void __stdcall SetVisible(bool b);
	virtual void __stdcall Play();
	virtual void __stdcall Stop();
	virtual void __stdcall Pause();
	virtual void __stdcall SetPosition(unsigned __int64 tick);
	virtual void __stdcall GetPosition(unsigned __int64 *tick);
	virtual void __stdcall GetStatus(tTVPVideoStatus *status);
	virtual void __stdcall GetEvent(long *evcode, long *param1, long *param2, bool *got);

	virtual void __stdcall FreeEventParams(long evcode, long param1, long param2);

	virtual void __stdcall Rewind();
	virtual void __stdcall SetFrame( int f );
	virtual void __stdcall GetFrame( int *f );
	virtual void __stdcall GetFPS( double *f );
	virtual void __stdcall GetNumberOfFrame( int *f );
	virtual void __stdcall GetTotalTime( __int64 *t );
	
	virtual void __stdcall GetVideoSize( long *width, long *height );
	virtual void __stdcall GetFrontBuffer( BYTE **buff );
	virtual void __stdcall SetVideoBuffer( BYTE *buff1, BYTE *buff2, long size );

	virtual void __stdcall SetStopFrame( int frame );
	virtual void __stdcall GetStopFrame( int *frame );
	virtual void __stdcall SetDefaultStopFrame();

	virtual void __stdcall SetPlayRate( double rate );
	virtual void __stdcall GetPlayRate( double *rate );

	virtual void __stdcall SetAudioBalance( long balance );
	virtual void __stdcall GetAudioBalance( long *balance );
	virtual void __stdcall SetAudioVolume( long volume );
	virtual void __stdcall GetAudioVolume( long *volume );

	virtual void __stdcall GetNumberOfAudioStream( unsigned long *streamCount );
	virtual void __stdcall SelectAudioStream( unsigned long num );
	virtual void __stdcall GetEnableAudioStreamNum( long *num );
	virtual void __stdcall DisableAudioStream( void );

	virtual void __stdcall GetNumberOfVideoStream( unsigned long *streamCount );
	virtual void __stdcall SelectVideoStream( unsigned long num );
	virtual void __stdcall GetEnableVideoStreamNum( long *num );

	virtual void __stdcall SetMixingBitmap( HDC hdc, RECT *dest, float alpha );
	virtual void __stdcall ResetMixingBitmap();

	virtual void __stdcall SetMixingMovieAlpha( float a );
	virtual void __stdcall GetMixingMovieAlpha( float *a );
	virtual void __stdcall SetMixingMovieBGColor( unsigned long col );
	virtual void __stdcall GetMixingMovieBGColor( unsigned long *col );

	virtual void __stdcall PresentVideoImage();

	virtual void __stdcall GetContrastRangeMin( float *v );
	virtual void __stdcall GetContrastRangeMax( float *v );
	virtual void __stdcall GetContrastDefaultValue( float *v );
	virtual void __stdcall GetContrastStepSize( float *v );
	virtual void __stdcall GetContrast( float *v );
	virtual void __stdcall SetContrast( float v );

	virtual void __stdcall GetBrightnessRangeMin( float *v );
	virtual void __stdcall GetBrightnessRangeMax( float *v );
	virtual void __stdcall GetBrightnessDefaultValue( float *v );
	virtual void __stdcall GetBrightnessStepSize( float *v );
	virtual void __stdcall GetBrightness( float *v );
	virtual void __stdcall SetBrightness( float v );

	virtual void __stdcall GetHueRangeMin( float *v );
	virtual void __stdcall GetHueRangeMax( float *v );
	virtual void __stdcall GetHueDefaultValue( float *v );
	virtual void __stdcall GetHueStepSize( float *v );
	virtual void __stdcall GetHue( float *v );
	virtual void __stdcall SetHue( float v );

	virtual void __stdcall GetSaturationRangeMin( float *v );
	virtual void __stdcall GetSaturationRangeMax( float *v );
	virtual void __stdcall GetSaturationDefaultValue( float *v );
	virtual void __stdcall GetSaturationStepSize( float *v );
	virtual void __stdcall GetSaturation( float *v );
	virtual void __stdcall SetSaturation( float v );
};



#endif // __MF_PLAYER_H__
