/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_POINT_GENERATOR_H_
#define _VESTA_POINT_GENERATOR_H_

#include "../Object.h"
#include "PseudorandomGenerator.h"
#include <Eigen/Core>


namespace vesta
{

/** PointGenerator produces particles that all begin at the same point
  * with the same velocity.
  */
class PointGenerator : public InitialStateGenerator
{
public:
    PointGenerator(const Eigen::Vector3f& position = Eigen::Vector3f::Zero(),
                   const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero()) :
       m_position(position),
       m_velocity(velocity)
    {
    }

    virtual void generateParticle(PseudorandomGenerator& /* gen */,
                                  Eigen::Vector3f& position, Eigen::Vector3f& velocity) const
    {
        position = m_position;
        velocity = m_velocity;
    }

    virtual float maxDistanceFromOrigin() const
    {
        return m_position.norm();
    }

    virtual float maxSpeed() const
    {
        return m_velocity.norm();
    }

private:
    Eigen::Vector3f m_position;
    Eigen::Vector3f m_velocity;
};

}

#endif // _VESTA_POINT_GENERATOR_H_
