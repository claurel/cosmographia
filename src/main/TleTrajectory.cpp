// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "TleTrajectory.h"
#include "astro/OsculatingElements.h"
#include <vesta/Units.h>
#include <vesta/GregorianDate.h>
#include <vesta/Debug.h>

using namespace vesta;
using namespace Eigen;

static const double EARTH_GM = 398600.4418;



TleTrajectory::TleTrajectory(tle_t* tle) :
    m_tle(tle),
    m_keplerianApproxLimit(daysToSeconds(3652500))
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

    // Switch to a Keplerian approximation outside of a year from the epoch
    setKeplerianApproximationLimit(daysToSeconds(365));
}


TleTrajectory::~TleTrajectory()
{
    delete m_tle;
}


StateVector
TleTrajectory::state(double tsec) const
{
    if (tsec < m_epoch - m_keplerianApproxLimit)
    {
        return ElementsToStateVector(m_keplerianBefore, tsec);
    }
    else if (tsec > m_epoch + m_keplerianApproxLimit)
    {
        return ElementsToStateVector(m_keplerianAfter, tsec);
    }
    else
    {
        return tleState(tsec);
    }
}


StateVector
TleTrajectory::tleState(double tsec) const
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
    double sma = pow((muEarth * pow(period(), 2.0)) / (4.0 * PI * PI), 1.0 / 3.0);

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


/** Copy the contents of another TLE trajectory.
  */
void
TleTrajectory::copy(TleTrajectory* other)
{
    if (other)
    {
        *m_tle = *other->m_tle;
        m_epoch = other->m_epoch;
        m_keplerianApproxLimit = other->m_keplerianApproxLimit;
        m_keplerianBefore = other->m_keplerianBefore;
        m_keplerianAfter = other->m_keplerianAfter;
        m_ephemerisType = other->m_ephemerisType;
        for (unsigned int i = 0; i < sizeof(m_satParams) / sizeof(m_satParams[0]); ++i)
        {
            m_satParams[i] = other->m_satParams[i];
        }
    }
}


/** Set the time from the TLE epoch at which a pure Keplerian
  * approximation will be used instead of SGP4. This is useful when we
  * have just a single TLE set for a long-lived object. Using SGP4 for a
  * time span of a decade will often give a trajectory that intersects
  * the Earth. Using a Keplerian trajectory instead doesn't give an accurate
  * position but the approximate semimajoraxis, inclination, and eccentricity
  * of the orbit will be preserved.
  */
void
TleTrajectory::setKeplerianApproximationLimit(double tsec)
{
    StateVector s0 = tleState(m_epoch - tsec);
    StateVector s1 = tleState(m_epoch + tsec);
    m_keplerianBefore = CalculateOsculatingElements(s0, EARTH_GM, m_epoch - tsec);
    m_keplerianAfter  = CalculateOsculatingElements(s1, EARTH_GM, m_epoch + tsec);

    /*
    // Testing code
    s0 = tleState(m_epoch - tsec);
    s1 = tleState(m_epoch + tsec);
    StateVector t0 = ElementsToStateVector(m_keplerianBefore, m_epoch - tsec);
    StateVector t1 = ElementsToStateVector(m_keplerianAfter, m_epoch + tsec);
    std::cerr << "before rdiff: " << (s0.position() - t0.position()).norm() << ", vdiff: " << (s0.velocity() - t0.velocity()).norm() << std::endl;
    std::cerr << "after rdiff: " << (s1.position() - t1.position()).norm() << ", vdiff: " << (s1.velocity() - t1.velocity()).norm() << std::endl;
    */

    m_keplerianApproxLimit = tsec;
}

