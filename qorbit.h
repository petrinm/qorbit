#ifndef QORBIT_H
#define QORBIT_H

#include <QMainWindow>

class PassCalculator;

namespace Ui {
class qOrbit;
}

class qOrbit : public QMainWindow
{
    Q_OBJECT

public:
    explicit qOrbit(QWidget *parent = 0);
    ~qOrbit();

private:
    Ui::qOrbit *ui;
    PassCalculator* calc;
};

#endif // QORBIT_H
