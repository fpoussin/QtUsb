TARGET = QtUsb
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtusb.qdocconf

DEFINES += QT_BUILD_USB_LIB

CONFIG(qtusb-static) {
    message("Build as static Qt module for $$QMAKE_PLATFORM")
    CONFIG += staticlib qtusb-as-static-module
    DEFINES += QTUSB_STATIC
} else {
    message("Build as Qt module for $$QMAKE_PLATFORM")
    CONFIG += qtusb-as-module
}

include($$PWD/files.pri)
load(qt_module)
