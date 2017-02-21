

#ifndef __SOUND_EVENT_THREAD_H__
#define __SOUND_EVENT_THREAD_H__

#include "ThreadIntf.h"
#include "NativeEventQueue.h"
#include <vector>

//---------------------------------------------------------------------------
// tTVPSoundEventThread : playing thread
//---------------------------------------------------------------------------
/*
	システムは1つだけplaying threadを持ちます。
	playing threadは、各ストリームのバッファキューを満たし、ラベルイベントのタイミングも管理します。
	このアルゴリズムで使用される技法は、Timerクラスの実装に似ています。
*/
class tTVPSoundEventThread : public tTVPThread
{
	tTVPThreadEvent Event;
	std::mutex SuspendMutex;
	bool SuspendThread;

	bool PendingLabelEventExists;
	bool WndProcToBeCalled;
	tjs_uint NextLabelEventTick;
	tjs_uint LastFilledTick;

	class tTVPSoundBuffers* Buffers;
	NativeEventQueue<tTVPSoundEventThread> EventQueue;
public:
	tTVPSoundEventThread( class tTVPSoundBuffers* parent );
	~tTVPSoundEventThread();

private:
	void UtilWndProc( NativeEvent& ev );

	void SetSuspend()
	{
		std::lock_guard<std::mutex> lock( SuspendMutex );
		SuspendThread = true;
	}
	void ResetSuspend()
	{
		std::lock_guard<std::mutex> lock( SuspendMutex );
		SuspendThread = false;
	}

public:
	void ReschedulePendingLabelEvent(tjs_int tick);

protected:
	void Execute(void);

public:
	void Start(void);
	void CheckBufferSleep();
};
/*
tTVPSoundEventThread to
call
tTJSNI_WaveSoundBuffer::FireLabelEventsAndGetNearestLabelEventStep()
tTJSNI_WaveSoundBuffer::Update

access
tTJSNI_WaveSoundBuffer::ThreadCallbackEnabled
*/
class tTJSNI_QueueSoundBuffer;
class tTVPSoundBuffers {
	std::vector<tTJSNI_QueueSoundBuffer *> Buffers;
	tTJSCriticalSection BufferCS;
	tTVPSoundEventThread* EventThread;

public:
	inline tTJSCriticalSection& GetCriticalSection() { return BufferCS; }
	inline std::vector<tTJSNI_QueueSoundBuffer *>::iterator Begin() { return Buffers.begin(); }
	inline std::vector<tTJSNI_QueueSoundBuffer *>::iterator End() { return Buffers.end(); }
	inline size_t Size() const { return Buffers.size(); }

	void ReleaseBuffers(bool disableevent=true);
	void Shutdown();
	void EnsureBufferWorking();
	void CheckAllSleep();
	void AddBuffer( tTJSNI_QueueSoundBuffer * buffer );
	void RemoveBuffer(tTJSNI_QueueSoundBuffer * buffer);
	void ReschedulePendingLabelEvent(tjs_int tick);
	void ResetVolumeToAllSoundBuffer();
};


#endif // __SOUND_EVENT_THREAD_H__
