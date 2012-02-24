/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANETOGRAPHIC_COORD_H_
#define _VESTA_PLANETOGRAPHIC_COORD_H_


namespace vesta
{

/** PlanetographicCoord3 specifies a point using latitude, longitude, and
  * height above a reference ellipsoid.
  */
class PlanetographicCoord3
{
public:
    /** Construct a new PlanetographicCoord3 given the specified latitude, longitude,
      * and height. Latitude and longitude are in radians.
      */
    PlanetographicCoord3(double latitude, double longitude, double height) :
        m_latitude(latitude),
        m_longitude(longitude),
        m_height(height)
    {
    }

    /** Get the latitude (in radians) */
    double latitude() const
    {
        return m_latitude;
    }

    /** Get the longitude (in radians) */
    double longitude() const
    {
        return m_longitude;
    }

    /** Get the height */
    double height() const
    {
        return m_height;
    }

private:
    double m_latitude;
    double m_longitude;
    double m_height;
};

}

#endif // _VESTA_PLANETOGRAPHIC_COORD_H_
