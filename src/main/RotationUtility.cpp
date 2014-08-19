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


