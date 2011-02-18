// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
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

#include "IAULunarRotationModel.h"
#include <vesta/Units.h>

using namespace vesta;
using namespace Eigen;
using namespace std;


Quaterniond
IAULunarRotationModel::orientation(double t) const
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

    return Quaterniond(AngleAxisd(toRadians(a0),        Vector3d::UnitZ())) *
           Quaterniond(AngleAxisd(toRadians(90.0 - d0), Vector3d::UnitX())) *
           Quaterniond(AngleAxisd(toRadians(90.0 + W),  Vector3d::UnitZ()));
}


Vector3d
IAULunarRotationModel::angularVelocity(double /* t */) const
{
    // TODO
    return Vector3d::UnitZ();
}

