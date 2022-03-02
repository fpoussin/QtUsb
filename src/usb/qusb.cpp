#include "qusb.h"
#include "qusb_p.h"
#include <QThread>

#include <QDebug>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()                 \
    if (m_log_level >= QUsb::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB()                            \
    if (info->logLevel() >= QUsb::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;

static int LIBUSB_CALL hotplugCallback(libusb_context *ctx,
                                       libusb_device *device,
                                       libusb_hotplug_event event,
                                       void *user_data)
{
    struct libusb_device_descriptor desc;
    (void)ctx;
    QUsb::IdList device_list;
    QUsb::Id id;
    QUsb *info = reinterpret_cast<QUsb *>(user_data);
    DbgPrintCB();

    uint8_t bus = libusb_get_bus_number(device);
    uint8_t port = libusb_get_port_number(device);

    if (info->logLevel() >= QUsb::logDebug)
        qDebug("hotplugCallback");

    libusb_get_device_descriptor(device, &desc);
    id.vid = desc.idVendor;
    id.pid = desc.idProduct;
    id.bus = bus;
    id.port = port;
    id.dClass = desc.bDeviceClass;
    id.dSubClass = desc.bDeviceSubClass;

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {

        // Add to list
        emit info->deviceInserted(id);

    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {

        // Remove from list
        emit info->deviceRemoved(id);

    } else {
        if (info->logLevel() >= QUsb::logWarning)
            qWarning("Unhandled hotplug event %d", event);
        return -1;
    }
    return 0;
}

QUsbPrivate::QUsbPrivate()
    : m_has_hotplug(false), m_ctx(Q_NULLPTR), m_refresh_timer(new QTimer)
{
    QThread *t = new QThread();
    m_refresh_timer->moveToThread(t);

    m_refresh_timer->setSingleShot(false);
    m_refresh_timer->setInterval(250);

    m_refresh_timer->connect(t, SIGNAL(started()), SLOT(start()));
    m_refresh_timer->connect(t, SIGNAL(finished()), SLOT(stop()));
    t->start();
}

QUsbPrivate::~QUsbPrivate()
{
    m_refresh_timer->thread()->exit();
    m_refresh_timer->thread()->wait();
    m_refresh_timer->thread()->deleteLater();

    m_refresh_timer->disconnect();
    m_refresh_timer->deleteLater();
}

/*!
    \class QUsb

    \brief This class handles hotplug and device detection.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb

    Handles USB events and searching.
    Can be used to monitor events for a list of devices or all system devices.

    \sa QUsbDevice
*/

/*!
    \fn void QUsb::deviceInserted(Id id)

    This is signal is emited when one or more new devices are detected, providing a \a list.
*/

/*!
    \fn void QUsb::deviceRemoved(Id id)

    This is signal is emited when one or more new devices are removed, providing a \a list.
*/

/*!
    \property QUsb::logLevel
    \brief the log level for hotplug/detection.
*/

/*!
    \enum QUsb::LogLevel

\value logNone      No debug output
    \value logError     Errors only
    \value logWarning   Warning and abose
    \value logInfo      Info and above
    \value logDebug     Everything
    \value logDebugAll  Everything + libusb debug output
                                             */

/*!
    \property QUsb::logLevel
    \property QUsbDevice::pid
    \property QUsbDevice::vid
    \property QUsbDevice::speed
    \property QUsbDevice::timeout

    \brief Various properties.
                                                     */

/*!
    \typedef QUsb::ConfigList
    \brief List of Config structs.
 */

/*!
    \typedef QUsb::IdList
    \brief List of Id structs.
 */

/*!
    \class QUsb::Config
    \brief Device configuration structure.
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \variable QUsb::Config::config
    \brief The configuration ID.
 */

/*!
    \variable QUsb::Config::interface
    \brief The interface ID.
 */

/*!
    \variable QUsb::Config::alternate
    \brief The alternate ID.
 */

/*!
    \class QUsb::Id
    \brief Device Ids structure.

                If some properties are equal to 0, they won't be taken into account for filtering.
        You only need PID and VID, or class and subclass to identify a device, but can be more specific if multiple devices using the same IDs are connected.
    \ingroup usb-main
    \inmodule QtUsb
                                                            */

/*!
    \variable QUsb::Id::vid
    \brief The vendor ID.
*/

/*!
    \variable QUsb::Id::pid
    \brief The product ID.
*/

/*!
    \variable QUsb::Id::bus
    \brief The USB bus number.

                Default is \a QUsbDevice::busAny, which matches all buses.
                */

/*!
    \variable QUsb::Id::port
    \brief The USB port number.

        Default is \a QUsbDevice::portAny, which matches all ports.
                */

/*!
    \variable QUsb::Id::dClass
    \brief The USB class.
*/

/*!
    \variable QUsb::Id::dSubClass
    \brief The USB Sub-class.
*/

QUsb::QUsb(QObject *parent)
    : QObject(*(new QUsbPrivate), parent), d_dummy(Q_NULLPTR)
{
    Q_D(QUsb);

    m_log_level = QUsb::logInfo;
    DbgPrintFuncName();
    int rc;

    qRegisterMetaType<QUsb::Id>("QUsb::Id");
    qRegisterMetaType<QUsb::Config>("QUsb::Config");

    qRegisterMetaType<QUsb::IdList>("QUsb::IdList");
    qRegisterMetaType<QUsb::ConfigList>("QUsb::ConfigList");

    rc = libusb_init(&d->m_ctx);
    if (rc < 0) {
        libusb_exit(d->m_ctx);
        qCritical("LibUsb Init Error %d", rc);
        return;
    }

    libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);

    // Populate list once
    m_system_list = devices();

    // Try hotplug first
    d->m_has_hotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
    if (d->m_has_hotplug) {

        rc = libusb_hotplug_register_callback(d->m_ctx,
                                              static_cast<libusb_hotplug_event>((LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)),
                                              LIBUSB_HOTPLUG_NO_FLAGS,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              reinterpret_cast<libusb_hotplug_callback_fn>(hotplugCallback),
                                              reinterpret_cast<void *>(this),
                                              &callback_handle);
        if (LIBUSB_SUCCESS != rc) {
            libusb_exit(d->m_ctx);
            qWarning("Error creating hotplug callback");
            return;
        }
    }

    connect(d->m_refresh_timer, SIGNAL(timeout()), this, SLOT(checkDevices()));
}

/*!
    Unregister callbacks and close the usb context.
 */
QUsb::~QUsb()
{
    Q_D(QUsb);
    DbgPrintFuncName();
    libusb_hotplug_deregister_callback(d->m_ctx, callback_handle);
    libusb_exit(d->m_ctx);
}

/*!
    Check devices present in system.

    This gets called by the internal timer.
 */
void QUsb::checkDevices()
{
    DbgPrintFuncName();
    Q_D(QUsb);
    QUsb::IdList list;

    timeval t = { 0, 0 };

    if (d->m_has_hotplug) {
        libusb_handle_events_timeout_completed(d->m_ctx, &t, Q_NULLPTR);
    } else {
        list = devices();
        monitorDevices(list);
    }
}

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libusb.h"

#if defined(_MSC_VER)
#define snprintf _snprintf
#define putenv _putenv
#endif

// Future versions of libusb will use usb_interface instead of interface
// in libusb_config_descriptor => catter for that
#define usb_interface interface

// Global variables
static bool binary_dump = false;
static bool extra_info = false;
static bool force_device_request = false;	// For WCID descriptor queries
static const char* binary_name = NULL;

static inline void msleep(int msecs)
{
#if defined(_WIN32)
    Sleep(msecs);
#else
    const struct timespec ts = { msecs / 1000, (msecs % 1000) * 1000000L };
    nanosleep(&ts, NULL);
#endif
}

static void perr(char const *format, ...)
{
    va_list args;

    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

#define ERR_EXIT(errcode) do { perr("   %s\n", libusb_strerror((enum libusb_error)errcode)); return -1; } while (0)
#define CALL_CHECK(fcall) do { int _r=fcall; if (_r < 0) ERR_EXIT(_r); } while (0)
#define CALL_CHECK_CLOSE(fcall, hdl) do { int _r=fcall; if (_r < 0) { libusb_close(hdl); ERR_EXIT(_r); } } while (0)
#define B(x) (((x)!=0)?1:0)
#define be_to_int32(buf) (((buf)[0]<<24)|((buf)[1]<<16)|((buf)[2]<<8)|(buf)[3])

#define RETRY_MAX                     5
#define REQUEST_SENSE_LENGTH          0x12
#define INQUIRY_LENGTH                0x24
#define READ_CAPACITY_LENGTH          0x08

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT                0x01
#define HID_GET_IDLE                  0x02
#define HID_GET_PROTOCOL              0x03
#define HID_SET_REPORT                0x09
#define HID_SET_IDLE                  0x0A
#define HID_SET_PROTOCOL              0x0B
#define HID_REPORT_TYPE_INPUT         0x01
#define HID_REPORT_TYPE_OUTPUT        0x02
#define HID_REPORT_TYPE_FEATURE       0x03

// Mass Storage Requests values. See section 3 of the Bulk-Only Mass Storage Class specifications
#define BOMS_RESET                    0xFF
#define BOMS_GET_MAX_LUN              0xFE

// Microsoft OS Descriptor
#define MS_OS_DESC_STRING_INDEX		0xEE
#define MS_OS_DESC_STRING_LENGTH	0x12
#define MS_OS_DESC_VENDOR_CODE_OFFSET	0x10
static const uint8_t ms_os_desc_string[] = {
    MS_OS_DESC_STRING_LENGTH,
    LIBUSB_DT_STRING,
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
};

// Section 5.1: Command Block Wrapper (CBW)
struct command_block_wrapper {
    uint8_t dCBWSignature[4];
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;
    uint8_t bCBWLUN;
    uint8_t bCBWCBLength;
    uint8_t CBWCB[16];
};

// Section 5.2: Command Status Wrapper (CSW)
struct command_status_wrapper {
    uint8_t dCSWSignature[4];
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t bCSWStatus;
};

static const uint8_t cdb_length[256] = {
//	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};

static enum test_type {
    USE_GENERIC,
    USE_PS3,
    USE_XBOX,
    USE_SCSI,
    USE_HID,
} test_mode;
static uint16_t VID, PID;

static void display_buffer_hex(unsigned char *buffer, unsigned size)
{
    unsigned i, j, k;

    for (i=0; i<size; i+=16) {
        qDebug("\n  %08x  ", i);
        for(j=0,k=0; k<16; j++,k++) {
            if (i+j < size) {
                qDebug("%02x", buffer[i+j]);
            } else {
                qDebug("  ");
            }
            qDebug(" ");
        }
        qDebug(" ");
        for(j=0,k=0; k<16; j++,k++) {
            if (i+j < size) {
                if ((buffer[i+j] < 32) || (buffer[i+j] > 126)) {
                    qDebug(".");
                } else {
                    qDebug("%c", buffer[i+j]);
                }
            }
        }
    }
    qDebug("\n" );
}

static char* uuid_to_string(const uint8_t* uuid)
{
    static char uuid_string[40];
    if (uuid == NULL) return NULL;
    snprintf(uuid_string, sizeof(uuid_string),
        "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
        uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
        uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    return uuid_string;
}

// Read the MS WinUSB Feature Descriptors, that are used on Windows 8 for automated driver installation
static void read_ms_winsub_feature_descriptors(libusb_device_handle *handle, uint8_t bRequest, int iface_number)
{
#define MAX_OS_FD_LENGTH 256
    int i, r;
    uint8_t os_desc[MAX_OS_FD_LENGTH];
    uint32_t length;
    void* le_type_punning_IS_fine;
    struct {
        const char* desc;
        uint8_t recipient;
        uint16_t index;
        uint16_t header_size;
    } os_fd[2] = {
        {"Extended Compat ID", LIBUSB_RECIPIENT_DEVICE, 0x0004, 0x10},
        {"Extended Properties", LIBUSB_RECIPIENT_INTERFACE, 0x0005, 0x0A}
    };

    if (iface_number < 0) return;
    // WinUSB has a limitation that forces wIndex to the interface number when issuing
    // an Interface Request. To work around that, we can force a Device Request for
    // the Extended Properties, assuming the device answers both equally.
    if (force_device_request)
        os_fd[1].recipient = LIBUSB_RECIPIENT_DEVICE;

    for (i=0; i<2; i++) {
        qDebug("\nReading %s OS Feature Descriptor (wIndex = 0x%04d):\n", os_fd[i].desc, os_fd[i].index);

        // Read the header part
        r = libusb_control_transfer(handle, (uint8_t)(LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|os_fd[i].recipient),
            bRequest, (uint16_t)(((iface_number)<< 8)|0x00), os_fd[i].index, os_desc, os_fd[i].header_size, 1000);
        if (r < os_fd[i].header_size) {
            qDebug("   Failed: %s", (r<0)?libusb_strerror((enum libusb_error)r):"header size is too small");
            return;
        }
        le_type_punning_IS_fine = (void*)os_desc;
        length = *((uint32_t*)le_type_punning_IS_fine);
        if (length > MAX_OS_FD_LENGTH) {
            length = MAX_OS_FD_LENGTH;
        }

        // Read the full feature descriptor
        r = libusb_control_transfer(handle, (uint8_t)(LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|os_fd[i].recipient),
            bRequest, (uint16_t)(((iface_number)<< 8)|0x00), os_fd[i].index, os_desc, (uint16_t)length, 1000);
        if (r < 0) {
            qDebug("   Failed: %s", libusb_strerror((enum libusb_error)r));
            return;
        } else {
            display_buffer_hex(os_desc, r);
        }
    }
}

static void print_device_cap(struct libusb_bos_dev_capability_descriptor *dev_cap)
{
    switch(dev_cap->bDevCapabilityType) {
    case LIBUSB_BT_USB_2_0_EXTENSION: {
        struct libusb_usb_2_0_extension_descriptor *usb_2_0_ext = NULL;
        libusb_get_usb_2_0_extension_descriptor(NULL, dev_cap, &usb_2_0_ext);
        if (usb_2_0_ext) {
            qDebug("    USB 2.0 extension:\n");
            qDebug("      attributes             : %02X\n", usb_2_0_ext->bmAttributes);
            libusb_free_usb_2_0_extension_descriptor(usb_2_0_ext);
        }
        break;
    }
    case LIBUSB_BT_SS_USB_DEVICE_CAPABILITY: {
        struct libusb_ss_usb_device_capability_descriptor *ss_usb_device_cap = NULL;
        libusb_get_ss_usb_device_capability_descriptor(NULL, dev_cap, &ss_usb_device_cap);
        if (ss_usb_device_cap) {
            qDebug("    USB 3.0 capabilities:\n");
            qDebug("      attributes             : %02X\n", ss_usb_device_cap->bmAttributes);
            qDebug("      supported speeds       : %04X\n", ss_usb_device_cap->wSpeedSupported);
            qDebug("      supported functionality: %02X\n", ss_usb_device_cap->bFunctionalitySupport);
            libusb_free_ss_usb_device_capability_descriptor(ss_usb_device_cap);
        }
        break;
    }
    case LIBUSB_BT_CONTAINER_ID: {
        struct libusb_container_id_descriptor *container_id = NULL;
        libusb_get_container_id_descriptor(NULL, dev_cap, &container_id);
        if (container_id) {
            qDebug("    Container ID:\n      %s\n", uuid_to_string(container_id->ContainerID));
            libusb_free_container_id_descriptor(container_id);
        }
        break;
    }
    default:
        qDebug("    Unknown BOS device capability %02x:\n", dev_cap->bDevCapabilityType);
    }
}

int QUsb::xusb(const Id &id, const Config &config)
{

    static char debug_env_str[] = "LIBUSB_DEBUG=4";	// LIBUSB_LOG_LEVEL_DEBUG

    const struct libusb_version* version;
    int j, r;
    size_t i, arglen;
    unsigned tmp_vid, tmp_pid;
    uint16_t endian_test = 0xBE00;
    char *error_lang = NULL, *old_dbg_str = NULL, str[256];

    // Default to generic, expecting VID:PID
    VID = id.vid;
    PID = id.pid;

    if (((uint8_t*)&endian_test)[0] == 0xBE) {
        qDebug("Despite their natural superiority for end users, big endian\n"
            "CPUs are not supported with this program, sorry.\n");
        return 0;
    }
    // xusb is commonly used as a debug tool, so it's convenient to have debug output during libusb_init(),
    // but since we can't call on libusb_set_option() before libusb_init(), we use the env variable method

    version = libusb_get_version();
    qDebug("Using libusb v%d.%d.%d.%d\n\n", version->major, version->minor, version->micro, version->nano);
    r = libusb_init(NULL);
    if (r < 0)
        return r;


    libusb_context *ctx;

    libusb_init(&ctx);
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

    libusb_device_handle *handle;
    libusb_device *dev;
    uint8_t bus, port_path[8];
    struct libusb_bos_descriptor *bos_desc;
    struct libusb_config_descriptor *conf_desc;
    const struct libusb_endpoint_descriptor *endpoint;
    int  k;
    int iface, nb_ifaces = -1;
    struct libusb_device_descriptor dev_desc;
    const char* const speed_name[6] = { "Unknown", "1.5 Mbit/s (USB LowSpeed)", "12 Mbit/s (USB FullSpeed)",
        "480 Mbit/s (USB HighSpeed)", "5000 Mbit/s (USB SuperSpeed)", "10000 Mbit/s (USB SuperSpeedPlus)" };
    uint8_t endpoint_in = 0, endpoint_out = 0;	// default IN and OUT endpoints

    qDebug("Opening device %04X:%04X...\n", id.vid, id.pid);
    handle = libusb_open_device_with_vid_pid(ctx, id.vid, id.pid);

    if (handle == NULL) {
        qDebug("  Failed.\n");
        return -1;
    }

    dev = libusb_get_device(handle);
    bus = libusb_get_bus_number(dev);
    if (extra_info) {
        r = libusb_get_port_numbers(dev, port_path, sizeof(port_path));
        if (r > 0) {
            qDebug("\nDevice properties:\n");
            qDebug("        bus number: %d\n", bus);
            qDebug("         port path: %d", port_path[0]);
            for (i=1; i<r; i++) {
                qDebug("->%d", port_path[i]);
            }
            qDebug(" (from root hub)\n");
        }
        r = libusb_get_device_speed(dev);
        if ((r<0) || (r>5)) r=0;
        qDebug("             speed: %s\n", speed_name[r]);
    }

    qDebug("\nReading device descriptor:\n");
    CALL_CHECK_CLOSE(libusb_get_device_descriptor(dev, &dev_desc), handle);
    qDebug("            length: %d\n", dev_desc.bLength);
    qDebug("      device class: %d\n", dev_desc.bDeviceClass);
    qDebug("               S/N: %d\n", dev_desc.iSerialNumber);
    qDebug("           VID:PID: %04X:%04X\n", dev_desc.idVendor, dev_desc.idProduct);
    qDebug("         bcdDevice: %04X\n", dev_desc.bcdDevice);
    qDebug("   iMan:iProd:iSer: %d:%d:%d\n", dev_desc.iManufacturer, dev_desc.iProduct, dev_desc.iSerialNumber);
    qDebug("          nb confs: %d\n", dev_desc.bNumConfigurations);
    // Copy the string descriptors for easier parsing


    qDebug("\nReading BOS descriptor: ");
    if (libusb_get_bos_descriptor(handle, &bos_desc) == LIBUSB_SUCCESS) {
        qDebug("%d caps\n", bos_desc->bNumDeviceCaps);
        for (i = 0; i < bos_desc->bNumDeviceCaps; i++)
            print_device_cap(bos_desc->dev_capability[i]);
        libusb_free_bos_descriptor(bos_desc);
    } else {
        qDebug("no descriptor\n");
    }

    qDebug("\nReading first configuration descriptor:\n");
    CALL_CHECK_CLOSE(libusb_get_config_descriptor(dev, 0, &conf_desc), handle);
    nb_ifaces = conf_desc->bNumInterfaces;
    qDebug("             nb interfaces: %d\n", nb_ifaces);
    for (i=0; i<nb_ifaces; i++) {
        qDebug("              interface[%d]: id = %d\n", i,
            conf_desc->usb_interface[i].altsetting[0].bInterfaceNumber);
        for (j=0; j<conf_desc->usb_interface[i].num_altsetting; j++) {
            qDebug("interface[%d].altsetting[%d]: num endpoints = %d\n",
                i, j, conf_desc->usb_interface[i].altsetting[j].bNumEndpoints);
            qDebug("   Class.SubClass.Protocol: %02X.%02X.%02X\n",
                conf_desc->usb_interface[i].altsetting[j].bInterfaceClass,
                conf_desc->usb_interface[i].altsetting[j].bInterfaceSubClass,
                conf_desc->usb_interface[i].altsetting[j].bInterfaceProtocol);
            if ( (conf_desc->usb_interface[i].altsetting[j].bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE)
              && ( (conf_desc->usb_interface[i].altsetting[j].bInterfaceSubClass == 0x01)
              || (conf_desc->usb_interface[i].altsetting[j].bInterfaceSubClass == 0x06) )
              && (conf_desc->usb_interface[i].altsetting[j].bInterfaceProtocol == 0x50) ) {
                // Mass storage devices that can use basic SCSI commands
                test_mode = USE_SCSI;
            }
            for (k=0; k<conf_desc->usb_interface[i].altsetting[j].bNumEndpoints; k++) {
                struct libusb_ss_endpoint_companion_descriptor *ep_comp = NULL;
                endpoint = &conf_desc->usb_interface[i].altsetting[j].endpoint[k];
                qDebug("       endpoint[%d].address: %02X\n", k, endpoint->bEndpointAddress);
                // Use the first interrupt or bulk IN/OUT endpoints as default for testing
                if ((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) & (LIBUSB_TRANSFER_TYPE_BULK | LIBUSB_TRANSFER_TYPE_INTERRUPT)) {
                    if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                        if (!endpoint_in)
                            endpoint_in = endpoint->bEndpointAddress;
                    } else {
                        if (!endpoint_out)
                            endpoint_out = endpoint->bEndpointAddress;
                    }
                }
                qDebug("           max packet size: %04X\n", endpoint->wMaxPacketSize);
                qDebug("          polling interval: %02X\n", endpoint->bInterval);
                libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint, &ep_comp);
                if (ep_comp) {
                    qDebug("                 max burst: %02X   (USB 3.0)\n", ep_comp->bMaxBurst);
                    qDebug("        bytes per interval: %04X (USB 3.0)\n", ep_comp->wBytesPerInterval);
                    libusb_free_ss_endpoint_companion_descriptor(ep_comp);
                }
            }
        }
    }
    libusb_free_config_descriptor(conf_desc);
    if(libusb_set_auto_detach_kernel_driver(handle, 1)) {
        for (iface = 0; iface < nb_ifaces; iface++)
        {
            if(libusb_kernel_driver_active(handle, iface)){
                qDebug("\nKernel driver attached for interface %d\n", iface);
                libusb_detach_kernel_driver(handle, iface); // detach it
            }
            qDebug("\nClaiming interface %d...\n", iface);
            r = libusb_claim_interface(handle, iface);
            if (r != LIBUSB_SUCCESS) {
                qDebug("   Failed.\n");
            }
        }
    }

#ifdef QUSB_READ_DESCRIPTORS
    char string[128];
    qDebug("\nReading string descriptors:\n");
    if (libusb_get_string_descriptor_ascii(handle, dev_desc.iManufacturer, (unsigned char*)string, sizeof(string)) > 0) {
        qDebug("   String (0x%02X): \"%s\"\n", dev_desc.iManufacturer, string);
    }
    if (libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, (unsigned char*)string, sizeof(string)) > 0) {
        qDebug("   String (0x%02X): \"%s\"\n", dev_desc.iProduct, string);
    }
    if (libusb_get_string_descriptor_ascii(handle, dev_desc.iSerialNumber, (unsigned char*)string, sizeof(string)) > 0) {
        qDebug("   String (0x%02X): \"%s\"\n", dev_desc.iSerialNumber, string);
    }
#endif

#ifdef Q_OS_WIN
    // Read the OS String Descriptor
    r = libusb_get_string_descriptor(handle, MS_OS_DESC_STRING_INDEX, 0, (unsigned char*)string, MS_OS_DESC_STRING_LENGTH);
    int first_iface = -1;
    if (nb_ifaces > 0)
        first_iface = conf_desc->usb_interface[0].altsetting[0].bInterfaceNumber;
    if (r == MS_OS_DESC_STRING_LENGTH && memcmp(ms_os_desc_string, string, sizeof(ms_os_desc_string)) == 0) {
        // If this is a Microsoft OS String Descriptor,
        // attempt to read the WinUSB extended Feature Descriptors
        read_ms_winsub_feature_descriptors(handle, string[MS_OS_DESC_VENDOR_CODE_OFFSET], first_iface);
    }

    //Read IAD's
    qDebug("\nReading interface association descriptors (IADs) for first configuration:\n");
    struct libusb_interface_association_descriptor_array *iad_array;
    r = libusb_get_interface_association_descriptors(dev, 0, &iad_array);
    if (r == LIBUSB_SUCCESS) {
        qDebug("    nb IADs: %d\n", iad_array->length);
        for (i=0; i<iad_array->length;i++) {
            const struct libusb_interface_association_descriptor *iad = &iad_array->iad[i];
            qDebug("      IAD %d:\n", i);
            qDebug("            bFirstInterface: %u\n", iad->bFirstInterface);
            qDebug("            bInterfaceCount: %u\n", iad->bInterfaceCount);
            qDebug("             bFunctionClass: %02X\n", iad->bFunctionClass);
            qDebug("          bFunctionSubClass: %02X\n", iad->bFunctionSubClass);
            qDebug("          bFunctionProtocol: %02X\n", iad->bFunctionProtocol);
            if (iad->iFunction) {
                if (libusb_get_string_descriptor_ascii(handle, iad->iFunction, (unsigned char*)string, sizeof(string)) > 0)
                    qDebug("                  iFunction: %u (%s)\n", iad->iFunction, string);
                else
                    qDebug("                  iFunction: %u (libusb_get_string_descriptor_ascii failed!)\n", iad->iFunction);
            }
            else
                qDebug("                  iFunction: 0\n");
        }
        libusb_free_interface_association_descriptors(iad_array);
    }
#endif
    qDebug("\n");
    for (iface = 0; iface<nb_ifaces; iface++) {
        qDebug("Releasing interface %d...\n", iface);
        libusb_release_interface(handle, iface);
    }

    qDebug("Closing device...\n");
    libusb_close(handle);
    return 0;
}

/*!
    \brief Returns all present \c devices.
 */
QUsb::IdList QUsb::devices()
{
    QUsb::IdList list;
    ssize_t cnt; // holding number of devices in list
    libusb_device **devs;
    libusb_context *ctx;
    struct hid_device_info *hid_devs, *cur_hid_dev;

    libusb_init(&ctx);
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
    cnt = libusb_get_device_list(ctx, &devs); // get the list of devices
    if (cnt < 0) {
        qCritical("libusb_get_device_list Error");
        libusb_free_device_list(devs, 1);
        return list;
    }

    for (int i = 0; i < cnt; i++) {
        libusb_device *dev = devs[i];
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            QUsb::Id id;
            id.pid = desc.idProduct;
            id.vid = desc.idVendor;
            id.bus = libusb_get_bus_number(dev);
            id.port = libusb_get_port_number(dev);
            id.configCount = desc.bNumConfigurations;
            if(id.configCount > 0)
            {
                for(quint8 count(0); count < id.configCount; ++count)
                {
                    libusb_config_descriptor* deviceConfiguration;
                    Config cg;
                    if(libusb_get_config_descriptor(dev, count, &deviceConfiguration) == 0)
                    {
                        if(deviceConfiguration)
                        {
                            // next get the interfaces
                            auto interfaceCount = deviceConfiguration->bNumInterfaces;
                            auto interfaces = deviceConfiguration->interface;
//                            qDebug() << "Found " << interfaceCount << " interfaces";
                            cg.config = count+1;
                            for(qint8 i(0); i < interfaceCount; ++i )
                            {
                                auto interface = interfaces[i];
                                for(quint8 j(0); j < interface.num_altsetting; ++j){
                                    auto interface_descriptor = interface.altsetting[j];
                                    auto endpointCount = interface_descriptor.bNumEndpoints;
                                    for(quint8 m(0); m < endpointCount; ++m)
                                    {
                                        cg.interface = i;
                                        cg.alternate = j;
                                        id.configurations.append(cg);
                                        auto endpoint_descriptor = interface_descriptor.endpoint[m];
                                        EndpointDescription desc(endpoint_descriptor.bEndpointAddress, endpoint_descriptor.bmAttributes, cg.config, cg.interface, cg.alternate);
                                        id.endpoints.append(desc);
                                    }
                                }
                            }
                            libusb_free_config_descriptor(deviceConfiguration);
                        }
                    } else {
                        qDebug() << "Failed to get config #" << count;
                    }
                }
            }            
            list.append(id);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    hid_devs = hid_enumerate(0x0, 0x0);
    cur_hid_dev = hid_devs;
    while (cur_hid_dev) {

        QUsb::Id id;
        id.pid = cur_hid_dev->product_id;
        id.vid = cur_hid_dev->vendor_id;
        id.bus = 0;
        id.port = 0;

        list.append(id);

        cur_hid_dev = cur_hid_dev->next;
    }
    hid_free_enumeration(hid_devs);

    return list;
}


/*!
      Check if \a id  device is present.

      Return bool true if present.
 */
bool QUsb::isPresent(const QUsb::Id &id) const
{
    DbgPrintFuncName();
    return this->findDevice(id, m_system_list) >= 0;
}

/*!
      Add an \a id device to the list.

      Returns false if device was already in the list, else true.
 */
bool QUsb::addDevice(const QUsb::Id &id)
{

    DbgPrintFuncName();
    if (this->findDevice(id, m_list) == -1) {
        m_list.append(id);
        return true;
    }
    return false;
}

/*!
      \brief Remove \a id device from the list.

      Return bool false if device was not in the list, else true.
 */
bool QUsb::removeDevice(const QUsb::Id &id)
{

    DbgPrintFuncName();
    const int pos = this->findDevice(id, m_list);
    if (pos > 0) {
        m_list.removeAt(pos);
        return true;
    }
    return true;
}

/*!
      Search an \a id device in a device \a list.

      Return index of the filter, returns -1 if not found.
 */
int QUsb::findDevice(const QUsb::Id &id,
                         const QUsb::IdList &list) const
{
    DbgPrintFuncName();
    for (int i = 0; i < list.length(); i++) {
        const QUsb::Id *d = &list.at(i);

        if (d->pid == id.pid && d->vid == id.vid) {
            if (id.bus == QUsb::busAny && id.port == QUsb::portAny) // Ignore bus/port if both == any
                return i;
            if (d->bus == id.bus && d->port == id.port) // Take bus/port into account for filtering when set
                return i;
        }
    }
    return -1;
}

/*!
    Set log \a level (only hotplug/detection).
 */
void QUsb::setLogLevel(QUsb::LogLevel level)
{
    DbgPrintFuncName();
    Q_D(QUsb);
    m_log_level = level;
    if (m_log_level >= QUsb::logDebug)
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    else if (m_log_level >= QUsb::logWarning)
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
    else
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
}

/*!
    Get current log level.
 */
QUsb::LogLevel QUsb::logLevel() const
{
    return m_log_level;
}

/*!
    Add a \a list to monitor.
 */
void QUsb::monitorDevices(const QUsb::IdList &list)
{

    DbgPrintFuncName();
    QUsb::IdList inserted, removed;
    QUsb::Id filter;

    for (int i = 0; i < list.length(); i++) {
        filter = list.at(i);
        if (this->findDevice(filter, m_system_list) < 0) {
            // It's not in the old system list
            inserted.append(filter);
        }
    }

    for (int i = 0; i < m_system_list.length(); i++) {
        filter = m_system_list.at(i);
        if (this->findDevice(filter, list) < 0) {
            // It's in the old system list but not in the current one
            removed.append(filter);
        }
    }

    for (int i = 0; i < inserted.length(); i++) {
        emit deviceInserted(inserted.at(i));
    }

    for (int i = 0; i < removed.length(); i++) {
        emit deviceRemoved(removed.at(i));
    }

    m_system_list = list;
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsb::Config::operator==(const QUsb::Config &other) const
{
    return other.config == config && other.interface == interface && other.alternate == alternate;
}

QUsb::Config::operator QString() const
{
    return QString::fromUtf8("Config(Config: %1, Interface: %2, Alternate: %3)").arg(config).arg(interface).arg(alternate);
}

/*!
    \brief Default constructor.
*/
QUsb::Config::Config(quint8 _config, qint8 _interface, quint8 _alternate)
    : config(_config), interface(_interface), alternate(_alternate)
{
}

/*!
    \brief Copy constructor.
*/
QUsb::Config::Config(const QUsb::Config &other)
    : config(other.config), interface(other.interface), alternate(other.alternate)
{
}

/*!
    \brief Copy operator.
*/
QUsb::Config &QUsb::Config::operator=(QUsb::Config other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
    return *this;
}

/*!
    \brief Default constructor.
*/
QUsb::Id::Id(quint16 _pid, quint16 _vid, quint8 _bus, quint8 _port, quint8 _class, quint8 _subclass)
    : pid(_pid), vid(_vid), bus(_bus), port(_port), dClass(_class), dSubClass(_subclass), configCount(0)
{
}

/*!
    \brief Copy constructor.
*/
QUsb::Id::Id(const QUsb::Id &other)
    : pid(other.pid), vid(other.vid), bus(other.bus), port(other.port), dClass(other.dClass), dSubClass(other.dSubClass),
      configCount(other.configCount), description(other.description)
{
    configurations = other.configurations;
    endpoints = other.endpoints;
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsb::Id::operator==(const QUsb::Id &other) const
{
    return other.pid == pid
            && other.vid == vid
            && other.bus == bus
            && other.port == port
            && other.dClass == dClass
            && other.dSubClass == dSubClass
            && other.configCount == configCount;
}

/*!
    \brief Copy operator.
 */
QUsb::Id &QUsb::Id::operator=(QUsb::Id other)
{
    pid = other.pid;
    vid = other.vid;
    bus = other.bus;
    port = other.port;
    dClass = other.dClass;
    dSubClass = other.dSubClass;
    configCount = other.configCount;
    configurations = other.configurations;
    endpoints = other.endpoints;
    description = other.description;
    return *this;
}

QUsb::Id::operator QString() const
{
    return QString::fromUtf8("Id(Name: %1, Vid: %2, Pid: %3, Bus: %4, Port: %5, Class: %6, Subclass: %7)")
            .arg(description)
            .arg(vid, 4, 16, QChar::fromLatin1('0'))
            .arg(pid, 4, 16, QChar::fromLatin1('0'))
            .arg(bus)
            .arg(port)
            .arg(dClass)
            .arg(dSubClass);
}

// EndpointDescription


/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsb::EndpointDescription::operator==(const QUsb::EndpointDescription &other) const
{
    return other.address == address && other.attributes == attributes && config == other.config;
}

QUsb::EndpointDescription::operator QString() const
{
    return QString::fromUtf8("EndpointDescription(Address: %1, EndpointType: %2, Config: %3)").arg(address).arg(attributes).arg(config);
}

/*!
    \brief Default constructor.
*/
QUsb::EndpointDescription::EndpointDescription(quint8 address, quint8 attributes, quint8 config, quint8 interface, quint8 alternate)
    : address(address), attributes(attributes), config(config), interface(interface), alternate(alternate)
{

}

/*!
    \brief Copy constructor.
*/
QUsb::EndpointDescription::EndpointDescription(const QUsb::EndpointDescription &other)
    : address(other.address), attributes(other.attributes), config(other.config), interface(other.interface), alternate(other.alternate)
{

}

/*!
    \brief Copy operator.
*/
QUsb::EndpointDescription &QUsb::EndpointDescription::operator=(QUsb::EndpointDescription other)
{
    address = other.address;
    attributes = other.attributes;
    config = other.config;
    interface = other.interface;
    alternate = other.alternate;
    return *this;
}
