// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "IAULunarRotationModel.h"
#include <vesta/Units.h>

using namespace vesta;
using namespace Eigen;
using namespace std;


static void
calcEulerAngles(double t, double* phi, double* theta, double* psi)
{
    double d = secondsToDays(t); // time in Julian days
    double T = d / 36525.0; // time in Julian centuries

    double E1  = toRadians(125.045 -  0.0529921 * d);
    double E2  = toRadians(250.089 -  0.1059842 * d);
    double E3  = toRadians(260.008 + 13.012009  * d);
    double E4  = toRadians(176.625 + 13.3407154 * d);
    double E5  = toRadians(357.529 +  0.9856993 * d);
    double E6  = toRadians(311.589 + 26.4057084 * d);
    double E7  = toRadians(134.963 + 13.0649930 * d);
    double E8  = toRadians(276.617 +  0.3287146 * d);
    double E9  = toRadians( 34.226 +  1.7484877 * d);
    double E10 = toRadians( 15.134 -  0.1589763 * d);
    double E11 = toRadians(119.743 +  0.0036096 * d);
    double E12 = toRadians(239.961 +  0.1643573 * d);
    double E13 = toRadians( 25.053 + 12.9590088 * d);

    double a0 = 269.9949
                + 0.0013*T
                - 3.8787 * sin(E1)
                - 0.1204 * sin(E2)
                + 0.0700 * sin(E3)
                - 0.0172 * sin(E4)
                + 0.0072 * sin(E6)
                - 0.0052 * sin(E10)
                + 0.0043 * sin(E13);

    double d0 = 66.5392
                + 0.0130 * T
                + 1.5419 * cos(E1)
                + 0.0239 * cos(E2)
                - 0.0278 * cos(E3)
                + 0.0068 * cos(E4)
                - 0.0029 * cos(E6)
                + 0.0009 * cos(E7)
                + 0.0008 * cos(E10)
                - 0.0009 * cos(E13);

    double W =    38.3213
                + 13.17635815 * d
                - 1.4e-12 * d * d
                + 3.5610 * sin(E1)
                + 0.1208 * sin(E2)
                - 0.0642 * sin(E3)
                + 0.0158 * sin(E4)
                + 0.0252 * sin(E5)
                - 0.0066 * sin(E6)
                - 0.0047 * sin(E7)
                - 0.0046 * sin(E8)
                + 0.0028 * sin(E9)
                + 0.0052 * sin(E10)
                + 0.0040 * sin(E11)
                + 0.0019 * sin(E12)
                - 0.0044 * sin(E13);

    *phi = a0;
    *theta = 90.0 - d0;
    *psi = 90.0 + W;
}


static void
calcEulerAngleDerivatives(double /* t */, double* dphi, double* dtheta, double* dpsi)
{
    double secPerDay = daysToSeconds(1.0);
    double secPerCentury = secPerDay * 36525.0;

    // These derivatives are only approximate; only the linear terms are accounted for
    *dphi = 0.0013 / secPerCentury;
    *dtheta = -0.0130 / secPerCentury;
    *dpsi = 13.17635815 / secPerDay;
}


Quaterniond
IAULunarRotationModel::orientation(double t) const
{
    double phi = 0.0;
    double theta = 0.0;
    double psi = 0.0;

    calcEulerAngles(t, &phi, &theta, &psi);
    return Quaterniond(AngleAxisd(toRadians(phi),   Vector3d::UnitZ())) *
           Quaterniond(AngleAxisd(toRadians(theta), Vector3d::UnitX())) *
           Quaterniond(AngleAxisd(toRadians(psi),   Vector3d::UnitZ()));
}


// Eigen is meant to deal with unit quaternions only and thus doesn't overload
// the + operator.
static Quaterniond sum(const Quaterniond& q1, const Quaterniond& q2)
{
    Quaterniond s;
    s.coeffs() = q1.coeffs() + q2.coeffs();
    return s;
}


// Eigen is meant to deal with unit quaternions only and thus doesn't overload
// the * for scalar multiplication
static Quaterniond scalarMul(double s, const Quaterniond& q)
{
    Quaterniond p;
    p.coeffs() = q.coeffs() * s;
    return p;
}

Vector3d
IAULunarRotationModel::angularVelocity(double t) const
{
    double phi = 0.0;
    double theta = 0.0;
    double psi = 0.0;
    calcEulerAngles(t, &phi, &theta, &psi);

    double dphi = 0.0;
    double dtheta = 0.0;
    double dpsi = 0.0;
    calcEulerAngleDerivatives(t, &dphi, &dtheta, &dpsi);

    Quaterniond q1(AngleAxisd(toRadians(phi),   Vector3d::UnitZ()));
    Quaterniond q2(AngleAxisd(toRadians(theta), Vector3d::UnitX()));
    Quaterniond q3(AngleAxisd(toRadians(psi),   Vector3d::UnitZ()));
    Quaterniond w1(0.0, 0.0, 0.0, toRadians(dphi));
    Quaterniond w2(0.0, toRadians(dtheta), 0.0, 0.0);
    Quaterniond w3(0.0, 0.0, 0.0, toRadians(dpsi));

    // Calculate the orientation
    Quaterniond q = q1 * q2 * q3;

    // Calculate the derivative of the orientation with respect to time (uses the chain rule to
    // compute the derivative of the product q1*q2*q3.)
    Quaterniond dq = scalarMul(0.5, sum((sum(w1 * q1 * q2, w2 * q2 * q1)) * q3, w3 * q3 * q1 * q2));

    // Compute the angular momentum vector
    Vector3d w = (dq * q.conjugate()).vec() * 2.0;

    return w;
}

