#include "groundstation.h"
#include <QThread>

GroundStation::GroundStation(QSerialPort* port, QFile *outputFile, QObject *parent) :
    QObject(parent),
    _port(port),
    _outputFile(outputFile)
{}

void GroundStation::process() {
    forever {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        _port->waitForReadyRead(1000);
        QByteArray data = _port->readAll();
        _inputBuffer.append(data);
        processInput();
    }

    _outputFile->flush(); // really make sure we flush all data
}

void GroundStation::processInput() {
    bool isLine;
    do {
        isLine = false;

        QByteArray line;
        int newline_count = 0;

        for (ssize_t i = 0; i < _inputBuffer.size(); i++) {
            char b = _inputBuffer.at(i);

            if (b == '\n') {
                newline_count++;
            }

            if (b != '\n' && newline_count % 2 != 0) {
                isLine = true;
                break;
            }

            line.append(b);
        }

        if (isLine) {
            _inputBuffer = _inputBuffer.right(_inputBuffer.length() - line.length());
            line.replace("\n\n", "\n");
            if (line.startsWith("rssi: ")) {
                line.chop(1); // remove \n
                line = line.right(line.length() - 6);
                setRSSI(line.toInt());
            } else {
                if (line.startsWith("data: ")) {
                    line = line.right(line.length() - 6);
                } else {
                    emit warning("Erro de comunicação com transceiver da estação base. Dados erróneos guardados em bruto.");
                }

                pushData(line);
            }
        }
    } while(isLine);
}

void GroundStation::pushData(QByteArray data) {
    DataLine l;
    l.ts = QDateTime::currentDateTime();
    l.line = data;

    if (_outputFile->write(data) == -1) {
        emit error("Falha a escrever dados em ficheiro de saída: "+data);
    }
    _outputFile->flush();

    _outputBufferMutex.lock();
    _outputBuffer.append(l);
    _outputBufferMutex.unlock();

    emit dataReady();
}

void GroundStation::setRSSI(int rssi) {
    _rssiMutex.lock();
    _rssi = rssi;
    _rssiMutex.unlock();

    emit rssiUpdate(rssi);
}

int GroundStation::rssi() {
    QMutexLocker lock(&_rssiMutex);
    return _rssi;
}

QList<DataLine> GroundStation::data() {
    QMutexLocker lock(&_outputBufferMutex);

    QList<DataLine> out;
    _outputBuffer.swap(out);

    return out;
}

GroundStation::~GroundStation() {
    _port->close();
    _outputFile->flush();
    _outputFile->close();

    delete _port;
    delete _outputFile;
}
