#include "qbaseusb.h"

QBaseUsbDevice::QBaseUsbDevice(QObject *parent) : QObject(parent) {
  this->setDefaults();
  mSpd = QtUsb::unknownSpeed;
}

QBaseUsbDevice::~QBaseUsbDevice() {}

QString QBaseUsbDevice::getSpeedString() {
  switch (mSpd) {
    case QtUsb::unknownSpeed:
      return "Unknown speed";
    case QtUsb::lowSpeed:
      return "Low speed";
    case QtUsb::fullSpeed:
      return "Full speed";
    case QtUsb::highSpeed:
      return "High speed";
    case QtUsb::superSpeed:
      return "Super speed";
  }

  return "Error";
}

qint32 QBaseUsbDevice::write(const QByteArray &buf) {
  return this->write(&buf, buf.size());
}

qint32 QBaseUsbDevice::read(QByteArray *buf) { return this->read(buf, 4096); }

bool QBaseUsbDevice::write(char c) {
  QByteArray buf(1, c);
  return this->write(buf) > 0;
}

bool QBaseUsbDevice::read(char *c) {
  QByteArray buf;
  Q_CHECK_PTR(c);
  if (this->read(&buf, 1) > 0) {
    *c = buf.at(0);
    return true;
  }
  return false;
}

void QBaseUsbDevice::close() {}

void QBaseUsbDevice::setTimeout(quint16 timeout) { mTimeout = timeout; }

void QBaseUsbDevice::showSettings() {
  qWarning() << "\n"
             << "mDebug" << mDebug << "\n"
             << "mConfig" << mConfig.config << "\n"
             << "mTimeout" << mTimeout << "\n"
             << "mReadEp" << QString::number(mConfig.readEp, 16) << "\n"
             << "mWriteEp" << QString::number(mConfig.writeEp, 16) << "\n"
             << "mInterface" << mConfig.interface << "\n"
             << "mDevice.pid" << QString::number(mFilter.pid, 16) << "\n"
             << "mDevice.vid" << QString::number(mFilter.vid, 16) << "\n";
}

void QBaseUsbDevice::setDefaults() {
  mConnected = false;
  mDebug = false;
  mTimeout = QtUsb::DefaultTimeout;
  mConfig.readEp = 0x81;
  mConfig.writeEp = 0x01;
  mConfig.config = 0x01;
  mConfig.interface = 0x00;
  mConfig.alternate = 0x00;
}
