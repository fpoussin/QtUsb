#ifndef QUSB_H
#define QUSB_H

#ifdef WIN32
    #include <qwinusb.h>
#else
    #include <qlibusb.h>
#endif

#endif // QUSB_H
