echo ok ? otherwise press ctrl+c
pause
cd ..\..
del /Q /S *.~y
del /Q /S *.~cpp
del /Q /S *.~nas
del /Q /S *.~c
del /Q /S *.~h
del /Q /S *.~hpp
del /Q /S *.~dfm
del /Q /S *.~bpr
del /Q /S *.~bpf
del /Q /S *.tds
del /Q /S *.aps
del /Q /S *.il?
del /Q /S *.obj
del /Q /S *.kep
del /Q /S *.sfap0
del /Q /S *.sfk
del /Q /S *.rpf
del /Q /S *.dump.txt
del /Q /S *.log
del /Q /S *.pdb
del /Q /S *.bsc
del /Q /S *.cgl
del /Q /S *.pch
del /Q /S *.idb
del /Q /S *.db
del /Q /S *.cgi
del /Q /S *.plg
del /Q /S *.avi
del /Q /S *.mpg
del /Q /S *.kdt
del /Q /S *.ksd
del /Q /S *.gex
del /Q /S *.opt
del /Q /S *.ncb
del /Q /S *.exp
del /Q /S *.dll
del /Q /S *.tpm
del /Q /S *.spi
del /Q /S *.exe
del environ\win32\intermediate\*.asm
del environ\win32\kag3tags\*.html
copy docs\j\contents\index.html %temp%
del docs\j\contents\*.html
copy %temp%\index.html docs\j\contents\
del docs\j_in\*.html
del docs\j_in\classes\*.html
del environ\win32\kag3tags\*.tdb
del environ\win32\krmovie.lib
del environ\win32\msgmap.tjs
del environ\win32\scr_shot\*.bmp
del environ\win32\snapshot\*.bmp
del environ\win32\callerexe\bin\*.lib
del environ\win32\callerexe\*.lib
copy environ\win32\kag3docs\contents\index.html %temp%
del environ\win32\kag3docs\contents\*.html
copy %temp%\index.html environ\win32\kag3docs\contents\
del environ\win32\kag3docs_in\*.html
del environ\win32\kag3docs_in\tag\*.html
del environ\win32\plugin\*.lib
del environ\win32\extrans\intermediate\*.asm
del environ\win32\extrans\intermediate\*.res
del /Q /S environ\win32\savedata\*.*
del environ\win32\tpconvert\*.ini
del environ\win32\*.ini
copy tjs2\docs\j\contents\index.html %temp%
del tjs2\docs\j\contents\*.html
copy %temp%\index.html tjs2\docs\j\contents\
del tjs2\docs\j_in\*.html
del tjs2\syntax\*.output
del tjs2\syntax\*.simple
del tjs2\syntax\*.c
del tjs2\syntax\*.h
del visual\win32\glgen\tvp*.*
del /Q visual\win32\krmovie\Debug\*.*
del /Q visual\win32\krmovie\Release\*.*
del /Q base\win32\plugin_kit\xp3filter\xp3enc\Debug\*.*
del /Q base\win32\plugin_kit\xp3filter\xp3enc\Release\*.*
del /Q base\win32\plugin_kit\xp3filter\xp3dec\Debug\*.*
del /Q base\win32\plugin_kit\xp3filter\xp3dec\Release\*.*
del /Q base\win32\plugin_kit\basetest\Debug\*.*
del /Q base\win32\plugin_kit\basetest\Release\*.*
del /Q base\win32\plugin_kit\wmrdump\Debug\*.*
del /Q base\win32\plugin_kit\wmrdump\Release\*.*
del /Q environ\win32\wuvorbis\Debug\*.*
del /Q environ\win32\wuvorbis\Release\*.*
del /Q environ\win32\wuvorbis\binary\*.*
del /Q /S environ\win32\wuvorbis\vorbis\*.*
del /Q /S environ\win32\wuvorbis\ogg\*.*
del /Q /S environ\win32\wuvorbis\vorbis_nightly_cvs\*.*
