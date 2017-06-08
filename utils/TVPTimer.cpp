
#include "tjsCommHead.h"
#include "TVPTimer.h"
#include "TimerIntf.h"

TVPTimer::TVPTimer() : event_(nullptr) {
	tTVPTimerThread::Add(this);
}

TVPTimer::~TVPTimer() {
	tTVPTimerThread::Remove(this);
	ZeroPendingCount();
	if( event_ ) delete event_;
}

void TVPTimer::SetInterval(tjs_uint64 n) {
	double interval = (double)n * (1<<TVP_SUBMILLI_FRAC_BITS);
	tTVPTimerBase::SetInterval((tjs_int64)(interval + 0.5));
}
