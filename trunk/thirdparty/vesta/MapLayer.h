/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_MAP_LAYER_H_
#define _VESTA_MAP_LAYER_H_

#include "Object.h"
#include "TextureMap.h"

namespace vesta
{

class MapLayerBounds
{
public:
    MapLayerBounds();
    MapLayerBounds(double west, double south, double east, double north);

    double east() const
    {
        return m_east;
    }

    double west() const
    {
        return m_west;
    }

    double north() const
    {
        return m_north;
    }

    double south() const
    {
        return m_south;
    }

private:
    double m_west;
    double m_south;
    double m_east;
    double m_north;
};


class MapLayer : public Object
{
public:
    MapLayer();
    ~MapLayer();

    /** Get the texture map used for this layer. */
    TextureMap* texture() const
    {
        return m_texture.ptr();
    }

    /** Set the texture map for this layer. A null texture map means
      * that the layer will be ignored.
      */
    void setTexture(TextureMap* texture)
    {
        m_texture = texture;
    }

    /** Get the opacity of this map layer. */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set the opacity for this layer: 0 for completely transparent (i.e.
      * not visible at all), 1 for opaque (obscures the base texture and
      * all underlying layers.
      */
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    /** Get the rectangular patch of the layer that
      * is actually visible.
      */
    MapLayerBounds box() const
    {
        return m_box;
    }

    /** Set which region of the layer will be visible.
      */
    void setBox(const MapLayerBounds& box)
    {
        m_box = box;
    }


private:
    counted_ptr<TextureMap> m_texture;
    float m_opacity;
    MapLayerBounds m_box;
};

}

#endif // _VESTA_MAP_LAYER_H_
