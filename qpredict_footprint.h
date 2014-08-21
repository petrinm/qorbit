#ifdef QPREDICT_FOOTPRINT_H
#define QPREDICT_FOOTPRINT_H




static int compare_coordinates_x(const QPointF& a, const QPointF& b) {
    if (a.x() < b.x())
        return -1;
    else if (a.x() > b.x())
        return 1;
    else
        return 0;
}

static int compare_coordinates_y(const QPointF& a, const QPointF& b) {
    if (a.y() < b.y())
        return -1;
    else if (a.y() > b.y())
        return 1;
    else
        return 0;
}

static int calculate_footprint(const CoordGeodetic& geo) {

    float sx, sy, msx, msy, ssx, ssy;
    double azimuth, num, dem;
    double rangelon, rangelat, mlon;
    bool warped = false;
    unsigned int numrc = 1;

    /* Range circle calculations.
     * Borrowed from gsat 0.9.0 by Xavier Crehueras, EB3CZS
     * who borrowed from John Magliacane, KD2BD.
     * Optimized by Alexandru Csete and William J Beksi.
     */
    double ssplat = geo.latitude * DEG2RAD;
    double ssplon = geo.longitude * DEG2RAD;
    double beta = (0.5 * sat->footprint) / xkmper;

    for (unsigned int azi = 0; azi < 180; azi++)    {
        azimuth = DEG2RAD * (double)azi;
        rangelat = asin(sin(ssplat) * cos(beta) + cos(azimuth) * sin(beta) * cos(ssplat));
        num = cos(beta) - (sin(ssplat) * sin(rangelat));
        dem = cos(ssplat) * cos(rangelat);

        if (azi == 0 && north_pole_is_covered(geo))
            rangelon = ssplon + M_PI;
        else if (azi == 180 && south_pole_is_covered(geo))
            rangelon = ssplon + M_PI;
        else if (fabs(num / dem) > 1.0)
            rangelon = ssplon;
        else {
            if ((180.0 - azi) >= 0)
                rangelon = ssplon - arccos (num, dem);
            else
                rangelon = ssplon + arccos (num, dem);
        }

        while (rangelon < -M_PI)
            rangelon += 2*M_PI;

        while (rangelon > M_PI)
            rangelon -= 2*M_PI;

        rangelat = rangelat / DEG2RAD;
        rangelon = rangelon / DEG2RAD;

        // mirror longitude
        if (mirror_lon (sat, rangelon, &mlon, satmap->left_side_lon))
            warped = true;

        lonlat_to_xy (rangelon, rangelat, &sx, &sy);
        lonlat_to_xy (mlon, rangelat, &msx, &msy);

        points1[azi] = QPointF(sx, sy);

        // Add mirrored point
        points1->coords[718-2*azi] = msx;
        points1->coords[719-2*azi] = msy;
    }

    // points1 now contains 360 pairs of map-based XY coordinates.
    //   Check whether actions 1, 2 or 3 have to be performed.


    // pole is covered => sort points1 and add additional points
    if (pole_is_covered (sat)) {
        sort_points_x (satmap, sat, points1, 360);
        numrc = 1;
    }
    // pole not covered but range circle has been warped => split points
    else if (warped == true) {
        lonlat_to_xy (satmap, sat->ssplon, sat->ssplat, &ssx, &ssy);
        split_points (satmap, sat, ssx);
        numrc = 2;
    }
    else { // the nominal condition => points1 is adequate
        numrc = 1;
    }

    return numrc;
}

static void split_points(QVector<QPointF>& points1, QVector<QPointF>& points2, const CoordGeodetic& geo, double sspx) {

    // initialize parameters
    int n = points1.size();
    QVector<QPointF> tps1, tps2;

    if (geo.longitude >= 179.4 || geo.longitude <= -179.4) {
        /* sslon = +/-180 deg.
           - copy points with (x > satmap->x0+satmap->width/2) to tps1
           - copy points with (x < satmap->x0+satmap->width/2) to tps2
           - sort tps1 and tps2
        */
        for (int i = 0; i < n; i++) {
            if (points1[i].x() > width() / 2)
                tps1.push_back( points1[i] );
            else
                tps2.push_back( points1[i] );
        }

        qSort(tps1.begin(), tps1.end(), compare_coordinates_y);
        qSort(tps2.begin(), tps2.end(), compare_coordinates_y);
    }
    else if (sspx < width() / 2) {
        /* We are on the left side of the map.
           Scan through points1 until we get to x > sspx (i=ns):

           - copy the points forwards until x < (x0+w/2) => tps2
           - continue to copy until the end => tps1
           - copy the points from i=0 to i=ns => tps1.

           Copy tps1 => points1 and tps2 => points2
        */
        int i = 0;
        while (points1[i].x() <= sspx)
            i++;

        int ns = i - 1;

        while (points1[i].x() > width()/2)
            tps2.push_back( points1[i++] );

        while (i < n)
            tps1.push_back( points1[i++] );

        for (i = 0; i <= ns; i++)
            tps1.push_back( points1[i] );

    }
    else {
        /* We are on the right side of the map.
           Scan backwards through points1 until x < sspx (i=ns):

           - copy the points i=ns,i-- until x >= x0+w/2  => tps2
           - copy the points until we reach i=0          => tps1
           - copy the points from i=n to i=ns            => tps1

        */
        int i = n - 1;
        while (points1[i].x() >= sspx)
            i--;

        int ns = i+1;

        while (points1[i].x() < width()/2)
            tps2.push_back( points1[i--] );

        while (i >= 0)
            tps1.push_back( points1[i--] );

        for (i = n-1; i >= ns; i--)
            tps1.push_back( points1[i] );

    }

    points1 = tps1;
    points2 = tps2;

    // stretch end points to map borders
    if (points1.first().x() < width()/2) {
        points1.first().setX(width());
        points1.last().setX(width());
        points2.first().setX(0);
        points2.last().setX(0);
    }
    else {
        points2.first().setX(width());
        points2.last().setX(width());
        points1.first().setX(0);
        points1.last().setX(0);
    }

}

static void sort_points_x(const CoordGeodetic& geo, QVector<QPointF> &points)
{
    assert(points.size() == 360);

    // call g_qsort_with_data
    qSort(points.begin(), points.end(), compare_coordinates_x);

    // move point at position 0 to position 1
    points[1] = QPointF(0, points[0].y());
    // move point at position N to position N-1
    points[358] = QPointF(width(), points[358].x());

    if (geo.latitude > 0.0) {
        // insert (x0-1, 0) into position 0
        points[0] = QPointF(0,0);
        // insert (width, 0) into position N
        points[359] = QPointF(width(), 0);
    }
    else {
        // insert (0, height) into position 0
        points[0] = QPointF(0, height());
        // insert (width, height) into position N
        points[359] = QPointF(width(), height());
    }
}

#endif // QPREDICT_FOOTPRINT_H
