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
LabelVisualizer::handleRayPick(const Eigen::Vector3d& pickOrigin, const Eigen::Vector3d& pickDirection, double pixelAngle) const
{
    if (m_label.isNull())
    {
        return false;
    }

    // Test to see if the label is invisible because it has a fade range
    if (m_label->fadeRange())
    {
        double cameraDistance = pickOrigin.norm();
        float pixelSize = m_label->fadeSize() / float(pixelAngle * cameraDistance);

        if (m_label->fadeRange()->opacity(pixelSize) < 0.1f)
        {
            return false;
        }
    }

    double cosAngle = pickDirection.dot(-pickOrigin.normalized());
    if (cosAngle > 0.0)
    {
        if (cosAngle >= 1.0 || acos(cosAngle) < m_label->apparentSize() / 2.0 * pixelAngle)
        {
            return true;
        }
    }

    return false;

}
