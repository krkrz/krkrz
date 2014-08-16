#include "oggstdafx.h"
#include "CustomOggChainGranuleSeekTable.h"
#undef min

CustomOggChainGranuleSeekTable::CustomOggChainGranuleSeekTable()
	:	AutoOggChainGranuleSeekTable(TEXT(""))
{

}

CustomOggChainGranuleSeekTable::~CustomOggChainGranuleSeekTable()
{
}

bool CustomOggChainGranuleSeekTable::buildTable()
{
    return true;
}

bool CustomOggChainGranuleSeekTable::buildTable(IAsyncReader* inReader)
{
    LONGLONG total = 0;
    LONGLONG available = 0;
    if (FAILED(inReader->Length(&total, &available)) || total > available)
    {
        return false;
    }

    const LONGLONG BUFFER_SIZE = 4096;
    unsigned char* buffer = new unsigned char[BUFFER_SIZE];

    bool isEnabled = true;
    LONGLONG position = 0;
    
    while (position < total) 
    {
        // This construction is needed because IAsyncReader::SyncRead
        // doesn't report the number of bytes actually read.
        LONG bytesToRead = static_cast<LONG>(std::min(BUFFER_SIZE, total - position));
        if (inReader->SyncRead(position, bytesToRead, buffer) == S_OK) 
        {
            if (mOggDemux->feed(buffer, bytesToRead) == OggDataBuffer::FEED_OK) 
            {
                position += bytesToRead;
                continue;
            }
        }
        //An error has occurred.
        isEnabled = false;
        break;
    }

    delete[] buffer;
    mIsEnabled = isEnabled;	

    return isEnabled;
}
