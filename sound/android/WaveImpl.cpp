//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wave Player implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "SystemControl.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "StorageIntf.h"
#include "WaveImpl.h"
#include "PluginImpl.h"
#include "SysInitIntf.h"
#include "ThreadIntf.h"
#include "Random.h"
#include "UtilStreams.h"
#include "TickCount.h"
#include "TVPTimer.h"
#include "Application.h"
#include "UserEvent.h"
#include "NativeEventQueue.h"

static bool TVPDeferedSettingAvailable = false;
//---------------------------------------------------------------------------
void TVPWaveSoundBufferCommitSettings()
{
	// commit all defered sound buffer settings
	if(TVPDeferedSettingAvailable)
	{
		TVPDeferedSettingAvailable = false;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// tTJSNI_WaveSoundBuffer
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
tTJSNI_WaveSoundBuffer::tTJSNI_WaveSoundBuffer()
{
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_WaveSoundBuffer::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_WaveSoundBuffer::Invalidate()
{
	inherited::Invalidate();
}
//---------------------------------------------------------------------------
bool tTJSNI_WaveSoundBuffer::FillL2Buffer(bool firstwrite, bool fromdecodethread) {}
bool tTJSNI_WaveSoundBuffer::FillBuffer(bool firstwrite, bool allowpause) {}
tjs_int tTJSNI_WaveSoundBuffer::FireLabelEventsAndGetNearestLabelEventStep(tjs_int64 tick) {}
tjs_int tTJSNI_WaveSoundBuffer::GetNearestEventStep() {}
void tTJSNI_WaveSoundBuffer::FlushAllLabelEvents() {}
void tTJSNI_WaveSoundBuffer::Play() {}
void tTJSNI_WaveSoundBuffer::Stop() {}
void tTJSNI_WaveSoundBuffer::SetPaused(bool b) {}
void tTJSNI_WaveSoundBuffer::Open(const ttstr & storagename) {}
void tTJSNI_WaveSoundBuffer::SetLooping(bool b) {}
tjs_uint64 tTJSNI_WaveSoundBuffer::GetSamplePosition() {}
void tTJSNI_WaveSoundBuffer::SetSamplePosition(tjs_uint64 pos) {}
tjs_uint64 tTJSNI_WaveSoundBuffer::GetPosition() {}
void tTJSNI_WaveSoundBuffer::SetPosition(tjs_uint64 pos) {}
tjs_uint64 tTJSNI_WaveSoundBuffer::GetTotalTime() {}
void tTJSNI_WaveSoundBuffer::SetVolumeToSoundBuffer() {}
void tTJSNI_WaveSoundBuffer::SetVolume(tjs_int v) {}
void tTJSNI_WaveSoundBuffer::SetVolume2(tjs_int v) {}
void tTJSNI_WaveSoundBuffer::SetPan(tjs_int v) {}
void tTJSNI_WaveSoundBuffer::SetGlobalVolume(tjs_int v) {}
void tTJSNI_WaveSoundBuffer::SetGlobalFocusMode(tTVPSoundGlobalFocusMode b) {}
void tTJSNI_WaveSoundBuffer::SetPos(float x, float y, float z) {}
void tTJSNI_WaveSoundBuffer::SetPosX(float v) {}
void tTJSNI_WaveSoundBuffer::SetPosY(float v) {}
void tTJSNI_WaveSoundBuffer::SetPosZ(float v) {}
void tTJSNI_WaveSoundBuffer::SetFrequency(tjs_int freq) {}
void tTJSNI_WaveSoundBuffer::SetUseVisBuffer(bool b) {}
void tTJSNI_WaveSoundBuffer::TimerBeatHandler() {}
void tTJSNI_WaveSoundBuffer::ResetVisBuffer() {}
void tTJSNI_WaveSoundBuffer::DeallocateVisBuffer() {}
void tTJSNI_WaveSoundBuffer::CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src,
	tjs_int numsamples, tjs_int channels) {}
tjs_int tTJSNI_WaveSoundBuffer::GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels,
	tjs_int aheadsamples) {}


//---------------------------------------------------------------------------
// tTJSNC_WaveSoundBuffer
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_WaveSoundBuffer::CreateNativeInstance()
{
	return new tTJSNI_WaveSoundBuffer();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCreateNativeClass_WaveSoundBuffer
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_WaveSoundBuffer()
{
	tTJSNativeClass *cls = new tTJSNC_WaveSoundBuffer();
	static tjs_uint32 TJS_NCM_CLASSID;
	TJS_NCM_CLASSID = tTJSNC_WaveSoundBuffer::ClassID;

//----------------------------------------------------------------------
// methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/freeDirectSound)  /* static */
{
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/freeDirectSound)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getVisBuffer)
{
	// get samples for visualization 
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
		/*var. type*/tTJSNI_WaveSoundBuffer);

	if(numparams < 3) return TJS_E_BADPARAMCOUNT;
	tjs_int16 *dest = (tjs_int16*)(tjs_int)(*param[0]);

	tjs_int ahead = 0;
	if(numparams >= 4) ahead = (tjs_int)*param[3];

	tjs_int res = _this->GetVisBuffer(dest, *param[1], *param[2], ahead);

	if(result) *result = res;

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getVisBuffer)
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(useVisBuffer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
			/*var. type*/tTJSNI_WaveSoundBuffer);

		*result = _this->GetUseVisBuffer();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
			/*var. type*/tTJSNI_WaveSoundBuffer);

		_this->SetUseVisBuffer(0!=(tjs_int)*param);

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, useVisBuffer)
//----------------------------------------------------------------------
	return cls;
}
//---------------------------------------------------------------------------

