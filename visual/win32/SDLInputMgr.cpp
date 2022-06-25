//---------------------------------------------------------------------------
/*
    TVP2 ( T Visual Presenter 2 )  A script authoring tool
    Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

    See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// SDL_GameController management
//---------------------------------------------------------------------------

#include "SDL.h"

#include "tjsCommHead.h"
#include "SysInitIntf.h"

#include "SDLInputMgr.h"


#include "tvpinputdefs.h"


WORD ConvertToTjsPad(size_t button);
WORD ConvertFromTjsPad(size_t button);
size_t ConvertToIndex(Uint8 aButton);
const char* ConvertToString(Uint8 aButton);

//---------------------------------------------------------------------------
// Constructs a tTVPSDLGameController from an instance of SDL_GameController
//---------------------------------------------------------------------------
tTVPSDLGameController::tTVPSDLGameController(SDL_GameController* aGameController)
    : mGameController{ aGameController }
{
    mName = SDL_GameControllerName(aGameController);
    mPreviousButtons.fill(false);
    mCurrentButtons.fill(false);

    LeftTrigger = 0.f;
    RightTrigger = 0.f;
    Reset();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Clears our key vectors, and sets our previous state to our current state
//---------------------------------------------------------------------------
void tTVPSDLGameController::Reset()
{
    mPreviousButtons = mCurrentButtons;
    UppedKeys.clear();
    RepeatKeys.clear();
    DownedKeys.clear();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Adjusts the trigger values SDL gives us to floats between 0.0f <-> 1.0f
//---------------------------------------------------------------------------
float tTVPSDLGameController::ToFloat(Sint16 aValue)
{
    return (static_cast<float>(aValue + 32768.f) / 65535.f);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Handles axis motion events per controller.
//---------------------------------------------------------------------------
void tTVPSDLGameController::HandleMotion(SDL_ControllerAxisEvent& aEvent)
{
    switch (aEvent.axis)
    {
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: LeftTrigger = 2.f * (ToFloat(aEvent.value) - .5f); return;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: RightTrigger = 2.f * (ToFloat(aEvent.value) - .5f); return;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Handles axis button events per controller.
//---------------------------------------------------------------------------
void tTVPSDLGameController::HandleButton(SDL_ControllerButtonEvent& aEvent)
{
    auto buttonIndex = ConvertToIndex(aEvent.button);
    mCurrentButtons[buttonIndex] = aEvent.state == SDL_PRESSED ? true : false;
    //printf("%s %s\n", ConvertToString(aEvent.button), aEvent.state == SDL_PRESSED ? "Pressed" : "Released");

    if (mCurrentButtons[buttonIndex] && !mPreviousButtons[buttonIndex])
    {
        DownedKeys.emplace_back(ConvertToTjsPad(buttonIndex));
        RepeatWait[buttonIndex].Time = GetTickCount64();
    }
    else if (!mCurrentButtons[buttonIndex] && mPreviousButtons[buttonIndex])
    {
        UppedKeys.emplace_back(ConvertToTjsPad(buttonIndex));
        RepeatWait[buttonIndex].Time = 0;
        RepeatWait[buttonIndex].Interval = false;
    }
}
//---------------------------------------------------------------------------


tTVPSDLSdlGameControllerMgr* tTVPSDLSdlGameControllerMgr::sInstance = NULL;
//---------------------------------------------------------------------------
// Runs the SDL Event loop to retrieve Controller events and convert them 
// into Events we can understand.
//---------------------------------------------------------------------------
tTVPSDLSdlGameControllerMgr::tTVPSDLSdlGameControllerMgr(HWND handle)
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    sInstance = this;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// To mimic the DInputMgr, we need to continuously check for updates to 
// HoldTime and IntervalTime, this function handles that for us.
//---------------------------------------------------------------------------
tTVPSDLSdlGameControllerMgr::~tTVPSDLSdlGameControllerMgr()
{
    SDL_Quit();
    sInstance = NULL;
}

//---------------------------------------------------------------------------
// To mimic the DInputMgr, we need to continuously check for updates to 
// HoldTime and IntervalTime, this function handles that for us.
//---------------------------------------------------------------------------
void tTVPSDLSdlGameControllerMgr::UpdateKeyRepeatTimes()
{
    static tjs_int ArgumentGeneration = 0;
    if (ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
    {
        ArgumentGeneration = TVPGetCommandLineArgumentGeneration();
        tTJSVariant val;
        if (TVPGetCommandLine(TJS_W("-paddelay"), &val))
        {
            HoldTime = (int)val;
        }
        if (TVPGetCommandLine(TJS_W("-padinterval"), &val))
        {
            IntervalTime = (int)val;
        }
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Runs the SDL Event loop to retrieve Controller events and convert them 
// into Events we can understand.
//---------------------------------------------------------------------------
void tTVPSDLSdlGameControllerMgr::Update()
{
    SDL_Event event;

    // Reset our controllers to prepare for receiving events.
    for (auto& controller : mControllers)
    {
        controller.second.Reset();
    }

    // Receive Controller events.
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_CONTROLLERDEVICEADDED:
            {
                auto deviceEvent = event.cdevice;

                auto pad = SDL_GameControllerOpen(deviceEvent.which);

                if (pad)
                {
                    auto name = SDL_GameControllerName(pad);
                    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(pad);
                    SDL_JoystickID instanceId = SDL_JoystickInstanceID(joystick);

                    printf("Added %s, %d\n", name, instanceId);
                    mControllers.emplace(instanceId, tTVPSDLGameController(pad));
                }
                break;
            }
            case SDL_CONTROLLERDEVICEREMOVED:
            {
                auto deviceEvent = event.cdevice;
                auto it = mControllers.find(deviceEvent.which);
                if (it != mControllers.end())
                {
                    mControllers.erase(it);
                }

                printf("Removed %d\n", deviceEvent.which);
                break;
            }
            case SDL_CONTROLLERDEVICEREMAPPED:
            {
                puts("Remaped\n");
                break;
            }
            case SDL_CONTROLLERAXISMOTION:
            {
                mControllers[event.caxis.which].HandleMotion(event.caxis);
                break;
            }
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                mControllers[event.cbutton.which].HandleButton(event.cbutton);
                break;
            }
        }
    }


    // Simulates Key Repeat.
    UpdateKeyRepeatTimes();
    auto now = GetTickCount64();

    for (auto& controllerAndKey : mControllers)
    {
        auto& controller = controllerAndKey.second;
        for (size_t i = 0; i < controller.mCurrentButtons.size(); ++i)
        {
            if (controller.mCurrentButtons[i] && controller.mPreviousButtons[i])
            {
                // Interval tells us if this is the first time to repeat (false) or a subsequent repeat,
                // this is needed because the first repeat traditionally has it's own delay.
                auto const waitTime = controller.RepeatWait[i].Interval ? IntervalTime : HoldTime;
                if ((now - controller.RepeatWait[i].Time) > waitTime)
                {
                    controller.RepeatKeys.emplace_back(ConvertToTjsPad(i));

                    // If we've hit this condition, definitionally subsequent iterations will be an 
                    // interval. Similarly, we need to update the last repeat time to now so that 
                    // subsequent iterations will have the correct delay.
                    controller.RepeatWait[i].Interval = true;
                    controller.RepeatWait[i].Time = now;
                }
            }
        }
    }

}

//---------------------------------------------------------------------------
// Returns the state of the requested button on the first gamepad.
//---------------------------------------------------------------------------
bool TVPGetSdlGameControllerAsyncState(tjs_uint keycode)
{
    auto code = ConvertFromTjsPad(keycode);

    if (tTVPSDLSdlGameControllerMgr::sInstance)
    {
        auto it = tTVPSDLSdlGameControllerMgr::sInstance->mControllers.find(0);
        if (it != tTVPSDLSdlGameControllerMgr::sInstance->mControllers.end())
        {
            return it->second.mCurrentButtons[code];
        }
        return false;
    }

	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
WORD ConvertToTjsPad(size_t button)
{
    switch (button)
    {
        case SDL_CONTROLLER_BUTTON_A: return VK_PAD1;
        case SDL_CONTROLLER_BUTTON_B: return VK_PAD2;
        case SDL_CONTROLLER_BUTTON_X: return VK_PAD3;
        case SDL_CONTROLLER_BUTTON_Y: return VK_PAD4;
        case SDL_CONTROLLER_BUTTON_BACK: return VK_PAD7;
        case SDL_CONTROLLER_BUTTON_START: return VK_PAD8;
        case SDL_CONTROLLER_BUTTON_GUIDE: return VK_PAD8;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return VK_PAD9;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return VK_PAD10;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return VK_PAD5;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return  VK_PAD6;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return  VK_PADUP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return  VK_PADDOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return  VK_PADLEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return  VK_PADRIGHT;
    }

    return VK_PAD2;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
WORD ConvertFromTjsPad(size_t button)
{
    switch (button)
    {
        case VK_PAD1: return SDL_CONTROLLER_BUTTON_A;
        case VK_PAD2: return SDL_CONTROLLER_BUTTON_B;
        case VK_PAD3: return SDL_CONTROLLER_BUTTON_X;
        case VK_PAD4: return SDL_CONTROLLER_BUTTON_Y;
        case VK_PAD7: return SDL_CONTROLLER_BUTTON_BACK;
        case VK_PAD8: return SDL_CONTROLLER_BUTTON_START;
        case VK_PAD9: return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case VK_PAD10: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        case VK_PAD5: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case VK_PAD6: return  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case VK_PADUP: return  SDL_CONTROLLER_BUTTON_DPAD_UP;
        case VK_PADDOWN: return  SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case VK_PADLEFT: return  SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case VK_PADRIGHT: return  SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    }

    return SDL_CONTROLLER_BUTTON_B;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Converts from SDL_CONTROLLER button to name, used for debugging.
//---------------------------------------------------------------------------
const char* ConvertToString(Uint8 aButton)
{
    switch (aButton)
    {
        case SDL_CONTROLLER_BUTTON_A: return "SDL_CONTROLLER_BUTTON_A";
        case SDL_CONTROLLER_BUTTON_B: return "SDL_CONTROLLER_BUTTON_B";
        case SDL_CONTROLLER_BUTTON_X: return "SDL_CONTROLLER_BUTTON_X";
        case SDL_CONTROLLER_BUTTON_Y: return "SDL_CONTROLLER_BUTTON_Y";
        case SDL_CONTROLLER_BUTTON_BACK: return "SDL_CONTROLLER_BUTTON_BACK";
        case SDL_CONTROLLER_BUTTON_GUIDE: return "SDL_CONTROLLER_BUTTON_GUIDE";
        case SDL_CONTROLLER_BUTTON_START: return "SDL_CONTROLLER_BUTTON_START";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "SDL_CONTROLLER_BUTTON_LEFTSTICK";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "SDL_CONTROLLER_BUTTON_RIGHTSTICK";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "SDL_CONTROLLER_BUTTON_LEFTSHOULDER";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return  "SDL_CONTROLLER_BUTTON_RIGHTSHOULDER";
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return  "SDL_CONTROLLER_BUTTON_DPAD_UP";
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return  "SDL_CONTROLLER_BUTTON_DPAD_DOWN";
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return  "SDL_CONTROLLER_BUTTON_DPAD_LEFT";
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return  "SDL_CONTROLLER_BUTTON_DPAD_RIGHT";
    }

    return "ERROR";
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Converts from SDL_CONTROLLER button to index in our array.
//---------------------------------------------------------------------------
size_t ConvertToIndex(Uint8 aButton)
{
    switch (aButton)
    {
        case SDL_CONTROLLER_BUTTON_A: return 0;
        case SDL_CONTROLLER_BUTTON_B: return 1;
        case SDL_CONTROLLER_BUTTON_X: return 2;
        case SDL_CONTROLLER_BUTTON_Y: return 3;
        case SDL_CONTROLLER_BUTTON_BACK: return 4;
        case SDL_CONTROLLER_BUTTON_GUIDE: return 5;
        case SDL_CONTROLLER_BUTTON_START: return 6;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return 7;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return 8;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return 9;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return 10;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return 11;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return 12;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return 13;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return 14;
    }

    return static_cast<size_t>(-1);
}
//---------------------------------------------------------------------------