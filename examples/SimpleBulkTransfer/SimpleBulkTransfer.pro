#-------------------------------------------------
#
# Project created by QtCreator 2015-07-28T13:55:22
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = SimpleBulkTransfer
CONFIG   += console
CONFIG   -= app_bundle
win32:CONFIG += winusb

TEMPLATE = app

include(../../QtUsb.pri)


SOURCES += main.cpp \
    usbexample.cpp

HEADERS += \
    usbexample.h
