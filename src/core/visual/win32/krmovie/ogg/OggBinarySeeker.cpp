#include "oggstdafx.h"
#include "OggBinarySeeker.h"

OggBinarySeeker::OggBinarySeeker(void)
{
}

OggBinarySeeker::~OggBinarySeeker(void)
{
}

OggSeekTable::tSeekPair OggBinarySeeker::getStartPos(LOOG_INT64 inTime) {
	return OggSeekTable::tSeekPair();
}
