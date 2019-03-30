INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/qusbglobal.h \
    $$PWD/qusbdevice.h \
    $$PWD/qusbinfo.h \
    $$PWD/qusbtransfer.h

PRIVATE_HEADERS += \
    $$PWD/qusbdevice_p.h \
    $$PWD/qusbinfo_p.h \
    $$PWD/qusbtransfer_p.h

SOURCES += \
    $$PWD/qusbinfo.cpp \
    $$PWD/qusbdevice.cpp \
    $$PWD/qusbtransfer.cpp

win32 {
    LIBS_PRIVATE += -L$$PWD/../ -L$$PWD/../../ Advapi32.lib
    CONFIG(debug, debug|release) {
        LIBS_PRIVATE += libusb-1.0d.lib
    }
    CONFIG(release, debug|release) {
        LIBS_PRIVATE += libusb-1.0.lib
    }
    INCLUDEPATH += $$PWD/libusb $$PWD/../libusb $$PWD/../../libusb
}

unix {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
