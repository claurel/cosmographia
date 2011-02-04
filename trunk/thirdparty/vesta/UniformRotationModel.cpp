/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "UniformRotationModel.h"

using namespace vesta;
using namespace Eigen;


/** UniformRotationModel represents a rotation of a constant rate about a
  * fixed axis.
  *
  * @param axis a non-zero vector giving the axis of rotation
  * @param rotationRate the constant rate of rotation in radians per second
  * @param meridianAngleAtEpoch the angle in radians of the meridian at the epoch date
  * @param epoch the epoch date in seconds elapsed since J2000.0
  */
UniformRotationModel::UniformRotationModel(const Eigen::Vector3d& axis,
                                           double rotationRate,
                                           double meridianAngleAtEpoch,
                                           double epoch) :
    m_axis(axis.normalized()),
    m_rotationRate(rotationRate),
    m_meridianAngleAtEpoch(meridianAngleAtEpoch),
    m_epoch(epoch)
{
}


Quaterniond
UniformRotationModel::orientation(double t) const
{
    double meridianAngle = m_meridianAngleAtEpoch + (t - m_epoch) * m_rotationRate;
    return Quaterniond(AngleAxis<double>(meridianAngle, m_axis));
}


Vector3d
UniformRotationModel::angularVelocity(double /* t */) const
{
    return m_axis * m_rotationRate;
}

