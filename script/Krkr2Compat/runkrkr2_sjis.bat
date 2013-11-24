@echo off
rmdir /S /Q data_sjis
tvpwin32.exe -export=sjis
krkr.exe data_sjis
pause
