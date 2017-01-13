perl gentext_android.pl
REM copy tjsErrorInc.h ..\..\tjs2\tjsErrorInc.h
copy MsgImpl.h ..\android\MsgImpl.h
REM copy MsgIntfInc.h ..\MsgIntfInc.h
copy MsgLoad.cpp ..\android\MsgLoad.cpp
copy arrays-ja.xml ..\..\android\app\src\main\res\values-ja\arrays.xml
copy arrays-en.xml ..\..\android\app\src\main\res\values\arrays.xml
pause

