/*
 * $Revision: 597 $ $Date: 2011-03-31 09:25:53 -0700 (Thu, 31 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "LabelVisualizer.h"
#include "PickContext.h"

using namespace vesta;
using namespace Eigen;


LabelVisualizer::LabelVisualizer(const std::string& text, TextureFont* font, const Spectrum& color, float iconSize) :
    Visualizer(NULL)
{
    LabelGeometry* label = new LabelGeometry(text, font, color, iconSize);
    setGeometry(label);
    m_label = label;
}


LabelVisualizer::~LabelVisualizer()
{
}


bool
LabelVisualizer::handleRayPick(const PickContext* pc, const Eigen::Vector3d& pickOrigin, double /* t */) const
{
    if (m_label.isNull())
    {
        return false;
    }

    // Test to see if the label is invisible because it has a fade range
    if (m_label->fadeRange())
    {
        double cameraDistance = pickOrigin.norm();
        float pixelSize = m_label->fadeSize() / float(pc->pixelAngle() * cameraDistance);

        if (m_label->fadeRange()->opacity(pixelSize) < 0.3f)
        {
            return false;
        }
    }

    float pickAdjust = m_label->pickSizeAdjustment();
    
    double cosAngle = pc->pickDirection().dot(-pickOrigin.normalized());
    if (cosAngle > 0.0)
    {
        // Return true if the label marker was clicked
        if (cosAngle >= 1.0 || acos(cosAngle) < (m_label->apparentSize() / 2.0 + pickAdjust) * pc->pixelAngle())
        {
            return true;
        }

        // The marker wasn't clicked; check whether the label text was clicked

        // Compute the intersection between the pick ray and the plane perpendicular to
        // camera plane that posses through the labeled object's center. Transform the
        // problem so that the origin is the object center and the camera plane is z=0
        Matrix3d cameraMatrix = pc->cameraOrientation().conjugate().toRotationMatrix();
        Vector3d rayOrigin = cameraMatrix * pickOrigin;
        Vector3d rayDirection = cameraMatrix * pc->pickDirection();
        double distanceToPlane = -rayOrigin.z() / rayDirection.z();

        Vector3d p = rayOrigin + rayDirection * distanceToPlane;
        float x = (atan(p.x() / rayOrigin.z())) / pc->pixelAngle();
        float y = (atan(p.y() / rayOrigin.z())) / pc->pixelAngle();

        if (m_label->font())
        {
            if (y > -pickAdjust && y < m_label->font()->maxAscent() + pickAdjust)
            {
                if (x > -pickAdjust && x < m_label->font()->textWidth(m_label->text()) + pickAdjust)
                {
                    return true;
                }
            }
        }
    }

    return false;
}
