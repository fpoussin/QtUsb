TARGET = QtUsb
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtusb.qdocconf

DEFINES += QT_BUILD_USB_LIB

include($$PWD/usb-lib.pri)

CONFIG(staticlib) {
    message("Build as Qt module (static library)")
} else {
    message("Build as Qt module (dynamic library)")
}

load(qt_build_config)
load(qt_module)
