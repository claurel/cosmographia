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

#ifndef _ASTRO_ROTATION_H_
#define _ASTRO_ROTATION_H_

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>


/** Return a unit quaternion representing a rotation of theta radians
  * about the x-axis.
  */
template<typename T>
Eigen::Quaternion<T> xRotation(T theta)
{
    return Eigen::Quaternion<T>(std::cos(theta * 0.5), std::sin(theta * 0.5), 0.0, 0.0);
}


/** Return a unit quaternion representing a rotation of theta radians
  * about the y-axis.
  */
template<typename T>
Eigen::Quaternion<T> yRotation(T theta)
{
    return Eigen::Quaternion<T>(std::cos(theta * 0.5), 0.0, std::sin(theta * 0.5), 0.0);
}


/** Return a unit quaternion representing a rotation of theta radians
  * about the z-axis.
  */
template<typename T>
Eigen::Quaternion<T> zRotation(T theta)
{
    return Eigen::Quaternion<T>(std::cos(theta * 0.5), 0.0, 0.0, std::sin(theta * 0.5));
}


/** Compute the quaternion derivative of a rotation about a fixed
 *  axis.
 *
 *  The derivative of a quaternion function q(t) (where t is a scalar) is
 *  given by:
 *    1/2 w(t) q(t)
 *
 *  Where w(t) given by a * v(t), with a the scalar angular velocity and
 *  v(t) a unit direction vector.
 *
 *  Note that the quaternion returned by this function is not necessarily
 *  a unit quaternion, and thus doesn't represent a rotation in 3D.
 *
 *  \param axis the axis of rotation
 *  \param theta the rotation angle
 *  \param dtheta derivative of the rotation angle
 */
template<typename T>
Eigen::Quaternion<T> qDerivative(const Eigen::Vector3d& axis,
                                 T theta,
                                 T dtheta)
{
    // Instead of actually multiplying the w(t) and q(t), we take advantage
    // of the fact that the imaginary parts of w(t) and q(t) are aligned,
    // canceling out many terms of the quaterion product.
    Eigen::Vector3d w = axis * std::cos(theta / 2);
    Eigen::Quaternion<T> q(-std::sin(theta / 2), w.x(), w.y(), w.z());
    q.coeffs() *= dtheta / 2;

    return q;
}

#endif // _ASTRO_ROTATION_H_
