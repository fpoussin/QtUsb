**QtUsb**
==
[![GitHub version](https://badge.fury.io/gh/fpoussin%2Fqtusb.svg)](https://badge.fury.io/gh/fpoussin%2Fqtusb)
[![Build Status](https://jenkins.netyxia.net/buildStatus/icon?job=QtUsb%2Fmaster)](https://jenkins.netyxia.net/job/QtUsb/job/master/)  

A Cross-platform USB Module for Qt.
Relies on libusb-1.0.

**Features**

- Bulk transfer
- Device insertion/removal detection
- Device search

**To build**

```shell
mkdir build && cd build
qmake ..
make install
```

**TODO**

- Interrupt transfer
- isochronous transfer

**Usage**

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

**Documentation**  

Doxygen documentation can be found here: http://fpoussin.github.io/doxygen/qtusb/

**Downloads**

Ubuntu PPA: https://launchpad.net/~fpoussin/+archive/ubuntu/ppa 
Windows libraries are coming soon.
