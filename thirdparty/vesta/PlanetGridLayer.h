/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANET_GRID_LAYER_H_
#define _VESTA_PLANET_GRID_LAYER_H_

#include "WorldLayer.h"
#include "Spectrum.h"


namespace vesta
{

class TextureFont;

/** PlanetGridLayer draws a longitude/latitude grid with a line spacing that
  * automatically adjusts for the apparent size of the planet. Grid lines fade
  * out completely when the observer is far away from the planet.
  */
class PlanetGridLayer : public WorldLayer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /** Construct a new planet grid with white grid lines and an opacity of 50%.
      */
    PlanetGridLayer();

    ~PlanetGridLayer();

    virtual void renderTile(RenderContext& rc, const WorldGeometry* world, const QuadtreeTile* tile) const;

    /** Get the color of the grid lines.
      */
    Spectrum gridColor() const
    {
        return m_gridColor;
    }

    /** Set the color of the grid lines.
      */
    void setGridColor(const Spectrum& color)
    {
        m_gridColor = color;
    }

    /** Get the opacity of the grid lines.
      */
    float gridOpacity() const
    {
        return m_gridOpacity;
    }

    /** Set the opacity of the grid lines.
      */
    void setGridOpacity(float opacity)
    {
        m_gridOpacity = opacity;
    }

    /** Get the font used for coordinate labels.
      */
    TextureFont* labelFont() const
    {
        return m_labelFont.ptr();
    }

    void setLabelFont(TextureFont* font);

private:
    Spectrum m_gridColor;
    float m_gridOpacity;
    counted_ptr<TextureFont> m_labelFont;
};

}

#endif // _VESTA_PLANET_GRID_LAYER_H_
