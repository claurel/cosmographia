/*
 * $Revision: 678 $ $Date: 2012-05-22 17:59:22 -0700 (Tue, 22 May 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Atmosphere.h"
#include "TextureMap.h"
#include "Units.h"
#include "Debug.h"
#include "Intersect.h"
#include "DataChunk.h"
#include "internal/InputDataStream.h"
#include "internal/OutputDataStream.h"
#include "OGLHeaders.h"
#include <Eigen/Array>
#include <cmath>

#include <iostream>
#include <fstream>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Indices of refraction are from http://physics.info/refraction/ )

/** Index of refraction of air at 0 degrees C.
  */
const double Atmosphere::IndexOfRefraction_Air_0 = 1.00029238;

/** Index of refraction of air at 15 degrees C.
  */
const double Atmosphere::IndexOfRefraction_Air_15 = 1.00027712;

// Density of air in kilograms per cubic meter at:
//   0 degrees C
//   15 degrees C
static const double Density_Air_0 = 1.292;
static const double Density_Air_15 = 1.225;

// Mass of one mole of air in kilograms
static const double MolarMass_Air = 0.0289644;

static const double Mole = 6.0221415e23;

static const float MieScattering_ClearSky = 2.10e-6f;

/** Molecules of air per cubic meter at sea level on Earth at 0 degrees C
  */
const double Atmosphere::MolecularDensity_Air_0 = Mole * Density_Air_0 / MolarMass_Air;

/** Molecules of air per cubic meter at sea level on Earth at 15 degrees C
  */
const double Atmosphere::MolecularDensity_Air_15 = Mole * Density_Air_15 / MolarMass_Air;

static const double EarthEquatorialRadius = 6378.14;

static const Vector3d standardWavelengths(650.0, 550.0, 440.0);

// Calculate the Rayleigh scattering coefficient for the specified
// wavelength (in nanometers), index of refraction n, and molecular
// density (particles per cubic meter.)
static double rayleighScattering(double wavelength, double n, double N)
{
    return (8.0 * pow(PI, 3.0) * pow(n * n - 1.0, 2.0)) / (3.0 * N * pow(wavelength * 1.0e-9, 4.0));
}


// Temporary workaround for an apparent Eigen bug with g++ 4.2 on Mac OS X. We need to avoid
// using the vector::resize() method on Eigen's vector specialization for objects that require
// alignment.
template<typename V> void resizeVector(V& v, typename V::size_type new_size, const typename V::value_type& x)
{
    if (new_size < v.size())
    {
        v.erase(v.begin() + new_size, v.end());
    }
    else if (new_size > v.size())
    {
        v.insert(v.end(), new_size - v.size(), x);
    }
}


/** Construct a new atmosphere with default values approximately correct
  * for Earth.
  */
Atmosphere::Atmosphere() :
    m_planetRadius(float(EarthEquatorialRadius)),
    m_rayleighScaleHeight(8.0f),
    m_mieScaleHeight(1.2f),
    m_mieScatteringCoeff(MieScattering_ClearSky),
    m_mieAsymmetry(0.76f),
    m_absorptionCoeff(Vector3f::Zero()),
    m_transmittanceHeightSamples(0),
    m_transmittanceViewAngleSamples(0),
    m_scatterHeightSamples(0),
    m_scatterViewAngleSamples(0),
    m_scatterSunAngleSamples(0)
{
    computeRayleighScatteringCoeff(IndexOfRefraction_Air_15, MolecularDensity_Air_15);
}


Atmosphere::~Atmosphere()
{
}


/** Compute realistic Rayleigh scattering coefficients for the specified index
  * of refraction n and molecular density N.
  *
  * @param n index of refraction
  * @param N molecular density at ground level in molecules / cubic meter.
  */
void
Atmosphere::computeRayleighScatteringCoeff(double n, double N)
{
    Vector3d coeff(rayleighScattering(standardWavelengths.x(), n, N),
                   rayleighScattering(standardWavelengths.y(), n, N),
                   rayleighScattering(standardWavelengths.z(), n, N));
    m_rayleighScatteringCoeff = coeff.cast<float>();
}


/** Get the approximate color of the atmosphere due to Rayleigh scattering
  * over the specified distance in meters. This is used for simplified
  * atmosphere rendering that doesn't include all the effects of scattering.
  */
Spectrum
Atmosphere::color(float distance) const
{
    Vector3f s = distance * m_rayleighScatteringCoeff;
    Vector3f rgb = Vector3f::Ones() - Vector3f(exp(-s.x()), exp(-s.y()), exp(-s.z()));

    // Normalize the color
    rgb /= rgb.maxCoeff();

    return Spectrum(rgb.x(), rgb.y(), rgb.z());
}


/** Get the height at which the atmosphere is effectively transparent.
  * The density of the atmosphere decreases exponentially with altitude. Although
  * it is never zero, in practice we need to choose some finite volume for rendering
  * the atmospheric halo around a planet. We choose a height large enough to avoid
  * a sharp cutoff artifact, but small enough so that the GPU doesn't waste cycles
  * drawing a lot of transparent pixels.
  */
float
Atmosphere::transparentHeight() const
{
    return 8.0f * max(m_rayleighScaleHeight, m_mieScaleHeight);
}


TextureMap*
Atmosphere::transmittanceTexture() const
{
    return m_transmittanceTexture.ptr();
}


TextureMap*
Atmosphere::scatterTexture() const
{
    return m_scatterTexture.ptr();
}


/** Build precomputed scattering tables. generateTextures() must be called after this function in
  * order to be able to render objects with precomputed atmospheric scattering.
  */
void
Atmosphere::computeScattering(unsigned int heightSamples, unsigned int viewAngleSamples, unsigned int sunAngleSamples)
{
    computeTransmittanceTable(DefaultTransmittanceTableHeightSamples,
                              DefaultTransmittanceTableViewAngleSamples);
    computeInscatterTable(heightSamples,
                          viewAngleSamples,
                          sunAngleSamples);
}


/** Build precomputed scattering tables with the default dimensions. generateTextures()
  * must be called after this function in order to be able to render objects with precomputed
  * atmospheric scattering.
  */
void
Atmosphere::computeScattering()
{
    computeScattering(DefaultScatterTableHeightSamples,
                      DefaultScatterTableViewAngleSamples,
                      DefaultScatterTableSunAngleSamples);
}


void
Atmosphere::generateTextures()
{
    generateTransmittanceTexture();
    generateInscatterTexture();
}


void
Atmosphere::generateTransmittanceTexture()
{
    // Precomputed atmospheric scattering requires features not available in
    // Open GL ES 2.0
#ifndef VESTA_OGLES2
    unsigned int tableSize = m_transmittanceHeightSamples * m_transmittanceViewAngleSamples;
    if (tableSize < 1)
    {
        VESTA_LOG("Zero size transmittance table for atmosphere");
        return;
    }

    assert(m_transmittanceTable.size() >= tableSize);

    for (unsigned int i = 0; i < m_transmittanceHeightSamples * m_transmittanceViewAngleSamples; ++i)
    {
        m_transmittanceTable[i] = Vector3f(max(0.00001f, min(256.0f, m_transmittanceTable[i].x())),
                                           max(0.00001f, min(256.0f, m_transmittanceTable[i].y())),
                                           max(0.00001f, min(256.0f, m_transmittanceTable[i].z())));
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB16F,
                 m_transmittanceViewAngleSamples, m_transmittanceHeightSamples,
                 0,
                 GL_RGB, GL_FLOAT,
                 &m_transmittanceTable[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Do not enable mipmapping, as it causes artifacts in some atmospheres (e.g. Titan)
    // at the outer edge. This could probably be resolved with a custom mipmap generation
    // algorithm, but for now, we'll just leave mipmaps off.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    m_transmittanceTexture = new TextureMap(texId, TextureProperties(TextureProperties::Clamp));

    if (GLEW_EXT_framebuffer_object)
    {
        glGenerateMipmapEXT(GL_TEXTURE_2D);
    }
    else
    {
        // Can't create mipmaps, so reset filtering to linear; it's unlikely that
        // we'll take this path since any GPU that supports floating point textures
        // and GLSL will also have FBOs.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}


void
Atmosphere::generateInscatterTexture()
{
    // Precomputed atmospheric scattering requires features not available in
    // Open GL ES 2.0
#ifndef VESTA_OGLES2
    GLuint scatterTexId = 0;
    glGenTextures(1, &scatterTexId);
    glBindTexture(GL_TEXTURE_3D, scatterTexId);

    // Clamp scatter table values before converting them to half-floats. On at least one driver,
    // the conversion from 32-bit float to 16-bit half float seems to be performed incorrectly for
    // values very near zero.
    unsigned int tableSize = m_scatterSunAngleSamples * m_scatterViewAngleSamples * m_scatterHeightSamples;
    for (unsigned int i = 0; i < tableSize; ++i)
    {
        m_inscatterTable[i] = Vector4f(max(0.00001f, min(256.0f, m_inscatterTable[i].x())),
                                       max(0.00001f, min(256.0f, m_inscatterTable[i].y())),
                                       max(0.00001f, min(256.0f, m_inscatterTable[i].z())),
                                       max(0.00001f, min(256.0f, m_inscatterTable[i].w())));
    }

    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 GL_RGBA16F,
                 m_scatterSunAngleSamples, m_scatterViewAngleSamples, m_scatterHeightSamples,
                 0,
                 GL_RGBA, GL_FLOAT,
                 &m_inscatterTable[0]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_scatterTexture = new TextureMap(scatterTexId, TextureProperties(TextureProperties::Clamp));

    glBindTexture(GL_TEXTURE_3D, 0);
#endif
}


static float sign(float x)
{
    if (x > 0.0f)
        return 1.0f;
    else if (x < 0.0f)
        return -1.0f;
    else
        return 0.0f;
}


// h is the viewer's height above the planet surface
// atmRadius must be larger than planetRadius
static float opticalPathLength(float planetRadius, float atmRadius, float h, float cosViewAngle)
{
    // Gamma is 180 - view angle
    float cosGamma = -cosViewAngle;
    float sinGamma2 = 1.0f - cosGamma * cosGamma;

    float r = planetRadius + h;
    float c = r * r * sinGamma2;

    float disc = planetRadius * planetRadius - c;
    if (disc > 0.0f && cosGamma > 0.0f)
    {
        return r * cosGamma - sqrt(disc);
    }
    else
    {
        disc = atmRadius * atmRadius - c;
        return r * cosGamma + sqrt(disc);
    }
}


// Analytic calculation of optical depth
// Based on approximation from E. Bruneton and F. Neyret, "Precomputed Atmospheric Scattering" (2008)
//     - r is distance of the eye from planet center
//     - cosZenithAngle is the cosine of the angle between the zenith and view direction
//     - pathLength is the distance that the ray travels through the atmosphere
//     - H is the scale height
static float opticalDepth(float r, float cosZenithAngle, float pathLength, float H, float planetRadius)
{    
    // C++ version of this GLSL function:
    // float opticalDepth(float r, float zAngle, float pathLength, float H)" << endl;
    // {
    //     float a = sqrt(r * (0.5 / H));
    //     vec2 b = a * vec2(zAngle, zAngle + pathLength / r);
    //     vec2 b2 = b * b;
    //     vec2 signB = sign(b);
    //     float x = signB.y > signB.x ? exp(b2.x) : 0.0;
    //     vec2 y = signB / (2.3193 * abs(b) + sqrt(1.52 * b2 + 4.0)) * vec2(1.0, exp(-pathLength / H * (pathLength / (2.0 * r) + zAngle)));
    //     return sqrt((6.283185 * H) * r) * exp((planetRadius - r) / H) * (x + dot(y, vec2(1.0, -1.0)));
    // }

    float a = sqrt(r * (0.5f / H));

    Vector2f b = a * Vector2f(cosZenithAngle, cosZenithAngle + pathLength / r);
    Vector2f b2 = b.cwise().square();
    Vector2f signB(sign(b.x()), sign(b.y()));

    float x = signB.y() > signB.x() ? exp(b2.x()) : 0.0f;

    float k = exp(-pathLength / H * (pathLength / (2.0f * r) + cosZenithAngle));
    float yx = signB.x() / (2.3193f * abs(b.x()) + sqrt(1.52f * b2.x() + 4.0f));
    float yy = signB.y() / (2.3193f * abs(b.y()) + sqrt(1.52f * b2.y() + 4.0f)) * k;
    return sqrt((6.283185f * H) * r) * exp((planetRadius - r) / H) * (x + yx - yy);
}


Vector3f
Atmosphere::transmittance(float r, float cosZenithAngle, float pathLength) const
{
    float odMie      = opticalDepth(r, cosZenithAngle, pathLength, m_mieScaleHeight, m_planetRadius);
    float odRayleigh = opticalDepth(r, cosZenithAngle, pathLength, m_rayleighScaleHeight, m_planetRadius);

    const Vector3f exR = m_rayleighScatteringCoeff * 1000.0f;
    const Vector3f exM = (Vector3f::Constant(m_mieScatteringCoeff) + m_absorptionCoeff) * 1000.0f;

    return (-odMie * exM - odRayleigh * exR).cwise().exp();
}


// Compute the transmittance by looking up the value in the precomputed table. Perform
// bilinear interpolation among table values.
Eigen::Vector3f
Atmosphere::transmittance(float r, float cosZenithAngle) const
{
    const unsigned int width = m_transmittanceViewAngleSamples;
    const unsigned int height = m_transmittanceHeightSamples;

    float u = cosZenithAngle * 0.5f + 0.5f;
    float v = sqrt((r - m_planetRadius) / transparentHeight());
    u = max(0.0f, min(0.99999f, u));
    v = max(0.0f, min(0.99999f, v));

    float x = u * (width - 1);
    float y = v * (height - 1);
    int ix = (int) x;
    int iy = (int) y;
    float fx = x - ix;
    float fy = y - iy;

    int index = width * iy + ix;
    Vector3f v0 = m_transmittanceTable[index] * (1.0f - fx) + m_transmittanceTable[index + 1] * fx;
    Vector3f v1 = m_transmittanceTable[index + width] * (1.0f - fx) + m_transmittanceTable[index + width + 1] * fx;

    return v0 * (1.0f - fy) + v1 * fy;
}


// Non-linear table parametrization:
//   0 <= t <= 1
//
//   height:             h(t) = t^2 * transparentHeight
//   cos(view angle):    mu(t) = toCosViewAngle()
//   cos(sun angle):     muS(t) = toCosSunAngle()
//
// Inverse mappings:
//   height:             t = sqrt(h / transparentHeight)
//   cos(view angle):    t =
//   cos(sun angle):     t =
//
// Notes:
//   - View and sun angles are both measured from the zenith
//

// Map a value in [0, 1] to the cosine of the viewing angle
// This function replaces the parametrization used in Bruneton's paper:
//     mu = -0.15f + tan(1.5f * v) / tan(1.5f) * 1.15f
//
// The change avoids an expensive arctangent function in the shader
// code.
//
// The mapping may be tuned by adjusting the value of the parameter b.
// b of 0.15 works well for Earth; a larger value should be chosen when the
// atmosphere extends higher relative to the planet radius.
static inline float toCosViewAngle(float u)
{
    float x = u * 2.0f - 1.0f;
    float sn = x < 0.0f ? 1.0f : -1.0f;
    return (x * (0.1f - 0.15f * sn) - 0.165f) / (sn * x + 1.1f);
}

// Map a value in [0, 1] to the cosine of the sun angle
static inline float toCosSunAngle(float u)
{
    // Modified from version used in Bruneton paper. This one covers a wider range
    // of sun angles, which is necessary for larger scale height / planet radius
    // ratios (e.g. Titan)
    return (log(1.0f - u * (1.0f - exp(-2.6f))) + 0.6f) / -2.0f;
}

// Fill a table with transmittance values.
//
// Transmittance in a spherical atmosphere can be described as a function of
// two parameters:
//    h - the height of the viewer above the planet surface
//    mu - the cosine of the view angle (angle between the view direction and the zenith)
void
Atmosphere::computeTransmittanceTable(unsigned int heightSamples,
                                      unsigned int viewAngleSamples)
{
    m_transmittanceHeightSamples = heightSamples;
    m_transmittanceViewAngleSamples = viewAngleSamples;
    m_transmittanceTable.resize(heightSamples * viewAngleSamples);

    float maxHeight = transparentHeight();
    float minHeight = m_planetRadius * 1.0e-6f;
    const unsigned int integrationSteps = 20;

    // Calculate the extinction coefficients. The are computed separately for Mie and Rayleigh
    // scattering particles since their densities will generally be described with different
    // scale heights.
    const Vector3f Er = m_rayleighScatteringCoeff * 1000.0f;
    const Vector3f Em = (Vector3f::Constant(m_mieScatteringCoeff) + m_absorptionCoeff) * 1000.0f;

    VESTA_LOG("Rayleigh extinction: %f %f %f", Er.x(), Er.y(), Er.z());
    VESTA_LOG("Mie extinction: %f %f %f", Em.x(), Em.y(), Em.z());

    for (unsigned int i = 0; i < heightSamples; ++i)
    {
        float v = float(i) / float(heightSamples);
        float h = minHeight + v * v * maxHeight;

        // Calculate the eye position from h
        Vector3f eye = Vector3f::UnitZ() * (m_planetRadius + h);

        for (unsigned int j = 0; j < viewAngleSamples; ++j)
        {
            float u = float(j) / float(viewAngleSamples - 1);
            float mu = toCosViewAngle(u);

            // Calculate the view direction from mu
            float cosTheta = mu;
            float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
            Vector3f viewDir(sinTheta, 0.0f, cosTheta);

            float pathLength = 0.0f;
            // The view ray will intersect either the planet or the atmosphere shell geometry
            if (!TestRaySphereIntersection(eye, viewDir, Vector3f::Zero(), m_planetRadius, &pathLength))
            {
                TestRaySphereIntersection(eye, viewDir, Vector3f::Zero(), m_planetRadius + maxHeight, &pathLength);
            }

            // Compute the intersection point
            Vector3f x0 = eye + pathLength * viewDir;

#if 0
            // Numerical integration to compute transmittance
            Vector3f step = (x0 - eye) / float(integrationSteps);
            float stepLength = pathLength / float(integrationSteps);

            // Sum to get the integral of optical depth between the eye and the intersection
            // point.
            Vector3f p = eye;
            float Tr = 0.0f;
            float Tm = 0.0f;

            for (unsigned int k = 0; k < integrationSteps; ++k)
            {
                float s = p.norm() - m_planetRadius;
                
                Tr += exp(-s / m_rayleighScaleHeight);
                Tm += exp(-s / m_mieScaleHeight);
                p += step;
            }
            Vector3f opticalDepth = (Er * Tr + Em * Tm) * stepLength;
            Vector3f xmit = (-opticalDepth).cwise().exp();
#else
            // Use analytic transmittance calculation
            Vector3f xmit = transmittance(eye.z(), viewDir.z(), pathLength);
#endif
            m_transmittanceTable[i * viewAngleSamples + j] = xmit;
        }
    }
}


// Fill a table with scattering values.
//
// Scattering in a spherical atmosphere can be described as a function of
// three parameters:
//    h - the height of the viewer above the planet surface
//    mu - the cosine of the view angle (angle between the view direction and the zenith)
//    muS - the cosine of the sun angle (angle between sun and zenith)
void
Atmosphere::computeInscatterTable(unsigned int heightSamples,
                                  unsigned int viewAngleSamples,
                                  unsigned int sunAngleSamples)
{
    m_scatterHeightSamples = heightSamples;
    m_scatterViewAngleSamples = viewAngleSamples;
    m_scatterSunAngleSamples = sunAngleSamples;

    unsigned int tableSize = m_scatterHeightSamples * m_scatterViewAngleSamples * m_scatterSunAngleSamples;
    if (tableSize < 1)
    {
        return;
    }

    //m_inscatterTable.resize(tableSize);
    resizeVector(m_inscatterTable, tableSize, Vector4f::Zero());
    if (m_inscatterTable.size() != tableSize)
    {
        return;
    }

    float maxHeight = transparentHeight();
    float minHeight = m_planetRadius * 1.0e-6f;
    const unsigned int integrationSteps = 25;

    float atmRadius = m_planetRadius + transparentHeight();

    // Calculate scattering coefficients. These are the same as the extinction coefficients
    // exception that absorption by Mie scattering particles isn't a factor.
    const Vector3f Sr = m_rayleighScatteringCoeff * 1000.0f;
    const float Sm = m_mieScatteringCoeff * 1000.0f;
    const Vector4f scatterFactors = Vector4f(Sr.x(), Sr.y(), Sr.z(), Sm);

    for (unsigned int i = 0; i < heightSamples; ++i)
    {
        VESTA_LOG("Scatter texture layer: %d", i);
        float w = float(i) / float(heightSamples);
        float h = minHeight + w * w * maxHeight;

        // Calculate the eye position from h
        Vector3f eye = Vector3f::UnitZ() * (m_planetRadius + h);

        for (unsigned int j = 0; j < viewAngleSamples; ++j)
        {
            float v = float(j) / float(viewAngleSamples - 1);
            //float mu = 2.0f * v - 1.0f;
            //float x = v * 2.0f - 1.0f;

            //float mu = (x * 0.1f) / (1.1f - abs(x));
            //float sn = x + 0.15f < 0.0f ? 1.0f : -1.0f;
            //float mu = (x * (0.1f - 0.15f * sn) - 0.165f) / (sn * x + 1.1f);
            float mu = toCosViewAngle(v);

            // Calculate the view direction from mu
            float cosTheta = mu;
            float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
            Vector3f viewDir(sinTheta, 0.0f, cosTheta);

            float pathLength = 0.0f;
            // The view ray will intersect either the planet or the atmosphere shell geometry
            if (!TestRaySphereIntersection(eye, viewDir, Vector3f::Zero(), m_planetRadius, &pathLength))
            {
                TestRaySphereIntersection(eye, viewDir, Vector3f::Zero(), m_planetRadius + maxHeight, &pathLength);
            }

            // Compute the intersection point
            Vector3f x0 = eye + pathLength * viewDir;

            Vector3f step = (x0 - eye) / float(integrationSteps);
            float stepLength = pathLength / float(integrationSteps);

            Vector3f viewRayTransmittance = transmittance(eye.z(), viewDir.z(), pathLength);
            //Vector3f viewRayTransmittance = transmittance(eye.z(), viewDir.z());

            for (unsigned int k = 0; k < sunAngleSamples; ++k)
            {
                float u = float(k) / float(sunAngleSamples - 1);
                //float muS = 2.0f * u - 1.0f;
                float muS = toCosSunAngle(u);//(log(1.0f - u * (1.0f - exp(-3.6f))) + 0.6f) / -3.0f;

                // Calculate the sun direction from mu
                float cosPhi = muS;
                float sinPhi2 = 1.0f - cosPhi * cosPhi;
                float sinPhi = sqrt(max(0.0f, sinPhi2));
                Vector3f sunDir(sinPhi, 0.0f, cosPhi);

                // Sum to get the integral of optical depth between the eye and the intersection
                // point.
                Vector3f p = eye;
                Vector4f inscatter = Vector4f::Zero();

                for (unsigned int l = 0; l < integrationSteps; ++l)
                {
                    float r = p.norm();
                    float s = r - m_planetRadius;

                    // Compute the transmittance along the view ray
                    Vector3f viewXmit = transmittance(eye.z(), viewDir.z(), l * stepLength);
                    //Vector3f viewXmit = viewRayTransmittance.cwise() / transmittance(r, p.dot(viewDir) / r, pathLength - (l + 1) * stepLength);
                    //Vector3f viewXmit = viewRayTransmittance.cwise() / transmittance(r, p.dot(viewDir) / r);

                    float cosPsi = p.dot(sunDir) / r;
                    float sinPsi2 = 1.0f - cosPsi * cosPsi;

                    // Compute the transmittance along the path to the sun
                    float sunPathLength = -r * cosPsi + sqrt(atmRadius * atmRadius - r * r * sinPsi2);
                    Vector3f sunXmit = transmittance(r, cosPsi, sunPathLength);
                    float d1 = opticalDepth(r, cosPsi, sunPathLength, m_rayleighScaleHeight, m_planetRadius);
                    float d2 = opticalDepth(r, cosPsi, sunPathLength, m_mieScaleHeight, m_planetRadius);

                    Vector3f xmit = sunXmit.cwise() * viewXmit;
                    inscatter.start<3>() += (exp(-s / m_rayleighScaleHeight) * stepLength) * xmit;
                    inscatter.w() += exp(-s / m_mieScaleHeight) * stepLength * xmit.x();

                    p += step;
                }

                m_inscatterTable[(i * viewAngleSamples + j) * sunAngleSamples + k] = inscatter.cwise() * scatterFactors;
            }
        }
    }
}


/** Load an atmosphere from the contents of a .atmscat file. generateTextures() must be
  * after this function in order to be able to render objects with precomputed
  * atmospheric scattering.
  *
  * atmscat file header format:
  *
  * bytes          contents
  * -------------------------------
  * 0-7            header string ("atmscatr")
  * 8-11           version identifier (uint32)
  * 12-15          Rayleigh scale height (float)
  * 16-27          Rayleigh scattering coefficients (3 floats)
  * 28-31          Mie scale height (float)
  * 32-35          Mie scattering coefficient (float)
  * 36-39          Mie asymmetry parameter (float)
  * 40-51          Absorption coefficients (3 floats)
  * 52-55          Planet radius (float)
  * 56-63          Transmittance table dimensions (2 uint32, width * height)
  * 64-75          Scattering table dimensions (3 uint32, width * height * depth)
  *
  * transmittance table (width * height * 3 floats)
  * scattering table (width * height * depth * 4 floats)
  */
Atmosphere*
Atmosphere::LoadAtmScat(const DataChunk* data)
{
    string str(data->data(), data->size());
    InputDataStream in(str);
    in.setByteOrder(InputDataStream::BigEndian);

    in.setByteOrder(InputDataStream::LittleEndian);

    char header[8];
    in.readData(header, sizeof(header));
    if (string(header, sizeof(header)) != "atmscatr")
    {
        VESTA_LOG("Incorrect header in atmscat file.");
        return NULL;
    }

    v_uint32 version = in.readInt32();
    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading header of atmscat file.");
        return NULL;
    }

    if (version != 1)
    {
        VESTA_LOG("Unsupported atmscat file version %u", version);
        return NULL;
    }

    float HR = in.readFloat();
    Vector3f rayleighCoeff;
    rayleighCoeff.x() = in.readFloat();
    rayleighCoeff.y() = in.readFloat();
    rayleighCoeff.z() = in.readFloat();
    float HM = in.readFloat();
    float mieCoeff = in.readFloat();
    float mieAsymmetry = in.readFloat();
    Vector3f absorptionCoeff;
    absorptionCoeff.x() = in.readFloat();
    absorptionCoeff.y() = in.readFloat();
    absorptionCoeff.z() = in.readFloat();
    float planetRadius = in.readFloat();

    unsigned int transmitWidth = in.readUint32();
    unsigned int transmitHeight = in.readUint32();
    unsigned int scatterWidth = in.readUint32();
    unsigned int scatterHeight = in.readUint32();
    unsigned int scatterDepth = in.readUint32();

    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading header of atmscat file.");
        return NULL;
    }

    if (transmitWidth == 0 || transmitHeight == 0)
    {
        VESTA_LOG("Bad atmscat file (zero dimension for transmittance table)");
        return NULL;
    }

    if (scatterWidth == 0 || scatterHeight == 0 || scatterDepth == 0)
    {
        VESTA_LOG("Bad atmscat file (zero dimension for inscatter table)");
        return NULL;
    }

    Atmosphere* atmosphere = new Atmosphere();
    atmosphere->setRayleighScaleHeight(HR);
    atmosphere->setRayleighScatteringCoeff(rayleighCoeff);
    atmosphere->setMieScaleHeight(HM);
    atmosphere->setMieScatteringCoeff(mieCoeff);
    atmosphere->setMieAsymmetry(mieAsymmetry);
    atmosphere->setAbsorptionCoeff(absorptionCoeff);
    atmosphere->setPlanetRadius(planetRadius);

    atmosphere->m_transmittanceHeightSamples = transmitHeight;
    atmosphere->m_transmittanceViewAngleSamples = transmitWidth;
    atmosphere->m_transmittanceTable.resize(transmitWidth * transmitHeight);
    if (atmosphere->m_transmittanceTable.size() != transmitWidth * transmitHeight)
    {
        VESTA_LOG("Out of memory error (allocating atmosphere transmittance table)");
        delete atmosphere;
        return NULL;
    }

    for (unsigned int i = 0; i < transmitWidth * transmitHeight; ++i)
    {
        Vector3f v;
        v.x() = in.readFloat();
        v.y() = in.readFloat();
        v.z() = in.readFloat();
        atmosphere->m_transmittanceTable[i] = v;
    }

    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading transmittance table in atmscat file.");
        delete atmosphere;
        return NULL;
    }

    unsigned int scatterTableEntries = scatterWidth * scatterHeight * scatterDepth;
    atmosphere->m_scatterHeightSamples = scatterDepth;
    atmosphere->m_scatterViewAngleSamples = scatterHeight;
    atmosphere->m_scatterSunAngleSamples = scatterWidth;
    //atmosphere->m_inscatterTable.resize(scatterTableEntries);
    resizeVector(atmosphere->m_inscatterTable, scatterTableEntries, Vector4f::Zero());
    if (atmosphere->m_inscatterTable.size() != scatterTableEntries)
    {
        VESTA_LOG("Out of memory error (allocating atmosphere inscatter table)");
        delete atmosphere;
        return NULL;
    }

    for (unsigned int i = 0; i < scatterTableEntries; ++i)
    {
        Vector4f v;
        v.x() = in.readFloat();
        v.y() = in.readFloat();
        v.z() = in.readFloat();
        v.w() = in.readFloat();
        atmosphere->m_inscatterTable[i] = v;
    }
    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading inscatter table in atmscat file.");
        delete atmosphere;
        return NULL;
    }

    return atmosphere;
}

/** Save an atmosphere to a .atmscat file.
  *
  * atmscat file header format:
  *
  * bytes          contents
  * -------------------------------
  * 0-7            header string ("atmscatr")
  * 8-11           version identifier (uint32)
  * 12-15          Rayleigh scale height (float)
  * 16-27          Rayleigh scattering coefficients (3 floats)
  * 28-31          Mie scale height (float)
  * 32-35          Mie scattering coefficient (float)
  * 36-39          Mie asymmetry parameter (float)
  * 40-51          Absorption coefficients (3 floats)
  * 52-55          Planet radius (float)
  * 56-63          Transmittance table dimensions (2 uint32, width * height)
  * 64-75          Scattering table dimensions (3 uint32, width * height * depth)
  *
  * transmittance table (width * height * 3 floats)
  * scattering table (width * height * depth * 4 floats)
  */
void
Atmosphere::SaveAtmScat(const char* filename)
{
    filebuf fb;
    fb.open (filename,ios::out | ios::binary);
    ostream os(&fb);
    OutputDataStream out(os);
    out.setByteOrder(OutputDataStream::LittleEndian);

    out.writeData("atmscatr", 8);

    out.writeInt32(1);
    if (out.status() != OutputDataStream::Good)
    {
        VESTA_LOG("Error writing header of atmscat file.");
        return;
    }

    out.writeFloat(m_rayleighScaleHeight);
    out.writeFloat(m_rayleighScatteringCoeff.x());
    out.writeFloat(m_rayleighScatteringCoeff.y());
    out.writeFloat(m_rayleighScatteringCoeff.z());
    out.writeFloat(m_mieScaleHeight);
    out.writeFloat(m_mieScatteringCoeff);
    out.writeFloat(m_mieAsymmetry);
    out.writeFloat(m_absorptionCoeff.x());
    out.writeFloat(m_absorptionCoeff.y());
    out.writeFloat(m_absorptionCoeff.z());
    out.writeFloat(m_planetRadius);
    out.writeUint32(m_transmittanceViewAngleSamples);
    out.writeUint32(m_transmittanceHeightSamples);
    out.writeUint32(m_scatterSunAngleSamples);
    out.writeUint32(m_scatterViewAngleSamples);
    out.writeUint32(m_scatterHeightSamples);

    if (out.status() != OutputDataStream::Good)
    {
        VESTA_LOG("Error writing header of atmscat file.");
        return;
    }

    for (unsigned int i = 0; i < m_transmittanceTable.size(); ++i)
    {
        out.writeFloat(m_transmittanceTable[i].x());
        out.writeFloat(m_transmittanceTable[i].y());
        out.writeFloat(m_transmittanceTable[i].z());
    }

    if (out.status() != OutputDataStream::Good)
    {
        VESTA_LOG("Error writing transmittance table in atmscat file.");
        return;
    }

    for (unsigned int i = 0; i < m_inscatterTable.size(); ++i)
    {
        out.writeFloat(m_inscatterTable[i].x());
        out.writeFloat(m_inscatterTable[i].y());
        out.writeFloat(m_inscatterTable[i].z());
        out.writeFloat(m_inscatterTable[i].w());
    }
    if (out.status() != OutputDataStream::Good)
    {
        VESTA_LOG("Error reading inscatter table in atmscat file.");
        return;
    }

    fb.close();
}
