INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/qusbglobal.h \
    $$PWD/qusb.h \
    $$PWD/qusbdevice.h \
    $$PWD/qusbtypes.h \
    $$PWD/qusbmanager.h

PRIVATE_HEADERS += \
    $$PWD/qbaseusb_p.h \
    $$PWD/qusbcompat_p.h

SOURCES += \
    $$PWD/qusbmanager.cpp \
    $$PWD/qbaseusb.cpp \
    $$PWD/qlibusb.cpp

win32 {
    LIBS_PRIVATE += -L$$PWD/ libusb-1.0.lib
}

unix {
   !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
   CONFIG += link_pkgconfig
   PKGCONFIG += libusb-1.0
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
