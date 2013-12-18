
#include "WMVEncoder.h"
#include <tchar.h>
//#include <nserror.h>
#include "DShowException.h"
#include "DebugLog.h"

const LPWSTR wszDefaultConnectionName = L"WMVEncoder";

CWMInput::CWMInput()
 : m_dwRate(30), m_dwScale(1), m_qwPresentTime( 0 ), m_dwInput( 0 ), m_pWFX(NULL), m_dwCurrentSample( 0 ), m_Enable(false)
 , m_pwszConnectionName(NULL)
{
	ZeroMemory( &m_Mt, sizeof( m_Mt ) );
}

void CWMInput::Cleanup()
{
	SAFE_ARRAYDELETE( m_Mt.pbFormat );
	SAFE_ARRAYDELETE( m_pwszConnectionName );
	if( m_pWFX ) {
		delete m_pWFX;
		m_pWFX = NULL;
	}
}


void CWMInput::SetAudioSource( WAVEFORMATEX* waveex, size_t samplesize  )
{
	if( !waveex )
		return;

	if( WAVE_FORMAT_PCM != waveex->wFormatTag ) {
		// not supported yet...
		return;
	}

//	m_dwSamples = 0;
	m_dwCurrentSample = 0;

	m_pWFX = new WAVEFORMATEX;
	memcpy( m_pWFX, waveex, sizeof(WAVEFORMATEX) );
	m_pWFX->cbSize = 0;

	m_Mt.majortype				= WMMEDIATYPE_Audio;
	m_Mt.subtype				= WMMEDIASUBTYPE_PCM;
	m_Mt.bFixedSizeSamples		= TRUE;
	m_Mt.bTemporalCompression	= FALSE;
	m_Mt.lSampleSize			= samplesize;
	m_Mt.formattype				= WMFORMAT_WaveFormatEx;
	m_Mt.pUnk					= NULL;
	m_Mt.cbFormat				= sizeof( WAVEFORMATEX );
	m_Mt.pbFormat				= (BYTE *)m_pWFX;
	m_Type						= WMMEDIATYPE_Audio;
	m_Enable = true;
}


void CWMInput::SetVideoSource( int width, int height, DWORD scale, DWORD rate )
{
	m_pWFX = NULL;
	m_dwCurrentSample = 0;

	DWORD cbVideoInfo = sizeof( WMVIDEOINFOHEADER );
	WMVIDEOINFOHEADER* pVideoInfo = new WMVIDEOINFOHEADER;

	int bpp = 32;
	pVideoInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pVideoInfo->bmiHeader.biWidth  = width;
	pVideoInfo->bmiHeader.biHeight = height;
	pVideoInfo->bmiHeader.biPlanes = 1;
	pVideoInfo->bmiHeader.biBitCount = bpp;
	pVideoInfo->bmiHeader.biCompression = BI_RGB;
	pVideoInfo->bmiHeader.biSizeImage = width * height * bpp / 8;
	pVideoInfo->bmiHeader.biXPelsPerMeter = 0;
	pVideoInfo->bmiHeader.biYPelsPerMeter = 0;
	pVideoInfo->bmiHeader.biClrUsed = 0;
	pVideoInfo->bmiHeader.biClrImportant = 0;

	pVideoInfo->rcSource.left   = 0;
	pVideoInfo->rcSource.top    = 0;
	pVideoInfo->rcSource.bottom = height;
	pVideoInfo->rcSource.right  = width;
	pVideoInfo->rcTarget        = pVideoInfo->rcSource;
	pVideoInfo->dwBitRate       = MulDiv( width * height * bpp, rate, scale );
	pVideoInfo->dwBitErrorRate  = 0;
	pVideoInfo->AvgTimePerFrame = 10000000 * (QWORD)scale / (QWORD)rate;

	m_Mt.majortype = WMMEDIATYPE_Video;
	if( bpp == 32 ) {
		m_Mt.subtype = WMMEDIASUBTYPE_RGB32;
	} else if( bpp == 24 ) {
		m_Mt.subtype = WMMEDIASUBTYPE_RGB24;
	}

	m_Mt.bFixedSizeSamples = FALSE;
	m_Mt.bTemporalCompression = FALSE;
	m_Mt.lSampleSize = 0;
	m_Mt.formattype = WMFORMAT_VideoInfo;
	m_Mt.pUnk = NULL;
	m_Mt.cbFormat = cbVideoInfo;
	m_Mt.pbFormat = (BYTE *)pVideoInfo;

	m_Type = WMMEDIATYPE_Video;
	m_Enable = true;
}

//------------------------------------------------------------------------------
// Desc: Loads a custom profile from file.
//------------------------------------------------------------------------------
HRESULT CWMVEncoder::LoadCustomProfile( LPCTSTR ptszProfileFile )
{
	HRESULT hr;

	// Create profile manager
	CComPtr<IWMProfileManager>	pProfileManager;
	if( FAILED( hr = WMCreateProfileManager( &pProfileManager ) ) )
		return hr;

	// Open the profile file
	HANDLE hFile = CreateFile( ptszProfileFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
		return HRESULT_FROM_WIN32( GetLastError() );

	if( FILE_TYPE_DISK != GetFileType( hFile ) ) {
		SAFE_CLOSEFILEHANDLE( hFile );
		return NS_E_INVALID_NAME;
	}

	DWORD dwLength = GetFileSize( hFile, NULL );
	if( -1 == dwLength ) {
		SAFE_CLOSEFILEHANDLE( hFile );
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	// Allocate memory for profile buffer
	LPWSTR pProfile = (WCHAR *)new BYTE[ dwLength + sizeof(WCHAR) ];
	if( NULL == pProfile ) {
		SAFE_CLOSEFILEHANDLE( hFile );
		return E_OUTOFMEMORY;
	}

	// The buffer must be null-terminated.
	memset( pProfile, 0, dwLength + sizeof(WCHAR) );

	// Read the profile to a buffer
	DWORD dwBytesRead = 0;
	if( !ReadFile( hFile, pProfile, dwLength, &dwBytesRead, NULL ) ) {
		SAFE_ARRAYDELETE( pProfile );
		SAFE_CLOSEFILEHANDLE( hFile );
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	// Load the profile from the buffer
	if( FAILED( hr = pProfileManager->LoadProfileByData( pProfile, &m_WMProfile ) ) ) {
		SAFE_ARRAYDELETE( pProfile );
		SAFE_CLOSEFILEHANDLE( hFile );
		return hr;
	}
	// Release all resources
	SAFE_ARRAYDELETE( pProfile );
	SAFE_CLOSEFILEHANDLE( hFile );

	return( hr );
}

// 音は0.5秒分ずつ渡すこと
HRESULT CWMVEncoder::WriteSample( CWMInput* pInput, void* sample, size_t sample_size )
{
	if( NULL == pInput )
		return( E_INVALIDARG );

	LONG cSamples = 0;
	if( WMMEDIATYPE_Audio == pInput->m_Type ) {
		cSamples = ( sample_size * 8 ) /  ( pInput->m_pWFX->nChannels * pInput->m_pWFX->wBitsPerSample );
	} else if( WMMEDIATYPE_Video == pInput->m_Type ) {
		cSamples = 1;
	} else {
		return E_INVALIDARG;
	}

	HRESULT		hr = S_OK;
	LONG cbSample = sample_size;
	CComPtr<INSSBuffer> pSample;
	if( FAILED( hr = m_WMWriter->AllocateSample( cbSample, &pSample ) ) )
		return hr;

	BYTE*		pbBuffer = NULL;
	DWORD		cbBuffer = 0;
	if( FAILED( hr = pSample->GetBufferAndLength( &pbBuffer, &cbBuffer ) ) ) {
		return hr;
	}

	memcpy( pbBuffer, sample, sample_size );

	if( FAILED( hr = pSample->SetLength( cbSample ) ) )
		return hr;

	DWORD dwFlags = 0;
	if( FAILED( hr = m_WMWriter->WriteSample( pInput->m_dwInput, pInput->m_qwPresentTime, dwFlags, pSample ) ) ) {
		if( hr == E_INVALIDARG ) {
			DebugLog2("The dwInputNum value is greater than the highest index number. \n");
		} else if( hr == E_UNEXPECTED ) {
			DebugLog2("The method failed for an unspecified reason.  \n");
//		} else if( hr == NS_E_INVALID_STATE ) {
//			DebugLog2("The writer is not running.  \n");
		} else if( hr == NS_E_INVALID_DATA ) {
			DebugLog2("The sample is not valid. This can occur when an input script stream contains a script sample that is not valid.  \n");
		}  else if( hr == NS_E_INVALID_NUM_PASSES ) {
			DebugLog2("The wrong number of preprocessing passes was used for the stream's output type.  \n");
			DebugLog2("Typically, this error will be returned if the stream configuration requires a preprocessing pass and a sample is passed without first configuring preprocessing. You can check for this error to determine whether a stream requires a preprocessing pass. Preprocessing passes are required only for bit-rate-based VBR. \n");
		} else if( hr == NS_E_LATE_OPERATION ) {
			DebugLog2("The writer has received samples whose presentation times differ by an amount greater than the maximum synchronization tolerance. You can set the synchronization tolerance by callingIWMWriterAdvanced::SetSyncTolerance.  \n");
		}  else if( hr == NS_E_TOO_MUCH_DATA ) {
			DebugLog2("Samples from a real-time source are arriving faster than expected. This error is returned only if IWMWriterAdvanced::SetLiveSource has been called to indicate a live source.  \n");
		}
		return hr;
	}

	pInput->m_dwCurrentSample += cSamples;
#if 0
    // The writer expects presentation times to be in 100-nanosecond units.
	if( WMMEDIATYPE_Audio == pInput->m_Type ) {
		//pInput->m_qwPresentTime += 10000000 * (QWORD)sample_size / pInput->m_pWFX->nAvgBytesPerSec;
		pInput->m_qwPresentTime = 10000000 * (QWORD)pInput->m_dwCurrentSample / pInput->m_pWFX->nSamplesPerSec;
	} else if( WMMEDIATYPE_Video == pInput->m_Type ) {
		pInput->m_qwPresentTime = 10000000 * (QWORD)pInput->m_dwCurrentSample * pInput->m_dwScale / pInput->m_dwRate;
	}
#endif
	return( hr );
}


HRESULT CWMVEncoder::Initial( const WCHAR* pwszOutFile, LPCTSTR ptszProfileFile, int width, int height, DWORD scale, DWORD rate, WAVEFORMATEX* waveex, size_t samplesize  )
{
	if( NULL == pwszOutFile || NULL == ptszProfileFile ) {
		return( E_INVALIDARG );
	}

	video_input_.SetVideoSource( width, height, scale, rate );
	audio_input_.SetAudioSource( waveex, samplesize  );

	HRESULT hr = S_OK;

	if( FAILED( hr = WMCreateWriter( NULL, &m_WMWriter ) ) )
		ThrowDShowException( TJS_W( "Failed to Create WMWriter" ), hr );

	if( FAILED( hr = LoadCustomProfile( ptszProfileFile ) ) )
		ThrowDShowException( TJS_W( "Failed to Load Profile" ), hr );

	if( FAILED( hr = UpdateProfile( m_WMProfile ) ) )
		ThrowDShowException( TJS_W( "Failed to update profile" ), hr );

	if( FAILED( hr = m_WMWriter->SetProfile( m_WMProfile ) ) )
		ThrowDShowException( TJS_W( "Failed to set profile" ), hr );

#if 0
	CComPtr<IWMWriterAdvanced>	writerAdv;
	if( FAILED( hr = m_WMWriter.QueryInterface( &writerAdv ) ) )
		ThrowDShowException( TJS_W( "Failed to query IWMWriterAdvanced" ), hr );

	if( FAILED( hr = writerAdv->SetSyncTolerance( 10000 ) ) )
		ThrowDShowException( TJS_W( "Failed to call IWMWriterAdvanced::SetSyncTolerance" ), hr );
#endif

	if( FAILED( hr = UpdateWriterInputs() ) )
		ThrowDShowException( TJS_W( "Failed to update writer inputs" ), hr );

	// Note: Indexing is automatically enabled when writing the uncompressed 
	// samples. We don't need to set up indexing manually. You can call 
	// IWMWriterFileSink3::SetAutoIndexing( FALSE ) to disable indexing. 
 	if( FAILED( hr = m_WMWriter->SetOutputFilename( pwszOutFile ) ) )
		ThrowDShowException( TJS_W( "Failed to set output filename" ), hr );

	return( hr );
}

//! プロファイルをパラメータから生成する版
HRESULT CWMVEncoder::Initial( const WCHAR* pwszOutFile )
{
	if( NULL == pwszOutFile ) return( E_INVALIDARG );

	video_input_.SetVideoSource( video_width_, video_height_, video_scale_, video_rate_ );
	audio_input_.SetAudioSource( NULL, 0 );	// とりあえず音はなし

	HRESULT hr = S_OK;

	if( FAILED( hr = WMCreateWriter( NULL, &m_WMWriter ) ) )
		ThrowDShowException( TJS_W( "Failed to Create WMWriter" ), hr );

	// Create an empty profile
	if( FAILED( hr = CreateEmptyProfile( &m_WMProfile ) ) )
		ThrowDShowException( TJS_W( "Could not create empty profile" ), hr );

	// 入力にプロファイルを同期させる(入力にあったプロファイルにする)
	if( FAILED( hr = UpdateProfile( m_WMProfile ) ) )
		ThrowDShowException( TJS_W( "Failed to update profile" ), hr );
#if 0
	if( add_SMPTE_ ) {
		// If fAddSMPTE is TRUE, we need to set up SMPTE.
		if( FAILED( hr = SetupSMPTE( m_WMProfile ) ) )
			ThrowDShowException( TJS_W( "Failed to get the video frame rate for SMPTE" ), hr );
	}
#endif
	//  Save profile to writer
    if( FAILED( hr = m_WMWriter->SetProfile( m_WMProfile ) ) )
		ThrowDShowException( TJS_W( "Failed to set profile" ), hr );
	
	if( FAILED( hr = UpdateWriterInputs() ) )
		ThrowDShowException( TJS_W( "Failed to update writer inputs" ), hr );

	// Note: Indexing is automatically enabled when writing the uncompressed 
	// samples. We don't need to set up indexing manually. You can call 
	// IWMWriterFileSink3::SetAutoIndexing( FALSE ) to disable indexing. 
 	if( FAILED( hr = m_WMWriter->SetOutputFilename( pwszOutFile ) ) )
		ThrowDShowException( TJS_W( "Failed to set output filename" ), hr );

//	SaveProfile( L"c:\\test.prx", m_WMProfile );

	return( hr );
}

HRESULT CWMVEncoder::UpdateProfile( IWMProfile* pProfile )
{
	HRESULT hr = S_OK;

	// List of streams defined in the current profile
	CTSimpleList<CProfileStreams> ProfStreamList;

	if( NULL == pProfile ) {
		return( E_INVALIDARG );
	}

	// Create stream list from the current profile
	hr = CreateProfileStreamList( pProfile, &ProfStreamList );

	CProfileStreams*		pProfStream = NULL;
	CWMInput*				pInput = NULL;
	El<CProfileStreams>*	pProfStreamElem = NULL;
	El<CWMInput>*			pInputElem = NULL;
	El<CWMInput>*			pInputElemRemove = NULL;

	CWMInput*	inputs[2] = {&video_input_,&audio_input_};
	for( int i = 0; i < 2; i++ ) {
		pInput = inputs[i];
		if( pInput->m_Enable ) {
			pProfStream = Find( &ProfStreamList, pInput, &pProfStreamElem );
			if( NULL != pProfStream && NULL != pProfStream->m_pwszConnectionName ) {
				// There is a matching input in the current profile. Save its number and connection
				// name and remove it from the list of outstanding profile inputs.
				pInput->m_pwszConnectionName = new WCHAR[ ( wcslen( pProfStream->m_pwszConnectionName ) + 1 ) ];
				if( NULL == pInput->m_pwszConnectionName ) {
					hr = E_OUTOFMEMORY;
					break;
				}
				wcscpy( pInput->m_pwszConnectionName, pProfStream->m_pwszConnectionName );
				ProfStreamList.Erase( pProfStreamElem );
			} else {
				// There is no matching input in the profile - create one
				do {
					WORD	wStreamNum = 0;
					LPWSTR	pwszConnectionName = NULL;
					if( WMMEDIATYPE_Video == pInput->m_Type ) {
						hr = AddVideoStream( pProfile, (WMVIDEOINFOHEADER *)pInput->m_Mt.pbFormat, &wStreamNum, video_quality_, video_sec_per_key_, &pwszConnectionName );
					} else if( WMMEDIATYPE_Audio == pInput->m_Type ) {
						hr = AddAudioStream( pProfile, pInput->m_pWFX->nSamplesPerSec, pInput->m_pWFX->nChannels, pInput->m_pWFX->wBitsPerSample, &wStreamNum, &pwszConnectionName );
					}
					if( SUCCEEDED( hr ) ) {
						// この入力のプロファイルの生成成功
						// 接続名を保持する
						pInput->m_pwszConnectionName = pwszConnectionName;
					} else {
						// 生成失敗。この入力をリストから削除するためにマークする
						pInputElemRemove = pInputElem;
					}
				} while( FALSE );
			}
		}
	}

	// Remove all unused inputs from the profile.
	pProfStream = ProfStreamList.Iterate( ITER_FIRST );
	while( NULL != pProfStream ) {
		hr = pProfile->RemoveStreamByNumber( pProfStream->m_wStreamNum );
		pProfStream = ProfStreamList.Iterate( ITER_NEXT );
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Desc: Configures and adds a video stream.
//------------------------------------------------------------------------------
HRESULT CWMVEncoder::AddVideoStream( IWMProfile* pIWMProfile, WMVIDEOINFOHEADER* pInputVIH, WORD* pwStreamNum, DWORD dwQuality, DWORD dwSecPerKey, WCHAR** pwszConnectionName )
{
	HRESULT hr = S_OK;

	IWMProfileManager*	pManager = NULL;
	IWMCodecInfo*		pCodecInfo = NULL;
	IWMStreamConfig*	pStreamConfig = NULL;
	IWMVideoMediaProps*	pMediaProps = NULL;
	WM_MEDIA_TYPE*		pMediaType = NULL;
	IWMPropertyVault*	pPropertyVault = NULL;

	if( NULL == pIWMProfile || NULL == pInputVIH || NULL == pwStreamNum || NULL == pwszConnectionName ) {
		return( E_INVALIDARG );
	}

	do {
		if( FAILED( hr = WMCreateProfileManager( &pManager ) ) ) break;
		if( FAILED( hr = pManager->QueryInterface( IID_IWMCodecInfo, (void **) &pCodecInfo ) ) ) break;

		DWORD cCodecs;
		if( FAILED( hr = pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &cCodecs ) ) ) break;

		// Search from the last codec because the last codec usually is the newest codec. 
		for( int i = cCodecs-1; i >= 0; i-- ) {
			DWORD cFormats;
			if( FAILED( hr = pCodecInfo->GetCodecFormatCount( WMMEDIATYPE_Video, i, &cFormats ) ) ) break;

			DWORD j;
			for( j = 0; j < cFormats; j++ ) {
				SAFE_RELEASE( pStreamConfig );
				if( FAILED( hr = pCodecInfo->GetCodecFormat( WMMEDIATYPE_Video, i, j, &pStreamConfig ) ) ) break;

				SAFE_RELEASE( pMediaProps );
				if( FAILED( hr = pStreamConfig->QueryInterface( IID_IWMVideoMediaProps, (void **) &pMediaProps ) ) ) break;

				DWORD cbMT;
				if( FAILED( hr = pMediaProps->GetMediaType( NULL, &cbMT ) ) ) break;

				SAFE_ARRAYDELETE( pMediaType );
				pMediaType = (WM_MEDIA_TYPE *) new BYTE[ cbMT ];
				if( !pMediaType ) {
					hr = E_OUTOFMEMORY;
					break;
				}

				if( FAILED( hr = pMediaProps->GetMediaType( pMediaType, &cbMT ) ) ) break;

				if( pMediaType->formattype != WMFORMAT_VideoInfo ) {
					SAFE_RELEASE( pStreamConfig );
					continue;
				}
                WMVIDEOINFOHEADER* pVIH = (WMVIDEOINFOHEADER *)pMediaType->pbFormat;
//				if( pVIH->bmiHeader.biBitCount >= pInputVIH->bmiHeader.biBitCount ) break;
				if( pVIH->bmiHeader.biBitCount >= 24 ) {
					if( pMediaType->subtype == WMMEDIASUBTYPE_WMV3 ) {
						break;
					}
				}
				SAFE_RELEASE( pStreamConfig );
			}
			if( FAILED( hr ) || NULL != pStreamConfig ) break;
		}
		if( FAILED( hr ) ) break;
		if( NULL == pStreamConfig ) {
			hr = NS_E_VIDEO_CODEC_NOT_INSTALLED;
			break;
		}
		WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;

		// Set the target bitrate to 1/30 of the source bitrate since it's compressing. 
		pVIH->dwBitRate = pInputVIH->dwBitRate / 30;
		pVIH->rcSource.right = pInputVIH->rcSource.right;
		pVIH->rcSource.bottom = pInputVIH->rcSource.bottom;
		pVIH->rcTarget.right = pInputVIH->rcTarget.right;
		pVIH->rcTarget.bottom = pInputVIH->rcTarget.bottom;
		pVIH->bmiHeader.biWidth = pInputVIH->bmiHeader.biWidth;
		pVIH->bmiHeader.biHeight = pInputVIH->bmiHeader.biHeight;
		pVIH->AvgTimePerFrame = pInputVIH->AvgTimePerFrame;
		if( FAILED( hr = pMediaProps->SetQuality( dwQuality ) ) ) break;
		if( FAILED( hr = pMediaProps->SetMaxKeyFrameSpacing( 10000000 * (QWORD)dwSecPerKey ) ) ) break;
        if( FAILED( hr = SetStreamBasics( pStreamConfig, pIWMProfile, L"Video Stream", L"Video", pVIH->dwBitRate, pMediaType ) ) ) break;

		*pwszConnectionName = new WCHAR[ wcslen( wszDefaultConnectionName ) + 4 ];
		if( NULL == *pwszConnectionName ) {
			hr = E_OUTOFMEMORY;
			break;
		}
		if( FAILED( hr = pIWMProfile->AddStream( pStreamConfig ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
        if( FAILED( hr = pStreamConfig->GetStreamNumber( pwStreamNum ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
		BOOL fIsVBR = TRUE;
		if( SUCCEEDED( pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ) ) ) {
			if( FAILED( hr = pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*) &fIsVBR, sizeof( fIsVBR ) ) ) ) break;
			if( FAILED( hr = pPropertyVault->SetProperty( g_wszVBRQuality, WMT_TYPE_DWORD, (BYTE*) &dwQuality, sizeof( DWORD ) ) ) ) break;
		}

		// Each stream in the profile has to have a unique connection name.
		// Let's use the stream number to create it.
		if( *pwStreamNum > 127 ) {
			hr = E_FAIL;
			break;
		}
		swprintf( *pwszConnectionName, L"%s%d", wszDefaultConnectionName, (DWORD)*pwStreamNum );
		if( FAILED( hr = pStreamConfig->SetConnectionName( *pwszConnectionName ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
		if( FAILED( hr = pIWMProfile->ReconfigStream( pStreamConfig ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
	} while( FALSE );

	SAFE_RELEASE( pPropertyVault );
	SAFE_RELEASE( pCodecInfo );
	SAFE_RELEASE( pStreamConfig );
	SAFE_RELEASE( pMediaProps );
	SAFE_RELEASE( pManager );
	SAFE_ARRAYDELETE( pMediaType );

	return( hr );
}

//------------------------------------------------------------------------------
// Desc: Configures and adds an audio stream.
//------------------------------------------------------------------------------
HRESULT CWMVEncoder::AddAudioStream( IWMProfile* pIWMProfile, DWORD dwSampleRate, DWORD dwChannels, WORD wBitsPerSample, WORD* pwStreamNum, WCHAR** pwszConnectionName )
{
	HRESULT				hr = S_OK;
	IWMProfileManager*	pIWMProfileManager = NULL;
	IWMStreamConfig*	pIWMStreamConfig = NULL;
	IWMMediaProps*		pIMP  = NULL;
	IWMCodecInfo*		pIWMInfo = NULL;
	WAVEFORMATEX*		pWfx = NULL;
	WM_MEDIA_TYPE*		pType = NULL;
	if( NULL == pIWMProfile || NULL == pwStreamNum || NULL == pwszConnectionName ) {
		return( E_INVALIDARG );
	}
	do {
		if( FAILED( hr = WMCreateProfileManager( &pIWMProfileManager ) ) ) break;
		if( FAILED( hr = pIWMProfileManager->QueryInterface( IID_IWMCodecInfo, (void **) &pIWMInfo ) ) ) break;

		DWORD i, j;
		DWORD cCodecs;
		if( FAILED( hr = pIWMInfo->GetCodecInfoCount( WMMEDIATYPE_Audio, &cCodecs ) ) ) break;
		for( i = 0; i < cCodecs; i++ ) {
			DWORD cFormats;
			if( FAILED( hr = pIWMInfo->GetCodecFormatCount( WMMEDIATYPE_Audio, i, &cFormats ) ) ) break;

			// Find a proper format in this codec 
			for( j = 0; j < cFormats; j++ ) {
				if( NULL != pType ) {
					SAFE_ARRAYDELETE( pType );
				}

				DWORD cbType = 0;
				if( FAILED( hr = pIWMInfo->GetCodecFormat( WMMEDIATYPE_Audio, i, j, &pIWMStreamConfig ) ) ) break;

				SAFE_RELEASE( pIMP );
				if( FAILED( hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, (void **)&pIMP ) ) ) break;
				if( FAILED( hr = pIMP->GetMediaType( NULL, &cbType ) ) ) break;

				pType = (WM_MEDIA_TYPE *) new BYTE[ cbType ];
				if( NULL == pType ) {
					hr = E_OUTOFMEMORY;
					break;
				}

				if( FAILED( hr = pIMP->GetMediaType( pType, &cbType ) ) ) {
					break;
				}
				if( pType->formattype != WMFORMAT_WaveFormatEx ) {
					hr = E_FAIL;
					break;
				}
				pWfx = (WAVEFORMATEX *) pType->pbFormat;

				// This sample will use this format only if it has the same sample rate, channels and more bits per sample. 
				// This is not necessary, because normally the codec can convert the sample rate and bits per sample for you. 
				if( pWfx->nSamplesPerSec == dwSampleRate && pWfx->nChannels == dwChannels && pWfx->wBitsPerSample >= wBitsPerSample ) {
					break;
				}
				SAFE_RELEASE( pIWMStreamConfig );
			}
			if( FAILED( hr ) || NULL != pIWMStreamConfig ) {
				break;
			}
		}
		if( FAILED( hr ) ) break;

		if( NULL == pIWMStreamConfig ) {
			hr = NS_E_AUDIO_CODEC_NOT_INSTALLED;
			break;
		}
		// We found a valid WAVEFORMATEX; go ahead and set up the stream.
		if( FAILED( hr = SetStreamBasics( pIWMStreamConfig, pIWMProfile, L"Audio Stream", L"Audio", pWfx->nAvgBytesPerSec * 8, pType ) ) ) break;

		*pwszConnectionName = new WCHAR[ wcslen( wszDefaultConnectionName ) + 4 ];
		if( NULL == *pwszConnectionName ) {
			hr = E_OUTOFMEMORY;
			break;
		}

		if( FAILED( hr = pIWMProfile->AddStream( pIWMStreamConfig ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}

		if( FAILED( hr = pIWMStreamConfig->GetStreamNumber( pwStreamNum ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}

		// Each stream in the profile has to have a unique connection name.
		// Let's use the stream number to create it.
		if( *pwStreamNum > 127 ) {
			hr = E_FAIL;
			break;
		}
		swprintf( *pwszConnectionName, L"%s%d", wszDefaultConnectionName, (DWORD)*pwStreamNum );
		if( FAILED( hr = pIWMStreamConfig->SetConnectionName( *pwszConnectionName ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
		if( FAILED( hr = pIWMProfile->ReconfigStream( pIWMStreamConfig ) ) ) {
			SAFE_ARRAYDELETE( *pwszConnectionName );
			break;
		}
	} while( FALSE );

	SAFE_ARRAYDELETE( pType );
	SAFE_RELEASE( pIWMInfo );
	SAFE_RELEASE( pIWMStreamConfig );
	SAFE_RELEASE( pIMP );
	SAFE_RELEASE( pIWMProfileManager );

	return( hr );
}

//------------------------------------------------------------------------------
// Desc: Creates and configures a stream.
//------------------------------------------------------------------------------
HRESULT CWMVEncoder::SetStreamBasics( IWMStreamConfig* pIWMStreamConfig, IWMProfile* pIWMProfile, LPWSTR pwszStreamName, LPWSTR pwszConnectionName, DWORD dwBitrate, WM_MEDIA_TYPE* pmt )
{
	HRESULT		hr = S_OK;
	WORD		wStreamNum = 0;
	CComPtr<IWMMediaProps>		pIWMMediaProps;
	CComPtr<IWMStreamConfig>	pIWMStreamConfig2;

	if( NULL == pIWMStreamConfig || NULL == pIWMProfile || NULL == pmt ) {
		return( E_INVALIDARG );
	}

	if( FAILED( hr = pIWMProfile->CreateNewStream( pmt->majortype, &pIWMStreamConfig2 ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig2->GetStreamNumber( &wStreamNum ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig->SetStreamNumber( wStreamNum ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig->SetStreamName( pwszStreamName ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig->SetConnectionName( pwszConnectionName ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig->SetBitrate( dwBitrate ) ) ) return( hr );
	if( FAILED( hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, (void **)&pIWMMediaProps ) ) ) return( hr );
	if( FAILED( hr = pIWMMediaProps->SetMediaType( pmt ) ) ) return( hr );

	return( hr );
}
#if 0
HRESULT CWMVEncoder::SetupSMPTE( IWMProfile* pProfile )
{
	HRESULT			hr = S_OK;
	WM_MEDIA_TYPE*	pMediaType = NULL;
	DWORD			cbMediaType = 0;
	DWORD			cStreams = 0;
	LPWSTR			pwszConnectionName = NULL;
	WORD			cchConnectionName = 0;
	GUID			guidStreamType;

	CComPtr<IWMStreamConfig>	pIWMStreamConfig;
	CComPtr<IWMStreamConfig2>	pIWMStreamConfig2;
	CComPtr<IWMMediaProps>		pMediaProps;

	if( NULL == pProfile ) return( E_INVALIDARG );


	if( FAILED( hr = pProfile->GetStreamCount( &cStreams ) ) ) return( hr );

	// Find the first video stream.
	// Currently, SMPTE only supports one video stream.
	for( DWORD i = 0; i < cStreams; i++ ) {
		if( FAILED( hr = pProfile->GetStream( i, &pIWMStreamConfig ) ) ) break;
		if( FAILED( hr = pIWMStreamConfig->GetStreamType( &guidStreamType ) ) ) break;

		if( WMMEDIATYPE_Video == guidStreamType ) break;
		pIWMStreamConfig.Release();
	}

	if( FAILED( hr ) ) return( hr );
	if( NULL == pIWMStreamConfig.p ) return( E_INVALIDARG );

	do {
		// We need to call IWMStreamConfig2::AddDataUnitExtension to add a data unit extension to store SMPTE code. 
		if( FAILED( hr = pIWMStreamConfig->QueryInterface( IID_IWMStreamConfig2, (void **)&pIWMStreamConfig2 ) ) ) break;
		if( FAILED( hr = pIWMStreamConfig2->AddDataUnitExtension( WM_SampleExtensionGUID_Timecode, sizeof(WMT_TIMECODE_EXTENSION_DATA), NULL, 0 ) ) )
			break;

		// Update the profile.
		if( FAILED( hr = pProfile->ReconfigStream( pIWMStreamConfig ) ) ) break;

		// Get the frame rate and input number of this video stream
		if( FAILED( hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, (void **)&pMediaProps ) ) ) break;
		if( FAILED( hr = pMediaProps->GetMediaType( NULL, &cbMediaType ) ) ) break;

		pMediaType = (WM_MEDIA_TYPE *) new BYTE[ cbMediaType ];
		if( !pMediaType ) {
			hr = E_OUTOFMEMORY;
			break;
		}
		if( FAILED( hr = pMediaProps->GetMediaType( pMediaType, &cbMediaType ) ) ) break;

		// Save the AvgTimePerFrame of this stream
		WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;
		m_qwSMPTEAvgTimePerFrame = pVIH->AvgTimePerFrame;
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName ) ) ) break;

		pwszConnectionName = new WCHAR[ cchConnectionName + 1 ];
		if( NULL == pwszConnectionName ) {
			hr = E_OUTOFMEMORY;
			break;
		}
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, &cchConnectionName ) ) ) break;

		// Look for an AVI input with the matching connection name and set the m_fAddSMPTE flag
		CWMInput* pInput = &video_input_;
		if( 0 != wcscmp( pInput->m_pwszConnectionName, pwszConnectionName ) ) {
			if( audio_enable_ ) {
				pInput = &audio_input_;
				if( 0 != wcscmp( pInput->m_pwszConnectionName, pwszConnectionName ) {
					pInput = NULL;
				}
			}
		}
		if( NULL == pInput ) {
			hr = E_INVALIDARG;
			break;
		} else {
//			pInput->m_fAddSMPTE = TRUE;
		}
	} while( FALSE );

	SAFE_ARRAYDELETE( pMediaType );
	SAFE_ARRAYDELETE( pwszConnectionName );

    return( hr );
}
#endif
//------------------------------------------------------------------------------
// Desc: Creates an empty profile.
//------------------------------------------------------------------------------
HRESULT CWMVEncoder::CreateEmptyProfile( IWMProfile** ppIWMProfile )
{
	HRESULT		hr = S_OK;
	CComPtr<IWMProfileManager>	pIWMProfileManager;
	if( NULL == ppIWMProfile ) return( E_POINTER );

	if( FAILED( hr = WMCreateProfileManager( &pIWMProfileManager ) ) ) return( hr );
	if( FAILED( hr = pIWMProfileManager->CreateEmptyProfile( WMT_VER_9_0, ppIWMProfile ) ) ) return( hr );

	return( hr );
}

HRESULT CWMVEncoder::CreateProfileStreamList( IWMProfile* pProfile, CTSimpleList<CProfileStreams>* pProfStreamList )
{
	if( NULL == pProfile  || NULL == pProfStreamList )
		return( E_INVALIDARG );

    DWORD cStreams = 0;
	HRESULT hr;
	if( FAILED( hr = pProfile->GetStreamCount( &cStreams ) ) )
		return( hr );

	WM_MEDIA_TYPE* pMediaType = NULL;

	// Create a list of inputs defined in the given profile
	for( DWORD i = 0; i < cStreams; i++ ) {
		CComPtr<IWMStreamConfig> pIWMStreamConfig;
		if( FAILED( hr = pProfile->GetStream( i, &pIWMStreamConfig ) ) )
			break;

		GUID guidInputType;
		if( FAILED( hr = pIWMStreamConfig->GetStreamType( &guidInputType ) ) )
			break;

		CComPtr<IWMMediaProps>	pMediaProps;
		if( FAILED( hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, (void **)&pMediaProps ) ) )
			break;

		DWORD cbMediaType = 0;
		if( FAILED( hr = pMediaProps->GetMediaType( NULL, &cbMediaType ) ) )
			break;

		pMediaType = (WM_MEDIA_TYPE *)new BYTE[ cbMediaType ];
		if( FAILED( hr = pMediaProps->GetMediaType( pMediaType, &cbMediaType ) ) ) {
			SAFE_ARRAYDELETE( pMediaType );
			break;
		}

		WORD cchConnectionName = 0;
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName ) ) ) {
			SAFE_ARRAYDELETE( pMediaType );
			break;
		}

		WCHAR* pwszConnectionName = new WCHAR[cchConnectionName];
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, &cchConnectionName ) ) ) {
			SAFE_ARRAYDELETE( pMediaType );
			SAFE_ARRAYDELETE( pwszConnectionName );
			break;
		}

		WORD wStreamNum = 0;
		if( FAILED( hr = pIWMStreamConfig->GetStreamNumber( &wStreamNum ) ) ) {
			SAFE_ARRAYDELETE( pMediaType );
			SAFE_ARRAYDELETE( pwszConnectionName );
			break;
		}

		// Only one stream for each connection is needed on this list
		CProfileStreams* pProfStream = pProfStreamList->Iterate( ITER_FIRST );
		while( NULL != pProfStream ) {
			if( 0 == wcscmp( pwszConnectionName, pProfStream->m_pwszConnectionName ) ) {
				// There is already a stream for this connection on the list; do not append
				// this one. This could happen if the profile is MBR. 
				break;
			}
			pProfStream = pProfStreamList->Iterate( ITER_NEXT );
		}
		if( NULL == pProfStream ) {
			// There is no stream for this connection on the list; append this one.
			// pProfStreamList will not allocate memory to save pwszConnectionName 
			// and pMediaType, so the memory of pwszConnectionName and pMediaType
			// should not be released now.
			CProfileStreams ProfileStream( guidInputType, wStreamNum, pMediaType, pwszConnectionName );

			if( pProfStreamList->Append( &ProfileStream ) ) {
				// Set the pointers to NULL, so their memory will not be released 
				// by this function.
				pMediaType = NULL;
				pwszConnectionName = NULL;
			}
		}

		SAFE_ARRAYDELETE( pwszConnectionName );
		SAFE_ARRAYDELETE( pMediaType );
	}
	SAFE_ARRAYDELETE( pMediaType );

	return( hr );
}

HRESULT CWMVEncoder::UpdateWriterInputs()
{
	if( NULL == m_WMWriter.p ) {
		return( E_INVALIDARG );
	}

	HRESULT hr = S_OK;
	DWORD cInputs = 0;
	if( FAILED( hr = m_WMWriter->GetInputCount( &cInputs ) ) )
		return( hr );

	WCHAR* pwszConnectionName = NULL;
	for( DWORD i = 0; i < cInputs; i++ ) {
		CComPtr<IWMInputMediaProps> pInputProps;
		if( FAILED( hr = m_WMWriter->GetInputProps( i, &pInputProps ) ) )
			break;

		CComPtr<IWMStreamConfig> pIWMStreamConfig;
		if( FAILED( hr = pInputProps->QueryInterface( IID_IWMStreamConfig, (void **) &pIWMStreamConfig ) ) )
			break;

		GUID guidInputType;
		if( FAILED( hr = pIWMStreamConfig->GetStreamType( &guidInputType ) ) )
			return hr;

		WORD cchConnectionName = 0;
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName ) ) )
			break;

		pwszConnectionName = new WCHAR[cchConnectionName];
		if( FAILED( hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, &cchConnectionName ) ) )
			break;

		if( WMMEDIATYPE_Video == guidInputType && ( 0 == wcscmp( video_input_.m_pwszConnectionName, pwszConnectionName ) ) ) {
			video_input_.m_dwInput = i;
			if( FAILED( hr = pInputProps->SetMediaType( &(video_input_.m_Mt) ) ) )
				break;
			if( FAILED( hr = m_WMWriter->SetInputProps( i, pInputProps ) ) )
				break;
		} else if( WMMEDIATYPE_Audio == guidInputType && ( 0 == wcscmp( audio_input_.m_pwszConnectionName, pwszConnectionName ) ) ) {
			audio_input_.m_dwInput = i;
			if( FAILED( hr = pInputProps->SetMediaType( &(audio_input_.m_Mt) ) ) )
				break;
			if( FAILED( hr = m_WMWriter->SetInputProps( i, pInputProps ) ) )
				break;
		}
		SAFE_ARRAYDELETE( pwszConnectionName );
	}
	SAFE_ARRAYDELETE( pwszConnectionName );
	return( hr );
}

HRESULT CWMVEncoder::SaveProfile( LPCTSTR ptszFileName, IWMProfile* pIWMProfile )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   * pIWMProfileManager = NULL;
    DWORD               dwLength = 0;
    LPWSTR              pBuffer = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               dwBytesWritten = 0;


    if( ( NULL == ptszFileName ) || ( NULL == pIWMProfile ) ) {
        return( E_INVALIDARG );
    }

    do {
        // Create profile manager
		if( FAILED( hr = WMCreateProfileManager( &pIWMProfileManager ) ) ) {
            break;
        }

        // Save profile to a buffer
		if( FAILED( hr = pIWMProfileManager->SaveProfile( pIWMProfile, NULL, &dwLength ) ) ) {
            break;
        }

        pBuffer = new WCHAR[ dwLength ];
        if( NULL == pBuffer ) {
            hr = E_OUTOFMEMORY;
            break;
        }

		if( FAILED( hr = pIWMProfileManager->SaveProfile( pIWMProfile, pBuffer, &dwLength ) ) ) {
            break;
        }

        hFile = CreateFile( ptszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( INVALID_HANDLE_VALUE == hFile ) {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( hFile ) ) {
            hr = NS_E_INVALID_NAME;
            break;
        }

        // Write profile buffer to file
        if( !WriteFile( hFile, pBuffer, dwLength * sizeof(WCHAR), &dwBytesWritten, NULL) || dwLength*sizeof(WCHAR) != dwBytesWritten ) {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }
    } while( FALSE );

    SAFE_CLOSEFILEHANDLE( hFile );
    SAFE_ARRAYDELETE( pBuffer );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}
HRESULT CWMVEncoder::Start()
{
	if( NULL == m_WMWriter.p ) {
		return( E_INVALIDARG );
	}

	HRESULT hr;
	if( FAILED( hr = m_WMWriter->BeginWriting() ) ) {
		if( NS_E_VIDEO_CODEC_NOT_INSTALLED == hr ) {
			ThrowDShowException( TJS_W( "BeginWriting failed: Video Codec not installed" ), hr );
		}
		if( NS_E_AUDIO_CODEC_NOT_INSTALLED == hr ) {
			ThrowDShowException( TJS_W( "BeginWriting failed: Audio Codec not installed" ), hr );
		} else if( NS_E_INVALID_OUTPUT_FORMAT == hr ) {
			ThrowDShowException( TJS_W( "BeginWriting failed: Invalid Output Format" ), hr );
		} else if( NS_E_VIDEO_CODEC_ERROR == hr ) {
			ThrowDShowException( TJS_W( "BeginWriting failed: An unexpected error occurred with the video codec" ), hr );
		} else if( NS_E_AUDIO_CODEC_ERROR == hr ) {
			ThrowDShowException( TJS_W( "BeginWriting failed: An unexpected error occurred with the audio codec" ), hr );
		} else {
			ThrowDShowException( TJS_W( "BeginWriting failed: Error" ), hr );
		}
		return( hr );
	}

	encoder_running_ = true;

	return hr;
}



HRESULT CWMVEncoder::Stop()
{
	if( encoder_running_ ) {
		encoder_running_ = false;

		if( NULL == m_WMWriter.p ) {
			return( E_INVALIDARG );
		}
		HRESULT hr;
		if( FAILED( hr = m_WMWriter->EndWriting() ) ) {
			ThrowDShowException( TJS_W( "IWMWriter::EndWriting failed: Error" ), hr );
		}
		return hr;
	} else {
		return S_OK;
	}
}

void CWMVEncoder::WriteVideoSample( void* sample, size_t sample_size, QWORD tick )
{
	HRESULT hr;
//	video_input_.m_qwPresentTime = tick * 10000;
	video_input_.m_qwPresentTime = tick;
	if( FAILED( hr = WriteSample( &video_input_, sample, sample_size ) ) ) {
		ThrowDShowException( TJS_W( "Failed to Write sample" ), hr );
	}
}
void CWMVEncoder::WriteAudioSample( void* sample, size_t sample_size, QWORD tick )
{
	HRESULT hr;
//	pInput->m_qwPresentTime = tick * 10000;
//	audio_input_.m_qwPresentTime = 10000000 * (QWORD)audio_input_.m_dwCurrentSample / audio_input_.m_pWFX->nSamplesPerSec;
//	audio_input_.m_qwPresentTime = tick * 10000;
//	audio_input_.m_qwPresentTime = 10000000 * (QWORD)tick / audio_input_.m_pWFX->nSamplesPerSec;
	audio_input_.m_qwPresentTime = tick;
	if( FAILED( hr = WriteSample( &audio_input_, sample, sample_size ) ) ) {
		ThrowDShowException( TJS_W( "Failed to Write sample" ), hr );
	}
}
void CWMVEncoder::ReleaseAll()
{
	video_input_.Cleanup();
	audio_input_.Cleanup();
	SAFE_RELEASE( m_WMWriter.p );
	SAFE_RELEASE( m_WMProfile.p );
}

BOOL CompareMediaTypes( const WM_MEDIA_TYPE * pMedia1, const WM_MEDIA_TYPE * pMedia2)
{
	if( pMedia1->majortype != pMedia2->majortype ) {
		return( FALSE );
	}
	return( TRUE );
}

