INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/qusbglobal.h \
    $$PWD/qusbdevice.h \
    $$PWD/qusbtypes.h \
    $$PWD/qusbmanager.h

PRIVATE_HEADERS += \
    $$PWD/qusbdevice_p.h \
    $$PWD/qusbmanager_p.h

SOURCES += $$PWD/qlibusb.cpp \
           $$PWD/qlibusbmanager.cpp

msvc {
    LIBS_PRIVATE += -L$$PWD libusb-1.0.lib Advapi32.lib
    INCLUDEPATH += $$PWD/libusb
}

unix {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
