@echo off
rmdir /S /Q k2compat
tvpwin32.exe -export=k2compat
krkrrel.exe -nowriterpf -out k2compat.xp3 -go k2compat
pause
