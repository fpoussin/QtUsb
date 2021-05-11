# Generated from usb.pro.

#####################################################################
## Usb Module:
#####################################################################

option(QTUSB_STATIC "Build as static library" OFF)
set(QTUSB_AS_MODULE OFF)
set(QTUSB_AS_STATIC_MODULE OFF)

set(QTUSB_SOURCES qhiddevice.cpp qhiddevice.h qhiddevice_p.h
qusb.cpp qusb.h qusb_p.h
qusbdevice.cpp qusbdevice.h qusbdevice_p.h
qusbendpoint.cpp qusbendpoint.h qusbendpoint_p.h)

if(QTUSB_STATIC)
  message(STATUS "Build as static Qt module")
  set(QTUSB_AS_STATIC_MODULE ON)
  qt_internal_add_module(Usb
      STATIC
      SOURCES
          ${QTUSB_SOURCES}
      DEFINES
          QT_BUILD_USB_LIB QTUSB_STATIC
      INCLUDE_DIRECTORIES
          ${CMAKE_CURRENT_SOURCE_DIR}
      LIBRARIES
          Qt::CorePrivate
      PUBLIC_LIBRARIES
          Qt::Core
      PRIVATE_MODULE_INTERFACE
          Qt::CorePrivate
  )
else()
  message(STATUS "Build as Qt module")
  set(QTUSB_AS_MODULE ON)
  qt_internal_add_module(Usb
      SOURCES
          ${QTUSB_SOURCES}
      DEFINES
          QT_BUILD_USB_LIB
      INCLUDE_DIRECTORIES
          ${CMAKE_CURRENT_SOURCE_DIR}
      LIBRARIES
          Qt::CorePrivate
      PUBLIC_LIBRARIES
          Qt::Core
      PRIVATE_MODULE_INTERFACE
          Qt::CorePrivate
  )
endif()

set(QUSBGLOBAL_S_CONT "\
#include <QtCore>\n\
#define Q_USB_EXPORT")

set(QUSBGLOBAL_M_CONT "\
#include <QtCore>\n\
\n\
#if !defined(QT_STATIC)\n\
  #if defined(QT_BUILD_USB_LIB)\n\
    #define Q_USB_EXPORT Q_DECL_EXPORT\n\
  #else\n\
    #define Q_USB_EXPORT Q_DECL_IMPORT\n\
  #endif\n\
#else\n\
  #define Q_USB_EXPORT\n\
#endif")

if(QTUSB_AS_MODULE)
  set(QUSBGLOBAL_CONT ${QUSBGLOBAL_M_CONT})
else()
  set(QUSBGLOBAL_CONT ${QUSBGLOBAL_S_CONT})
endif()
configure_file(qusbglobal.h.in ${CMAKE_CURRENT_SOURCE_DIR}/qusbglobal.h)
qt_internal_extend_target(Usb
    SOURCES
        qusbglobal.h)

## Scopes:
#####################################################################

qt_internal_extend_target(Usb CONDITION WIN32
    SOURCES
        ../../hidapi/windows/hid.c
        ../../libusb/libusb/core.c
        ../../libusb/libusb/descriptor.c
        ../../libusb/libusb/hotplug.c
        ../../libusb/libusb/io.c
        ../../libusb/libusb/os/events_windows.c
        ../../libusb/libusb/os/threads_windows.c
        ../../libusb/libusb/os/windows_common.c
        ../../libusb/libusb/os/windows_usbdk.c
        ../../libusb/libusb/os/windows_winusb.c
        ../../libusb/libusb/strerror.c
        ../../libusb/libusb/sync.c
    INCLUDE_DIRECTORIES
        ../../hidapi
        ../../hidapi/hidapi
        ../../libusb
        ../../libusb/libusb
        ../deps/msvc
    LIBRARIES
        Advapi32.lib
        Setupapi.lib
)

qt_internal_extend_target(Usb CONDITION ANDROID
    SOURCES
        ../../hidapi/libusb/hid.c
        ../../libusb/libusb/core.c
        ../../libusb/libusb/descriptor.c
        ../../libusb/libusb/hotplug.c
        ../../libusb/libusb/io.c
        ../../libusb/libusb/os/linux_netlink.c
        ../../libusb/libusb/os/linux_usbfs.c
        ../../libusb/libusb/os/poll_posix.c
        ../../libusb/libusb/os/threads_posix.c
        ../../libusb/libusb/strerror.c
        ../../libusb/libusb/sync.c
    INCLUDE_DIRECTORIES
        ../../hidapi/hidapi
        ../../libusb/android
        ../../libusb/libusb
        ../../libusb/libusb/os
        libusb-1.0
)

# FIXME: files.pri does the following for ANDROID scope:
#   # We have to copy the header for includes to work in our library
#   system("mkdir -p $$PWD/libusb-1.0 && cp $$LIBUSB_ROOT_REL/libusb/libusb.h $$PWD/libusb-1.0/")

if(UNIX)
    find_package(LibUSB1 REQUIRED)
    if(MACOS)
        find_package(HIDAPI REQUIRED)
    else()
        # NOTE: libusb already found earlier
        find_package(HIDAPI REQUIRED COMPONENTS libusb)
    endif()
endif()

qt_internal_extend_target(Usb CONDITION UNIX               LIBRARIES LibUSB1::LibUSB1)
qt_internal_extend_target(Usb CONDITION MACOS              LIBRARIES HIDAPI::hidapi)
qt_internal_extend_target(Usb CONDITION UNIX AND NOT MACOS LIBRARIES HIDAPI::hidapi-libusb)

qt_internal_add_docs(Usb doc/qtusb.qdocconf)
