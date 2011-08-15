/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

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
