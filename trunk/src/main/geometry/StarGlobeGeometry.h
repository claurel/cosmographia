// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _STAR_GLOBE_GEOMETRY_H_
#define _STAR_GLOBE_GEOMETRY_H_

#include <vesta/Geometry.h>


class StarGlobeGeometry : public vesta::Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    StarGlobeGeometry();
    virtual ~StarGlobeGeometry();

    void render(vesta::RenderContext& rc, double clock);

    virtual float boundingSphereRadius() const;

    void setEllipsoidAxes(const Eigen::Vector3f& axes)
    {
        m_ellipsoidAxes = axes;
    }

    Eigen::Vector3f ellipsoidAxes() const
    {
        return m_ellipsoidAxes;
    }

    virtual bool isEllipsoidal() const
    {
        return true;
    }

    virtual vesta::AlignedEllipsoid ellipsoid() const
    {
        return vesta::AlignedEllipsoid(m_ellipsoidAxes.cast<double>() * 0.5);
    }

private:
    Eigen::Vector3f m_ellipsoidAxes;
};

#endif // _STAR_GLOBE_GEOMETRY_H_
