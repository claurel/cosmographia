/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INITIAL_STATE_GENERATOR_H_
#define _VESTA_INITIAL_STATE_GENERATOR_H_

#include "../Object.h"
#include "PseudorandomGenerator.h"
#include <Eigen/Core>


namespace vesta
{

/** An initial state generator is reponsible for computing
 *  initial particle positions and velocities for an emitter.
 */
class InitialStateGenerator : public Object
{
public:
    virtual ~InitialStateGenerator()
    {
    }

    /** Get the state for a particle. The pseudorandom generator is used compute the state, which
      * is returned by seting the values of the position and velocity parameters.
      */
    virtual void generateParticle(PseudorandomGenerator& gen, Eigen::Vector3f& position, Eigen::Vector3f& velocity) const = 0;

    /** Get the maximum distance of any generated position from the origin. This is used when computing
      * a bounding sphere for a particle emitter.
      */
    virtual float maxDistanceFromOrigin() const = 0;

    /** Get the length of the largest velocity vector that may be generated. This value is used when
      * compute a bounding sphere for a particle emitter.
      */
    virtual float maxSpeed() const = 0;
};

}

#endif // _VESTA_INITIAL_STATE_GENERATOR_H_
