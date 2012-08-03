/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ParticleEmitter.h"
#include "ParticleRenderer.h"
#include "PseudorandomGenerator.h"
#include "PointGenerator.h"
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


ParticleEmitter::ParticleEmitter() :
    m_startTime(0.0),
    m_endTime(0.0),
    m_particleLifetime(1.0),
    m_spawnRate(1.0),
    m_force(Vector3f::Zero()),
    m_blockingPlaneEnabled(false),
    m_colorCount(1),
    m_velocityVariation(0.0f),
    m_traceLength(0.0f),
    m_emissive(true),
    m_phaseAsymmetry(0.0f)
{
    m_colorKeys[0] = Vector4f::Ones();
    m_generator = new PointGenerator();
}


ParticleEmitter::~ParticleEmitter()
{
}


static Vector3f
randomPointInUnitSphere(PseudorandomGenerator& gen)
{
    // Generate random points in the unit cube, discarding the ones that don't
    // fall inside the unit sphere. Since the sphere fills just over 50% of the
    // cube, this won't result in too many wasted calls to randSignedFloat().
    Vector3f p;
    do
    {
        p = Vector3f(gen.randSignedFloat(), gen.randSignedFloat(), gen.randSignedFloat());
    } while (p.squaredNorm() > 1.0f);

    return p;
}


void
ParticleEmitter::generateParticles(double simulationTime,
                                   std::vector<Particle>& particleBuffer,
                                   ParticleRenderer* renderer) const
{
    if (simulationTime > m_endTime + m_particleLifetime)
    {
        // No particles left
        return;
    }
    
    if (simulationTime < m_startTime)
    {
        // No particles emitted yet
        return;
    }

    // t is the time elapsed since the start time
    double t = simulationTime - m_startTime;

    // Particles are always emitted at a constant rate
    double spawnInterval = 1.0 / m_spawnRate;

    // Location within the particle stream; more than maxint particles
    // may be emitted over the lifetime of the particle system, so we
    // have to wrap.
    double streamLocation = std::fmod(t * m_spawnRate, (double) (0x80000000u));

    // Compute the particle index; this will be used to initialize the
    // random number generator, so that the particle always has the
    // same properties.
    int particleIndex = (int) (streamLocation);

    // Age of the first particle
    double age = (streamLocation - particleIndex) * spawnInterval;

    float invLifetime = (float) (1.0 / m_particleLifetime);
    
    double maxAge = min((double) m_particleLifetime, t);
    if (simulationTime > m_endTime)
    {
        int skipParticles = (int) ((simulationTime - m_endTime) * m_spawnRate);
        particleIndex -= skipParticles;
        age += skipParticles * spawnInterval;
    }
    
    particleBuffer.clear();

    // Scale factor used in color interpolation; subtract
    // a small value to avoid having to do an extra range
    // check for particles right at the end of their lifetimes.
    float colorKeyScale = float(m_colorCount) - 1.00001f;

    while (age < maxAge)
    {
        float w0 = (float) age * invLifetime;
        float w1 = 1.0f - w0;

        // Initialize the pseudorandom number generator with a value
        // that's based on the current particle index. This ensures that
        // the same initial state is always generated for the particle.
        // We can't just use the unmodified particle index, as this produces
        // obvious correlations between particles when initial properties
        // are generated with a simple linear congruential number generator.
        PseudorandomGenerator gen((v_uint64(particleIndex) * 1103515245) ^ 0xaaaaaaaaaaaaaaaaULL);

        // Compute the initial state
        Vector3f p0;
        Vector3f v0;
        m_generator->generateParticle(gen, p0, v0);

        if (m_velocityVariation > 0.0f)
        {
            v0 += randomPointInUnitSphere(gen) * m_velocityVariation;
        }

        // Compute the state of the particle at the current time
        Particle particle;

        // Compute the particle size
        particle.size = w0 * m_endSize + w1 * m_startSize;

        // Calculate the color of the particle. This can be done very
        // inexpensively in a shader.
        if (m_colorCount < 2)
        {
            particle.color = m_colorKeys[0].start<3>();
            particle.opacity = m_colorKeys[0].w();
        }
        else
        {
            float s = w0 * colorKeyScale;
            int colorIndex = (unsigned int) s;
            float t = s - colorIndex;
            Vector4f interpolatedColor = (1 - t) * m_colorKeys[colorIndex] + t * m_colorKeys[colorIndex + 1];

            particle.color = interpolatedColor.start<3>();
            particle.opacity = interpolatedColor.w();
        }

        // Calculate particle position as p0 + v0*t + (1/2)at^2
        particle.velocity = v0 + static_cast<float>(age) * m_force;
        particle.position = p0 + static_cast<float>(age) * (v0 + (static_cast<float>(age) * 0.5f) * m_force);

        // Rotation (if enabled)
        // TODO

        // Flush the particle buffer if it's full
        if (particleBuffer.size() == particleBuffer.capacity())
        {
            renderer->renderParticles(particleBuffer);
            particleBuffer.clear();
        }

        // Add the particle to the buffer
        particleBuffer.push_back(particle);
        
        // Older particles were emitted earlier, and therefore have a
        // lower index.
        particleIndex--;
        age += spawnInterval;
    }

    // Render any particles remaining in the buffer
    if (!particleBuffer.empty())
    {
        renderer->renderParticles(particleBuffer);
        particleBuffer.clear();
    }
}


/** Get the radius of an origin-centered sphere that is large
 *  enough to contain any particle produced by the emitter.
 *  This value is used for visibility culling of particle
 *  systems.
 */
float
ParticleEmitter::boundingRadius() const
{
    // Easy to compute this value, since the motion of any particle
    // is completely described by a quadratic.
    float maxSpeed = m_generator->maxSpeed() + m_velocityVariation;
    return m_generator->maxDistanceFromOrigin() +
           static_cast<float>(m_particleLifetime) * maxSpeed +
           0.5f * static_cast<float>(m_particleLifetime * m_particleLifetime) * m_force.norm();
}

