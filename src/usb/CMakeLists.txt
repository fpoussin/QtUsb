# Generated from usb.pro.

#####################################################################
## Usb Module:
#####################################################################

# User-adjustable options:
option(QTUSB_MODULE "Build as Qt module rather than a conventional library" ON)

# These variables are determined automatically from the options above:
set(QTUSB_AS_DYNAMIC_MODULE OFF)
set(QTUSB_AS_STATIC_MODULE  OFF)
set(QTUSB_AS_DYNAMIC_LIB    OFF)
set(QTUSB_AS_STATIC_LIB     OFF)

# Target name in case library option is selected
set(QTUSB_LIB_NAME ExtUsb)

# Auto-generate DLL export header
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

if(BUILD_SHARED_LIBS)
  set(QUSBGLOBAL_CONT ${QUSBGLOBAL_M_CONT})
else()
  set(QUSBGLOBAL_CONT ${QUSBGLOBAL_S_CONT})
endif()
configure_file(qusbglobal.h.in ${CMAKE_CURRENT_SOURCE_DIR}/qusbglobal.h)

# These variables hold all files:
set(QTUSB_SOURCES         qhiddevice.cpp qusb.cpp qusbdevice.cpp qusbendpoint.cpp)
set(QTUSB_PUBLIC_HEADERS  qhiddevice.h qusb.h qusbdevice.h qusbendpoint.h qusbglobal.h)
set(QTUSB_PRIVATE_HEADERS qhiddevice_p.h qusb_p.h qusbdevice_p.h qusbendpoint_p.h)
set(QTUSB_PROXY_HEADERS   extusb/QHidDevice extusb/QUsb extusb/QUsbDevice extusb/QUsbEndpoint extusb/QUsbGlobal)

# Define the actual targets for building
if(NOT QTUSB_MODULE)
  if(NOT BUILD_SHARED_LIBS)
    message(STATUS "Build as static library")
    set(QTUSB_AS_STATIC_LIB ON)
    add_library(${QTUSB_LIB_NAME} STATIC ${QTUSB_SOURCES} ${QTUSB_PROXY_HEADERS} ${QTUSB_PUBLIC_HEADERS} ${QTUSB_PRIVATE_HEADERS})
    target_compile_definitions(${QTUSB_LIB_NAME} PRIVATE QTUSB_STATIC)
  else()
    message(STATUS "Build as dynamic library")
    set(QTUSB_AS_DYNAMIC_LIB ON)
    add_library(${QTUSB_LIB_NAME} SHARED ${QTUSB_SOURCES} ${QTUSB_PROXY_HEADERS} ${QTUSB_PUBLIC_HEADERS} ${QTUSB_PRIVATE_HEADERS})
  endif()

  target_compile_definitions(${QTUSB_LIB_NAME} PRIVATE QT_BUILD_USB_LIB)
  target_include_directories(${QTUSB_LIB_NAME} PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR})
  target_include_directories(${QTUSB_LIB_NAME} PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/extusb)
  target_link_libraries(${QTUSB_LIB_NAME}      PRIVATE Qt::CorePrivate)
  target_link_libraries(${QTUSB_LIB_NAME}      PUBLIC  Qt::Core)

  qt_wrap_cpp(QTUSB_GENERATED_FILES ${QTUSB_PUBLIC_HEADERS} TARGET ${QTUSB_LIB_NAME})
  target_sources(${QTUSB_LIB_NAME} PRIVATE ${QTUSB_GENERATED_FILES})

  install(TARGETS ${QTUSB_LIB_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES ${QTUSB_PUBLIC_HEADERS} DESTINATION include/${QTUSB_LIB_NAME})
  install(FILES ${QTUSB_PROXY_HEADERS} DESTINATION include/${QTUSB_LIB_NAME})
else()
  if(NOT BUILD_SHARED_LIBS)
    message(STATUS "Build as static Qt module")
    set(QTUSB_AS_STATIC_MODULE ON)
    qt_internal_add_module(Usb
        EXCEPTIONS
        STATIC
        SOURCES                  ${QTUSB_SOURCES} ${QTUSB_PUBLIC_HEADERS} ${QTUSB_PRIVATE_HEADERS}
        DEFINES                  QT_BUILD_USB_LIB QTUSB_STATIC
        INCLUDE_DIRECTORIES      ${CMAKE_CURRENT_SOURCE_DIR}
        LIBRARIES                Qt::CorePrivate
        PUBLIC_LIBRARIES         Qt::Core
        PRIVATE_MODULE_INTERFACE Qt::CorePrivate
    )
  else()
    message(STATUS "Build as dynamic Qt module")
    set(QTUSB_AS_DYNAMIC_MODULE ON)
    qt_internal_add_module(Usb
        EXCEPTIONS
        SOURCES                  ${QTUSB_SOURCES} ${QTUSB_PUBLIC_HEADERS} ${QTUSB_PRIVATE_HEADERS}
        DEFINES                  QT_BUILD_USB_LIB
        INCLUDE_DIRECTORIES      ${CMAKE_CURRENT_SOURCE_DIR}
        LIBRARIES                Qt::CorePrivate
        PUBLIC_LIBRARIES         Qt::Core
        PRIVATE_MODULE_INTERFACE Qt::CorePrivate
    )
  endif()
endif()


## Scopes:
#####################################################################

if(UNIX AND NOT MACOS)
  find_package(Libusb1 REQUIRED)
  find_package(HIDAPI REQUIRED COMPONENTS libusb)
endif()

set(QTUSB_EXTRA_WIN32_SOURCES
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
)

set(QTUSB_WIN32_DEPS_DIR ${CMAKE_CURRENT_BINARY_DIR}/libinclude)
set(QTUSB_EXTRA_WIN32_INCLUDE_DIRECTORIES
  ${QTUSB_WIN32_DEPS_DIR}/hidapi
  ${QTUSB_WIN32_DEPS_DIR}/hidapi/hidapi
  ${QTUSB_WIN32_DEPS_DIR}/libusb
  ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb
  ../deps/msvc
)

if(WIN32)
  # On Windows, the compiler attempts to check every single file present in the include directory (not just headers).
  # This causes unnecessary build errors. For that reason, we copy all headers into an auxiliary directory, which
  # we fully control, and can guarantee will be devoid of any non-header files. We then use this directory in the include path.
  
  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR})

  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR}/hidapi)
  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR}/hidapi/hidapi)
  configure_file(../../hidapi/hidapi/hidapi.h               ${QTUSB_WIN32_DEPS_DIR}/hidapi/hidapi/hidapi.h                COPYONLY)

  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR}/libusb)
  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb)
  file(MAKE_DIRECTORY ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os)
  configure_file(../../libusb/libusb/hotplug.h              ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/hotplug.h               COPYONLY)
  configure_file(../../libusb/libusb/libusb.h               ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/libusb.h                COPYONLY)
  configure_file(../../libusb/libusb/libusbi.h              ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/libusbi.h               COPYONLY)
  configure_file(../../libusb/libusb/version_nano.h         ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/version_nano.h          COPYONLY)
  configure_file(../../libusb/libusb/version.h              ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/version.h               COPYONLY)
  configure_file(../../libusb/libusb/os/events_windows.h    ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os/events_windows.h     COPYONLY)
  configure_file(../../libusb/libusb/os/threads_windows.h   ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os/threads_windows.h    COPYONLY)
  configure_file(../../libusb/libusb/os/windows_common.h    ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os/windows_common.h     COPYONLY)
  configure_file(../../libusb/libusb/os/windows_usbdk.h     ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os/windows_usbdk.h      COPYONLY)
  configure_file(../../libusb/libusb/os/windows_winusb.h    ${QTUSB_WIN32_DEPS_DIR}/libusb/libusb/os/windows_winusb.h     COPYONLY)
endif()

set(QTUSB_EXTRA_WIN32_LIBRARIES
  Advapi32.lib
  Setupapi.lib
)

set(QTUSB_EXTRA_MACOS_SOURCES
  ../../hidapi/mac/hid.c
  ../../libusb/libusb/core.c
  ../../libusb/libusb/descriptor.c
  ../../libusb/libusb/hotplug.c
  ../../libusb/libusb/io.c
  ../../libusb/libusb/os/events_posix.c
  ../../libusb/libusb/os/threads_posix.c
  ../../libusb/libusb/os/darwin_usb.c
  ../../libusb/libusb/strerror.c
  ../../libusb/libusb/sync.c
)

set(QTUSB_EXTRA_MACOS_INCLUDE_DIRECTORIES
  ../../hidapi/hidapi
  ../../libusb
  ../../libusb/libusb
  ../../libusb/Xcode
)

set(QTUSB_EXTRA_MACOS_LIBRARIES
  "-framework CoreFoundation"
  "-framework IOKit"
  "-framework AppKit"
)

set(QTUSB_EXTRA_ANDROID_SOURCES
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
)

set(QTUSB_EXTRA_ANDROID_INCLUDE_DIRECTORIES
  ../../hidapi/hidapi
  ../../libusb/android
  ../../libusb/libusb
  ../../libusb/libusb/os
  libusb-1.0
)

set(QTUSB_EXTRA_ANDROID_LIBRARIES)

if(QTUSB_AS_STATIC_MODULE OR QTUSB_AS_DYNAMIC_MODULE)
  qt_internal_extend_target(Usb
      CONDITION           WIN32
      SOURCES             ${QTUSB_EXTRA_WIN32_SOURCES}
      INCLUDE_DIRECTORIES ${QTUSB_EXTRA_WIN32_INCLUDE_DIRECTORIES}
      LIBRARIES           ${QTUSB_EXTRA_WIN32_LIBRARIES})

  qt_internal_extend_target(Usb
      CONDITION           MACOS
      SOURCES             ${QTUSB_EXTRA_MACOS_SOURCES}
      INCLUDE_DIRECTORIES ${QTUSB_EXTRA_MACOS_INCLUDE_DIRECTORIES}
      LIBRARIES           ${QTUSB_EXTRA_MACOS_LIBRARIES})

  qt_internal_extend_target(Usb
      CONDITION           ANDROID
      SOURCES             ${QTUSB_EXTRA_ANDROID_SOURCES}
      INCLUDE_DIRECTORIES ${QTUSB_EXTRA_ANDROID_INCLUDE_DIRECTORIES}
      LIBRARIES           ${QTUSB_EXTRA_ANDROID_LIBRARIES})

  # FIXME: files.pri does the following for ANDROID scope:
  #   # We have to copy the header for includes to work in our library
  #   system("mkdir -p $$PWD/libusb-1.0 && cp $$LIBUSB_ROOT_REL/libusb/libusb.h $$PWD/libusb-1.0/")

  qt_internal_extend_target(Usb CONDITION UNIX AND NOT MACOS LIBRARIES LibUSB1::LibUSB1 HIDAPI::hidapi-libusb)

  qt_internal_add_docs(Usb doc/qtusb.qdocconf)
else()
  # We need to translate the UNIX macro, which is defined/undefined into a 0/1 variable for the generator expressions below.
  # Stupid but works.
  set(IS_UNIX 0)
  if(UNIX)
    set(IS_UNIX 1)
  endif()

  target_link_libraries(
    ${QTUSB_LIB_NAME} PRIVATE
    $<$<AND:${IS_UNIX},$<NOT:${MACOS}>>:LibUSB1::LibUSB1>
    $<$<AND:${IS_UNIX},$<NOT:${MACOS}>>:HIDAPI::hidapi-libusb>
  )

  # What follows is a poor man's implementation of qt_internal_extend_target():

  if(WIN32)
    target_link_libraries(${QTUSB_LIB_NAME}      PRIVATE ${QTUSB_EXTRA_WIN32_LIBRARIES})
    target_sources(${QTUSB_LIB_NAME}             PRIVATE ${QTUSB_EXTRA_WIN32_SOURCES})
    target_include_directories(${QTUSB_LIB_NAME} PRIVATE ${QTUSB_EXTRA_WIN32_INCLUDE_DIRECTORIES})
  endif()

  if(MACOS)
    target_link_libraries(${QTUSB_LIB_NAME}      PRIVATE ${QTUSB_EXTRA_MACOS_LIBRARIES})
    target_sources(${QTUSB_LIB_NAME}             PRIVATE ${QTUSB_EXTRA_MACOS_SOURCES})
    target_include_directories(${QTUSB_LIB_NAME} PRIVATE ${QTUSB_EXTRA_MACOS_INCLUDE_DIRECTORIES})
  endif()

  if(ANDROID)
    target_link_libraries(${QTUSB_LIB_NAME}      PRIVATE ${QTUSB_EXTRA_ANDROID_LIBRARIES})
    target_sources(${QTUSB_LIB_NAME}             PRIVATE ${QTUSB_EXTRA_ANDROID_SOURCES})
    target_include_directories(${QTUSB_LIB_NAME} PRIVATE ${QTUSB_EXTRA_ANDROID_INCLUDE_DIRECTORIES})

    # FIXME: ANDROID scope incomplete (missing copy command)
  endif()
endif()
