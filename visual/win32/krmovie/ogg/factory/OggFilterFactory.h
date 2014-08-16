
#pragma once

struct IBaseFilter;
extern "C" {
IBaseFilter* __stdcall CreateVorbisDecoder();
IBaseFilter* __stdcall CreateTheoraDecoder();
IBaseFilter* __stdcall CreateOggSplitter();
};
