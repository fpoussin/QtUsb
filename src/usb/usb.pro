TARGET = QtUsb
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtusb.qdocconf

DEFINES += QT_BUILD_USB_LIB

include($$PWD/usb-lib.pri)

CONFIG(static_lib) {
	message("Build as static library")
	TEMPLATE = lib
	CONFIG += staticlib
} else {
	message("Build as Qt module (dynamic library)")
	load(qt_build_config)
	load(qt_module)
}
