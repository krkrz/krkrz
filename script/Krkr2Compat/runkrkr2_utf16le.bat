@echo off
rmdir /S /Q data_utf16le
tvpwin32.exe -export=utf16le
krkr.exe data_utf16le
pause
