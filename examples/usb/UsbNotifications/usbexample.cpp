#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) : QObject(parent) {
  QObject::connect(&m_usb_manager, SIGNAL(deviceInserted(QUsbDevice::FilterList)),
                   this, SLOT(onDevInserted(QUsbDevice::FilterList)));
  QObject::connect(&m_usb_manager, SIGNAL(deviceRemoved(QUsbDevice::FilterList)), this,
                   SLOT(onDevRemoved(QUsbDevice::FilterList)));

  m_usb_manager.setDebug(false);
  qInfo("Starting...");
  qInfo("Press CTRL+C to close.");
}

UsbExample::~UsbExample() {
  qInfo("Closing...");
}

void UsbExample::onDevInserted(QUsbDevice::FilterList list) {
  qInfo("devices inserted");
  for (int i = 0; i < list.length(); i++) {
    QUsbDevice::DeviceFilter f = list.at(i);
    qInfo("V%04x:P%04x", f.vid, f.pid);
  }
}

void UsbExample::onDevRemoved(QUsbDevice::FilterList list) {
  qInfo("devices removed");
  for (int i = 0; i < list.length(); i++) {
    QUsbDevice::DeviceFilter f = list.at(i);
    qInfo("V%04x:P%04x", f.vid, f.pid);
  }
}
