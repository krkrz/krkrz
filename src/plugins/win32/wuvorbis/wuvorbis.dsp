# Microsoft Developer Studio Project File - Name="wuvorbis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=wuvorbis - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "wuvorbis.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "wuvorbis.mak" CFG="wuvorbis - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "wuvorbis - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "wuvorbis - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wuvorbis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\bin\win32\plugin\"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WUVORBIS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W3 /GX /Zi /O2 /I "vorbis/include" /I "ogg/include" /I "..\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WUVORBIS_EXPORTS" /D "DECODE_ONLY" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x11000000" /dll /machine:I386 /COMMENT:"(--has-option--)"
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "wuvorbis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\bin\win32\plugin\"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WUVORBIS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MTd /W3 /Gm /GX /ZI /Od /I "vorbis/include" /I "ogg/include" /I "..\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WUVORBIS_EXPORTS" /D "DECODE_ONLY" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /i "c:\vorbis\lib\vorbis" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x11000000" /dll /debug /machine:I386 /pdbtype:sept /COMMENT:"(--has-option--)"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "wuvorbis - Win32 Release"
# Name "wuvorbis - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ogg\src\bitwise.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\block.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\codebook.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\floor0.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\floor1.c
# End Source File
# Begin Source File

SOURCE=.\ogg\src\framing.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\info.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lpc.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lsp.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\mapping0.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\mdct.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\registry.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\res0.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\sharedbook.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\synthesis.c
# End Source File
# Begin Source File

SOURCE=..\tp_stub.cpp
# End Source File
# Begin Source File

SOURCE=.\tvpsnd.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\vorbisfile.c
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\window.c
# End Source File
# Begin Source File

SOURCE=.\wuvorbis.def
# End Source File
# Begin Source File

SOURCE=.\wuvorbis.rc
# End Source File
# Begin Source File

SOURCE=.\WuVorbisMainUnit.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\vorbis\lib\backends.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\bitrate.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\codebook.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\codec_internal.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lookup.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lookup_data.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lpc.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\lsp.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\masking.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\mdct.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\os.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\psy.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\registry.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\scales.h
# End Source File
# Begin Source File

SOURCE=..\tp_stub.h
# End Source File
# Begin Source File

SOURCE=.\vorbis\lib\window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\..\..\core\visual\IA32\detect_cpu.obj
# End Source File
# End Target
# End Project
