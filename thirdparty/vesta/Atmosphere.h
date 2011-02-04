/*
 * $Revision: 455 $ $Date: 2010-08-24 17:00:53 -0700 (Tue, 24 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ATMOSPHERE_H_
#define _VESTA_ATMOSPHERE_H_

#include "Object.h"
#include "Spectrum.h"
#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>


namespace vesta
{

class TextureMap;
class DataChunk;

/** Atmosphere contains parameters required for visual representation of planetary
  * atmospheres.
  */
class Atmosphere : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Atmosphere();
    ~Atmosphere();

    /** Get the radius of the planet in kilometers.
     */
    float planetRadius() const
    {
        return m_planetRadius;
    }

    /** Set the radius of the planet in kilometers.
      */
    void setPlanetRadius(float radius)
    {
        m_planetRadius = radius;
    }

    /** Get the height in kilometers at which the density of Rayleigh scattering
     *  particles is half that at ground level.
     */
    float rayleighScaleHeight()
    {
        return m_rayleighScaleHeight;
    }

    /** Set the height in kilometers at which the density of Rayleigh scattering
     *  particles is half that at ground level.
     */
    void setRayleighScaleHeight(float height)
    {
        m_rayleighScaleHeight = height;
    }

    /** Get the height in kilometers at which the density of Mie scattering
     *  particles (aerosols) is half that at ground level.
     */
    float mieScaleHeight()
    {
        return m_mieScaleHeight;
    }

    /** Set the height in kilometers at which the density of Mie scattering
     *  particles (aerosols) is half that at ground level.
     */
    void setMieScaleHeight(float height)
    {
        m_mieScaleHeight = height;
    }

    /** Get the Rayleigh scattering coefficients at ground level. The returned
      * value is a vector giving scattering per meter at three wavelengths
      * (680nm, 550nm, and 440nm), corresponding to red, green, and blue.
      */
    Eigen::Vector3f rayleighScatteringCoeff() const
    {
        return m_rayleighScatteringCoeff;
    }

    /** Set the Rayleigh scattering coefficients at ground level. The three
      * coefficients give the scattering per meter at three wavelengths
      * (680nm, 550nm, and 440nm), corresponding to red, green, and blue.
      * For realistic atmospheres, computeRayleightScatteringCoeff() is more
      * convenient.
      */
    void setRayleighScatteringCoeff(const Eigen::Vector3f& coeff)
    {
        m_rayleighScatteringCoeff = coeff;
    }

    void computeRayleighScatteringCoeff(double n, double N);

    /** Get the Mie scattering coefficient at ground level. Mie scattering
      * is treated as wavelength independent.
      */
    float mieScatteringCoeff() const
    {
        return m_mieScatteringCoeff;
    }

    /** Get the Mie scattering coefficient at ground level. Mie scattering
      * is treated as wavelength independent.
      */
    void setMieScatteringCoeff(float coeff)
    {
        m_mieScatteringCoeff = coeff;
    }

    /** Get the absorption scattering coefficients at ground level. Absorption
      * is due to the Mie scattering particles. The return value is
      * a vector giving extinction per meter at three wavelengths
      * (680nm, 550nm, and 440nm), corresponding to red, green, and blue.
      */
    Eigen::Vector3f absorptionCoeff() const
    {
        return m_absorptionCoeff;
    }

    /** Set the absorption scattering coefficients at ground level. Absorption
      * is due to the Mie scattering particles. The return value is
      * a vector giving extinction per meter at three wavelengths
      * (680nm, 550nm, and 440nm), corresponding to red, green, and blue.
      */
    void setAbsorptionCoeff(const Eigen::Vector3f& coeff)
    {
        m_absorptionCoeff = coeff;
    }

    /** Get the value of the Mie asymmetry parameter. This value is typically called
      * 'g' in a phase function.
      */
    float mieAsymmetry() const
    {
        return m_mieAsymmetry;
    }

    /** Set the value of the Mie asymmetry parameter. This value is typically called
      * 'g' in a phase function. g = 0 indicates an isotropic phase function. g > 0
      * describes forward scattering medium, and g < 0 is appropriate for back scattering
      * media. g = 0.76 is a realistic value for the aerosols in Earth's atmosphere.
      */
    void setMieAsymmetry(float g)
    {
        m_mieAsymmetry = g;
    }

    Spectrum color(float distance) const;

    float transparentHeight() const;

    TextureMap* transmittanceTexture() const;
    TextureMap* scatterTexture() const;
    void generateTextures();

    void computeScattering();
    void computeScattering(unsigned int heightSamples,
                           unsigned int viewAngleSamples,
                           unsigned int sunAngleSamples);

    void SaveAtmScat(char* filename);

    static const double IndexOfRefraction_Air_0;
    static const double IndexOfRefraction_Air_15;
    static const double MolecularDensity_Air_0;
    static const double MolecularDensity_Air_15;

    static const unsigned int DefaultTransmittanceTableHeightSamples      =  128;
    static const unsigned int DefaultTransmittanceTableViewAngleSamples   = 1024;
    static const unsigned int DefaultScatterTableHeightSamples            =   32;
    static const unsigned int DefaultScatterTableViewAngleSamples         =  256;
    static const unsigned int DefaultScatterTableSunAngleSamples          =   32;

    static Atmosphere* LoadAtmScat(const DataChunk* data);

private:
    void computeTransmittanceTable(unsigned int heightSamples, unsigned int viewAngleSamples);
    void computeInscatterTable(unsigned int heightSamples, unsigned int viewAngleSamples, unsigned int sunAngleSamples);
    void generateTransmittanceTexture();
    void generateInscatterTexture();

    Eigen::Vector3f transmittance(float r, float cosZenithAngle, float pathLength) const;
    Eigen::Vector3f transmittance(float r, float cosZenithAngle) const;

private:
    float m_planetRadius;
    float m_rayleighScaleHeight;
    Eigen::Vector3f m_rayleighScatteringCoeff;
    float m_mieScaleHeight;
    float m_mieScatteringCoeff;
    float m_mieAsymmetry;
    Eigen::Vector3f m_absorptionCoeff;

    std::vector<Eigen::Vector3f> m_transmittanceTable;
    std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f> > m_inscatterTable;

    counted_ptr<TextureMap> m_transmittanceTexture;
    counted_ptr<TextureMap> m_scatterTexture;

    unsigned int m_transmittanceHeightSamples;
    unsigned int m_transmittanceViewAngleSamples;
    unsigned int m_scatterHeightSamples;
    unsigned int m_scatterViewAngleSamples;
    unsigned int m_scatterSunAngleSamples;
};

}

#endif // _VESTA_ATMOSPHERE_H_
