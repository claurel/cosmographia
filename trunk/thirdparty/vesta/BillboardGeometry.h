/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BILLBOARD_GEOMETRY_H_
#define _VESTA_BILLBOARD_GEOMETRY_H_

#include <Eigen/Core>
#include "Geometry.h"
#include "Material.h"
#include <vector>

namespace vesta
{
/** BillboardGeometry is a geometry type used for drawing screen aligned, textured
  * squares, for icons and similar items.
  */
class BillboardGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    BillboardGeometry();
    ~BillboardGeometry();

    void render(RenderContext& rc,
                double clock) const;

    float boundingSphereRadius() const;

    float size() const
    {
        return m_size;
    }

    void setSize(float size);

    TextureMap* texture() const;
    void setTexture(TextureMap* texture);
    Spectrum color() const;
    void setColor(const Spectrum& color);
    float opacity() const;
    void setOpacity(float opacity);
    bool hasFixedScreenSize() const
    {
        return m_fixedScreenSize;
    }

    void setFixedScreenSize(bool enable)
    {
        m_fixedScreenSize = enable;
    }

    Material::BlendMode blendMode() const;
    void setBlendMode(Material::BlendMode blendMode);

    bool isOpaque() const;

private:
    float m_size;    
    bool m_fixedScreenSize;
    Material::BlendMode m_blendMode;
    Material m_material;
};

}

#endif // _VESTA_BILLBOARD_GEOMETRY_H_

