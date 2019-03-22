#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) : QObject(parent) {
  QObject::connect(&m_usb_manager, SIGNAL(deviceInserted(QtUsb::FilterList)),
                   this, SLOT(onDevInserted(QtUsb::FilterList)));
  QObject::connect(&m_usb_manager, SIGNAL(deviceRemoved(QtUsb::FilterList)), this,
                   SLOT(onDevRemoved(QtUsb::FilterList)));

  m_usb_manager.setDebug(true);
  qInfo("Starting...");
  qInfo("Press CTRL+C to close.");
}

UsbExample::~UsbExample() {}

void UsbExample::onDevInserted(QtUsb::FilterList list) {
  qInfo("devices inserted");
  for (int i = 0; i < list.length(); i++) {
    QtUsb::DeviceFilter f = list.at(i);
    qInfo("V%04x:P%04x", f.vid, f.pid);
  }
}

void UsbExample::onDevRemoved(QtUsb::FilterList list) {
  qInfo("devices removed");
  for (int i = 0; i < list.length(); i++) {
    QtUsb::DeviceFilter f = list.at(i);
    qInfo("V%04x:P%04x", f.vid, f.pid);
  }
}
