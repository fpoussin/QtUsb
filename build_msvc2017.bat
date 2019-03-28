@echo off
setlocal

IF "%1"=="" (
  echo "Missing architecture [x86|x64]"
  pause
  exit /b 1c
)

IF "%2"=="" (
  echo "Missing QT path ie: C:\Qt\5.12.2\msvc2017"
  pause
  exit /b 1c
)

SET ARCH=%1
SET QTDIR=%2

SET BUILDTOOL=jom
SET vcarch=%ARCH%
SET usbarch=%ARCH%
IF "%ARCH%" == "x64" SET vcarch=amd64
IF "%ARCH%" == "x86" SET usbarch=Win32

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %vcarch%

SET PATH=%QTDIR%\bin;C:\Qt\Tools\QtCreator\bin;%PATH%

WHERE /Q jom
IF %errorlevel% NEQ 0 set BUILDTOOL=nmake

git submodule update --init --recursive
CD libusb
msbuild msvc\libusb_static_2017.vcxproj /p:Configuration=Release
msbuild msvc\libusb_static_2017.vcxproj /p:Configuration=Debug
COPY %usbarch%\Release\lib\libusb-1.0.lib ..\libusb-1.0.lib
COPY %usbarch%\Debug\lib\libusb-1.0.lib ..\libusb-1.0d.lib
CD ..

RMDIR /S /Q build
MKDIR build
CD build
qmake ..
%BUILDTOOL% install sub-tests
CD tests\auto
nmake check

endlocal
