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

namespace vesta
{
    class WorldGeometry;
    class GLShaderProgram;
}


class StarGlobeGeometry : public vesta::Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    StarGlobeGeometry();
    virtual ~StarGlobeGeometry();

    void render(vesta::RenderContext& rc, double clock) const;

    virtual float boundingSphereRadius() const;

    void setEllipsoidAxes(const Eigen::Vector3f& axes);
    Eigen::Vector3f ellipsoidAxes() const;

    virtual bool isEllipsoidal() const
    {
        return true;
    }

    virtual vesta::AlignedEllipsoid ellipsoid() const;

private:
    Eigen::Vector3f m_ellipsoidAxes;
    vesta::counted_ptr<vesta::WorldGeometry> m_globe;

    // These are only mutable because render() is const; need to
    // change this.
    static vesta::counted_ptr<vesta::GLShaderProgram> ms_starShader;
    static bool ms_shaderCompiled;
};

#endif // _STAR_GLOBE_GEOMETRY_H_
