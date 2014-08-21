#ifndef __SATELLITE_H__
#define __SATELLITE_H__

#include <QString>

#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>

class Satellite : public SGP4
{

public:

    Satellite(const Tle& tle, QString name = "") :
        SGP4(tle),
        mName(name),
        mNORADID(tle.Name().c_str())
    {
        if(name.isEmpty())
            name = tle.Name().c_str();

    }

    virtual ~Satellite() {}

    const QString name() const { return mName; }

    const QString NORADId() const { return mNORADID; }

    const CoordGeodetic& latestPosition() const { return latestPos; }

    void update() {
        DateTime now = DateTime::Now(true);
        Eci eci = FindPosition(now);
        latestPos = eci.ToGeodetic();
    }


private:

    QString mName;
    QString mNORADID;

    CoordGeodetic latestPos;
};


#endif // __SATELLITE_H__
