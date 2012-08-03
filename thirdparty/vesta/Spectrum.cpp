/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Spectrum.h"
#include <Eigen/Array>
#include <Eigen/LU>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


static float XYZtoSRGBMatrixValues[9] =
{
    3.2410f, -1.5374f, -0.4986f,
   -0.9692f,  1.8760f,  0.0416f,
    0.0556f, -0.2040f,  1.0570f
};

static Matrix3f XYZtoSRGB = Matrix3f::Map(XYZtoSRGBMatrixValues).transpose();
static Matrix3f SRGBtoXYZ = XYZtoSRGB.inverse();


/** Normalize the spectrum so that the largest component is equal
  * to 1.0.
  */
void
Spectrum::normalize()
{
    float maxValue = m_samples.cwise().abs().maxCoeff();
    if (maxValue > 0.0f)
    {
        m_samples /= maxValue;
    }
}


/** Convert from CIE XYZ color space to linear sRGB. sRGB gamma
  * correction must be applied in order to convert to the standard
  * sRGB color space. \see Spectrum::LinearSRGBtoSRGB
  */
Spectrum
Spectrum::XYZtoLinearSRGB(const Spectrum& xyz)
{
    Vector3f srgb = XYZtoSRGB * xyz.m_samples.start<3>();
    return Spectrum(srgb.x(), srgb.y(), srgb.z());
}


/** Convert from linear sRGB color space to CIE XYZ.
  */
Spectrum
Spectrum::LinearSRGBtoXYZ(const Spectrum& srgb)
{
    Vector3f xyz = SRGBtoXYZ * srgb.m_samples.start<3>();
    return Spectrum(xyz.x(), xyz.y(), xyz.z());
}


static float fromLinearSRGB(float x)
{
    return x < 0.0031308f ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}


static float toLinearSRGB(float x)
{
    return x < 0.04045f ? x / 12.92f : pow((x + 0.055f) / 1.055f, 2.4f);
}


/** Apply the inverse sRGB gamma correction step to convert from 'linear sRGB'
  * color to sRGB color.
  */
Spectrum
Spectrum::LinearSRGBtoSRGB(const Spectrum& srgb)
{
    return Spectrum(fromLinearSRGB(srgb.red()),
                    fromLinearSRGB(srgb.green()),
                    fromLinearSRGB(srgb.blue()));
}


/** Apply the sRGB gamma correction step to convert from sRGB color space to
  * a linear color space that uses the sRGB tristimulus values.
  */
Spectrum
Spectrum::SRGBtoLinearSRGB(const Spectrum& srgb)
{
    return Spectrum(toLinearSRGB(srgb.red()),
                    toLinearSRGB(srgb.green()),
                    toLinearSRGB(srgb.blue()));
}
