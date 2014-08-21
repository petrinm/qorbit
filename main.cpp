#include "qorbit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qOrbit w;
    w.show();

    return a.exec();
}
