#ifndef PASSDETAILS_H
#define PASSDETAILS_H

#include <QString>
#include <SGP4.h>
#include <Util.h>

class PassDetails
{
public:
    PassDetails();

    DateTime aos;
    DateTime los;

    double max_elevation;

    // duration

    QString duration() const {
        TimeSpan d = los - aos;

        if(d.Minutes() > 0)
            return QString("%1m %2s").arg(d.Minutes()).arg(d.Seconds());
        else
            return QString("%1s").arg(d.Seconds());

    }
    // satellite
};

#endif // PASSDETAILS_H
