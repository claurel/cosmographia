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
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


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

    // Transform the plane
    Hyperplane<double, 3> p1 = p;
    p1.transform(M);  // TODO: This calls a general matrix inversion function; unnecessarily general and slow

    if (p1.offset() < 1.0)
    {
        if (foundIntersection)
        {
            *foundIntersection = true;
        }

        // Compute the radius of the circle of intersection
        double r = 1.0 - p1.offset() * p1.offset();

        // n is the plane normal, v0 and v1 are spanning vectors of the plane
        double mag = p1.normal().norm();
        Vector3d n  = p1.normal() / mag;
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
