QT += core-private
CONFIG += c++11

INCLUDEPATH += $$PWD

H = $${LITERAL_HASH}

QUSBGLOBAL_S_CONT = \
    "$${H}pragma once" \
    "$${H}define Q_USB_EXPORT" \
    ""

QUSBGLOBAL_M_CONT = \
    "$${H}pragma once" \
    "$${H}include <QtCore/qglobal.h>" \
    "" \
    "$${H}if !defined(QT_STATIC)" \
    "  $${H}if defined(QT_BUILD_USB_LIB)" \
    "    $${H}define Q_USB_EXPORT Q_DECL_EXPORT" \
    "  $${H}else" \
    "    $${H}define Q_USB_EXPORT Q_DECL_IMPORT" \
    "  $${H}endif" \
    "$${H}else" \
    "  $${H}define Q_USB_EXPORT" \
    "$${H}endif" \
    ""

CONFIG(qtusb-as-module) {
    QUSBGLOBAL_CONT = $$QUSBGLOBAL_M_CONT
}
else {
    QUSBGLOBAL_CONT = $$QUSBGLOBAL_S_CONT
}

write_file("$$PWD/qusbglobal.h", QUSBGLOBAL_CONT)|error()

PUBLIC_HEADERS += \
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
    LIBS_PRIVATE += Advapi32.lib Setupapi.lib
    # Build the hid library ourselves since it's only one file
    HIDAPI_ROOT_REL = $$PWD/../../hidapi
    SOURCES += $$HIDAPI_ROOT_REL/windows/hid.c

    LIBUSB_ROOT_REL = $$PWD/../../libusb
    SOURCES += \
    $$LIBUSB_ROOT_REL/libusb/core.c \
    $$LIBUSB_ROOT_REL/libusb/descriptor.c \
    $$LIBUSB_ROOT_REL/libusb/hotplug.c \
    $$LIBUSB_ROOT_REL/libusb/io.c \
    $$LIBUSB_ROOT_REL/libusb/os/poll_windows.c \
    $$LIBUSB_ROOT_REL/libusb/strerror.c \
    $$LIBUSB_ROOT_REL/libusb/sync.c \
    $$LIBUSB_ROOT_REL/libusb/os/threads_windows.c \
    $$LIBUSB_ROOT_REL/libusb/os/windows_nt_common.c \
    $$LIBUSB_ROOT_REL/libusb/os/windows_usbdk.c \
    $$LIBUSB_ROOT_REL/libusb/os/windows_winusb.c

    INCLUDEPATH += $$PWD/../deps/msvc $$LIBUSB_ROOT_REL $$LIBUSB_ROOT_REL/libusb $$HIDAPI_ROOT_REL/hidapi
}

# We build libusb and hidapi ourselves instead of using a library
else:android {
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

    # Build hidapi-libusb
    HIDAPI_ROOT_REL = $$PWD/../../hidapi
    SOURCES += $$HIDAPI_ROOT_REL/libusb/hid.c
    INCLUDEPATH += $$HIDAPI_ROOT_REL/hidapi
}

else:unix {
    !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using pkg-config")
    PKGCONFIG += libusb-1.0

    osx {
        !packagesExist(hidapi):error("Could not find hidapi using pkg-config")
        PKGCONFIG += hidapi
    }

    else {
        !packagesExist(hidapi-libusb):error("Could not find hidapi-libusb using pkg-config")
        PKGCONFIG += hidapi-libusb
    }
    CONFIG += link_pkgconfig
}

else {
    error("Platform unsupported, aborting.")
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
