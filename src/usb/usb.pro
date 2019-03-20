TARGET = QtUsb
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtusb.qdocconf
DEFINES += QT_BUILD_USB_LIB

include($$PWD/usb-lib.pri)

load(qt_module)

PRECOMPILED_HEADER =
