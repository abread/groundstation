#include "portselection.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PortSelection w;
    w.show();

    return a.exec();
}
