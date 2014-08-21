#ifndef PASSTABLE_H
#define PASSTABLE_H

#include <QTableWidget>
#include "PassDetails.h"

class PassTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit PassTable(QWidget *parent = 0);

signals:

public slots:

    void updateList(const QList<PassDetails>& passList);

private:

};

#endif // PASSTABLE_H
