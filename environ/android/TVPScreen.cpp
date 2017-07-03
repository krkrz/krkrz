
#include "tjsCommHead.h"

#include "TVPScreen.h"
#include "Application.h"


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "krkrz", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "krkrz", __VA_ARGS__))

int tTVPScreen::GetWidth() {
	return Application->GetScreenWidth();
}
int tTVPScreen::GetHeight() {
	return Application->GetScreenHeight();
}
// 上部のシステムバーなどを考慮して返す方がよいかもしれない
int tTVPScreen::GetDesktopLeft() {
	return 0;
}
int tTVPScreen::GetDesktopTop() {
	return 0;
}
int tTVPScreen::GetDesktopWidth() {
	return Application->GetScreenWidth();
}
int tTVPScreen::GetDesktopHeight() {
	return Application->GetScreenHeight();
}

