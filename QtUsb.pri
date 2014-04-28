
DEPENDPATH += QtUsb
INCLUDEPATH += QtUsb

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
    message(Building with LibUsb 1.0 support.)
    DEFINES += QLIBUSB
    SOURCES += qlibusb.cpp
    HEADERS += qlibusb.h
    LIBS    += -lusb-1.0
}

else {
    error("Platform not supported!")
}

