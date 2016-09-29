
#include "tjsCommHead.h"
#include "TVPTimer.h"

TVPTimer::TVPTimer() : event_(NULL), interval_(1000), enabled_(true) {
	// CreateUtilWindow();

	// Application->addEventHandler しつつ、TimerでApplicationにpostEvent
	// 定期的にハンドリングする

	// http://www.ops.dti.ne.jp/~allergy/thread/thread.html
	// http://smdn.jp/programming/tips/posix_timer/
	
	// http://yamanetoshi.github.io/blog/2014/09/25/do-it-after-a-few-seconds/
	// timer_create( CLOCK_MONOTONIC, 
}

TVPTimer::~TVPTimer() {
}
void TVPTimer::Destroy() {
}
void TVPTimer::UpdateTimer() {
}

