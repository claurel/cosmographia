// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

