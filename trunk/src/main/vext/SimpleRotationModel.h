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

#ifndef _VEXT_SIMPLE_ROTATION_MODEL_H_
#define _VEXT_SIMPLE_ROTATION_MODEL_H_

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

#endif // _VEXT_SIMPLE_ROTATION_MODEL_H_

