/*
 * $Revision: 375 $ $Date: 2010-07-20 12:25:37 -0700 (Tue, 20 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_CONSTELLATIONS_LAYER_H_
#define _VESTA_CONSTELLATIONS_LAYER_H_

#include "SkyLayer.h"
#include "StarCatalog.h"
#include "IntegerTypes.h"
#include <Eigen/Core>
#include <vector>


namespace vesta
{

class ConstellationsLayer : public SkyLayer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    explicit ConstellationsLayer(StarCatalog* starCatalog);
    ~ConstellationsLayer();

    StarCatalog* starCatalog() const
    {
        return m_starCatalog.ptr();
    }

    virtual void render(RenderContext& rc);

    void setDefaultConstellations();

    /** Get the color of the constellation diagram lines.
      */
    Spectrum diagramColor() const
    {
        return m_diagramColor;
    }

    /** Set the color of the constellation diagram lines.
      */
    void setDiagramColor(const Spectrum& color)
    {
        m_diagramColor = color;
    }

public:
    struct ConstellationSegment
    {
        v_uint32 starId0;
        v_uint32 starId1;
    };

private:
    counted_ptr<StarCatalog> m_starCatalog;
    std::vector<Eigen::Vector3f> m_segments;
    Spectrum m_diagramColor;
};

}
#endif // _VESTA_CONSTELLATIONS_LAYER_H_
