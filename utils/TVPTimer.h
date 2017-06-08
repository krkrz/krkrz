
#ifndef __TVP_TIMER_H__
#define __TVP_TIMER_H__

#include "TimerThread.h"

class TVPTimerEventIntarface {
public:
	virtual void Handle() = 0;
};

template<typename T>
class TVPTimerEvent : public TVPTimerEventIntarface {
	void (T::*handler_)();
	T* owner_;

public:
	TVPTimerEvent( T* owner, void (T::*Handler)() ) : owner_(owner), handler_(Handler) {}
	void Handle() override { (owner_->*handler_)(); }
};

class TVPTimer : public tTVPTimerBase {
	TVPTimerEventIntarface* event_;

protected:
	// タイマー処理実態。メインスレッドで呼ばれる。
	virtual void Fire(tjs_uint n) override {
		if( event_ ) {
			event_->Handle();
		}
	}

	// イベントをキャンセルする。SetEnabled/SetIntervalをコールした時内部で呼ばれる。
	virtual void CancelEvents() override {}

private:
	// 隠ぺいする
	// Interval値は16ビットシフトされたものになっている
	tjs_uint64 GetInterval() const override {
		return tTVPTimerBase::GetInterval();
	}
public:
	TVPTimer();
	virtual ~TVPTimer();

	template<typename T>
	void SetOnTimerHandler( T* owner, void (T::*Handler)() ) {
		if( event_ ) delete event_;
		event_ = new TVPTimerEvent<T>( owner, Handler );
	}

	void SetInterval(tjs_uint64 n) override;
};


#endif // __TVP_TIMER_H__
