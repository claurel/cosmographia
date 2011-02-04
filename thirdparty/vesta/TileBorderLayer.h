/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TILE_BORDER_LAYER_H_
#define _VESTA_TILE_BORDER_LAYER_H_

#include "WorldLayer.h"
#include "Spectrum.h"


namespace vesta
{

/** TileBorderLayer is a world layer that shows the borders of planet tiles resulting
  * from VESTA's quadtree planet renderer. It is intended mainly for debugging of custom
  * world layers.
  */
class TileBorderLayer : public WorldLayer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /** Construct a new planet grid with white grid lines and an opacity of 50%.
      */
    TileBorderLayer();

    ~TileBorderLayer();

    virtual void renderTile(RenderContext& rc, const WorldGeometry* world, const QuadtreeTile* tile) const;

    /** Get the color of the tile borders
      */
    Spectrum color() const
    {
        return m_color;
    }

    /** Set the color of the tile borders
      */
    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    /** Get the opacity of the tile borders
      */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set the opacity of the tile borders
      */
    void setGridOpacity(float opacity)
    {
        m_opacity = opacity;
    }
private:
    Spectrum m_color;
    float m_opacity;
};

}

#endif // _VESTA_TILE_BORDER_LAYER_H_
