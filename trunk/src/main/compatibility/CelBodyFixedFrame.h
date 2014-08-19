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

#ifndef _VESTA_CEL_BODY_FIXED_FRAME_H_
#define _VESTA_CEL_BODY_FIXED_FRAME_H_

#include <vesta/Frame.h>
#include <vesta/Entity.h>


class CelBodyFixedFrame : public vesta::Frame
{
public:
    CelBodyFixedFrame(vesta::Entity* body);
    virtual ~CelBodyFixedFrame();

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;

    vesta::Entity* body() const;

private:
    vesta::counted_ptr<vesta::Entity> m_body;
};

#endif // _VESTA_CEL_BODY_FIXED_FRAME_H_
