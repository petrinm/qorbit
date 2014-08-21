#ifndef __QSIMPLESATELLITEMAP_H__
#define __QSIMPLESATELLITEMAP_H__

#include <QWidget>
#include <QPixmap>
#include <QList>
#include <QTimer>

#include "CoordTopocentric.h"
#include "CoordGeodetic.h"
#include "Observer.h"
#include "SGP4.h"

#include "Satellite.h"

class QSimpleSatelliteMap : public QWidget
{
	Q_OBJECT
public:

    QSimpleSatelliteMap(QWidget* parent = 0);

    ~QSimpleSatelliteMap();


public slots:

    void setObserversPosition(const CoordGeodetic& geo);

protected:

    void paintEvent(QPaintEvent *);

private:

    void drawMarker(QPainter& painter, QString label, const QPointF& point, const QColor& color);


    QPointF latLonToXy(const CoordGeodetic& geo);
    QPointF latLonToXy(double lat, double lon);
    void xyTolatLon(const QPointF& p, double& lat, double& lon);
    void drawGridLines(QPainter& painter);

    void drawPath(QPainter& painter, const SGP4& sat);
    void drawFootprint(QPainter& painter);
    void drawTerminator(QPainter& painter);

	QPixmap background;
    QList<Satellite> satellites;

    QTimer updateTimer;
    CoordGeodetic obsPosition;


    void computeFootprint(const CoordGeodetic& geo);
    bool footprintSplit, footprintHighLat;
    QVector<QPointF> footprintPos;

};

#endif // __QSIMPLESATELLITEMAP_H__
