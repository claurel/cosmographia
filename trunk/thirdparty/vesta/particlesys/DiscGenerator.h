// DiscGenerator.h
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// VESTA is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// VESTA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// VESTA. If not, see <http://www.gnu.org/licenses/>.

#ifndef _VESTA_DISC_GENERATOR_H_
#define _VESTA_DISC_GENERATOR_H_

#include "../Object.h"
#include "PseudorandomGenerator.h"
#include <Eigen/Core>


namespace vesta
{

/** DiscGenerator produces particles with the same velocity and random positions
  * that lie within an origin centered disc in the xy-plane.
  */
class DiscGenerator : public InitialStateGenerator
{
public:
    DiscGenerator(float radius,
                  const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero()) :
       m_radius(radius),
       m_velocity(velocity)
    {
    }

    virtual void generateParticle(PseudorandomGenerator& gen,
                                  Eigen::Vector3f& position, Eigen::Vector3f& velocity) const
    {
        // Generate a random points in the unit disc using the rejection method
        Eigen::Vector2f p;
        do
        {
            p = Eigen::Vector2f(gen.randSignedFloat(), gen.randSignedFloat());
        } while (p.squaredNorm() > 1.0f);

        position = Eigen::Vector3f(p.x() * m_radius, p.y() * m_radius, 0.0f);
        velocity = m_velocity;
    }

    virtual float maxDistanceFromOrigin() const
    {
        return m_radius;
    }

    virtual float maxSpeed() const
    {
        return m_velocity.norm();
    }

private:
    float m_radius;
    Eigen::Vector3f m_velocity;
};

}

#endif // _VESTA_DISC_GENERATOR_H_
