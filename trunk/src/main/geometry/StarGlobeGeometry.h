// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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
