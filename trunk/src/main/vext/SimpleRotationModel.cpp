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
