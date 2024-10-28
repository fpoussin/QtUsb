# QtUsb [![GitHub version](https://badge.fury.io/gh/fpoussin%2Fqtusb.svg)](https://badge.fury.io/gh/fpoussin%2Fqtusb)  

GCC: [![Build Status](https://github.com/fpoussin/QtUsb/actions/workflows/build.yml/badge.svg)](https://github.com/fpoussin/QtUsb/actions/)  
MSVC: [![Build status](https://ci.appveyor.com/api/projects/status/4ns2jbdoveyj8n0y?svg=true)](https://ci.appveyor.com/project/fpoussin/qtusb)  


A Cross-platform USB Module for Qt built around libusb-1.0 and libhidapi  
Can be used as a library, or included directly into the project  

## Features

- Bulk transfer
- Interrupt transfer
- Hotplug detection
- Device enumeration and filtering
- HID

## Install library 

**Ubuntu** (stable versions only)  
```
sudo add-apt-repository ppa:fpoussin/ppa
sudo apt install libqt5usb5 libqt5usb5-dev
```

**Windows**  
Check the [releases](https://github.com/fpoussin/QtUsb/releases) page or [appveyor build artifacts](https://ci.appveyor.com/project/fpoussin/qtusb) for binary archives  

## Build  

**Unix**  
You need `libusb-1.0-0-dev`, `libhidapi-dev` and `pkg-config` packages installed  
If using system packages on Ubuntu, you'll need `qt6-base-private-dev` as well.  
```shell
mkdir build && cd build
cmake ..
make install
```

Alternatively build as static module (best for portability)  
```shell
mkdir build && cd build
cmake -DQTUSB_AS_STATIC_MODULE=ON ..
make install
```

**MSVC**  
You need the Windows SDKs to compile libusb  
These are available from the Visual Studio Installer  
The following script builds and deploys QtUsb into your Qt installation  
```
build_msvc.bat 2017|2019 x64|x86 module|static QT_PATH
ie: build_msvc.bat 2017 x64 static C:\Qt\5.14.1\msvc2017_64
```

**Qt Creator**  
The module can also be built normally within QT creator regardless of the platform.  
All dependencies are built with cmake.  

## Using  

**Option 1: Using the module (static or dynamic)**  
You'll need to add the module to into CMakeLists.txt:
```
find_package(Qt6 REQUIRED COMPONENTS QtUsb)
```
Include headers:  
```
#include <QUsbDevice>
#include <QUsbEndpoint>
```

**Option 2: Importing the code in your project**  
This will tie your app to a specific Qt version as it uses private headers  
You need to add the QtUsb subdirectoty into CMakeLists.txt:
```
add_subdirectory(QtUsb)
```
Include headers:  
```
#include "qusbdevice.h"
#include "qusbendpoint.h"
```

## Documentation

QCH files can be found with each release, they are also included in ubuntu packages.  
You have to manually install them in Qt Creator on Windows.  
Online documentation can be found [here](https://fpoussin.github.io/doxygen/qtusb/0.7.x/)  

## Downloads

[Ubuntu PPA](https://launchpad.net/~fpoussin/+archive/ubuntu/ppa)  
Windows binaries are [in the releases section](https://github.com/fpoussin/QtUsb/releases).  
