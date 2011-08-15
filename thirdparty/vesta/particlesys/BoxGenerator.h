/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BOX_GENERATOR_H_
#define _VESTA_BOX_GENERATOR_H_

#include "../Object.h"
#include "PseudorandomGenerator.h"
#include <Eigen/Core>


namespace vesta
{

/** BoxGenerator produces particles with the same velocity and random positions
  * that lie within an axis-aligned box
  */
class BoxGenerator : public InitialStateGenerator
{
public:
    BoxGenerator(const Eigen::Vector3f& sideLengths = Eigen::Vector3f::Ones(),
                 const Eigen::Vector3f& center = Eigen::Vector3f::Zero(),
                 const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero()) :
       m_sideLengths(sideLengths),
       m_center(center),
       m_velocity(velocity),
       m_maxDist((sideLengths * 0.5f).norm() + center.norm())
    {
    }

    virtual void generateParticle(PseudorandomGenerator& gen,
                                  Eigen::Vector3f& position, Eigen::Vector3f& velocity) const
    {
        // Generate a random points in the unit disc using the rejection method
        Eigen::Vector3f randVector(gen.randSignedFloat(), gen.randSignedFloat(), gen.randSignedFloat());
        position = m_center + 0.5f * (randVector.cwise() * m_sideLengths);
        velocity = m_velocity;
    }

    virtual float maxDistanceFromOrigin() const
    {
        return m_maxDist;
    }

    virtual float maxSpeed() const
    {
        return m_velocity.norm();
    }

private:
    Eigen::Vector3f m_sideLengths;
    Eigen::Vector3f m_center;
    Eigen::Vector3f m_velocity;
    float m_maxDist; // store the max distance from oriin
};

}

#endif // _VESTA_BOX_GENERATOR_H_
