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

#ifndef _VEXT_ARC_STRIP_PARTICLE_GENERATOR_H_
#define _VEXT_ARC_STRIP_PARTICLE_GENERATOR_H_

#include <vesta/particlesys/InitialStateGenerator.h>
#include <vector>


/** The ArcStrip generator creates particles with initial positions that
  * lie on a set of arcs. If the points are all the same distance from the origin,
  * these arcs are great circles on a sphere. The initial velocities of the
  * of the particles are in the direction opposite the origin.
  */
class ArcStripParticleGenerator : public vesta::InitialStateGenerator
{
public:
    ArcStripParticleGenerator(const std::vector<Eigen::Vector3f>& positions, const std::vector<float>& speeds) :
        m_speeds(speeds),
        m_arcCount(0),
        m_boundingRadius(0.0f),
        m_maxSpeed(0.0f)
    {
        if (positions.empty() || speeds.empty())
        {
            m_arcCount = 0;
        }
        else
        {
            m_arcCount = std::min(positions.size(), speeds.size()) - 1;
        }

        // Build arrays of normalized positions
        for (unsigned int i = 0; i <= positions.size(); ++i)
        {
            m_normPositions.push_back(positions[i].normalized());
            m_radii.push_back(positions[i].norm());
        }

        // Compute bounds
        for (unsigned int i = 0; i <= m_arcCount; ++i)
        {
            m_boundingRadius = std::max(m_boundingRadius, m_radii[i]);
            m_maxSpeed = std::max(m_maxSpeed, m_speeds[i]);
        }
    }

    virtual void generateParticle(vesta::PseudorandomGenerator& gen,
                                  Eigen::Vector3f& position, Eigen::Vector3f& velocity) const
    {
        if (m_arcCount > 0)
        {
            unsigned int arcIndex = gen.randUint() % m_arcCount;
            float alpha = gen.randFloat();
            Eigen::Vector3f n  = (1.0f - alpha) * m_normPositions[arcIndex] + alpha * m_normPositions[arcIndex + 1];
            float r     = (1.0f - alpha) * m_radii[arcIndex] + alpha * m_radii[arcIndex + 1];
            float speed = (1.0f - alpha) * m_speeds[arcIndex] + alpha * m_speeds[arcIndex + 1];
            n.normalize();

            position = n * r;
            velocity = n * speed;
        }
        else
        {
            position = velocity = Eigen::Vector3f::Zero();
        }
    }

    virtual float maxDistanceFromOrigin() const
    {
        return m_boundingRadius;
    }

    virtual float maxSpeed() const
    {
        return m_maxSpeed;
    }

private:
    std::vector<Eigen::Vector3f> m_normPositions;
    std::vector<float> m_radii;
    std::vector<float> m_speeds;
    unsigned int m_arcCount;
    float m_boundingRadius;
    float m_maxSpeed;
};

#endif // _VEXT_ARC_STRIP_PARTICLE_GENERATOR_H_
