
#include "QSimpleSatelliteMap.h"
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QDebug>
#include "Satellite.h"
#include "SolarPosition.h"

#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


// Colors :D
static QColor infoColor(0, 255, 0, 255);
static QColor infoBgColor(0, 0, 0, 255);
static QColor qthColor(0, 255, 255, 255);
static QColor satColor(240, 240, 0, 255);
static QColor selectedSatColor(255, 255, 255, 255);
static QColor covAreaColor(255, 255, 255, 31);
static QColor gridColor(127, 127, 127, 200);
static QColor terminatorColor(255, 255, 0, 128);
static QColor earthShadowColor(0, 0, 0, 96);
static QColor tickColor(127, 127, 127, 200);
static QColor trackColor(255, 18, 0, 187);
static QColor shadowColor(0, 0, 0, 221);


QSimpleSatelliteMap::QSimpleSatelliteMap(QWidget* parent) :
    QWidget(parent)
{

    background.load("media/nasa-topo_1024.jpg");

    connect(&updateTimer,SIGNAL(timeout()), SLOT(update()));
    updateTimer.setInterval(1000);
    updateTimer.start();

    footprintPos.resize(360/5);

    // http://www.celestrak.com/NORAD/elements/cubesat.txt

    Tle tle ("ESTCUBE 1",
             "1 39161U 13021C   14226.14653900  .00000766  00000-0  13565-3 0  3794",
             "2 39161  98.0947 306.6139 0009384 203.0001 157.0783 14.70096049 68114");
    satellites.append( Satellite(tle, "ESTCube-1") );

    /*tle = Tle("UKUBE-1",
             "1 40074U 14037F   14230.48592373  .00000814  00000-0  11405-3 0  1349",
             "2 40074  98.4003 283.9971 0006146 127.3142 232.8617 14.80435637  6025");
    satellites.append( Satellite(tle, "UKube-1") );*/
}

QSimpleSatelliteMap::~QSimpleSatelliteMap() {

}

void QSimpleSatelliteMap::setObserversPosition(const CoordGeodetic& geo) {
    obsPosition = geo;
}


void QSimpleSatelliteMap::computeFootprint(const CoordGeodetic& geo) {
    double r0 = 6353.0; // ephem.earth_radius
    double alt = geo.altitude + r0; // sat.elevation + r0;

    QVector<Vector> vert(360/5); // = np.zeros(((360/5),3), np.float)
    QVector<Vector> r1vert(360/5); // = np.zeros(((360/5),3), np.float)
    Vector r2vert; // = np.zeros((3,), np.float)

    double x0 = r0 * r0 / alt;
    double phi = acos(x0 / r0);
    double theta;
    double y0 = r0 * sin(phi);

    if ((geo.latitude + phi) > M_PI/2 || (geo.latitude - phi) < -M_PI/2) {
        //print self.sat, self.sat.sublat, self.sat.sublong, math.degrees(phi)
        footprintHighLat = true;
    }
    else {
        footprintHighLat = false;
    }

    for (int i = 0; i < vert.size(); i++) {
        vert[i].x = x0;
        vert[i].y = y0 * sin(DEG2RAD * (i * 5));
        vert[i].z = y0 * cos(DEG2RAD * (i * 5));
    }

    // rotate about y axis by sat.sublat
    double c = cos(geo.latitude);
    double s = sin(geo.latitude);

    for (int i = 0; i < vert.size(); i++) {
        r1vert[i].x = c * vert[i].x + s * vert[i].z;
        r1vert[i].y = vert[i].y;
        r1vert[i].z = -s * vert[i].x + c * vert[i].z;
    }

    Vector midp(c*x0, 0, -s*x0);

    // rotate about z axis by  sat.sublon
    c = cos(geo.longitude);
    s = sin(geo.longitude);
    Vector midpt(c * midp.x - s * midp.y, s * midp.x + c * midp.y, midp.z);

    double max_x = -9999999;
    double min_x = 9999999;

    for (int i = 0; i < vert.size(); i++) {
        r2vert.x = c * r1vert[i].x - s * r1vert[i].y;
        r2vert.y = s * r1vert[i].x + c * r1vert[i].y;
        r2vert.z = r1vert[i].z;

        phi = asin(r2vert.z / r0);
        theta = atan2(r2vert.y, r2vert.x);

        footprintPos[i].setX( (theta + M_PI) * width() / (2.0*M_PI) );
        footprintPos[i].setY( (phi + M_PI/2) * height() / M_PI );

        if (footprintPos[i].x() > max_x)
            max_x = footprintPos[i].x();
        if (footprintPos[i].x() < min_x)
            min_x = footprintPos[i].x();
    }

    phi = asin(midpt.z / r0);
    theta = atan2(midpt.y, midpt.x);

    QPointF midptpix( (theta + M_PI) * width() / (2.0*M_PI), (phi + M_PI/2) * height() / M_PI);

    footprintSplit = false;
    if (fabs((min_x+max_x)/2 - midptpix.x()) > 10) {
        //print 'Splitting',self.sat,midptpix, 'Min', min_x, 'Max', max_x
        footprintSplit = true;
    }
}

void QSimpleSatelliteMap::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.drawPixmap(rect(), background);

    drawGridLines(painter);
    //drawTerminator(painter);

    // Satellites
    DateTime now = DateTime::Now(true);
    //now = DateTime(63543987740107401); // debug
    //now = now.AddMinutes(-2);
   // now = now.AddMinutes(+10 + 1 * 98);

    qDebug() << now.Ticks();

    foreach(const Satellite& satellite, satellites) {

        drawPath(painter, satellite);

        CoordGeodetic geo = satellite.FindPosition(now).ToGeodetic();

        computeFootprint(geo);
        drawFootprint(painter);

        drawMarker(painter, satellite.name(), latLonToXy(RAD2DEG * geo.latitude, RAD2DEG * geo.longitude), satColor);

    }

    // Random text
    QRectF textRect = rect().marginsRemoved(QMargins(3,3,3,3));
    painter.setPen(infoColor);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, "Otaniemi - Espoo, Finland");

    // Groundstation
    drawMarker(painter, "Otaniemi", latLonToXy(obsPosition), qthColor);

}


void QSimpleSatelliteMap::drawPath(QPainter& painter, const SGP4& sat) {

    double period = 98; // minutes :TODO: Hardcoded value for ESTCube-1!

    DateTime tims = DateTime::Now(true).AddMinutes(-period);
    CoordGeodetic geo = sat.FindPosition(tims).ToGeodetic();
    CoordGeodetic prevGeo = geo;

    QPainterPath path;
    painter.setPen(trackColor);
    path.moveTo(latLonToXy(geo));

    for(int i = 0; i < 3*period; i++ ) {

        geo = sat.FindPosition(tims).ToGeodetic();

        // Flip?
        if( prevGeo.longitude < 0 && geo.longitude > 0) {
            path.lineTo(latLonToXy(geo) - QPointF(width(), 0));
            painter.drawPath(path);
            path = QPainterPath();
            path.moveTo(latLonToXy(prevGeo) + QPointF(width(), 0));
        }

        path.lineTo(latLonToXy(geo));

        tims = tims.AddMinutes(1);
        prevGeo = geo;
    }

    painter.setPen(trackColor);
    painter.drawPath(path);

}

void QSimpleSatelliteMap::drawFootprint(QPainter& painter) {

    painter.setPen( satColor );
    painter.setBrush( QBrush(covAreaColor, Qt::SolidPattern) );

    if (footprintHighLat) {
        QPolygonF poly1;

        double min_value = 9999999;
        int min_pos = -1;
        double max_value = -9999999;
        int max_pos = -1;

        for (int i = 0; i < footprintPos.size(); i++) {
            if (footprintPos[i].x() < min_value) {
                min_value = footprintPos[i].x();
                min_pos = i;
            }
            if (footprintPos[i].x() > max_value) {
                max_value = footprintPos[i].x();
                max_pos = i;
            }
        }

        for (int i = 0; i < footprintPos.size(); i++) {
            if (i == min_pos && footprintPos[0].y() < height()/2) {
                poly1.append(QPointF(-1.0, -1.0));
                poly1.append(QPointF(-1.0, footprintPos[MAX(i-1,0)].y()));
            }
            else if (i == max_pos && footprintPos[0].y() > height()/2) {
                poly1.append(QPointF(width()+1, height()+1));
                poly1.append(QPointF(width()+1, footprintPos[MAX(i-1,0)].y()));
            }

            poly1.append(footprintPos[i]);

            if (i == min_pos && footprintPos[0].y() > height()/2) {
                poly1.append(QPointF(-1.0, footprintPos[MIN(i+1,footprintPos.size()-1)].y()));
                poly1.append(QPointF(-1.0, height()+1));

            }
            else if (i == max_pos && footprintPos[0].y() < height()/2) {
                poly1.append(QPointF(width()+1, footprintPos[MIN(i+1,footprintPos.size()-1)].y()));
                poly1.append(QPointF(width()+1, -1.0));

            }
        }

        /*footprintItem[0].setPolygon(poly1);
        footprintItem[1].hide();*/
        painter.drawPolygon(poly1);
    }
    else {
        if (footprintSplit) {
            QPolygonF poly1, poly2;

            bool f = footprintPos[0].x() < width()/2;

            for (int i = 0; i < footprintPos.size(); i++) {
                if (footprintPos[i].x() < width()/2) {

                    if(f == false) {

                        poly2.append(QPointF(width()+1, footprintPos[i].y()));
                        poly1.append(QPointF(-1, footprintPos[i].y()));
                        f = true;
                    }

                    poly1.append(footprintPos[i]);


                }
                else {

                    if(f == true) {
                        poly1.append(QPointF(-1, footprintPos[i].y()));
                        poly2.append(QPointF(width()+1, footprintPos[i].y()));
                        f = false;
                    }

                    poly2.append(footprintPos[i]);
                }
            }
            /*footprintItem[0].setPolygon(poly1);
            footprintItem[1].setPolygon(poly2);
            footprintItem[1].show();*/
            painter.drawPolygon(poly1);
            painter.drawPolygon(poly2);
        }
        else {
            QPolygonF poly;
            for (int i = 0; i < footprintPos.size(); i++)
                poly.append(footprintPos[i]);

            /*footprintItem[0].setPolygon(poly);
            footprintItem[1].hide();*/
            painter.drawPolygon(poly);
        }
    }
}


void QSimpleSatelliteMap::drawMarker(QPainter& painter, QString label, const QPointF& point, const QColor& color) {

    // Marker shadow
    QRectF r(0,0,5,5);
    r.moveCenter(point + QPointF(1, 1));
    painter.fillRect(r, QBrush(shadowColor));

    // Marker
    r.moveCenter(point);
    painter.fillRect(r, QBrush(color));

    // Label shadow
    r.setSize(QSizeF(100, 15));
    r.moveCenter(point + QPointF(1, 11));
    painter.setPen( shadowColor );
    painter.drawText(r, Qt::AlignCenter, label);

    // Label
    painter.setPen(color);
    r.moveCenter(point + QPointF(0, 10));
    painter.drawText(r, Qt::AlignCenter, label);

}

void QSimpleSatelliteMap::drawGridLines(QPainter& painter) {

    painter.setPen( QPen(QBrush(gridColor), 0.5) );
    double xstep = (30.0 * width() / 360.0);
    double ystep = (30.0 * height() / 180.0);

    // Horizontal
    for(int i = 0; i < 5; i++) {
        painter.drawLine(0, (i+1) * ystep, width(), (i+1) * ystep);
    }

    // Vertical
    for(int i = 0; i < 11; i++) {
        painter.drawLine((i+1) * xstep, 0, (i+1) * xstep, height());
    }

}

inline double sgn(double a) { return (a < 0) ? -1.0 : 1.0; }

void QSimpleSatelliteMap::drawTerminator(QPainter& painter) {

    DateTime now = DateTime::Now(false);
    SolarPosition sun;

    Eci eci = sun.FindPosition(now);
    CoordGeodetic geo = eci.ToGeodetic();

    double sx = cos(geo.latitude) * cos(geo.longitude);
    double sy = cos(geo.latitude) * sin(-geo.longitude);
    double sz = sin(geo.latitude);

    QPainterPath path;
    path.moveTo( latLonToXy(sz < 0.0 ? 90.0 : -90.0, -180.0) );

    for (int longitude = -180; longitude <= 180; ++longitude) {
        double lx = cos(DEG2RAD * (longitude + sgn(sz) * 90));
        double ly = sin(DEG2RAD * (longitude + sgn(sz) * 90));
        /* lz = 0.0; */

        double rx = ly*sz /* -lz*sy */;
        double ry = /* lz*sx */ - lx*sz;
        double rz = - lx*sy - ly*sx;

        double length = sqrt (rx*rx + ry*ry + rz*rz);
        path.lineTo( latLonToXy(asin(rz / length) * (1.0 / DEG2RAD), longitude) );

    }

    path.lineTo( latLonToXy(sz < 0.0 ? 90.0 : -90.0, 180.0) );

    painter.setBrush(QBrush(Qt::NoBrush));
    painter.setPen(QColor(0, 0, 255));
    painter.drawPath(path);

}

QPointF QSimpleSatelliteMap::latLonToXy(const CoordGeodetic& geo) {
    return latLonToXy(RAD2DEG * geo.latitude, RAD2DEG * geo.longitude);
}

QPointF QSimpleSatelliteMap::latLonToXy(double lat, double lon) {
    double x = width() * (180 + lon) / 360;
    double y = height() * (90 - lat) / 180;
    while(x < 0) x += width();
    while(x > width()) x -= width();
    return QPointF(x, y);
}

void QSimpleSatelliteMap::xyTolatLon(const QPointF& p, double& lat, double& lon) {
    lat = 90 - (180  / height()) * p.y();
    lon = (360 / width()) * p.x();
    while(lon > 180) lon -= 360;
    while(lon < -180) lon += 360;
}
