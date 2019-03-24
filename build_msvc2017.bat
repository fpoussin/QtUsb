
SET ARCH=%1
SET QTDIR=%2

SET vcarch=%ARCH%
SET usbarch=%ARCH%
IF "%ARCH%" == "x64" SET vcarch=amd64
IF "%ARCH%" == "x86" SET usbarch=Win32

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %vcarch%

SET PATH=%QTDIR%\bin;%PATH%

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
nmake install
