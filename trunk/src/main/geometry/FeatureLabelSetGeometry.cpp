// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#include "FeatureLabelSetGeometry.h"
#include <vesta/RenderContext.h>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


FeatureLabelSetGeometry::FeatureLabelSetGeometry() :
    m_maxFeatureDistance(0.0f)
{
}


FeatureLabelSetGeometry::~FeatureLabelSetGeometry()
{
}


/** \reimpl
  */
void
FeatureLabelSetGeometry::render(RenderContext& rc, double /* clock */) const
{
    // Render during the opaque pass if opaque or during the translucent pass if not.
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        for (vector<Feature>::const_iterator iter = m_features.begin(); iter != m_features.end(); ++iter)
        {
            rc.pushModelView();
            rc.translateModelView(iter->position);
            float cameraDistance = rc.modelview().translation().norm();
            float pixelSize = iter->size / (rc.pixelSize() * cameraDistance);

            if (pixelSize > 10.0f)
            {
                rc.drawText(Vector3f::Zero(), iter->label, m_font.ptr(), Spectrum(1.0f, 1.0f, 1.0f), 1.0f);
            }

            rc.popModelView();
        }
    }
}


/** \reimpl
  */
float
FeatureLabelSetGeometry::boundingSphereRadius() const
{
    return m_maxFeatureDistance;
}


/** Add a new labeled feature
  *
  * \param a UTF-8 string containing the feature name
  * \param the position in the body-fixed frame of planet to which the label set will be attached
  * \param radius size of the feature in units of kilometers
  */
void
FeatureLabelSetGeometry::addFeature(const string& label, const Vector3f& position, float radius)
{
    Feature feature;
    feature.label = label;
    feature.position = position;
    feature.size = radius;

    m_features.push_back(feature);

    m_maxFeatureDistance = max(m_maxFeatureDistance, position.norm());
}
