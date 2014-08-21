#include "qorbit.h"
#include "ui_qorbit.h"
#include <QTimer>

#include "PassCalculator.h"

qOrbit::qOrbit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qOrbit)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("Petri Niemelä, 2014");

    CoordGeodetic otaniemi(60.18870, 24.83070, 0);

    calc = new PassCalculator();
    ui->polarWidget->setObserversPosition(otaniemi);
    ui->mapWidget->setObserversPosition(otaniemi);


    calc->refresh();

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), calc, SLOT(refresh()));
    timer->setInterval(5 * 1000);
    timer->start();
    connect(calc, SIGNAL(listUpdated(QList<PassDetails>)), ui->tableWidget, SLOT(updateList(QList<PassDetails>)));

}

qOrbit::~qOrbit()
{
    delete calc;
    delete ui;
}
