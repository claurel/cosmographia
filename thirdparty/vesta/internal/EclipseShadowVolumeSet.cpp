/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "EclipseShadowVolumeSet.h"
#include "../Entity.h"
#include "../Geometry.h"
#include <cmath>
#include <cassert>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Eclipse Shadow Implementation Details
//
// The EclipseShadowVolumeSet tracks a list of shadow volumes cast by ellipsoidal
// bodies illuminated by a spherical light source. This allows accurate calculation
// of shadows from large solar system bodies, which can be approximated as
// ellipsoids: generally oblate, but triaxial in the case of some outer planet
// satellites.
//
// For quick intersection testing, we maintain shadow bounding volumes which
// truncated cones with circular cross sections; this bounding volume is identical
// to the shadow volume for a spherical occluding body.
//
// Usage:
// An EclipseShadowVolumeSet is initialized by calling clear method. Then,
//   1. Fill the set with shadow volumes by calling addShadow()
//   2. Call frustumCull to create a list of just those shadows that affect
//      objects contained in a view frustum.
//   3. For each object rendered:
//      a. Call findIntersectingShadows; if it returns false, no shadows affect the object
//      b. Otherwise, check the insideUmbra; if set, the object is completely in shadow and
//         can be rendered correctly by simply disabling the Sun
//      c. If the object is not completely in shadow, call intersectingShadows() to get
//         the list and handle each appropriately (e.g. by setting up the required shader
//         parameters.)
//
// Notes:
//   - clear() should be called for each frame rendered.
//   - There is currently no spatial hierarchy created for shadow volumes; the
//     number of ellipsoidal bodies in a solar system is expected to be low. However,
//     the class interface was designed so that it doesn't need to be changed if some
//     structure to accelerate intersection testing is eventually added.

// Test whether the cone completely contains a sphere
static bool
coneContainsSphere(const Vector3d& apex,
                   const Vector3d& direction,
                   double coneLength,
                   double cosConeAngle,
                   double sinConeAngle,
                   const Vector3d& center,
                   double r)
{
    Vector3d p = center - apex;

    // Since the cone is radially symmetric and the is completely symmetric, we can
    // reduce the problem to two dimensions by projecting it into the plane containing
    // the cone axis and the sphere center.

    // Compute the projected coordinates of the sphere center. The origin of the 2D coordinate
    // system is the cone apex and the x-axis is the cone axis. The x coordinate is the distance
    // from the apex to the projection of the sphere center onto the cone axis. Y is the distance
    // from the sphere center to the cone axis.
    Vector2d pp(p.dot(direction), p.cross(direction).norm());
    Vector2d n(sinConeAngle, -cosConeAngle);

    // Test against one line of the projected cone
    double signedDist = n.dot(pp);
    if (signedDist <= r)
    {
        return false;
    }

    // Test the other line
    n.y() = -n.y();
    signedDist = n.dot(pp);
    if (signedDist <= r)
    {
        return false;
    }

    // Test against the front plane
    if (pp.x() - r <= 0.0)
    {
        return false;
    }

    // Test against the back plane
    if (pp.x() + r >= coneLength)
    {
        return false;
    }

    return true;
}



EclipseShadowVolumeSet::EclipseShadowVolumeSet() :
    m_insideUmbra(false)
{
}


EclipseShadowVolumeSet::~EclipseShadowVolumeSet()
{
}


/** Clear the list of shadow volumes. This should be called
  * before rendering each frame.
  */
void
EclipseShadowVolumeSet::clear()
{
    m_allShadows.clear();
    m_frustumShadows.clear();
    m_intersectingShadows.clear();
}


/** Generate the list of shadow volumes to test against by
  * filtering out shadows that don't intersect the view
  * frustum.
  *
  * \returns true if there were any shadows intersecting the frustum
  */
bool
EclipseShadowVolumeSet::frustumCull(const Frustum& /* frustum */)
{
    m_frustumShadows.clear();

    for (unsigned int i = 0; i < m_allShadows.size(); ++i)
    {
        // This step isn't implemented yet. For now, we'll just copy every
        // shadow into the frustum intersection list
        m_frustumShadows.push_back(&m_allShadows.at(i));
    }

    return !m_frustumShadows.empty();
}


/** Find all shadows intersecting a given sphere. The list of
  * intersecting shadows is available via the intersectingShadows()
  * method. Calling this method will also set the insideUmbra flag
  * if the sphere lies completely inside the umbra of any shadow
  * volume.
  *
  * \returns true if there were any intersections, false if not
  */
bool
EclipseShadowVolumeSet::findIntersectingShadows(const Entity* entity, const Vector3d& sphereCenter, double sphereRadius)
{
    m_intersectingShadows.clear();
    m_insideUmbra = false;

    for (vector<ConicShadowVolume*>::const_iterator iter = m_frustumShadows.begin(); iter != m_frustumShadows.end(); ++iter)
    {
        ConicShadowVolume* cone = *iter;
        if (entity != cone->occluder && coneIntersectsSphere(*cone, sphereCenter, sphereRadius))
        {
            bool planarOccluder = cone->occluder->geometry()->ellipsoid().isDegenerate();

            // Only compute the ellipse the first time that it's needed; for most shadow volumes,
            // ellipse will never be required because no objects will lie within cone.
            if (!cone->ellipseComputed)
            {
                Matrix3d r = cone->orientation.cast<double>().toRotationMatrix();

                if (planarOccluder)
                {
                    // For planar occluders, we'll just store the actual ellipse rather
                    // than the projection. For now, we assume the occluder lies in the
                    // xy-plane.
                    const AlignedEllipsoid& ellipsoid = cone->occluder->geometry()->ellipsoid();
                    cone->ellipse = GeneralEllipse(cone->center,
                                                   r * (Vector3d::UnitX() * ellipsoid.semiAxes().x()),
                                                   r * (Vector3d::UnitY() * ellipsoid.semiAxes().y()));
                }
                else
                {
                    // Calculate the limb of the occluding body as seen from the apex of
                    // the shadow cone. We rotate the apex into the fixed frame of the occluder,
                    // compute the limb ellipse, then rotate that ellipse back into the ICRF.
                    Vector3d p = r.transpose() * (cone->apex - cone->center);
                    GeneralEllipse projection = cone->occluder->geometry()->ellipsoid().orthogonalProjection(p.normalized());

                    projection = GeneralEllipse(r * projection.center(), r * projection.v0(), r * projection.v1());
                    Matrix<double, 3, 2> projAxes = projection.principalSemiAxes();
                    cone->ellipse = GeneralEllipse(projection.center() + cone->center,
                                                   projAxes.col(0),
                                                   projAxes.col(1));
                }

                cone->ellipseComputed = true;
            }

            EclipseShadow shadow;
            shadow.occluder    = cone->occluder;
            shadow.direction   = cone->direction;
            shadow.position    = cone->center;
            shadow.projection  = cone->ellipse;

            // TODO: Remove the assumption that the light source is larger than the occluder;
            // the negation below shouldn't be necessary.
            shadow.umbraSlope    = float(-cone->sinUmbraConeAngle / cone->cosUmbraConeAngle);
            shadow.penumbraSlope = float(cone->sinAngle / cone->cosAngle);

            m_intersectingShadows.push_back(shadow);

            // Check whether the object lies completely inside the shadow umbra, i.e. it
            // receives no light at all from the light source.
            // We treat degenerate ellipsoids specially; they are used to represent ring
            // shadows, which will not completely obscure light.
            if (!planarOccluder &&
                coneContainsSphere(cone->center + cone->umbraLength * cone->direction,
                                   -cone->direction,
                                   cone->umbraLength,
                                   cone->cosUmbraConeAngle,
                                   cone->sinUmbraConeAngle,
                                   sphereCenter,
                                   sphereRadius))
            {
                m_insideUmbra = true;
            }
        }
    }

    return !m_intersectingShadows.empty();
}


static const double MaxShadowVolumeExtent = 1.0e12;

// When the apparent diameter of the occluder is less that MinCoverage times that of
// the light source, the shadow is considered to faint to be noticeable. This allows
// us to truncate the shadow volumes, thereby making culling more effective.
static const double MinCoverage = 0.1;

/** Add a new shadow volume to the set.
  */
void
EclipseShadowVolumeSet::addShadow(const Entity* occluder,
                                  const Vector3d& occluderPosition,
                                  const Quaternionf& occluderOrientation,
                                  const Vector3d& lightPosition,
                                  double lightRadius)
{
    assert(occluder->geometry());
    assert(occluder->geometry()->isEllipsoidal());

    double occluderRadius = occluder->geometry()->ellipsoid().semiMajorAxisLength();
    Vector3d direction = occluderPosition - lightPosition;
    double d = direction.norm();
    direction /= d;

    // TODO: reject occluders that intersect the light source sphere

    // Calculate the distance of the shadow cone apex from the light source center
    double apexDistance = lightRadius * d / (lightRadius + occluderRadius);

    double cosConeAngle = apexDistance / sqrt(lightRadius * lightRadius + apexDistance * apexDistance);

    // Calculate the cone length. We'll truncate at the point that any shadow would
    // be unnoticeably faint because the occluding object appears so much smaller than
    // the light source. Of course, this will never happen when the occluder is
    // larger than the light source.
    double coneLength;
    double cR = MinCoverage * lightRadius;
    if (cR > occluderRadius)
    {
        double minCoverageDistance = (cR * d) / (cR - occluderRadius);
        coneLength = minCoverageDistance - apexDistance;
    }
    else
    {
        // Occluder is too large relative to light source; minimum coverage condition
        // will never be met so just use the max. shadow volume extent
        coneLength = MaxShadowVolumeExtent;
    }

    ConicShadowVolume cone;
    cone.occluder = occluder;
    cone.center = occluderPosition;
    cone.apex = lightPosition + direction * apexDistance;
    cone.direction = direction;
    cone.front = d - apexDistance;
    cone.back = coneLength;
    cone.cosAngle = cosConeAngle;
    cone.sinAngle = sqrt(max(0.0, 1.0 - cosConeAngle * cosConeAngle));

    // Calculate the umbra cone parameters
    double r = occluder->geometry()->ellipsoid().semiAxes().minCoeff();
    if (lightRadius > r)
    {
        double u = d * (lightRadius / (lightRadius - r) - 1.0);

        cone.umbraLength = u;
        cone.cosUmbraConeAngle = u / sqrt(r * r + u * u);
        cone.sinUmbraConeAngle = sqrt(max(0.0, 1.0 - cone.cosUmbraConeAngle * cone.cosUmbraConeAngle));
    }
    else
    {
        cone.umbraLength = MaxShadowVolumeExtent;
        cone.cosUmbraConeAngle = 0.99999;
        cone.sinUmbraConeAngle = sqrt(max(0.0, 1.0 - cone.cosUmbraConeAngle * cone.cosUmbraConeAngle));
    }

    cone.orientation = occluderOrientation;
    cone.ellipseComputed = false;

    m_allShadows.push_back(cone);
}


bool
EclipseShadowVolumeSet::coneIntersectsSphere(const ConicShadowVolume& cone, const Vector3d& center, double r)
{
    Vector3d p = center - cone.apex;

    // Since the cone is radially symmetric and the sphere is completely symmetric, we can
    // reduce the problem to two dimensions by projecting it into the plane containing
    // the cone axis and the sphere center.

    // Compute the projected coordinates of the sphere center. The origin of the 2D coordinate
    // system is the cone apex and the x-axis is the cone axis. The x coordinate is the distance
    // from the apex to the projection of the sphere center onto the cone axis. Y is the distance
    // from the sphere center to the cone axis.
    Vector2d pp(p.dot(cone.direction), p.cross(cone.direction).norm());

    Vector2d n(cone.sinAngle, -cone.cosAngle);

    // TODO: Replace two tests below with one: n.x() * pp.x() + abs(n.y()) * pp.y() < -r;
    // Test against one line of the projected cone
    double signedDist = n.dot(pp);
    if (signedDist <= -r)
    {
        return false;
    }

    // Test the other line
    n.y() = -n.y();
    signedDist = n.dot(pp);
    if (signedDist <= -r)
    {
        return false;
    }

    // Test against the front plane
    if (pp.x() + r <= cone.front)
    {
        return false;
    }

    // Test against the back plane
    if (pp.x() - r >= cone.back)
    {
        return false;
    }

    // The sphere is completely outside the cone, but it may still intersect the sphere
    // that is the 'nose' of the shadow volume.
    // NOT YET IMPLEMENTED

    return true;
}


#if 0
// For use when frustum culling shadow volumes
static bool cullConePlane(const Vector3d& coneApex, const Vector3d& coneDirection, double cosConeAngle, double coneH,
                          const Hyperplane<double, 3>& plane)
{
    double NdC = plane.normal().dot(coneApex);
    double NdD = plane.normal().dot(coneDirection);
    double root = sqrt(1.0 - NdD * NdD);
    if (root > cosConeAngle && NdD < 0)
    {
        return NdC < plane.offset();
    }

    // double r =  h * NdD + coneH * tanConeAngle * root;
    // return NdC + r < plane.offset();
    return true;
}
#endif
