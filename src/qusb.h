#ifndef QUSB_H
#define QUSB_H

#include "qusb_global.h"

#ifdef WIN32
    #include <qwinusb.h>
#else
    #include <qlibusb.h>
#endif

#include <qusbmanager.h>

#endif // QUSB_H
