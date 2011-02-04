// ParticleRenderer.h
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

#ifndef _VESTA_PARTICLE_RENDERER_H_
#define _VESTA_PARTICLE_RENDERER_H_

#include "ParticleEmitter.h"

namespace vesta
{

/** Interface for a class that will handle the rendering of particles
 *  produced by an emitter.
 */
class ParticleRenderer
{
public:
    ParticleRenderer() {};
    virtual ~ParticleRenderer() {};

    /** renderParticles() is called whenever a batch of particles is
     *  ready to be drawn.
     */
    virtual void renderParticles(const std::vector<ParticleEmitter::Particle>& particles) = 0;
};

}

#endif // _VESTA_PARTICLE_RENDERER_H_
