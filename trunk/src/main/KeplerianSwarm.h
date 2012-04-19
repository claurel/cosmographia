// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#ifndef _VESTA_KEPLERIAN_SWARM_H_
#define _VESTA_KEPLERIAN_SWARM_H_

#include <vesta/Geometry.h>
#include <vesta/Spectrum.h>
#include <vesta/OrbitalElements.h>
#include <Eigen/Core>
#include <vector>


namespace vesta
{

class GLShaderProgram;
class VertexBuffer;
class VertexSpec;

class KeplerianSwarm : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    KeplerianSwarm();
    ~KeplerianSwarm();

    virtual void render(vesta::RenderContext& rc, double clock) const;
    virtual float boundingSphereRadius() const;
    virtual bool isOpaque() const;
    virtual float nearPlaneDistance(const Eigen::Vector3f& cameraPosition) const;

    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    float opacity() const
    {
        return m_opacity;
    }

    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    double epoch() const
    {
        return m_epoch;
    }

    void setEpoch(double epoch)
    {
        m_epoch = epoch;
    }

    float pointSize() const
    {
        return m_pointSize;
    }

    void setPointSize(float pointSize)
    {
        m_pointSize = pointSize;
    }

    /** Get projected size (in pixels) of the swarm where it becomes
     *  completely invisible. Fading is disabled when fadeSize is zero.
     *
     *  \see setFadeSize
     */
    float fadeSize() const
    {
        return m_fadeSize;
    }
    
    /** Set projected size (in pixels) of the swarm where it becomes
     *  completely invisible. Setting the fade size to zero disables fading.
     *  Fading is useful to prevent the swarm from appearing to bright and
     *  dense when zoomed out.
     *
     *  \see setFadeSize
     */
    void setFadeSize(float fadeSize)
    {
        m_fadeSize = fadeSize;
    }
    
    void addObject(const OrbitalElements& elements, double discoveryTime);
    void clear();

private:
    struct KeplerianObject
    {
        float sma;
        float ecc;
        float meanAnomaly;
        float meanMotion;
        float qw;
        float qx;
        float qy;
        float qz;
        float discoveryDate;
    };

    VertexSpec* m_vertexSpec;
    std::vector<KeplerianObject> m_objects;

    double m_epoch;
    float m_boundingRadius;
    Spectrum m_color;
    float m_opacity;
    float m_pointSize;
    float m_fadeSize;

    // These are only mutable because render() is const; need to
    // change this.
    mutable counted_ptr<GLShaderProgram> m_swarmShader;
    mutable bool m_shaderCompiled;
    mutable counted_ptr<VertexBuffer> m_vertexBuffer;
};

}

#endif // _VESTA_KEPLERIAN_SWARM_H_

