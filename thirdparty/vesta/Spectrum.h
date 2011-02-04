/*
 * $Revision: 394 $ $Date: 2010-07-30 11:50:49 -0700 (Fri, 30 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */
#ifndef _VESTA_SPECTRUM_H_
#define _VESTA_SPECTRUM_H_

#include <Eigen/Core>


namespace vesta
{

/** Spectrum represents the spectral power distribution of emission from a light source
  * or reflectance from a surface. Since VESTA is designed for graphics hardware, the
  * implementation is simple, with a single floating point value for wavelengths
  * representing red, green, and blue.
  */
class Spectrum
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Spectrum() :
        m_samples(Eigen::Vector4f::Zero())
    {
    }

    Spectrum(float r, float g, float b) :
        m_samples(Eigen::Vector4f(r, g, b, 0.0f))
    {
    }

    explicit Spectrum(float* data) :
        m_samples(data[0], data[1], data[2], 0.0f)
    {
    }

    bool operator==(const Spectrum& other) const
    {
        return this->m_samples == other.m_samples;
    }

    Spectrum operator+(const Spectrum& other) const
    {
        return Spectrum(this->m_samples + other.m_samples);
    }

    Spectrum operator-(const Spectrum& other) const
    {
        return Spectrum(this->m_samples - other.m_samples);
    }

    Spectrum operator*(const Spectrum& other) const
    {
        return Spectrum(this->m_samples.cwise() * other.m_samples);
    }

    Spectrum operator*(float f) const
    {
        return Spectrum(m_samples * f);
    }

    const float* data() const
    {
        return m_samples.data();
    }

    bool isBlack() const
    {
        return m_samples.isZero();
    }

    float red() const
    {
        return m_samples.x();
    }

    float green() const
    {
        return m_samples.y();
    }

    float blue() const
    {
        return m_samples.z();
    }

    void normalize();

    static Spectrum White()
    {
        return Spectrum(1.0f, 1.0f, 1.0f);
    }

    static Spectrum Black()
    {
        return Spectrum(0.0f, 0.0f, 0.0f);
    }

    static Spectrum Flat(float f)
    {
        return Spectrum(f, f, f);
    }

    static Spectrum XYZtoLinearSRGB(const Spectrum& xyz);
    static Spectrum LinearSRGBtoXYZ(const Spectrum& srgb);
    static Spectrum LinearSRGBtoSRGB(const Spectrum& srgb);
    static Spectrum SRGBtoLinearSRGB(const Spectrum& srgb);

private:
    Spectrum(const Eigen::Vector4f& v) :
        m_samples(v)
    {
    }

private:
    // Use a Vector4 instead of a Vector3 so that the data is aligned, thus making
    // operations vectorizable.
    Eigen::Vector4f m_samples;
};

}

#endif // _VESTA_SPECTRUM_H_
