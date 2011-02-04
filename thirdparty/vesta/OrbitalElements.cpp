/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "OrbitalElements.h"

using namespace vesta;
using namespace Eigen;


/** Create a new set of orbital elements with all values initialized to zero.
  */
OrbitalElements::OrbitalElements() :
    periapsisDistance(0.0),
    eccentricity(0.0),
    inclination(0.0),
    longitudeOfAscendingNode(0.0),
    argumentOfPeriapsis(0.0),
    meanAnomalyAtEpoch(0.0),
    meanMotion(0.0),
    epoch(0.0)
{
}


double
OrbitalElements::eccentricAnomaly(double ecc, double M)
{
    // TODO: Add methods to handle parabolic and hyperbolic orbits.
    // TODO: Need some error analysis here
    if (ecc < 0.3)
        return eccentricAnomaly_Standard(ecc, M, 5);
    else
        return eccentricAnomaly_LaguerreConway(ecc, M, 8);
}


double
OrbitalElements::eccentricAnomaly_Standard(double ecc,
                                           double M,
                                           unsigned int maxIterations)
{
    double E = M;
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        E = M + ecc * sin(E);
    }

    return E;
}


double
OrbitalElements::eccentricAnomaly_LaguerreConway(double ecc,
                                                 double M,
                                                 unsigned int maxIterations)
{
    // initial guess
    double E = M + 0.85 * ecc * (sin(M) < 0 ? -1.0 : 1.0);

    for (unsigned int i = 0; i < maxIterations; i++)
    {
        double s = ecc * sin(E);
        double c = ecc * cos(E);
        double z = E - s - M;
        double z1 = 1 - c;
        double signz1 = z1 < 0 ? -1 : 1;
        double z2 = s;
        E += -5 * z / (z1 + signz1 * sqrt(std::abs(16 * z1 * z1 - 20 * z * z2)));
    }

    return E;
}


Quaterniond
OrbitalElements::orbitOrientation(double inclination,
                                  double longitudeOfAscendingNode,
                                  double argumentOfPeriapsis)
{
    return (Quaterniond(AngleAxis<double>(longitudeOfAscendingNode, Vector3d::UnitZ())) *
            Quaterniond(AngleAxis<double>(inclination, Vector3d::UnitX())) *
            Quaterniond(AngleAxis<double>(argumentOfPeriapsis, Vector3d::UnitZ())));
}



