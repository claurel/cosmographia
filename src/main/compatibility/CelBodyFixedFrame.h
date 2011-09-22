// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
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
