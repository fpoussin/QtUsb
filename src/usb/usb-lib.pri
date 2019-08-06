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

# We build libusb ourselves instead of using a library
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

    INCLUDEPATH += $$LIBUSB_ROOT_REL/libusb $$LIBUSB_ROOT_REL/libusb/os $$LIBUSB_ROOT_REL/android $$PWD/libusb-1.0
}
else:unix {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using PKGCONFIG")
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
