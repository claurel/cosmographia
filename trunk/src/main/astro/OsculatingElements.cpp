// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#include "OsculatingElements.h"
#include <vesta/Units.h>
#include <cmath>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** This function currently does not work correctly for:
  *   - parabolic or hyperbolic orbits
  *   - circular orbits
  *   - orbits with zero (or nearly zero) inclination
  */
OrbitalElements
CalculateOsculatingElements(const StateVector& state, double gm, double epoch)
{
    // Compute the orbital angular momentum vector (perpendicular to the orbital plane)
    Vector3d h = state.position().cross(state.velocity());

    // Compute the line of nodes; not valid when inclination is zero
    Vector3d n = Vector3d::UnitZ().cross(h);

    // Compute the eccentricity vector
    double r = state.position().norm();
    double v = state.velocity().norm();
    double rv = state.position().dot(state.velocity());
    Vector3d e = ((v * v - gm / r) * state.position() - rv * state.velocity()) / gm;

    double ecc = e.norm();
    double xi = (v * v) / 2.0 - gm / r;

    // Parabolic case is not really addressed yet
    bool isParabolic = ecc == 1.0;
    OrbitalElements el;
    double a = -gm / (2 * xi);
    if (isParabolic)
    {
        el.periapsisDistance = h.squaredNorm() / gm;
    }
    else
    {
        el.periapsisDistance = a * (1 - ecc);
    }
    el.eccentricity = ecc;
    el.inclination = acos(h.z() / h.norm());

    h.normalize();
    e.normalize();
    n.normalize();

    // The vector u defines a right handed coordinate system with h and e
    Vector3d u = h.cross(e);

    el.longitudeOfAscendingNode = atan2(h.y(), h.x()) + PI / 2.0;
    el.argumentOfPeriapsis = acos(n.dot(e));
    if (e.z() < 0.0)
    {
        el.argumentOfPeriapsis = 2.0 * PI - el.argumentOfPeriapsis;
    }

    // Compute the sine and cosine of true anomaly nu
    double cosNu = e.dot(state.position() / r);
    double sinNu = u.dot(state.position() / r);

    // Compute the eccentric anomaly E
    double sinE = sinNu * sqrt(1 - ecc * ecc) / (1 + ecc * cosNu);
    double cosE = (ecc + cosNu) / (1.0 + ecc * cosNu);
    double E = atan2(sinE, cosE);

    el.meanAnomalyAtEpoch = E - ecc * sin(E);

    el.meanMotion = sqrt(gm / (a * a * a));
    el.epoch = epoch;

    return el;
}


StateVector
ElementsToStateVector(const OrbitalElements& el, double t)
{
    double e = el.eccentricity;
    double M = el.meanAnomalyAtEpoch + el.meanMotion * (t - el.epoch);
    double E = OrbitalElements::eccentricAnomaly(e, M);
    double sinE = sin(E);
    double cosE = cos(E);
    double w = sqrt(1.0 - e * e);

    double semiMajorAxis = el.periapsisDistance / (1.0 - e);
    Vector3d r(semiMajorAxis * (cosE - e), semiMajorAxis * w * sinE, 0.0);

    double edot = el.meanMotion / (1 - e * cosE);
    Vector3d v(-semiMajorAxis * sinE * edot, semiMajorAxis * w * cosE * edot, 0.0);

    Quaterniond q(OrbitalElements::orbitOrientation(el.inclination, el.longitudeOfAscendingNode, el.argumentOfPeriapsis));
    return StateVector(q * r, q * v);
}


#if UNIT_TEST
void TestOsculatingElements()
{
    const double earthGM = 398600.4418;
    const double epoch = 0.0;

    double dt = 60.0;
    for (unsigned int i = 0; i < 10; ++i)
    {
        double t = epoch + i * dt;

        OrbitalElements elTest;
        double sma = 6748.0;
        elTest.eccentricity = 0.0010649;
        elTest.periapsisDistance = sma * (1 - elTest.eccentricity);
        elTest.inclination = toRadians(51.6379);
        elTest.longitudeOfAscendingNode = toRadians(149.2978);
        elTest.argumentOfPeriapsis = toRadians(56.2411);
        elTest.meanAnomalyAtEpoch = toRadians(208.0247 + i * 30.0);
        elTest.meanMotion = sqrt(earthGM / pow(sma, 3.0));
        elTest.epoch = t;

        StateVector s0 = el2state(elTest, t);
        OrbitalElements el = CalculateOsculatingElements(s0, earthGM, t);
        StateVector s1 = el2state(el, t);

        std::cerr << "a: " << el.periapsisDistance / (1 - el.eccentricity) <<
                     ", ecc: " << el.eccentricity <<
                     ", inc: " << toDegrees(el.inclination) <<
                     ", raan: " << toDegrees(el.longitudeOfAscendingNode) <<
                     ", peri: " << toDegrees(el.argumentOfPeriapsis) <<
                     ", M: " << toDegrees(el.meanAnomalyAtEpoch) <<
                     ", T: " << 2 * PI / el.meanMotion / 60.0 << "m" <<
                     std::endl;

        std::cerr << "rdiff: " << (s0.position() - s1.position()).norm() << ", vdiff: " << (s0.velocity() - s1.velocity()).norm() << std::endl;
    }
}
#endif
