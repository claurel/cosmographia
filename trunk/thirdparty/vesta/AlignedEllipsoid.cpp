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

    return GeneralEllipse(Vector3d::Zero(), plane_v0, plane_v1);
}


/** Convert rectangular coordinates to planetographic coordinates.
  */
PlanetographicCoord3
AlignedEllipsoid::rectangularToPlanetographic(const Vector3d& r)
{
    // Find the nearest point to r on the surface of the ellipsoid
    Vector3d surfacePoint = nearestPoint(r);

    // Get the surface normal at that point
    Vector3d n = normal(surfacePoint);

    double longitude = atan2(n.y(), n.x());
    double latitude = asin(n.z());

    Vector3d h = r - surfacePoint;
    double height = h.norm();

    // Return a negative height for points inside the ellipsoid.
    if (h.dot(r) < 0.0)
    {
        height = -height;
    }

    return PlanetographicCoord3(latitude, longitude, height);
}


/** Calculate the distance from the specified point to the
  * ellipsoid surface.
  */
double
AlignedEllipsoid::distance(const Vector3d& r)
{
    // Find the nearest point to r on the surface of the ellipsoid
    Vector3d surfacePoint = nearestPoint(r);

    // Return the distance to the nearest point
    return (r - surfacePoint).norm();
}


/** Find the point on the surface of the ellipsoid that is nearest to
  * some arbitrary point.
  */
Vector3d
AlignedEllipsoid::nearestPoint(const Vector3d& v)
{
    // No closed form solution available, so we'll iterate to find
    // the point. Decreasing maxErr improves the accuracy of the
    // solution at the cost of additional iterations.
    const double maxErr = 1.0e-10;
    static const unsigned int maxIterations = 20;

    const Vector3d invSemiAxes = m_semiAxes.cwise().inverse();
    const Vector3d invSemiAxes2 = invSemiAxes.cwise().square();
    const Vector3d semiAxes2 = m_semiAxes.cwise().square();
    const Vector3d semiAxes4 = semiAxes2.cwise().square();

    // beta is scale factor that will produce the planetocentric surface
    // point for v, i.e. beta*v will lie on the surface of the ellipsoid.
    // This point, beta*v, is our initial guess for the nearest point on the
    // surface of the ellipsoid. In general, the guess in only the actual nearest
    // point in the special case of a sphere.
    const double beta = 1.0 / (v.cwise() * invSemiAxes).norm();

    // n is the unnormalized surface normal at the initial guess point
    Vector3d n = beta * (v.cwise() * invSemiAxes2);
    double nMag = n.norm();
    double vMag = v.norm();

    Vector3d v2 = v.cwise().square();
    Vector3d d(Vector3d::Zero());

    double alpha = (1.0 - beta) * (vMag / nMag);
    double s = 0.0;
    double ds = 1.0;
    unsigned int i = 0;

    // Newton-Raphson iteration until error is sufficiently small
    do
    {
        alpha -= (s / ds);

        d = Vector3d::Ones() + alpha * invSemiAxes2;
        Vector3d d2 = d.cwise().square();
        Vector3d d3 = d.cwise() * d2;

        s = (v2.cwise() / (semiAxes2.cwise() * d2)).sum() - 1.0;

        ds = -2.0 * (v2.cwise() / (semiAxes4.cwise() * d3)).sum();
    }
    while (abs(s) > maxErr && ++i < maxIterations);

    return v.cwise() * d.cwise().inverse();
}
