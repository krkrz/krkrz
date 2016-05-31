#pragma once

DECLARE_INTERFACE_(IOggSeekTable, IUnknown)
{
public:
	virtual void buildSeekTable() = 0;
};
