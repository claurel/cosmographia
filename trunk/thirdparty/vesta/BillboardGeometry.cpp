/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "BillboardGeometry.h"
#include "RenderContext.h"
#include <Eigen/Core>
#include <cassert>

using namespace vesta;
using namespace Eigen;


/** Construct a new BillboardGeometry. Billboards are rendered as screen-aligned textured
  * squares. The texture may be modified by a color and opacity setting. The billboard can
  * be set to have a either fixed apparent size in pixels or a fixed physical size in kilometers;
  */
BillboardGeometry::BillboardGeometry() :
    m_fixedScreenSize(true)
{
    m_material.setEmission(Spectrum(1.0f, 1.0f, 1.0f));
}


BillboardGeometry::~BillboardGeometry()
{
}


float
BillboardGeometry::boundingSphereRadius() const
{
    if (!m_fixedScreenSize)
    {
        return m_size;
    }
    else
    {
        return 0.001f;
    }
}


void
BillboardGeometry::render(RenderContext& rc, double /* clock */) const
{
    // Render during the opaque pass if opaque or during the translucent pass if not.
    if ((rc.pass() == RenderContext::TranslucentPass) ^ isOpaque())
    {
        float scale = m_size;
        if (m_fixedScreenSize)
        {
            float cameraDistance = rc.modelview().translation().norm();
            scale *= rc.pixelSize() * cameraDistance;
        }

        rc.bindMaterial(&m_material);
        rc.drawBillboard(Vector3f::Zero(), scale);
    }
}


/** Set the size of the billboard. The interpretation of size depends on the setting of the
  * fixedScreenSize flag. If fixedScreenSize is true, size is in pixels. Otherwise, the
  * apparent size of the billboard shrinks with increasing distance (like ordinary geometry)
  * and the size is in kilometers.
  */
void
BillboardGeometry::setSize(float size)
{
    m_size = size;
}


/** Get the billboard texture.
  */
TextureMap*
BillboardGeometry::texture() const
{
    return m_material.baseTexture();
}


/** Set the billboard texture.
  */
void
BillboardGeometry::setTexture(TextureMap* texture)
{
    m_material.setBaseTexture(texture);
}


/** Get the opacity of the billboard. The opacity is multiplied
  * with the alpha channel (if any) of the billboard texture.
  */
float
BillboardGeometry::opacity() const
{
    return m_material.opacity();
}


/** Set the opacity of the billboard. The opacity is multiplied
  * with the alpha channel (if any) of the billboard texture.
  */
void
BillboardGeometry::setOpacity(float opacity)
{
    m_material.setOpacity(opacity);
}


/** Get the color that modifies the billboard texture.
  */
Spectrum
BillboardGeometry::color() const
{
    return m_material.emission();
}


/** Set the color that will modify the billboard texture. By default, the
  * color is white, which will leave the texture unmodified. The color is
  * multiplied (modulated) with the texture colors.
  */
void
BillboardGeometry::setColor(const Spectrum& color)
{
    m_material.setEmission(color);
}


/** Get the blend mode that will be used to draw the billboard.
  */
Material::BlendMode
BillboardGeometry::blendMode() const
{
    return m_material.blendMode();
}


/** Set the blend mode that will be used to draw the billboard.
  */
void
BillboardGeometry::setBlendMode(Material::BlendMode blendMode)
{
    m_material.setBlendMode(blendMode);
}


bool
BillboardGeometry::isOpaque() const
{
    return opacity() == 1.0f && m_material.blendMode() != Material::AdditiveBlend;
}
