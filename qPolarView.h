#ifndef __QPOLARVIEW_H__
#define __QPOLARVIEW_H__

#include "Satellite.h"
#include <QWidget>
#include <QVector>

#include <CoordGeodetic.h>

class QPolarView : public QWidget
{
	Q_OBJECT
public:

    QPolarView(QWidget* parent = 0);

    QList<Satellite> satellites() const;

    void addSatellite(Satellite sat);

    const CoordGeodetic& getObserver() const { return obsPosition; }

protected:

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent* event);

public slots:

    void setLocationText(QString text);
    void setEventText(QString text);

    void setObserversPosition(const CoordGeodetic &geo);

private:

    QPoint azelToXy(double azimutz, double elevation) const;
    void xyToAzel(QPointF xy, double &azimutz, double &elevation);

    void drawMarker(QPainter& painter, QString label, const QPointF& point, const QColor& color);

    QList<Satellite> mSatellites;

    QString locationText;
    QString eventText;
    QString cursorText;
    QString satelliteText;
    CoordGeodetic obsPosition;

};

#endif // __QPOLARVIEW_H__
