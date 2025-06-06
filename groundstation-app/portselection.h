#ifndef PORTSELECTION_H
#define PORTSELECTION_H

#include "datawindow.h"
#include <QMainWindow>
#include <QFile>

namespace Ui {
class PortSelection;
}

struct OutFiles {
    QFile* out;
    QFile* log;
};

class PortSelection : public QMainWindow
{
    Q_OBJECT

public:
    explicit PortSelection(QWidget *parent = nullptr);
    ~PortSelection();

private slots:
    void on_pushButton_refresh_clicked();

    void on_pushButton_connect_clicked();

private:
    Ui::PortSelection *ui;
    DataWindow *dataWindow = nullptr;

    void init_defaults();
    void refreshPortList();
    OutFiles pickOutputFile();
    void connect(QString port, OutFiles files);
};

#endif // PORTSELECTION_H
