#ifndef DATAWINDOW_H
#define DATAWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "groundstation.h"

namespace Ui {
class DataWindow;
}

class DataWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataWindow(GroundStation *g, QWidget *parent = nullptr);
    ~DataWindow();

public slots:
    void clear();
    void setAutoscroll(bool);

private slots:
    void showWarning(QString msg);
    void showError(QString msg);
    void updateRSSI(int rssi);
    void recvData(QByteArray msg);

private:
    void showMsg(const char *tag, QByteArray msg);
    inline void showMsg(const char *tag, QString msg) { showMsg(tag, msg.toUtf8()); }

    Ui::DataWindow *ui;
    GroundStation *station;
    QThread *stationThread;

    int numRead = 0;
    int numShown = 0;
    bool _autoscroll = true;
};

#endif // DATAWINDOW_H
