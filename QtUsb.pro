#-------------------------------------------------
#
# Project created by QtCreator 2014-04-14T14:17:36
#
#-------------------------------------------------

VERSION = 0.1

QT      -= gui

TARGET   = QtUsb
TEMPLATE = lib

CONFIG  += static_and_shared

include(QtUsb.pri)

headers_install.files = $$HEADERS

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
        headers_install.path = /opt/usr/include/qt4/QtUsb
    } else {
        target.path = /usr/lib
        headers_install.path = /usr/include/qt4/QtUsb
    }
    INSTALLS += target headers_install
}
