/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_WORLD_LAYER_H_
#define _VESTA_WORLD_LAYER_H_

#include "Object.h"


namespace vesta
{

class RenderContext;
class QuadtreeTile;
class WorldGeometry;

/** A WorldLayer contains geometry that is overlaid on the surface of a
  * globe (WorldGeometry.) The layer is drawn as a set of spherical surface
  * tiles. Subclasses must implement the renderTile method. A WorldLayer
  * by default not visible. To make the layer visible, call setVisibility(true).
  */
class WorldLayer : public Object
{
public:
    WorldLayer() :
        m_visible(false),
        m_drawOrder(0)
    {
    }

    virtual ~WorldLayer()
    {
    }

    /** Subclasses must implement renderTile. The method is only called for
      * visible tiles, so WorldLayer implementations don't need to perform
      * their own visibility culling.
      */
    virtual void renderTile(RenderContext& rc, const WorldGeometry* world, const QuadtreeTile* tile) const = 0;

    /** Return true if the layer is visible, false if it is not. */
    bool isVisible() const
    {
        return m_visible;
    }

    /** Set whether the layer should be visible or hidden. */
    void setVisibility(bool visible)
    {
        m_visible = visible;
    }

    /** The draw order defines how overlapping areas of world layers will be drawn. A
      * world layer that has a higher draw order will be drawn on top of a layer with
      * a lower draw order.
      */
    int drawOrder() const
    {
        return m_drawOrder;
    }

    /** Set the draw order for this layer. The draw order defines how overlapping areas
      * of world layers will be drawn. A world layer that has a higher draw order will be
      * drawn on top of a layer with a lower draw order. The default draw order
      * is zero.
      */
    void setDrawOrder(int order)
    {
        m_drawOrder = order;
    }

private:
    bool m_visible;
    int m_drawOrder;
};

}

#endif // _VESTA_WORLD_LAYER_H_
