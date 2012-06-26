/*
 * $Revision: 658 $ $Date: 2012-03-23 01:08:30 -0700 (Fri, 23 Mar 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ParticleSystemGeometry.h"
#include "particlesys/ParticleEmitter.h"
#include "Material.h"
#include "RenderContext.h"
#include <algorithm>

using namespace vesta;


ParticleSystemGeometry::ParticleSystemGeometry()
{
}


ParticleSystemGeometry::~ParticleSystemGeometry()
{
}


void
ParticleSystemGeometry::render(RenderContext& rc, double clock) const
{
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        Material material;
        material.setEmission(Spectrum(1.0f, 1.0f, 0.0f));
        material.setDiffuse(Spectrum(1.0f, 1.0f, 1.0f));
        material.setBlendMode(Material::AdditiveBlend);

        for (unsigned int i = 0; i < m_emitters.size(); ++i)
        {
            material.setBaseTexture(m_particleTextures[i].ptr());
            rc.bindMaterial(&material);

            ParticleEmitter* emitter = m_emitters[i].ptr();
            rc.drawParticles(emitter, clock);
        }
    }
}


float
ParticleSystemGeometry::boundingSphereRadius() const
{
    float radius = 0.0f;
    for (unsigned int i = 0; i < m_emitters.size(); ++i)
    {
        radius = std::max(radius, m_emitters[i]->boundingRadius());
    }

    return radius;
}


void
ParticleSystemGeometry::addEmitter(ParticleEmitter* emitter, TextureMap* particleTexture)
{
    m_emitters.push_back(counted_ptr<ParticleEmitter>(emitter));
    m_particleTextures.push_back(counted_ptr<TextureMap>(particleTexture));
}

ParticleEmitter* ParticleSystemGeometry::emitter(int index)
{
    return m_emitters[index].ptr();
}
