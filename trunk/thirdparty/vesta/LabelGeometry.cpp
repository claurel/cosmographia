/*
 * $Revision: 672 $ $Date: 2012-04-30 21:09:59 -0700 (Mon, 30 Apr 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "LabelGeometry.h"
#include "RenderContext.h"
#include "Material.h"
#include <Eigen/Core>

using namespace vesta;
using namespace Eigen;
using namespace std;


LabelGeometry::LabelGeometry() :
    m_iconColor(Spectrum::White())
{
    setFixedApparentSize(true);
}


LabelGeometry::LabelGeometry(const std::string& text, TextureFont* font, const Spectrum& color, float iconSize) :
    m_text(text),
    m_font(counted_ptr<TextureFont>(font)),
    m_color(color),
    m_opacity(1.0f),
    m_iconSize(iconSize),
    m_iconColor(Spectrum::White()),
    m_fadeSize(1.0f)
{
    setFixedApparentSize(true);
    //setClippingPolicy(ZeroExtent);
}


LabelGeometry::~LabelGeometry()
{
}


void
LabelGeometry::render(RenderContext& rc, double /* clock */) const
{
    bool hasIcon = !m_icon.isNull();

    Vector3f labelOffset = Vector3f::Zero();
    if (hasIcon)
    {
        labelOffset.x() = std::floor(m_iconSize / 2.0f) + 1.0f;
    }

    float opacity = 0.99f * m_opacity;
    if (m_fadeRange.isValid())
    {
        float cameraDistance = rc.modelview().translation().norm();
        float pixelSize = m_fadeSize / (rc.pixelSize() * cameraDistance);

        opacity *= m_fadeRange->opacity(pixelSize);
    }

    if (opacity == 0.0f)
    {
        return;
    }

    // Render during the opaque pass if opaque or during the translucent pass if not.
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        // Keep the screen size of the icon fixed by adding a scale factor equal
        // to the distance from the eye.
        float distanceScale = rc.modelview().translation().norm();

        // Draw the label string as long as it's not empty
        if (!m_text.empty())
        {
            rc.drawText(labelOffset, m_text, m_font.ptr(), m_color, opacity);
        }

        if (hasIcon)
        {
            Material material;
            material.setEmission(m_iconColor);
            material.setOpacity(opacity);
            material.setBaseTexture(m_icon.ptr());
            rc.bindMaterial(&material);
            rc.drawBillboard(Vector3f::Zero(), m_iconSize * rc.pixelSize() * distanceScale);
        }
    }
}


float
LabelGeometry::boundingSphereRadius() const
{
    return 0.1f;
}


float
LabelGeometry::apparentSize() const
{
    // TODO: currently, the size of the label is ignored and just the icon is considered
    if (m_icon.isNull())
    {
        return 0.0f;
    }
    else
    {
        return m_iconSize;
    }
}


void
LabelGeometry::setFadeRange(FadeRange *fadeRange)
{
    m_fadeRange = fadeRange;
}
