#ifndef PASSCALCULATOR_H
#define PASSCALCULATOR_H

#include <QObject>
#include "PassDetails.h"
#include "Satellite.h"

class PassCalculator : public QObject
{
    Q_OBJECT
public:
    explicit PassCalculator(QObject *parent = 0);



signals:

    void listUpdated(const QList<PassDetails>& passList);


public slots:

    void refresh();
    void setObserversPosition(const CoordGeodetic &geo);
    void setMiniumElevation(double elev);

private:

    QList<PassDetails> passList;

    CoordGeodetic obsPosition;
    double miniumElevation;

    double FindMaxElevation(const CoordGeodetic& user_geo, SGP4& sgp4, const DateTime& aos, const DateTime& los);

    DateTime FindCrossingPoint(const CoordGeodetic& user_geo, SGP4& sgp4, DateTime time1,DateTime time2,bool finding_aos);

    QList<PassDetails> GeneratePassList(const CoordGeodetic& user_geo, SGP4& sgp4, const DateTime& start_time, const DateTime& end_time, const int time_step);


};

#endif // PASSCALCULATOR_H
