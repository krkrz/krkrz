#pragma once

#include <array>
#include <map>
#include <memory>
#include <vector>

#include <Windows.h>

#include "tjsTypes.h"

#include "SDL.h"

const char* ConvertToString(Uint8 aButton);
size_t ConvertToIndex(Uint8 aButton);

class Controller
{
public:
    Controller()
    {
        __debugbreak();
    }

    Controller(SDL_GameController* aGameController);

    void Reset();
    float ToFloat(Sint16 aValue);
    void HandleMotion(SDL_ControllerAxisEvent& aEvent);
    void HandleButton(SDL_ControllerButtonEvent& aEvent);

    SDL_GameController* mGameController;
    const char* mName;

    std::array<bool, 15> mPreviousButtons;
    std::array<bool, 15> mCurrentButtons;

    struct RepeatInfo
    {
        ULONGLONG Time = 0; // When the key was pressed/last repeated
        bool Interval = false; // Is this a subsequent repeat?
    };
    std::array<RepeatInfo, 15> RepeatWait;

    float LeftTrigger;
    float RightTrigger;

    const std::vector<WORD>& GetUppedKeys() const { return UppedKeys; }
    const std::vector<WORD>& GetDownedKeys() const { return DownedKeys; }
    const std::vector<WORD>& GetRepeatKeys() const { return RepeatKeys; }

    std::vector<WORD> UppedKeys;
    std::vector<WORD> DownedKeys;
    std::vector<WORD> RepeatKeys;
};

class SdlInputMgr
{
public:
	SdlInputMgr(HWND handle);
	~SdlInputMgr();

	void Update();
    static SdlInputMgr* sInstance;
    std::map<SDL_JoystickID, Controller> mControllers;
private:
    void UpdateKeyRepeatTimes();

    INT32 HoldTime = 500; // keyboard key-repeats hold-time
    INT32 IntervalTime = 30; // keyboard key-repeats interval-time
};

bool SdlGetJoyPadAsyncState(tjs_uint keycode, bool getcurrent);
