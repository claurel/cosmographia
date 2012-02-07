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
#include <vesta/Intersect.h>
#include <Eigen/LU>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


FeatureLabelSetGeometry::FeatureLabelSetGeometry() :
    m_maxFeatureDistance(0.0f),
    m_occludingEllipsoid(Vector3d::Zero())
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
        AlignedEllipsoid testEllipsoid(m_occludingEllipsoid.semiAxes() * 0.999);
        Vector3f ellipsoidSemiAxes = testEllipsoid.semiAxes().cast<float>();

        // Get the position of the camera in the body-fixed frame of the labeled object
        Transform3f inv = Transform3f(rc.modelview().inverse(Affine)); // Assuming an affine modelview matrix
        Vector3f cameraPosition = inv.translation();

        Vector3f viewDir = -cameraPosition.normalized();
        double distanceToEllipsoid = 0.0;
        TestRayEllipsoidIntersection(cameraPosition, viewDir, ellipsoidSemiAxes, &distanceToEllipsoid);

        Hyperplane<float, 3> labelPlane(viewDir, cameraPosition + viewDir * float(distanceToEllipsoid));

        for (vector<Feature>::const_iterator iter = m_features.begin(); iter != m_features.end(); ++iter)
        {
            Vector3f r = iter->position - cameraPosition;

            Vector3f labelPosition = labelPlane.projection(iter->position);
            float k = -(labelPlane.normal().dot(cameraPosition) + labelPlane.offset()) / (labelPlane.normal().dot(r));
            labelPosition = cameraPosition + k * r;

            rc.pushModelView();
            rc.translateModelView(labelPosition);
            float featureDistance = rc.modelview().translation().norm();
            float pixelSize = iter->size / (rc.pixelSize() * featureDistance);

            float d = r.norm();
            r /= d;
            double t = 0.0;
            TestRayEllipsoidIntersection(cameraPosition, r, ellipsoidSemiAxes, &t);

            if (pixelSize > 10.0f && d < t)
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
