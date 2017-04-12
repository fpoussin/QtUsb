
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

linux-g++* {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
}

msvc {
  LIBS += -L$$_PRO_FILE_PWD_ libusb-1.0.lib
}
else:LIBS += -lusb-1.0

