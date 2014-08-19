// SpiceRotationModel.h
//
// Copyright (C) 2013 Chris Laurel <claurel@gmail.com>
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

#ifndef _SPICE_ROTATION_MODEL_H_
#define _SPICE_ROTATION_MODEL_H_

#include <vesta/RotationModel.h>

class SpiceRotationModel : public vesta::RotationModel
{
public:
    SpiceRotationModel(const char* fromFrame, const char* toFrame);
    ~SpiceRotationModel();

    virtual Eigen::Quaterniond orientation(double tdbSec) const;
    virtual Eigen::Vector3d angularVelocity(double tdbSec) const;

private:
    std::string m_fromFrame;
    std::string m_toFrame;
};

#endif // _SPICE_ROTATION_MODEL_H_
