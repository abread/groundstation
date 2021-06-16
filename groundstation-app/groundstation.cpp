#include "groundstation.h"
#include <QThread>

// actually it's probably something like 62, but let's leave it oversized
#define MAX_MSG_SIZE 512

GroundStation::GroundStation(QSerialPort* port, QFile *outputFile, QObject *parent) :
    QObject(parent),
    _port(port),
    _outputFile(outputFile)
{}

void GroundStation::process() {
    forever {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        _port->waitForReadyRead(5000);
        QByteArray data = _port->read(MAX_MSG_SIZE);
        if (data.isEmpty()) {
            if (_port->error() != QSerialPort::TimeoutError && _port->error() != QSerialPort::NoError) {
                emit error("Erro de comunicação entre computador e estação base: " + _port->errorString());
                _port->clearError();
            }
        } else {
            _inputBuffer.append(data);
            processInput();
        }

    }

    _outputFile->flush(); // really make sure we flush all data
}

void GroundStation::processInput() {
    bool isLine;
    do {
        isLine = false;

        QByteArray line;
        int newline_count = 0;

        for (int i = 0; i < _inputBuffer.size(); i++) {
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

            // normalize newlines
            line.replace("\n\n", "\n");

            // remove trailing CRLF
            line.chop(2);

            handleMessage(line);
        }
    } while(isLine);

    if (_inputBuffer.size() > 2*MAX_MSG_SIZE) {
        emit warning("Problema na comunicação entre computador e estação base: mensagem demasiado grande. Os dados atuais serão interpretados como se fossem uma mensagem completa, o que provavelmente vai causar um erro. ESTE PROBLEMA NÃO É DO CANSAT");

        // just assume it's all one message, and probably error out
        handleMessage(_inputBuffer);

        // clear the buffer
        _inputBuffer.clear();
    }
}

void GroundStation::handleMessage(QByteArray msg) {
    if (msg.startsWith("rssi: ")) {
        msg = msg.right(msg.length() - 6);
        setRSSI(msg.toInt());
    } else if (msg.startsWith("data: ")) {
        msg = msg.right(msg.length() - 6);

        if (_outputFile->write(msg) == -1) {
            emit error("Falha a escrever dados em ficheiro de saída: " + msg);
        }
        _outputFile->flush();

        emit dataReady(msg);
    } else {
        emit error("Erro de comunicação com transceiver da estação base: mensagem inválida recebida. Isto NÃO é problema do CanSat, é quase de certeza culpa do criador deste programa. Dados="+msg);
    }
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

GroundStation::~GroundStation() {
    _port->close();
    _outputFile->flush();
    _outputFile->close();

    delete _port;
    delete _outputFile;
}
