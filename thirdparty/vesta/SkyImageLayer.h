/*
 * $Revision: 408 $ $Date: 2010-08-03 14:38:16 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SKY_IMAGE_LAYER_H_
#define _VESTA_SKY_IMAGE_LAYER_H_

#include "SkyLayer.h"
#include "Spectrum.h"
#include <Eigen/Geometry>


namespace vesta
{

class TextureMap;
class QuadtreeTileAllocator;

class SkyImageLayer : public SkyLayer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    SkyImageLayer();
    ~SkyImageLayer();

    /** Get the orientation of the image layer (with respect to the
      * EME J2000 frame.)
      */
    Eigen::Quaterniond orientation() const
    {
        return m_orientation;
    }

    /** Get the orientation of the image layer (with respect to the
      * EME J2000 frame.)
      */
    void setOrientation(const Eigen::Quaterniond& orientation)
    {
        m_orientation = orientation;
    }

    /** Get the opacity of the layer.
      */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set the opacity of the layer.
      */
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    /** Get the tint color that will be applied to the image. */
    Spectrum tintColor() const
    {
        return m_tintColor;
    }

    /** Set the tint color that will be applied to the image. The image
      * colors are multiplied by the tint color.
      */
    void setTintColor(const Spectrum& color)
    {
        m_tintColor = color;
    }

    /** Get the texture.
      */
    TextureMap* texture() const
    {
        return m_texture.ptr();
    }

    /** Set the texture.
      */
    void setTexture(TextureMap* texture);

    virtual void render(RenderContext& rc);

private:
    Eigen::Quaterniond m_orientation;
    float m_opacity;
    Spectrum m_tintColor;
    counted_ptr<TextureMap> m_texture;

    // TODO: move tile allocator to RenderContext so that it can be shared
    QuadtreeTileAllocator* m_tileAllocator;
};

}
#endif // _VESTA_SKY_IMAGE_LAYER_H_
