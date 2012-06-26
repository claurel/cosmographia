/*
 * $Revision: 658 $ $Date: 2012-03-23 01:08:30 -0700 (Fri, 23 Mar 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PARTICLE_SYSTEM_GEOMETRY_H_
#define _VESTA_PARTICLE_SYSTEM_GEOMETRY_H_

#include "Geometry.h"
#include "TextureMap.h"
#include <vector>

namespace vesta
{
class ParticleEmitter;
class TextureMapLoader;

/** ParticleSystemGeoemtry is a Geometry object that contains one or more particle
  * emitters.
  */
class ParticleSystemGeometry : public Geometry
{
public:
    ParticleSystemGeometry();
    virtual ~ParticleSystemGeometry();

    void render(RenderContext& rc,
                double clock) const;

    float boundingSphereRadius() const;
    virtual bool isOpaque() const { return false; }

    void addEmitter(ParticleEmitter* emitter, TextureMap* particleTexture);

    ParticleEmitter* emitter(int index);

private:
    std::vector< counted_ptr<ParticleEmitter> > m_emitters;
    std::vector< counted_ptr<TextureMap> > m_particleTextures;
};

}

#endif // _VESTA_PARTICLE_SYSTEM_GEOMETRY_H_

