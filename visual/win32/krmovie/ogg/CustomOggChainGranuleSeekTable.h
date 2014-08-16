#pragma once

#include "libOOOggSeek/AutoOggChainGranuleSeekTable.h"

class CustomOggChainGranuleSeekTable : public AutoOggChainGranuleSeekTable
{
public:
	CustomOggChainGranuleSeekTable();
	virtual ~CustomOggChainGranuleSeekTable();

	virtual bool buildTable();
    virtual bool buildTable(IAsyncReader* inReader);
};
