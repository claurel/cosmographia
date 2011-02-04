/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ORBITAL_ELEMENTS_H_
#define _VESTA_ORBITAL_ELEMENTS_H_

#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

class OrbitalElements
{
public:
    OrbitalElements();

    double periapsisDistance;         // Distance in kilometers
    double eccentricity;
    double inclination;               // Angle in radians
    double longitudeOfAscendingNode;  // Angle in radians
    double argumentOfPeriapsis;       // Angle in radians
    double meanAnomalyAtEpoch;        // Angle in radians
    double meanMotion;                // Radians per second
    double epoch;                     // Time in seconds past J2000

    /** Calculate the eccentric anomaly using the optimal technique
      * for the eccentricity.
      * @param ecc the orbital eccentricity
      * @param M the mean anomaly in radians
      */
    static double eccentricAnomaly(double ecc, double M);

    /** Calculate the eccentric anomaly using the standard technique of
      * iterating E = M + e sin(E)
      * @param ecc the orbital eccentricity
      * @param M the mean anomaly in radians
      * @param maxIterations
      */
    static double eccentricAnomaly_Standard(double ecc, double M, unsigned int maxInterations);

    /** Calculate the eccentric anomaly using the Laguerre-Conway method.
      * Converges much faster than the standard method when the eccentricity
      * is high.
      * @param ecc the orbital eccentricity
      * @param M the mean anomaly in radians
      * @param maxIterations the maximum number of iterations
      */
    static double eccentricAnomaly_LaguerreConway(double ecc, double M, unsigned int maxIterations);

    /** Compute the orientation of the orbit with respect to the base
      * coordinate system.
      * @param inclination the orbital inclination in radians
      * @param longitudeOfAscendingNode an angle in radians
      * @param argumentOfPeriapsis an angle in radians
      */
    static Eigen::Quaterniond orbitOrientation(double inclination,
                                               double longitudeOfAscendingNode,
                                               double argumentOfPeriapsis);
};

}

#endif // _VESTA_ORBITAL_ELEMENTS_H_
