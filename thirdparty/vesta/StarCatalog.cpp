/*
 * $Revision: 684 $ $Date: 2012-07-16 21:12:16 -0700 (Mon, 16 Jul 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "StarCatalog.h"
#include "Spectrum.h"
#include "Debug.h"
#include <Eigen/Core>
#include <cmath>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create an empty star catalog.
  */
StarCatalog::StarCatalog()
{
}


StarCatalog::~StarCatalog()
{
}


// Convert a Johnson B-V color index to the effective surface temperature. Uses the
// relation from Sekiguchi and Fukugita, "A Study of the B-V Color-Temperature Relation."
// (Astronomical Journal, Aug 2000).
// http://iopscience.iop.org/1538-3881/120/2/1072/pdf/1538-3881_120_2_1072.pdf
//
// bv is the Johnson bv color index
// metallicity is Fe/H
static float BVColorIndexToTeff(float bv, float metallicity = 0.0f, float logG = 0.0f)
{
    const float c0 = 3.939654f;
    const float c1 = -0.395361f;
    const float c2 = 0.2082113f;
    const float c3 = -0.0604097f;
    const float f1 = 0.027153f;
    const float f2 = 0.005036f;
    const float g1 = 0.007367f;
    const float h1 = -0.01069f;

    float logT = c0 + c1 * bv + c2 * bv * bv + c3 * bv * bv * bv +
                 f1 * metallicity + f2 * metallicity * metallicity +
                 g1 * logG + h1 * bv * logG;

    return pow(10.0f, logT);
}


// Convert the CIE chromaticity coordinates for a black body of the specified
// temperature. The calculation uses a piecewise cubic approximation that is
// valid for temperatures above 1667 K. For cooler temperatures, we simply clamp
// them to 1667 K. This is adequate for our use the function to compute star
// colors as only brown dwarf stars are cooler, and these are so faint that
// they don't need to be represented in VESTA.
static Vector2f planckianLocus(float T)
{
    // Clamp to a valid range
    T = std::max(T, 1667.0f);

    float t = 1000.0f / T;
    float t2 = t * t;
    float t3 = t2 * t;

    float x;
    if (T < 4000.0f)
    {
        x = -0.266162f * t3 - 0.2343580f * t2 + 0.8776956f * t + 0.179910f;
    }
    else
    {
        // Valid from 4000K - 25000K
        x = -3.0258469f * t3 + 2.1070379f * t2 + 0.2226347f * t + 0.24039f;
    }

    float x2 = x * x;
    float x3 = x2 * x;
    float y;
    if (T < 2222)
    {
        // Valid from 1667K - 2222K
        y = -1.1063814f * x3 - 1.3481102f * x2  + 2.18555832f * x - 0.20219683f;
    }
    else if (T < 4000)
    {
        // Valid from 2222K - 4000K
        y = -0.9549976f * x3 - 1.3741859f * x2 + 2.0913702f * x - 0.16748867f;
    }
    else
    {
        // Valid from 4000K - 25000K
        y = 3.0817580f * x3 - 5.8338670f * x2 + 3.75112997f * x - 0.37001483f;
    }

    return Vector2f(x, y);
}


static Vector3f xyToXYZ(const Vector2f& xy)
{
    return Vector3f(xy.x() / xy.y(), 1.0f, (1.0f - xy.x() - xy.y()) / xy.y());
}


static Spectrum linearSrgbStarColor(float bv)
{
    float Teff = BVColorIndexToTeff(bv);
    Vector2f ciexy = planckianLocus(Teff);
    Vector3f cieXYZ = xyToXYZ(ciexy);

    Spectrum srgb = Spectrum::XYZtoLinearSRGB(Spectrum(cieXYZ.x(), cieXYZ.y(), cieXYZ.z()));
    srgb.normalize();
    return srgb;
}


/** Add a new star to the catalog.
  * @param ra the right ascension (in radians)
  * @param dec the declination (in radians)
  * @param vmag the apparent V magnitude in the Johnson photometric system (mean wavelength 540nm)
  * @param bv the value of B-V color index in the Johnson photometric system
  */
void
StarCatalog::addStar(v_uint32 identifier, double ra, double dec, double vmag, double bv)
{
    StarRecord star;
    star.identifier = identifier;
    star.RA = float(ra);
    star.declination = float(dec);
    star.apparentMagnitude = float(vmag);
    star.bvColorIndex = float(bv);

    m_starData.push_back(star);
}


/** Compute the approximate color of a star from it's Johnson B-V color index. The
  * color returned is in the CIE XYZ color space.
  */
Spectrum StarCatalog::StarColor(float bv)
{
    float Teff = BVColorIndexToTeff(bv);
    Vector2f ciexy = planckianLocus(Teff);
    Vector3f cieXYZ = xyToXYZ(ciexy);

    return Spectrum(cieXYZ.x(), cieXYZ.y(), cieXYZ.z());
}


class StarIdPredicate
{
public:
    StarIdPredicate() {}
    bool operator()(const StarCatalog::StarRecord& star0, const StarCatalog::StarRecord& star1) const
    {
        return star0.identifier < star1.identifier;
    }
};


/** Index the star catalog by identifier. This method must be called before star lookups by identifier
  * will work.
  */
void
StarCatalog::buildCatalogIndex()
{
    sort(m_starData.begin(), m_starData.end(), StarIdPredicate());
}


/** Lookup a star by its identifier. Returns null if the star isn't present in the
  * catalog. buildCatalogIndex() must be called once before findStarIdentifier will
  * work.
  */
const StarCatalog::StarRecord*
StarCatalog::findStarIdentifier(v_uint32 id)
{
    StarRecord match;
    match.identifier = id;

    vector<StarRecord>::const_iterator pos = lower_bound(m_starData.begin(), m_starData.end(), match, StarIdPredicate());
    if (pos == m_starData.end() || pos->identifier != id)
    {
        return NULL;
    }
    else
    {
        return &(*pos);
    }
}
