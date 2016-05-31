call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
MSBuild plugins.msbuild
MSBuild theora\krdstheora.sln /t:Rebuild /p:Configuration=Release /p:platform=Win32
MSBuild theora\krdstheora.sln /t:Rebuild /p:Configuration=Release /p:platform=x64
MSBuild postbuild.msbuild
pause:
