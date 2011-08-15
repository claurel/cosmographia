/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

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
