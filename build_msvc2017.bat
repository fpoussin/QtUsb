
SET ARCH=%1
SET QTDIR=%2

SET vcarch=%ARCH%
IF "%ARCH%" == "x64" SET vcarch=amd64
IF "%ARCH%" == "Win32" SET vcarch=x86

SET usbarch=%vcarch%
IF "%vcarch%" == "x64" SET usbarch=x64
IF "%vcarch%" == "x86" SET usbarch=Win32

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %vcarch%

SET PATH=%QTDIR%\bin;%PATH%

git submodule update --init --recursive
CD libusb
msbuild msvc\libusb_static_2017.vcxproj /p:Configuration=Release
COPY %ARCH%\Release\lib\libusb-1.0.lib ..\
CD ..

MKDIR build
CD build
nmake distclean
qmake ..
nmake install
