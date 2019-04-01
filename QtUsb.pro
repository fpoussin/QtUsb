lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 2) {
    message("Cannot build current QtUsb sources with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.2.0")
}

load(configure)
load(qt_parts)

CONFIG += testcase
