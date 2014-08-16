#pragma once

//Interface to set/get downmix
DECLARE_INTERFACE_(IDownmixAudio, IUnknown)
{
    virtual void __stdcall setDownmixAudio(const bool setDownmix) = 0;
    virtual bool __stdcall getDownmixAudio() = 0;
};
