/*
 * $Revision: 517 $ $Date: 2010-10-02 11:17:17 -0700 (Sat, 02 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "KeplerianTrajectory.h"
#include "Units.h"

using namespace vesta;
using namespace Eigen;


KeplerianTrajectory::KeplerianTrajectory(const OrbitalElements& elements) :
    m_elements(elements),
    m_orbitOrientation(OrbitalElements::orbitOrientation(elements.inclination,
                                                         elements.longitudeOfAscendingNode,
                                                         elements.argumentOfPeriapsis))
{
}


StateVector
KeplerianTrajectory::state(double t) const
{
    double ecc = m_elements.eccentricity;
    double meanAnomaly = m_elements.meanAnomalyAtEpoch + m_elements.meanMotion * (t - m_elements.epoch);
    double E = OrbitalElements::eccentricAnomaly(ecc, meanAnomaly);
    double sinE = sin(E);
    double cosE = cos(E);
    double w = sqrt(1.0 - ecc * ecc);

    // TODO: Add parabolic and hyperbolic cases; the below is valid only for ecc < 1.0
    double semiMajorAxis = m_elements.periapsisDistance / (1.0 - ecc);
    Vector3d position(semiMajorAxis * (cosE - ecc),
                      semiMajorAxis * w * sinE,
                      0.0);

    double edot = m_elements.meanMotion / (1 - ecc * cosE);
    Vector3d velocity(-semiMajorAxis * sinE * edot,
                       semiMajorAxis * w * cosE * edot,
                       0.0);

    return StateVector(m_orbitOrientation * position, m_orbitOrientation * velocity);
}


double
KeplerianTrajectory::boundingSphereRadius() const
{
    // TODO: Need some strategy for hyperbolic orbits (such as time bounds)
    double semiMajorAxis = m_elements.periapsisDistance / (1.0 - m_elements.eccentricity);
    return semiMajorAxis;
}


double
KeplerianTrajectory::period() const
{
    if (m_elements.eccentricity >= 1.0)
    {
        // Hyperbolic and parabolic orbits are not periodic
        return 0.0;
    }
    else
    {
        return 2.0 * PI / m_elements.meanMotion;
    }
}

