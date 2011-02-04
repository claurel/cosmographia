/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_UNITS_H_
#define _VESTA_UNITS_H_


namespace vesta
{
    const double PI = 3.14159265358979323;

    // 12:00:00 Jan 1 2000
    const double J2000 = 2451545.0;

    // Angle between J2000 mean equator and the ecliptic plane.
    const double J2000_OBLIQUITY = 23.4392911;


    /** Convert degrees to radians.
      */
    inline double toRadians(double degrees)
    {
        return degrees * (PI / 180.0);
    }

    /** Convert radians to degrees.
      */
    inline double toDegrees(double radians)
    {
        return radians * (180.0 / PI);
    }

    /** Convert an angle given in arcseconds to radians.
      */
    inline double arcsecToRadians(double arcsec)
    {
        return arcsec * (PI / (180.0 * 3600.0));
    }

    /** Convert an angle in radians to arcseconds.
      */
    inline double radiansToArcsec(double radians)
    {
        return radians * (3600.0 * 180.0 / PI);
    }

    /** Convert a time in seconds to Julian days.  A Julian
      * day is defined as exactly 86400 SI seconds.
      */
    inline double secondsToDays(double seconds)
    {
        return seconds * (1.0 / 86400.0);
    }

    /** Convert a duration in Julian days to seconds. A Julian
      * day is defined as exactly 86400 SI seconds.
      */
    inline double daysToSeconds(double days)
    {
        return days * 86400.0;
    }


}

#endif // _VESTA_UNITS_H_
