/*
 * $Revision: 315 $ $Date: 2010-06-29 17:35:39 -0700 (Tue, 29 Jun 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_LIGHTING_ENVIRONMENT_H_
#define _VESTA_LIGHTING_ENVIRONMENT_H_

#include "TextureMap.h"
#include "BoundingSphere.h"
#include <vector>

namespace vesta
{

class ReflectionRegion
{
public:
    BoundingSphere<float> region;
    TextureMap* cubeMap;
};

// This class will eventually store additional state required
// during a render pass, including:
//    - Light positions
//    - Atmospheres
//    - Shadow regions
//    - Any other information required to render geometry that's
//      not contained in the geometry object itself.

/** LightingEnvironment contains state required during a render pass by
  * UniverseRender.
  */
class LightingEnvironment
{
public:
    LightingEnvironment()
    {
    }

    ~LightingEnvironment()
    {
    }

    /** Clear all render environment state.
      */
    void reset();

    const std::vector<ReflectionRegion>& reflectionRegions() const
    {
        return m_reflectionRegions;
    }

    std::vector<ReflectionRegion>& reflectionRegions()
    {
        return m_reflectionRegions;
    }

private:
    std::vector<ReflectionRegion> m_reflectionRegions;
};

}

#endif // _VESTA_RENDER_ENVIRONMENT_H_
