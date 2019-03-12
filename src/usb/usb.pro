TARGET = QtUsb
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtusb.qdocconf

include($$PWD/usb-lib.pri)

load(qt_module)

PRECOMPILED_HEADER =
