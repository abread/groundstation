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
    void updateRSSI(int16_t rssi);
    void pollData();

private:
    Ui::DataWindow *ui;
    GroundStation *station;
    QThread *stationThread;

    int numRead = 0;
    int numShown = 0;
    bool _autoscroll = true;
};

#endif // DATAWINDOW_H
