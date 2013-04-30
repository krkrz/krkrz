echo off
del tvpgl_ia32.lib
del *.obj
for %%d in (*.nas) do nasmw -g -f obj %%d
for %%d in (*.obj) do tlib tvpgl_ia32.lib + %%d
perl summary.pl
del *.bak
perl copy_if_differ.pl tvpgl_ia32_intf_.c tvpgl_ia32_intf.c
perl copy_if_differ.pl tvpgl_ia32_intf_.h tvpgl_ia32_intf.h
pause
cd ..\..\base\win32
call makestub.bat
