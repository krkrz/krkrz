# Microsoft Developer Studio Project File - Name="krmovie" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=krmovie - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "krmovie.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "krmovie.mak" CFG="krmovie - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "krmovie - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "krmovie - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "krmovie - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\bin\win32"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRMOVIE_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /O2 /I "..\..\..\..\plugins\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRMOVIE_EXPORTS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 strmbase.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib winmm.lib quartz.lib /nologo /base:"0x62000000" /dll /machine:I386 /COMMENT:"(--has-option--)"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "krmovie - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\bin\win32"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRMOVIE_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\plugins\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRMOVIE_EXPORTS" /D "DEBUG" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmbasd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib winmm.lib quartz.lib /nologo /base:"0x62000000" /dll /debug /machine:I386 /pdbtype:sept /COMMENT:"(--has-option--)"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "krmovie - Win32 Release"
# Name "krmovie - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\asyncio.cpp
# End Source File
# Begin Source File

SOURCE=.\asyncrdr.cpp
# End Source File
# Begin Source File

SOURCE=.\BufferRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\DShowException.cpp
# End Source File
# Begin Source File

SOURCE=.\dslayerd.cpp
# End Source File
# Begin Source File

SOURCE=.\dsmovie.cpp
# End Source File
# Begin Source File

SOURCE=.\dsoverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\IBufferRenderer_i.c
# End Source File
# Begin Source File

SOURCE=.\IRendererBufferAccess_i.c
# End Source File
# Begin Source File

SOURCE=.\IRendererBufferVideo_i.c
# End Source File
# Begin Source File

SOURCE=.\krlmovie.cpp
# End Source File
# Begin Source File

SOURCE=.\krmovie.cpp
# End Source File
# Begin Source File

SOURCE=.\krmovie.def
# End Source File
# Begin Source File

SOURCE=.\krmovie.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\..\plugins\win32\tp_stub.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\asyncio.h
# End Source File
# Begin Source File

SOURCE=.\asyncrdr.h
# End Source File
# Begin Source File

SOURCE=.\BufferRenderer.h
# End Source File
# Begin Source File

SOURCE=.\CIStream.h
# End Source File
# Begin Source File

SOURCE=.\DShowException.h
# End Source File
# Begin Source File

SOURCE=.\dslayerd.h
# End Source File
# Begin Source File

SOURCE=.\dsmovie.h
# End Source File
# Begin Source File

SOURCE=.\dsoverlay.h
# End Source File
# Begin Source File

SOURCE=.\IBufferRenderer.h
# End Source File
# Begin Source File

SOURCE=.\IRendererBufferAccess.h
# End Source File
# Begin Source File

SOURCE=.\IRendererBufferVideo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\plugins\win32\tp_stub.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
