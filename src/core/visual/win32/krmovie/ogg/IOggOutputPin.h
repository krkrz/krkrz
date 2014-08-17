
#pragma once
class IOggOutputPin {
public:
	virtual bool notifyStreamBaseTime(__int64 inStreamTime) = 0;
	virtual __int64 getGlobalBaseTime() = 0;

};