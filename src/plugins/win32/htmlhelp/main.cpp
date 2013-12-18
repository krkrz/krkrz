#include <windows.h>
#include <tp_stub.h>
#include <ncbind.hpp>
#include <Htmlhelp.h>
#include <string>


// ttstrをUTF8文字列へ変換
std::string convertTtstrToUtf8String(ttstr &buf)
{
  int maxlen = buf.length() * 6 + 1;
  char *dat = new char[maxlen];
  int len = TVPWideCharToUtf8String(buf.c_str(), dat);
  std::string result(dat, len);
  delete[] dat;
  return result;
}


// クッキー
DWORD sCookie = NULL;

// HTMLヘルプを初期化
void RegisterFunc(void)
{
  HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&sCookie);
}

NCB_PRE_REGIST_CALLBACK(RegisterFunc);

// HTMLヘルプを終了
void UnregisterFunc(void)
{
  HtmlHelp(NULL, NULL, HH_UNINITIALIZE, (DWORD)sCookie);
}

NCB_POST_UNREGIST_CALLBACK(UnregisterFunc);

// クラスに登録
class HtmlHelpClass
{
public:
  // トピックを表示する
  static void displayTopic(ttstr path) {
    HtmlHelp(NULL,
	     path.c_str(),
	     HH_DISPLAY_TOPIC,
	     NULL);
  }
};

NCB_REGISTER_CLASS_DIFFER(HtmlHelp, HtmlHelpClass)
{
  NCB_METHOD(displayTopic);
};
