#-------------------------------------------------
#
# Project created by QtCreator 2014-04-14T14:17:36
#
#-------------------------------------------------

VERSION = 0.1

QT      -= gui

TARGET   = QtUsb
TEMPLATE = lib

DEFINES += QUSB_LIBRARY

CONFIG  += static_and_shared

SOURCES += \
    qbaseusb.cpp

HEADERS +=  \
    qusb_global.h \
    compat.h \
    qbaseusb.h \
    qusb.h

win32 {
    message(Building with WinUSB support.)
    DEFINES += QWINUSB
    SOURCES += qwinusb.cpp
    HEADERS += qwinusb.h
}

else:unix {
    message(Building with LibUsb support.)
    DEFINES += QLIBUSB
    SOURCES += qlibusb.cpp
    HEADERS += qlibusb.h
    LIBS    += -lusb-1.0
}

else {
    error("Platform not supported!")
}

headers_install.files = $$HEADERS

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
        headers_install.path = /opt/usr/include/qt4/QtUsb
    } else {
        target.path = /usr/lib
        headers_install.path = /usr/include/qt4/QtUsb
    }
    INSTALLS += target headers_install
}
