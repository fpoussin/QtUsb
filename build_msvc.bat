@echo off
setlocal

IF "%1"=="" (
  echo "Missing QT path ie: C:\Qt\5.14.1\msvc2017"
  pause
  exit /b 1c
)

SET QTDIR=%1
SET PROJDIR=%PWD%
SET BUILDTOOL=jom

CALL "C:\Program Files (x86)\Microsoft Visual Studio\%MSVC%\Community\VC\Auxiliary\Build\vcvarsall.bat" %vcarch%

SET PATH=%QTDIR%\bin;C:\Qt\Tools\QtCreator\bin;%PATH%

WHERE /Q jom
IF %errorlevel% NEQ 0 set BUILDTOOL=nmake

git submodule update --init --recursive

RMDIR /S /Q build
MKDIR ../build-qtusb-msvc
CD ../build-qtusb-msvc
%QTDIR%\bin\qmake.exe %PROJDIR%
%BUILDTOOL% install
CD tests\auto
%BUILDTOOL% check

endlocal
