
DEFINES += QUSB_LIBRARY QLIBUSB

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
VPATH += $$PWD

SOURCES += \
    qbaseusb.cpp \
    qusbmanager.cpp \
    qlibusb.cpp

HEADERS +=  \
    qusb_global.h \
    qusb_compat.h \
    qbaseusb.h \
    qusb.h \
    QUsb \
    qusbmanager.h \
    qusb_types.h \
    qlibusb.h

unix {
   !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
   CONFIG += link_pkgconfig
   PKGCONFIG += libusb-1.0
}

msvc {
  HEADERS += $$PWD/libusb-1.0/libusb.h
  LIBS += -L$$PWD/ libusb-1.0.lib
}

