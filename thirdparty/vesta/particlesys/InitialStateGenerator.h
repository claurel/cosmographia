// InitialStateGenerator.h
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
