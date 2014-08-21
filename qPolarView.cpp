
#include "qPolarView.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QMouseEvent>
#include <cmath>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

static const int margin = 25;

static QColor bgColor(255, 255, 255, 255);
static QColor axisColor(15, 15, 15, 127);
static QColor tickColor(0, 127, 0, 255);
static QColor satColor(143, 0, 0, 255);
static QColor selectedSatColor(255, 13, 11, 255);
static QColor trackColor(0, 0, 255, 255);
static QColor infoColor(0, 0, 127, 255);


QPolarView::QPolarView(QWidget* parent) :
    QWidget(parent),
    locationText("Otaniemi"),
    eventText("Next: ESTCube-1\nin 12:34"),
    satelliteText("")
{
    setAttribute(Qt::WA_StaticContents);
    setMinimumSize(200,200);
    setMouseTracking(true);
}

QList<Satellite> QPolarView::satellites() const {
    return mSatellites;
}

void QPolarView::addSatellite(Satellite sat) {
    mSatellites.push_back(sat);
}

void QPolarView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int line_extra = 5;

    int radius = MIN(width(), height()) / 2 - margin;
    QPoint center = 0.5f * QPoint(width(), height());

    // White background
    painter.fillRect(0, 0, width(), height(), QBrush(bgColor));

    // Draw 0, 30 and 60 deg elevation circles
    painter.setPen(axisColor);
    painter.drawEllipse(center, radius, radius);
    painter.drawEllipse(center, 0.6667f*radius, 0.6667f*radius);
    painter.drawEllipse(center, 0.3333f*radius, 0.3333f*radius);

    // Horizontal and vertical lines
    painter.drawLine(center - QPoint(line_extra + radius, 0), center + QPoint(line_extra + radius, 0)); // POLV_LINE_EXTRA
    painter.drawLine(center - QPoint(0, line_extra + radius), center + QPoint(0, line_extra + radius));

    // Draw labels
    painter.setPen(tickColor);
    painter.setFont(QFont("Helvetica", 10));
    QRect textRect(0,0,15,15);
    textRect.moveCenter(center - QPoint(0, 15 + radius));
    painter.drawText(textRect, Qt::AlignCenter, "N");
    textRect.moveCenter(center + QPoint(0, 15 + radius));
    painter.drawText(textRect, Qt::AlignCenter, "S");
    textRect.moveCenter(center - QPoint(15 + radius, 0));
    painter.drawText(textRect, Qt::AlignCenter, "W");
    textRect.moveCenter(center + QPoint(15 + radius, 0));
    painter.drawText(textRect, Qt::AlignCenter, "E");

    // Tooltips
    painter.setPen(infoColor);
    painter.setFont(QFont("Helvetica", 8));
    QRectF r(rect());
    r.setSize(QSizeF(MIN(width(), height()),MIN(width(), height())));
    r.moveCenter(center);

    // Location info
    painter.drawText(r, Qt::AlignLeft | Qt::AlignTop, locationText);
    // Next event
    painter.drawText(r, Qt::AlignRight | Qt::AlignTop, eventText);
    // Cursor text
    painter.drawText(r, Qt::AlignLeft | Qt::AlignBottom, cursorText);
    // Selected satellite text
    painter.drawText(r, Qt::AlignRight | Qt::AlignBottom, satelliteText);

    // Draw satellites
    /*foreach(Satellite& satellite, satellites) {
        /*QPainterPath path;

        painter.setPen(trackColor);
        painter.drawPath(path);*

        drawMarker(painter, satellite.name(), azelToXy(0,24), satColor);
    }*/

    // Draw current pointing

}

void QPolarView::drawMarker(QPainter& painter, QString label, const QPointF& point, const QColor& color) {

    // Marker
    QRectF r(0,0,5,5);
    r.moveCenter(point);
    painter.fillRect(r, QBrush(color));

    // Label
    painter.setPen(color);
    r.moveCenter(point + QPointF(0, 10));
    painter.drawText(r, Qt::AlignCenter, label);

}

QPoint QPolarView::azelToXy(double azimutz, double elevation) const {
    if (elevation < 0.0) {
        return QPoint(0,0);
    }

    int radius = MIN(width(), height()) / 2 - margin;
    QPointF center = 0.5f * QPointF(width(), height());

    int dist = radius - (2 * radius * DEG2RAD * elevation) / M_PI;
    int x = (center.x() + dist * sin(DEG2RAD * azimutz));
    int y = (center.y() - dist * cos(DEG2RAD * azimutz));

    return QPoint(x, y);
}

void QPolarView::xyToAzel(QPointF xy, double &azimutz, double &elevation) {

    int radius = MIN(width(), height()) / 2 - margin;
    QPointF rel = 0.5f * QPointF(width(), height()) - xy;

    double dist = sqrt(rel.x() * rel.x() + rel.y() * rel.y());

    if (dist > radius) {
        azimutz = -1.0;
        elevation = -1.0;
        return;
    }

    elevation = 90 - 90 * dist / radius;
    azimutz = RAD2DEG * atan2(-rel.x(), rel.y());

    if(azimutz < 0)
        azimutz += 360.0;
}

void QPolarView::setObserversPosition(const CoordGeodetic &geo) {
    obsPosition = geo;
}

void QPolarView::mouseMoveEvent(QMouseEvent* event) {

    double azimutz = event->localPos().x() * 0.21352;
    double elevation = event->localPos().y() * 0.3341;

    xyToAzel(event->localPos(), azimutz, elevation);

    if (elevation > 0)
        cursorText = QString("AZ %1°\nEL %2°").arg(azimutz,0,'f',0).arg(elevation,0,'f',0);
    else
        cursorText = "";

    update();
}

void QPolarView::setLocationText(QString text) {
    locationText = text;
    update();
}

void QPolarView::setEventText(QString text) {
    eventText = text;
    update();
}
