#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QMutex>

class GroundStation : public QObject
{
    Q_OBJECT
public:
    explicit GroundStation(QSerialPort* port, QFile* out, QObject *parent = nullptr);
    ~GroundStation();

    int rssi();

public slots:
    void process();

signals:
    void rssiUpdate(int rssi);
    void dataReady(QByteArray l);
    void error(QString msg);
    void warning(QString msg);
    void finished();

private:
    void processInput();
    void handleMessage(QByteArray msg);
    void setRSSI(int rssi);

    QSerialPort *_port;
    QFile *_outputFile;
    QByteArray _inputBuffer;

    int _rssi;
    QMutex _rssiMutex;
};

#endif // SERIALPORTREADER_H
