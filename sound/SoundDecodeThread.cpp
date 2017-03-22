//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Sound Decode Thead for QueueSoundBuffer
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "MsgIntf.h"
#include "SoundDecodeThread.h"
#include "QueueSoundBufferImpl.h"
#include "SoundSamples.h"

//---------------------------------------------------------------------------
static const tTVPThreadPriority TVPDecodeThreadHighPriority = ttpHigher;
//---------------------------------------------------------------------------
tTVPSoundDecodeThread::tTVPSoundDecodeThread( tTJSNI_QueueSoundBuffer * owner )
 : Owner(owner), DecodedSamples(0) {
	StartTread();
}
//---------------------------------------------------------------------------
tTVPSoundDecodeThread::~tTVPSoundDecodeThread() {
	Terminate();
	ClearQueue();
	Event.Set();
	WaitFor();
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::Execute(void) {
	SetPriority(TVPDecodeThreadHighPriority);
	while( !GetTerminated() ) {
		tjs_uint32 count = 0;
		{
			tTJSCriticalSectionHolder cs_holder(OneLoopCS);
			count = Samples.size();
			if( count ) {
				// バッファにデコードしたSampleを入れる
				auto itr = Samples.begin();
				tTVPSoundSamplesBuffer* buf = *itr;
				buf->Decode();
				buf->SetDecodePosition( DecodedSamples );
				DecodedSamples += buf->GetInSamples();

				// デコード済みSampleを再生ストリームへ移動
				Owner->PushPlayStream( buf );
				Samples.erase( itr );
				count = Samples.size();
			}
		}

		if( GetTerminated() ) break;

		if( count == 0 ) {
			// buffer is empty; sleep infinite
			Event.WaitFor(0);
		} else {
			// buffer is not full; sleep shorter
			// ダブルバッファリングなのでここに来る可能性は低いが、デコードが極度に送れている場合は来る
			// 一応スレッド切り替えして占有は避け、次のバッファをデコードする
			std::this_thread::yield();
		}
	}
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::Interrupt() {
	ClearQueue();
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::StartDecoding( tjs_int64 predecoded ) {
	DecodedSamples = predecoded;
	Event.Set();
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::PushSamplesBuffer( tTVPSoundSamplesBuffer* buf ) {
	// キューにバッファを入れ、スレッドを起こす
	tTJSCriticalSectionHolder cs_holder(OneLoopCS);
	Samples.push_back(buf);
	Event.Set();
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::ClearQueue() {
	// キューを空にする
	tTJSCriticalSectionHolder cs_holder(OneLoopCS);
	Samples.clear();
}
//---------------------------------------------------------------------------
