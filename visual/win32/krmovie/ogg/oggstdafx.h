#pragma once

#include <windows.h>

# if (defined(WIN32) || defined(WINCE))
# define LOOG_INT64 signed __int64
# define LOOG_UINT64 unsigned __int64
#else
# define LOOG_INT64 int64_t
# define LOOG_UINT64 uint64_t
#endif

#include <string>
#include <vector>
#include <algorithm>

#include <atlbase.h>
#include <atlcom.h>

#include <dshow.h>
#include <qnetwork.h>

#include <streams.h>
#include <pullpin.h>
#include <dvdmedia.h>

#include "AbstractTransformInputPin.h"
#include "AbstractTransformOutputPin.h"
#include "AbstractTransformFilter.h"

#include "libOOOgg.h"
#include "libOOOggSeek.h"

#include "AbstractTransformFilter.h"
#include "AbstractTransformInputPin.h"
#include "AbstractTransformOutputPin.h"

#include "VorbisDecodeInputPin.h"
#include "VorbisDecodeOutputPin.h"
#include "VorbisDecodeFilter.h"

#include "iLE_Math.h"
#include "OggPacket.h"
#include "ogglog.h"


// using namespace std;
