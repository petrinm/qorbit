#include "PassTable.h"

#include <QStringList>
#include <QDebug>

#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

PassTable::PassTable(QWidget *parent) :
    QTableWidget(parent)
{

    QStringList columns;
    columns << "AOS" << "LOS" << "Duration" << "Peak Elev";

    setColumnCount(columns.size());
    setHorizontalHeaderLabels(columns);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    //sortItems(0);
}

void PassTable::updateList(const QList<PassDetails>& passList) {


    while(rowCount() > 0)
        removeRow(0);
    setSortingEnabled(false);

    foreach (const PassDetails& details, passList) {
        qDebug() << details.aos.ToString().c_str();

        bool found = false;

        /*
        for(int i = 0; i < this->rowCount(); i++) {

            QTableWidgetItem* item = item(i)
        }


        if (found == false) {


        }*/
        int r = rowCount();

        insertRow(r);
        //setItem(r, 0, new QTableWidgetItem("ESTCube-1"));
        setItem(r, 0, new QTableWidgetItem(QString("%1:%2").arg((int)details.aos.Hour()).arg((int)details.aos.Minute())) );
        setItem(r, 1, new QTableWidgetItem(QString("%1:%2").arg(details.los.Hour()).arg(details.los.Minute())) );
        setItem(r, 2, new QTableWidgetItem(details.duration()) );
        setItem(r, 3, new QTableWidgetItem(QString("%1").arg(RAD2DEG*details.max_elevation,0,'f',1)) );

    }

}
