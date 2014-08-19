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

#ifndef _VEXT_STRIP_PARTICLE_GENERATOR_H_
#define _VEXT_STRIP_PARTICLE_GENERATOR_H_

#include <vesta/particlesys/InitialStateGenerator.h>
#include <vector>


class StripParticleGenerator : public vesta::InitialStateGenerator
{
public:
    StripParticleGenerator(const std::vector<Eigen::Vector3f>& states) :
        m_states(states),
        m_lineCount(std::max((unsigned int) states.size() / 2, 1u) - 1)
    {
        m_boundingRadius = 0.0f;
        m_maxSpeed = 0.0f;
        for (unsigned int i = 0; i <= m_lineCount; ++i)
        {
            m_boundingRadius = std::max(m_boundingRadius, m_states[i * 2].norm());
            m_maxSpeed = std::max(m_maxSpeed, states[i * 2 + 1].norm());
        }
    }

    virtual void generateParticle(vesta::PseudorandomGenerator& gen,
                                  Eigen::Vector3f& position, Eigen::Vector3f& velocity) const
    {
        if (m_lineCount > 0)
        {
            unsigned int lineIndex = gen.randUint() % m_lineCount;
            float alpha = gen.randFloat();
            position = (1.0f - alpha) * m_states[lineIndex * 2] + alpha * m_states[lineIndex * 2 + 2];
            velocity = (1.0f - alpha) * m_states[lineIndex * 2 + 1] + alpha * m_states[lineIndex * 2 + 3];
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
    std::vector<Eigen::Vector3f> m_states;
    unsigned int m_lineCount;
    float m_boundingRadius;
    float m_maxSpeed;
};

#endif // _VEXT_STRIP_PARTICLE_GENERATOR_H_
