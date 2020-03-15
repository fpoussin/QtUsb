@echo off
setlocal

IF "%1"=="" (
  echo "Missing MSVC version [2017|2019]"
  pause
  exit /b 1c
)

IF "%2"=="" (
  echo "Missing architecture [x86|x64]"
  pause
  exit /b 1c
)

IF "%3"=="" (
  echo "Missing QT path ie: C:\Qt\5.14.1\msvc2017"
  pause
  exit /b 1c
)

SET MSVC=%1
SET ARCH=%2
SET QTDIR=%3

SET BUILDTOOL=jom
SET vcarch=%ARCH%
SET usbarch=%ARCH%
IF "%ARCH%" == "x64" SET vcarch=amd64
IF "%ARCH%" == "x86" SET usbarch=Win32
IF "%MSVC%" == "2017" SET MSVC_VER=v141
IF "%MSVC%" == "2019" SET MSVC_VER=v142
SET PROJDIR=%CD%
SET BUILDDIR=%PROJDIR%\..\build-qtusb-%ARCH%

CALL "C:\Program Files (x86)\Microsoft Visual Studio\%MSVC%\Community\VC\Auxiliary\Build\vcvarsall.bat" %vcarch%

SET PATH=%QTDIR%\bin;C:\Qt\Tools\QtCreator\bin;%PATH%

WHERE /Q jom
IF %errorlevel% NEQ 0 set BUILDTOOL=nmake

git submodule update --init --recursive.

echo %BUILDDIR%

RMDIR /S /Q %BUILDDIR%
MKDIR %BUILDDIR%
CD %BUILDDIR%
%QTDIR%\bin\qmake.exe CONFIG+=static_lib %PROJDIR%
%BUILDTOOL% sub-tests
CD tests\auto
nmake check

endlocal
