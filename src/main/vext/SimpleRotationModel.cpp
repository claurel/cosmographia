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

#include "SimpleRotationModel.h"

using namespace vesta;
using namespace Eigen;


/** SimpleRotationModel represents a constant rotation rate about a fixed
  * axis.
  *
  * \param inclination angle in radians between rotation axis and local +z direction
  * \param ascendingNode
  * \param rotationRate rate of rotation in radians per second
  * \param meridianAngleAtEpoch angle of the prime meridian at the epoch date
  * \param epoch epoch date as seconds from J2000
  */
SimpleRotationModel::SimpleRotationModel(double inclination,
                                         double ascendingNode,
                                         double rotationRate,
                                         double meridianAngleAtEpoch,
                                         double epoch) :
    m_rotationRate(rotationRate),
    m_meridianAngleAtEpoch(meridianAngleAtEpoch),
    m_epoch(epoch),
    m_rotation(AngleAxisd(ascendingNode, Vector3d::UnitZ()) * AngleAxisd(inclination, Vector3d::UnitX()))
{
}


Quaterniond
SimpleRotationModel::orientation(double t) const
{
    double meridianAngle = m_meridianAngleAtEpoch + (t - m_epoch) * m_rotationRate;
    return m_rotation * Quaterniond(AngleAxisd(meridianAngle, Vector3d::UnitZ()));
}


Vector3d
SimpleRotationModel::angularVelocity(double /* t */) const
{
    return m_rotation * (Vector3d::UnitZ() * m_rotationRate);
}
