
#include "tjsCommHead.h"
#include "TVPTimer.h"

TVPTimer::TVPTimer() : event_(nullptr) {
	tTVPTimerThread::Add(this);
}

TVPTimer::~TVPTimer() {
	tTVPTimerThread::Remove(this);
	ZeroPendingCount();
}


