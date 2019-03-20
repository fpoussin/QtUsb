# QtUsb [![GitHub version](https://badge.fury.io/gh/fpoussin%2Fqtusb.svg)](https://badge.fury.io/gh/fpoussin%2Fqtusb)  

GCC: [![Build Status](https://jenkins.netyxia.net/buildStatus/icon?job=QtUsb%2Fmaster)](https://jenkins.netyxia.net/job/QtUsb/job/master/)  
MSVC: [![Build status](https://ci.appveyor.com/api/projects/status/4ns2jbdoveyj8n0y?svg=true)](https://ci.appveyor.com/project/fpoussin/qtusb)  

A Cross-platform USB Module for Qt built around libusb-1.0  

## Features

- Bulk transfer
- Hotplug detection
- Device search

## Install

```
sudo add-apt-repository ppa:fpoussin/ppa
sudo apt install libqt5usb5 libqt5usb5-dev
```

## Build

**Unix**  
```shell   
mkdir build && cd build
qmake ..
make install
```

**MSVC 2017**  
```
build_msvc2017.bat [x64|x86] QT_PATH
ie: build_msvc2017.bat x64 C:\Qt\5.12.1\msvc2017_64
```

## Using

Documentation is not complete yet, you can have a look at the examples in the meanwhile.

You'll need to add the module to your project file:

```
qt += usb
```

Then include it into your headers:

```
#include <QUsb>

QUsbDevice mydev;
```

## TODO
- Interrupt transfer
- isochronous transfer

## Documentation

Doxygen documentation can be found here: http://fpoussin.github.io/doxygen/qtusb/

## Downloads

Ubuntu PPA: https://launchpad.net/~fpoussin/+archive/ubuntu/ppa 
Windows libraries are coming soon.
