/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "AlignedEllipsoid.h"
#include <Eigen/LU>
#include <Eigen/QR>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;



static void
planeToSpanningVectors(const Hyperplane<double, 3>& plane,
                       Vector3d& origin, Vector3d& v0, Vector3d& v1)
{
    origin = plane.normal() * -plane.offset();
    v0 = plane.normal().unitOrthogonal();
    v1 = v0.cross(plane.normal());
}


/* Only required for alternate implementation of plane intersection method

static Hyperplane<double, 3> spanningVectorsToPlane(const Vector3d& origin,
                                                    const Vector3d& v0,
                                                    const Vector3d& v1)
{
    Vector3d n = v0.normalized().cross(v1.normalized());
    return Hyperplane<double, 3>(n, n.dot(origin));
}

*/

#define USE_ALTERNATE_INTERSECTION_METHOD 0

/** Compute the of this ellipsoid with a plane. The intersection--if it exists--will
  * be either a point or an ellipse. This method treats the point case as if no
  * intersection occurred.
  *
  * \param p the plane to test for intersection
  * \param foundIntersect set to true if there was an intersection, false otherwise. OK if null.
  */
GeneralEllipse
AlignedEllipsoid::intersection(const Hyperplane<double, 3>& p, bool* foundIntersection) const
{
    // We will reduce the problem to the simpler one of finding the intersection of
    // a plane with a unit sphere:
    //    1. Computing transformation M that maps the ellipsoid to a unit sphere
    //    2. Apply this transformation to the plane
    //    3. Find the intersection of the transformed plane and the unit sphere
    //    4. Apply the inverse transformation to get the ellipse of intersection

    // Compute the transformation that maps the ellipsoid to a unit sphere
    Matrix3d M = m_semiAxes.cwise().inverse().asDiagonal();

    Vector3d plane_origin, plane_v0, plane_v1;
    planeToSpanningVectors(p, plane_origin, plane_v0, plane_v1);

    // Transform the plane
    plane_origin = M * plane_origin;
    plane_v0     = M * plane_v0;
    plane_v1     = M * plane_v1;

#if USE_ALTERNATE_INTERSECTION_METHOD
    Hyperplane<double, 3> p1 = spanningVectorsToPlane(plane_origin, plane_v0, plane_v1);
    planeToSpanningVectors(p1, plane_origin, plane_v0, plane_v1);

    double d = plane_origin.norm();
    if (d < 1.0)
    {
        if (foundIntersection)
        {
            *foundIntersection = true;
        }

        // Compute the radius of the circle of intersection
        double r = 1.0 - d * d;

        // Transform the spanning vectors and center back to the original space
        /*DiagonalMatrix<Vector3d>*/ Matrix3d invM = m_semiAxes.asDiagonal();
        plane_origin = invM * plane_origin;
        plane_v0     = invM * (plane_v0 * r);
        plane_v1     = invM * (plane_v1 * r);

        return GeneralEllipse(plane_origin, plane_v0, plane_v1);
    }
    else
    {
        if (foundIntersection)
        {
            *foundIntersection = false;
        }

        // Return an empty ellipse
        return GeneralEllipse(Vector3d::Zero(), Vector3d::Zero(), Vector3d::Zero());
    }
#else
    Hyperplane<double, 3> p1 = p;
    p1.transform(M);

    if (abs(p1.offset()) < 1.0)
    {
        if (foundIntersection)
        {
            *foundIntersection = true;
        }

        // Compute the radius of the circle of intersection
        double r = 1.0 - p1.offset() * p1.offset();

        // n is the plane normal, v0 and v1 are spanning vectors of the plane
        Vector3d n  = p1.normal();
        Vector3d v0 = n.unitOrthogonal();
        Vector3d v1 = v0.cross(n);

        // Transform the spanning vectors and center back to the original space
        DiagonalMatrix<Vector3d> D = m_semiAxes.asDiagonal();
        Vector3d center = D * (n * p1.offset());
        v0              = D * (v0 * r);
        v1              = D * (v1 * r);

        return GeneralEllipse(center, v0, v1);
    }
    else
    {
        if (foundIntersection)
        {
            *foundIntersection = false;
        }

        // Return an empty ellipse
        return GeneralEllipse(Vector3d::Zero(), Vector3d::Zero(), Vector3d::Zero());
    }
#endif
}


/** Compute the ellipse that is the limb of the ellipsoid when viewed from
  * some point outside the ellipsoid.
  */
GeneralEllipse
AlignedEllipsoid::limb(const Vector3d& p) const
{
    // For a point L on the limb, the surface normal N(L) is
    // perpendicular to the view direction L - P, thus:
    //    (L - P) dot N(L) = 0
    //

    Vector3d n = m_semiAxes.cwise().square().cwise().inverse().asDiagonal() * p;
    double mag = n.norm();
    Hyperplane<double, 3> limbPlane(n / mag, 1.0 / mag);

    bool found = false;

    return intersection(limbPlane, &found);
}


static Vector3d projectPoint(const Vector3d& point, const Vector3d& planeNormal)
{
    return point - planeNormal * (point.dot(planeNormal));
}


/** Calculate the projection of the ellipsoid onto plane with specified normal and
  * containing the ellipsoid center.
  */
GeneralEllipse
AlignedEllipsoid::orthogonalProjection(const Vector3d& planeNormal) const
{
    // Compute the transformation that maps the ellipsoid to a unit sphere
    Matrix3d M = m_semiAxes.cwise().inverse().asDiagonal();

    Hyperplane<double, 3> p(planeNormal, 0.0);
    Hyperplane<double, 3> p1 = p;
    p1.transform(M);
    p1.normal().normalize();

    Vector3d plane_origin, plane_v0, plane_v1;
    planeToSpanningVectors(p1, plane_origin, plane_v0, plane_v1);

    DiagonalMatrix<Vector3d> invM = m_semiAxes.asDiagonal();
    plane_v0     = invM * (plane_v0);
    plane_v1     = invM * (plane_v1);
    std::cerr << "lengths: " << plane_v0.norm() << ", " << plane_v1.norm() << endl;

    return GeneralEllipse(Vector3d::Zero(), plane_v0, plane_v1);
}
