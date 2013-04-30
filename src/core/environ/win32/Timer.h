
#ifndef __T_TIMER_H__
#define __T_TIMER_H__

class TTimer {
public:
	TTimer( class TApplication* app ) {}
	void SetInterval( int ) {}
	void SetOnTimerHandler( void* ) {}
	void SetEnabled( bool b ) {}
	void SetOnTimer( void* p ) {}
};

#endif // __T_TIMER_H__
