@echo off

IF NOT DEFINED clset (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\build\vcvarsall.bat" x64)

:: docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=vs-2019

set MultithreadCompile=-MT
set NoLogo=-nologo
set RuntimeInformation=-GR-
set ModernCatchExceptions=-EHa-
set NoExternCExceptions=-EHsc
set DisableOptimization=-Od
set GenerateIntrinsics=-Oi
set WarningLevel=-W2
set WarningAsErrors=-WX
set DisplayFullSourcePath=-FC
set GenerateVersion7Debugging=-Z7
set Runtime=-MD
set GarbageWindows=kernel32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib

set IgnoreConversionOfGreaterSize=-wd4312
set IgnoreTrunctionOfPointerType=-wd4311 -wd4302

set IncludeRegistryLib=Advapi32.lib

set CommonCompilerFlags=%NoLogo% %RuntimeInformation% %ModernCatchExceptions% %NoExternCExceptions% %DisableOptimization% %GenerateIntrinsics% %WarningLevel% %WarningsAsErrors% %DisplayFullSourcePaths% %GenerateVersion7Debugging% %IgnoreConversionOfGreaterSize% %IgnoreTrunctionOfPointerType% %Runtime%

set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib %IncludeRegistryLib% %GarbageWindows%
set ProjectLinkerFlags=glew32.lib glew32s.lib glfw3.lib glfw3dll.lib
set ImguiProjectFiles= 
set Default=

IF NOT EXIST ..\build  mkdir ..\build
pushd ..\build

del *.obj /Q

REM 64 bit build
cl %CommonCompilerFlags% /I ..\include ..\source\GLWindow.cpp /link %ProjectLinkerFlags% %CommonLinkerFlags% /libpath:..\lib /NODEFAULTLIB:library

popd