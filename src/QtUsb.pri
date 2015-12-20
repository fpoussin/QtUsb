
DEFINES += QUSB_LIBRARY

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
VPATH += $$PWD

SOURCES += \
    qbaseusb.cpp \
    qusbmanager.cpp

HEADERS +=  \
    qusb_global.h \
    qusb_compat.h \
    qbaseusb.h \
    qusb.h \
    QUsb \
    qusbmanager.h \
    qusb_types.h

win32-msvc2015:message("MSVC2015 has some performance issues on Win7; stick to 2013 or earlier.")

msvc {
    message(Building QtUsb with WinUSB support.)
    DEFINES += QWINUSB
    SOURCES += qwinusb.cpp
    HEADERS += qwinusb.h
    LIBS += setupapi.lib winusb.lib
}

else:unix {
    message(Building QtUsb with LibUsb 1.0 support.)
    DEFINES += QLIBUSB
    SOURCES += qlibusb.cpp
    HEADERS += qlibusb.h
    LIBS    += -lusb-1.0
}

else {
    error("Platform not supported!")
}

