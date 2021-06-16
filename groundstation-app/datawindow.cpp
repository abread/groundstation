#include "datawindow.h"
#include "ui_datawindow.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

DataWindow::DataWindow(GroundStation *g, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataWindow),
    station(g),
    stationThread(new QThread(this))
{
    ui->setupUi(this);

    station->moveToThread(stationThread);
    connect(stationThread, &QThread::started, station, &GroundStation::process);
    connect(stationThread, &QThread::finished, station, &GroundStation::deleteLater);

    connect(station, &GroundStation::error, this, &DataWindow::showError);
    connect(station, &GroundStation::warning, this, &DataWindow::showWarning);
    connect(station, &GroundStation::dataReady, this, &DataWindow::recvData);
    connect(station, &GroundStation::rssiUpdate, this, &DataWindow::updateRSSI);

    connect(ui->checkBox, &QCheckBox::clicked, this, &DataWindow::setAutoscroll);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &DataWindow::clear);

    stationThread->start();
}

void DataWindow::updateRSSI(int rssi) {
    ui->label_rssi->setNum(rssi);
}

void DataWindow::recvData(QByteArray msg) {
    showMsg("DADOS", msg);

    ui->label_numShown->setNum(++numShown);
    ui->label_numReceived->setNum(++numRead);

    if (_autoscroll) {
        ui->textEdit_data->ensureCursorVisible();
    }
}

void DataWindow::clear() {
    numShown = 0;
    ui->textEdit_data->clear();
}

void DataWindow::setAutoscroll(bool a) {
    _autoscroll = a;
}

void DataWindow::showError(QString msg) {
    showMsg("ERRO", msg);
    //QMessageBox::critical(this, "ERRO", msg);
}

void DataWindow::showWarning(QString msg) {
    showMsg("AVISO", msg);
    //QMessageBox::warning(this, "Aviso", msg);
}

void DataWindow::showMsg(const char *tag, QByteArray msg) {
    auto ts = QDateTime::currentDateTime();
    auto color = "black";
    if (QString::compare(tag, "AVISO") == 0) {
        color = "orange";
    } else if (QString::compare(tag, "ERRO") == 0) {
        color = "red";
    }

    QString start = "";
    start += "<b style=\"color:";
    start += color;
    start += ";\">[";
    start += ts.toString("hh:mm:ss");
    start += ' ';
    start += tag;
    start += "]</b>&nbsp;";
    ui->textEdit_data->insertHtml(start);
    ui->textEdit_data->insertPlainText(msg);

    if (QString::compare(tag, "DADOS") == 0 && msg.back() != '\n') {
        ui->textEdit_data->insertHtml("&nbsp;<b>[sem fim de linha]</b>");
    }

    ui->textEdit_data->insertHtml("<br>");

    qDebug() << '[' << ts.toString("hh:mm:ss") << ' ' << tag << "] "
             << msg
             << ((QString::compare(tag, "DADOS") == 0 && msg.back() != '\n') ? "[sem fim de linha]" : "")
             << '\n';
}

DataWindow::~DataWindow()
{
    stationThread->quit();
    stationThread->requestInterruption();
    stationThread->wait(5000);

    delete ui;
}
