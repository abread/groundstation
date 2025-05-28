#include "groundstation.h"
#include <QThread>

// actually it's probably something like 62, but let's leave it oversized
#define MAX_MSG_SIZE 512

GroundStation::GroundStation(QSerialPort* port, QFile *outputFile, QFile *logFile, QObject *parent) :
    QObject(parent),
    _port(port),
    _outputFile(outputFile),
    _logFile(logFile)
{
    if (_logFile->write("timestamp;rssi;msg...") == -1) {
        emit error("Falha a escrever cabeçalho no registo de execução.");
    }
    _logFile->flush();
}

void GroundStation::process() {
    forever {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        _port->waitForReadyRead(500);
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

    // really make sure we flush all data
    _outputFile->flush();
    _logFile->flush();
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

        QString logMsgPrologue;
        logMsgPrologue.append(QString::number(QDateTime::currentMSecsSinceEpoch()));
        logMsgPrologue.append(';');
        logMsgPrologue.append(QString::number(this->rssi()));
        logMsgPrologue.append(';');
        QByteArray logMsgPrologueBytes = logMsgPrologue.toUtf8();

        if (_logFile->write(logMsgPrologueBytes) == -1 || _logFile->write(msg) == -1 || _logFile->write("\n") == -1) {
            emit error("Falha a escrever dados em registo de execução");
        }
        _logFile->flush();

        if (_outputFile->write(msg) == -1) {
            emit error("Falha a escrever dados em ficheiro de saída: " + msg);
        }
        _outputFile->flush();

        emit dataReady(msg);
    } else {
        emit error("Erro de comunicação com transceiver da estação base: mensagem inválida recebida. Confirme que está a tentar ligar-se à estação base e não ao CanSat, e que a estação base tem o seu programa carregado (e não o do CanSat). Dados="+msg);
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
