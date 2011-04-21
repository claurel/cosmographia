// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "RotationUtility.h"

using namespace Eigen;


/** Calculate the rotation required to make an observer at
  * the position 'from' point directly at the position 'to', with
  * the constraint that the local y-axis points in the direction of
  * the specified up vector.
  *
  * Note: the vector between 'from' and 'to' must not be exactly aligned
  * with the up vector, otherwise the orientation is undefined.
  */
Quaterniond LookRotation(const Eigen::Vector3d& from,
                         const Eigen::Vector3d& to,
                         const Eigen::Vector3d& up)
{
    Vector3d lookDir = to - from;
    if (lookDir.isZero())
    {
        return Quaterniond::Identity();
    }

    Vector3d zAxis = -lookDir.normalized();

    // x-axis normal to both the z-axis and the up vector
    Vector3d xAxis = up.cross(zAxis);
    if (xAxis.isZero())
    {
        // Up vector is parallel to the look direction; choose instead an
        // arbitrary vector orthogonal to the look direction.
        xAxis = zAxis.cross(zAxis.unitOrthogonal());
    }

    xAxis.normalize();
    Vector3d yAxis = zAxis.cross(xAxis);

    Matrix3d m;
    m << xAxis, yAxis, zAxis;

    return Quaterniond(m);

}


