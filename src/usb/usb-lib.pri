QT += core-private
CONFIG += c++11

INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/qusbglobal.h \
    $$PWD/qusbdevice.h \
    $$PWD/qusbinfo.h \
    $$PWD/qusbendpoint.h \
    $$PWD/qhiddevice.h

PRIVATE_HEADERS += \
    $$PWD/qusbdevice_p.h \
    $$PWD/qusbinfo_p.h \
    $$PWD/qusbendpoint_p.h \
    $$PWD/qhiddevice_p.h

SOURCES += \
    $$PWD/qusbendpoint.cpp \
    $$PWD/qusbdevice.cpp \
    $$PWD/qusbinfo.cpp \
    $$PWD/qhiddevice.cpp

win32 {
    LIBS_PRIVATE += -L$$PWD/../ -L$$PWD/../../ Advapi32.lib Setupapi.lib
    CONFIG(debug, debug|release) {
        LIBS_PRIVATE += libusb-1.0d.lib
    }
    CONFIG(release, debug|release) {
        LIBS_PRIVATE += libusb-1.0.lib
    }
    # Build the hid library ourselves since it's only one file
    SOURCES += $$PWD/../../hidapi/windows/hid.c

    INCLUDEPATH += $$PWD/libusb $$PWD/../../libusb
    INCLUDEPATH += $$PWD/hidapi $$PWD/../../hidapi/hidapi
}

# We build libusb and hidapi ourselves instead of using a library
android {
    LIBUSB_ROOT_REL = $$PWD/../../libusb
    SOURCES += \
    $$LIBUSB_ROOT_REL/libusb/core.c \
    $$LIBUSB_ROOT_REL/libusb/descriptor.c \
    $$LIBUSB_ROOT_REL/libusb/hotplug.c \
    $$LIBUSB_ROOT_REL/libusb/io.c \
    $$LIBUSB_ROOT_REL/libusb/sync.c \
    $$LIBUSB_ROOT_REL/libusb/strerror.c \
    $$LIBUSB_ROOT_REL/libusb/os/linux_usbfs.c \
    $$LIBUSB_ROOT_REL/libusb/os/poll_posix.c \
    $$LIBUSB_ROOT_REL/libusb/os/threads_posix.c \
    $$LIBUSB_ROOT_REL/libusb/os/linux_netlink.c

    # We have to copy the header for includes to work in our library
    system("mkdir -p $$PWD/libusb-1.0 && cp $$LIBUSB_ROOT_REL/libusb/libusb.h $$PWD/libusb-1.0/")

    INCLUDEPATH += \
    $$LIBUSB_ROOT_REL/libusb \
    $$LIBUSB_ROOT_REL/libusb/os \
    $$LIBUSB_ROOT_REL/android $$PWD/libusb-1.0

    # HIDAPI
    HIDAPI_ROOT_REL = $$PWD/../../hidapi
    SOURCES += $$HIDAPI_ROOT_REL/libusb/hid.c
    INCLUDEPATH += $$HIDAPI_ROOT_REL/hidapi
}
else:unix {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
    PKGCONFIG += libusb-1.0

    !packagesExist(hidapi-libusb):error("Could not find hidapi-libusb using PKGCONFIG")
    PKGCONFIG += hidapi-libusb

    CONFIG += link_pkgconfig
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
