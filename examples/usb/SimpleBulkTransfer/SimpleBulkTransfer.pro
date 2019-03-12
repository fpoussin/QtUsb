#-------------------------------------------------
#
# Project created by QtCreator 2015-07-28T13:55:22
#
#-------------------------------------------------

QT += core usb
QT -= gui

TARGET = SimpleBulkTransfer
TEMPLATE = app

SOURCES += main.cpp \
    usbexample.cpp

HEADERS += \
    usbexample.h

target.path = $$[QT_INSTALL_EXAMPLES]/usb/SimpleBulkTransfer
INSTALLS += target
