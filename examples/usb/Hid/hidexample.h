#ifndef HidExample_H
#define HidExample_H

#include <QObject>
#include <QHidDevice>

class HidExample : public QObject
{
    Q_OBJECT
public:
    explicit HidExample(QObject *parent = Q_NULLPTR);
    ~HidExample(void);
    bool openDevice(void);
    void closeDevice(void);
    void read(QByteArray *buf);
    void write(QByteArray *buf);

signals:

private:
    QHidDevice *m_hid_dev;
    QByteArray m_send, m_recv;
};

#endif // HidExample_H
