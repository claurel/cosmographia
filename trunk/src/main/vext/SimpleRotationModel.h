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

#include <vesta/RotationModel.h>


class SimpleRotationModel : public vesta::RotationModel
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    SimpleRotationModel(double inclination,
                        double ascendingNode,
                        double rotationRate,
                        double meridianAngleAtEpoch,
                        double epoch);
    Eigen::Quaterniond orientation(double t) const;
    Eigen::Vector3d angularVelocity(double t) const;

private:
    double m_rotationRate;
    double m_meridianAngleAtEpoch;
    double m_epoch;
    Eigen::Quaterniond m_rotation;
};
