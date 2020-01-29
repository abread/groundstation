#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QMutex>

struct DataLine {
    QByteArray line;
    QDateTime ts;
};

class GroundStation : public QObject
{
    Q_OBJECT
public:
    explicit GroundStation(QSerialPort* port, QFile* out, QObject *parent = nullptr);
    ~GroundStation();

    void stop();

    int16_t rssi();
    QList<DataLine> data();

public slots:
    void process();

signals:
    void rssiUpdate(int16_t rssi);
    void dataReady();
    void error(QString msg);
    void warning(QString msg);
    void finished();

private:
    void processInput();
    void pushData(QByteArray data);
    void setRSSI(int16_t rssi);
    bool shouldStop();

    QSerialPort *_port;
    QFile *_outputFile;
    QByteArray _inputBuffer;

    int16_t _rssi;
    QMutex _rssiMutex;

    QList<DataLine> _outputBuffer;
    QMutex _outputBufferMutex;

    bool _stop = false;
    QMutex _stopMutex;
};

#endif // SERIALPORTREADER_H
