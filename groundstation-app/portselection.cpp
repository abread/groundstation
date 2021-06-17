#include "portselection.h"
#include "ui_portselection.h"
#include "datawindow.h"
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>

PortSelection::PortSelection(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PortSelection)
{
    ui->setupUi(this);
    init_defaults();
}

void PortSelection::init_defaults() {
    refreshPortList();

    QDateTime now = QDateTime::currentDateTimeUtc();
    ui->lineEdit_outputFilePath->setText("dados_cansat_"+now.toString("yyyy-MM-dd_hh:mm")+".csv");
}

void PortSelection::refreshPortList() {
    ui->comboBox_port->setEnabled(false);
    ui->comboBox_port->clear();

    const auto ports = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &port_info : ports) {
        QString desc = port_info.portName();
        desc.append(" (");
        desc.append(port_info.description());
        desc.append(" - ");
        desc.append(port_info.manufacturer());
        desc.append(")");

        if (port_info.isBusy()) {
            desc.append(" [JÁ EM USO POR OUTRO PROGRAMA]");
        }

        ui->comboBox_port->addItem(desc, QVariant(port_info.systemLocation()));
    }

    ui->comboBox_port->setEnabled(true);
}
void PortSelection::pickOutputFile() {
    ui->lineEdit_outputFilePath->setEnabled(false);

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setWindowTitle("Salvar dados em");

    QStringList filters;
    filters << "text/csv"
            << "text/plain"
            << "application/octet-stream";
    dialog.setMimeTypeFilters(filters);
    dialog.selectMimeTypeFilter("text/csv");
    dialog.setDefaultSuffix(".csv");

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        if (files.length() == 1) {
            ui->lineEdit_outputFilePath->setText(files.first());
        }
    }

    ui->lineEdit_outputFilePath->setEnabled(true);
}

void PortSelection::connect(QString portPath, QString outputPath) {
    this->hide();

    QSerialPort *port = new QSerialPort(portPath, this);
    QFile *outputFile = new QFile(outputPath, this);

    if (!outputFile->open(QFile::WriteOnly | QFile::Append)) {
        QMessageBox::critical(this, "Erro fatal", "Erro ao abrir ficheiro de saída: "+outputFile->errorString());
        delete outputFile;
        delete port;
        return;
    }

    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
    if (!port->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erro fatal", "Erro ao abrir porta de série: "+port->errorString());
        outputFile->close();
        delete outputFile;
        delete port;
        return;
    }

    GroundStation *g = new GroundStation(port, outputFile);
    port->setParent(g);
    outputFile->setParent(g);
    port = nullptr;
    outputFile = nullptr;

    dataWindow = new DataWindow(g);
    g->setParent(dataWindow);
    g = nullptr;

    dataWindow->show();
}

PortSelection::~PortSelection()
{
    delete ui;
    delete dataWindow;
}

void PortSelection::on_pushButton_refresh_clicked()
{
    refreshPortList();
}

void PortSelection::on_toolButton_fileSelect_clicked()
{
    pickOutputFile();
}

void PortSelection::on_pushButton_connect_clicked()
{
    QVariant port = ui->comboBox_port->currentData();
    if (port.isNull() || !port.isValid() || port.type() != QVariant::String) {
        QMessageBox::warning(this, "Porta de série inválida", "Seleciona a porta de série onde está ligado o transceiver.");
        return;
    }

    QString outputPath = ui->lineEdit_outputFilePath->text();
    if (outputPath.length() == 0) {
        QMessageBox::warning(this, "Ficheiro de saída inválido", "Escolhe um ficheiro para guardar os dados.");
        return;
    }

    connect(port.toString(), outputPath);
}
