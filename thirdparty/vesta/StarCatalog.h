/*
 * $Revision: 405 $ $Date: 2010-08-03 13:05:54 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_STAR_CATALOG_H_
#define _VESTA_STAR_CATALOG_H_

#include "Entity.h"
#include "Spectrum.h"
#include "IntegerTypes.h"
#include <vector>

namespace vesta
{

class StarCatalog : public Object
{
public:
    StarCatalog();
    ~StarCatalog();

    unsigned int size() const
    {
        return m_starData.size();
    }

    void addStar(v_uint32 identifier, double ra, double dec, double vmag, double bv);
    void buildCatalogIndex();

public:
    struct StarRecord
    {
        v_uint32 identifier;
        float RA;
        float declination;
        float apparentMagnitude;
        float bvColorIndex;
    };

    const StarRecord& star(unsigned int index)
    {
        return m_starData[index];
    }

    const StarRecord* findStarIdentifier(v_uint32 id);

    static Spectrum StarColor(float bv);

private:
    std::vector<StarRecord> m_starData;
};

}

#endif // _VESTA_STAR_CATALOG_H_
