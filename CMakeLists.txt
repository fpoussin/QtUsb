# Generated from QtUsb.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtUsb
    VERSION         6.4.0 # FIXME: this needs to match host's Qt version
    DESCRIPTION     "A cross-platform USB Module for Qt."
    HOMEPAGE_URL    "https://github.com/fpoussin/QtUsb"
    LANGUAGES       CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED
    COMPONENTS
        BuildInternals
        Core
        Test
)
find_package(Qt6 ${PROJECT_VERSION} CONFIG
    OPTIONAL_COMPONENTS
)

qt_build_repo()
