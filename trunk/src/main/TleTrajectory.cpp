// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#include "TleTrajectory.h"
#include <vesta/Units.h>
#include <vesta/GregorianDate.h>
#include <vesta/Debug.h>

using namespace vesta;
using namespace Eigen;


TleTrajectory::TleTrajectory(tle_t* tle) :
    m_tle(tle)
{
    // Select the ephemeris type. At the moment, we don't use
    // SGP8 or SDP8
    if (select_ephemeris(m_tle) != 0)
    {
        m_ephemerisType = TLE_EPHEMERIS_TYPE_SDP4;
    }
    else
    {
        m_ephemerisType = TLE_EPHEMERIS_TYPE_SGP4;
    }

    switch (m_ephemerisType)
    {
    case TLE_EPHEMERIS_TYPE_SGP:
       SGP_init(m_satParams, m_tle);
       break;
    case TLE_EPHEMERIS_TYPE_SGP4:
       SGP4_init(m_satParams, m_tle);
       break;
    case TLE_EPHEMERIS_TYPE_SDP4:
       SDP4_init(m_satParams, m_tle);
       break;
    case TLE_EPHEMERIS_TYPE_SGP8:
       SGP8_init(m_satParams, m_tle);
       break;
    case TLE_EPHEMERIS_TYPE_SDP8:
       SDP8_init(m_satParams, m_tle);
       break;
    }

    // Epoch in TLE is a year and day number converted to a UTC Julian day number. The problem
    // is that UTC isn't a uniform time scale because of leap seconds. We'll convert the epoch
    // back to a calendar date and convert that to a TDB Julian day number.

    // The epoch isn't really a TDB date, but we need to recover the calendar date without
    // any leap second correction.
    GregorianDate calendarDate = GregorianDate::TDBDateFromTDBJD(m_tle->epoch);
    calendarDate.setTimeScale(TimeScale_UTC);

    m_epoch = calendarDate.toTDBSec();
}


TleTrajectory::~TleTrajectory()
{
    delete m_tle;
}


StateVector
TleTrajectory::state(double tsec) const
{
    // Convert time to minutes past epoch
    double tmin = (tsec - m_epoch) / 60.0;

    Vector3d position;
    Vector3d velocity;
    switch (m_ephemerisType)
    {
    case TLE_EPHEMERIS_TYPE_SGP:
       SGP(tmin, m_tle, m_satParams, position.data(), velocity.data());
       break;
    case TLE_EPHEMERIS_TYPE_SGP4:
       SGP4(tmin, m_tle, m_satParams, position.data(), velocity.data());
       break;
    case TLE_EPHEMERIS_TYPE_SDP4:
       SDP4(tmin, m_tle, m_satParams, position.data(), velocity.data());
       break;
    case TLE_EPHEMERIS_TYPE_SGP8:
       SGP8(tmin, m_tle, m_satParams, position.data(), velocity.data());
       break;
    case TLE_EPHEMERIS_TYPE_SDP8:
       SDP8(tmin, m_tle, m_satParams, position.data(), velocity.data());
       break;
    }

    // Velocity must be converted from km/min to km/sec
    return StateVector(position, velocity / 60.0);
}


double
TleTrajectory::boundingSphereRadius() const
{
    // Standard gravitational parameter for Earth
    const double muEarth = 3.986004418e5;

    // Derive the semimajor axis from the mean motion
    double period = daysToSeconds(1.0 / m_tle->xno);
    double sma = pow((muEarth * period * period) / (4.0 * PI * PI), 1.0 / 3.0);

    // Compute the bounding radius from the semimajor axis and
    // eccentricity, allowing a generous 10% slack since the orbital elements
    // evolve slightly over time.
    return sma * (1.0 + m_tle->eo) * 1.1;
}


bool
TleTrajectory::isPeriodic() const
{
    return true;
}


double
TleTrajectory::period() const
{
    // Mean motion is stored as radians per minute; convert to seconds
    return (1.0 / m_tle->xno) * 2.0 * PI * 60.0;
}


TleTrajectory*
TleTrajectory::Create(const std::string& line1, const std::string& line2)
{
    tle_t* tle = new tle_t;
    int tleError = parse_elements(line1.c_str(), line2.c_str(), tle);

    if (tleError != 0)
    {
        if (tleError == 3)
        {
            VESTA_LOG("TLE parse error.");
        }
        else
        {
            VESTA_LOG("TLE checksum error.");
        }

        delete tle;
        return NULL;
    }

    return new TleTrajectory(tle);
}
