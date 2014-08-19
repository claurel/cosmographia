// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FeatureLabelSetGeometry.h"
#include <vesta/RenderContext.h>
#include <vesta/Intersect.h>
#include <Eigen/LU>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


float FeatureLabelSetGeometry::ms_globalOpacity = 1.0f;


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
    const float visibleSizeThreshold = 20.0f; // in pixels

    // No need to draw anything if the labels are turned off with an opacity
    // setting near 0.
    if (ms_globalOpacity <= 0.01f)
    {
        return;
    }

    // Render during the opaque pass if opaque or during the translucent pass if not.
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        // Get the position of the camera in the body-fixed frame of the labeled object
        Transform3f inv = Transform3f(rc.modelview().inverse(Affine)); // Assuming an affine modelview matrix
        Vector3f cameraPosition = inv.translation();
        float overallPixelSize = boundingSphereRadius() / (rc.pixelSize() * cameraPosition.norm());

        // Only draw individual labels if the overall projected size of the set exceeds the threshold
        if (overallPixelSize > visibleSizeThreshold)
        {
            // Labels are treated as either completely visible or completely occluded. A label is
            // visible when the labeled point isn't blocked by the occluding ellipsoid.
            AlignedEllipsoid testEllipsoid(m_occludingEllipsoid.semiAxes() * 0.999);
            Vector3f ellipsoidSemiAxes = testEllipsoid.semiAxes().cast<float>();

            Vector3f viewDir = -cameraPosition.normalized();
            double distanceToEllipsoid = 0.0;

            // Instead of computing the ellipsoid intersection (as the line below), just treat the planet as a sphere
            //TestRayEllipsoidIntersection(cameraPosition, viewDir, ellipsoidSemiAxes, &distanceToEllipsoid);
            distanceToEllipsoid = (cameraPosition.norm() - ellipsoidSemiAxes.maxCoeff()) * 0.99f;

            // We don't want labels partially hidden by the planet ellipsoid, so we'll project them onto a
            // plane that lies just in front of the planet ellipsoid and which is parallel to the view plane
            Hyperplane<float, 3> labelPlane(viewDir, cameraPosition + viewDir * float(distanceToEllipsoid));

            for (vector<Feature, Eigen::aligned_allocator<Feature> >::const_iterator iter = m_features.begin(); iter != m_features.end(); ++iter)
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

                if (pixelSize > visibleSizeThreshold && d < t)
                {
                    rc.drawEncodedText(Vector3f::Zero(), iter->label, m_font.ptr(), TextureFont::Utf8, iter->color, ms_globalOpacity);
                }

                rc.popModelView();
            }
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
FeatureLabelSetGeometry::addFeature(const string& label, const Vector3f& position, float radius, const Spectrum& color)
{
    Feature feature;
    feature.label = label;
    feature.position = position;
    feature.size = radius;
    feature.color = color;

    m_features.push_back(feature);

    m_maxFeatureDistance = max(m_maxFeatureDistance, position.norm());
}
