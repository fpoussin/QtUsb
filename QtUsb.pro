#-------------------------------------------------
#
# Project created by QtCreator 2014-04-14T14:17:36
#
#-------------------------------------------------

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

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

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
