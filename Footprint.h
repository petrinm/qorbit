static bool pole_is_covered(const Satellite& sat)
{
	if (north_pole_is_covered(sat) ||south_pole_is_covered(sat))
		return true;
	else
		return false;
}

static bool north_pole_is_covered (const Satellite& sat) 
{
	int ret1;
	double qrb1, az1;

	ret1 = qrb (sat->ssplon, sat->ssplat, 0.0, 90.0, &qrb1, &az1);
	if (ret1 != RIG_OK) {
		sat_log_log (SAT_LOG_LEVEL_ERROR, 
					 _("%s: Bad data measuring distance to North Pole %f %f."),
					 __FUNCTION__, sat->ssplon, sat->ssplat);
	}
	if (qrb1 <= 0.5*sat->footprint) {
		return true;
	}
	return false;
}

/** Check whether the footprint covers the South pole. */
static bool south_pole_is_covered (const Satellite& sat) 
{
	int ret1;
	double qrb1, az1;

	ret1 = qrb (sat->ssplon, sat->ssplat, 0.0, -90.0, &qrb1, &az1);
	if (ret1 != RIG_OK) {
		sat_log_log (SAT_LOG_LEVEL_ERROR, 
					 _("%s: Bad data measuring distance to South Pole %f %f."),
					 __FUNCTION__, sat->ssplon, sat->ssplat);
	}
	if (qrb1 <= 0.5*sat->footprint) {
		return true;
	}
	return FALSE;
}


static bool mirror_lon(const Satellite& sat, double rangelon, double *mlon, double mapbreak)
{
	double diff;
	bool warped = FALSE;
  
	/* make it so rangelon is on left of ssplon */
	diff = (sat->ssplon - rangelon);
	while (diff < 0 )
		diff += 360;
	while (diff > 360 )
		diff -= 360;

	*mlon = sat->ssplon + fabs(diff);
	while (*mlon > 180)
		*mlon -= 360;
	while (*mlon < -180)
		*mlon += 360;
	
	if (((sat->ssplon >= mapbreak) && (sat->ssplon < mapbreak + 180)) ||
		((sat->ssplon < mapbreak - 180) && (sat->ssplon >= mapbreak - 360))) {
		if (((rangelon >= mapbreak) && (rangelon < mapbreak + 180)) ||
			((rangelon < mapbreak - 180) && (rangelon >= mapbreak - 360))) {
		} else {
			warped = TRUE;
			//printf ("sat %s warped for first \n",sat->nickname);
		} 
	} else {
		if (((*mlon >= mapbreak) && (*mlon < mapbreak + 180)) || 
			((*mlon < mapbreak - 180) && (*mlon >= mapbreak - 360))) {
			 warped = TRUE;
			//printf ("sat %s warped for second \n",sat->nickname);
		}
	}

	return warped;
}


/** \brief Calculate satellite footprint and coverage area.
 *  \param satmap TheGtkSatMap widget.
 *  \param sat The satellite.
 *  \param points1 Initialised GooCanvasPoints structure with 360 points.
 *  \param points2 Initialised GooCanvasPoints structure with 360 points.
 *  \return The number of range circle parts.
 *
 * This function calculates the "left" side of the range circle and mirrors
 * the points in longitude to create the "right side of the range circle, too.
 * In order to be able to use the footprint points to create a set of subsequent
 * lines conencted to each other (poly-lines) the function may have to perform
 * one of the following three actions:
 *
 * 1. If the footprint covers the North or South pole, we need to sort the points
 *    and add two extra points: One to begin the range circle (e.g. -180,90) and
 *    one to end the range circle (e.g. 180,90). This is necessary to create a
 *    complete and consistent set of points suitable for a polyline. The addition
 *    of the extra points is done by the sort_points function.
 *
 * 2. Else if parts of the range circle is on one side of the map, while parts of
 *    it is on the right side of the map, i.e. the range circle runs off the border
 *    of the map, it calls the split_points function to split the points into two
 *    complete and consistent sets of points that are suitable to create two 
 *    poly-lines.
 *
 * 3. Else nothing needs to be done since the points are already suitable for
 *    a polyline.
 *
 * The function will re-initialise points1 and points2 according to its needs. The
 * total number of points will always be 360, even with the addition of the two
 * extra points. 
 */
static guint
calculate_footprint (GtkSatMap *satmap, sat_t *sat)
{
    unsigned int azi;
    float sx, sy, msx, msy, ssx, ssy;
    double ssplat, ssplon, beta, azimuth, num, dem;
    double rangelon, rangelat, mlon;
    bool warped = FALSE;
    unsigned int numrc = 1;

    /* Range circle calculations.
     * Borrowed from gsat 0.9.0 by Xavier Crehueras, EB3CZS
     * who borrowed from John Magliacane, KD2BD.
     * Optimized by Alexandru Csete and William J Beksi.
     */
    ssplat = sat->ssplat * de2ra;
    ssplon = sat->ssplon * de2ra;
    beta = (0.5 * sat->footprint) / xkmper;

    QVector<QPoint> points;
    for (azi = 0; azi < 180; azi++)    {
        azimuth = de2ra * (double)azi;
        rangelat = asin (sin (ssplat) * cos (beta) + cos (azimuth) *
                         sin (beta) * cos (ssplat));
        num = cos (beta) - (sin (ssplat) * sin (rangelat));
        dem = cos (ssplat) * cos (rangelat);
            
        if (azi == 0 && north_pole_is_covered(sat))
            rangelon = ssplon + pi;
            
        else if (azi == 180 && south_pole_is_covered(sat))
            rangelon = ssplon + pi;
                
        else if (fabs (num / dem) > 1.0)
            rangelon = ssplon;
                
        else {
            if ((180.0 - azi) >= 0)
                rangelon = ssplon - arccos (num, dem);
            else
                rangelon = ssplon + arccos (num, dem);
        }
                
        while (rangelon < -pi)
            rangelon += twopi;
        
        while (rangelon > (pi))
            rangelon -= twopi;
                
        rangelat = rangelat / de2ra;
        rangelon = rangelon / de2ra;

        /* mirror longitude */
        if (mirror_lon (sat, rangelon, &mlon, satmap->left_side_lon))
            warped = TRUE;

        lonlat_to_xy (satmap, rangelon, rangelat, &sx, &sy);
        lonlat_to_xy (satmap, mlon, rangelat, &msx, &msy);

        points1->coords[2*azi] = sx;
        points1->coords[2*azi+1] = sy;
    
        /* Add mirrored point */
        points1->coords[718-2*azi] = msx;
        points1->coords[719-2*azi] = msy;
    }

    /* points1 now contains 360 pairs of map-based XY coordinates.
       Check whether actions 1, 2 or 3 have to be performed.
    */

    /* pole is covered => sort points1 and add additional points */
    if (pole_is_covered (sat)) {

        sort_points_x (satmap, sat, points1, 360);
        numrc = 1;
    } 

    /* pole not covered but range circle has been warped
       => split points */
    else if (warped == TRUE) {

        lonlat_to_xy (satmap, sat->ssplon, sat->ssplat, &ssx, &ssy);
        split_points (satmap, sat, ssx);
        numrc = 2;

    }

    /* the nominal condition => points1 is adequate */
    else {

        numrc = 1;

    }

    return numrc;
}