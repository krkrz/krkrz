#pragma once

DECLARE_INTERFACE_(IOggBaseTime, IUnknown)
{
public:
	
	virtual __int64 getGlobalBaseTime() = 0;
};
