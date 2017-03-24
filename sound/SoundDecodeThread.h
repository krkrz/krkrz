//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Sound Decode Thead for QueueSoundBuffer
//---------------------------------------------------------------------------
#ifndef __SOUND_DECODE_THREAD_H__
#define __SOUND_DECODE_THREAD_H__

#include "ThreadIntf.h"
#include <vector>

class tTVPSoundSamplesBuffer;
class tTJSNI_QueueSoundBuffer;
class tTVPSoundDecodeThread : public tTVPThread
{
	tTJSNI_QueueSoundBuffer * Owner;
	tTVPThreadEvent Event;
	tTJSCriticalSection OneLoopCS;
	std::vector<tTVPSoundSamplesBuffer*> Samples;
	tjs_int64 DecodedSamples;

public:
	tTVPSoundDecodeThread(tTJSNI_QueueSoundBuffer * owner);
	~tTVPSoundDecodeThread();

	void Execute(void);

	// デコード中断を要求する(キューを空にする)
	void Interrupt();
	// デコード処理を開始する
	void StartDecoding( tjs_int64 predecoded );
	// サンプルバッファを追加する
	void PushSamplesBuffer( tTVPSoundSamplesBuffer* buf );
	// サンプルバッファキューを空にする
	void ClearQueue();
};


#endif // __SOUND_DECODE_THREAD_H__