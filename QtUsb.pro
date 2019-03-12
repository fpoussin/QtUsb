lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtUsb sources with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.0.0")
}

load(configure)
load(qt_parts)
